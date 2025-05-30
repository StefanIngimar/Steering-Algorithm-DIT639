import logging
import os
from pathlib import Path
from typing import Sequence, List, Optional

from sqlalchemy.orm import Session

import matplotlib.pyplot as plt

from crud.dataclasses import FrameInfo, VideoCommit
from crud.git_commit import get_last_two_commits_data

_logger = logging.getLogger(__name__)


def generate_plots(session: Session) -> None:
    """
    Generate comparison plots for the last two commits. 
    """
    try:
        _logger.info("Getting two last commits data")
        commit_data = get_last_two_commits_data(db=session)

        _logger.info("Plotting commit comparison")
        plot_commit_comparison(video_commits=commit_data)
    except Exception as e:
        _logger.error(f"Error generating comparison plots: {e}")


def plot_commit_comparison(
    video_commits: Sequence[VideoCommit],
    output_dir: str = "data/plots",
):
    """
    Generate one plot per video feed, comparing actual vs calculated steering angles
    for the two most recent commits.
    """

    if len(video_commits) < 2:
        _logger.warning(f"Need at least two commits to generate comparison plot but got '{len(video_commits)}' instead")
        return

    os.makedirs(output_dir, exist_ok=True) 

    video_feed_files_names = set()
    for video_commit in video_commits:
        for frame_data in video_commit.video_frame_data:
            video_feed_files_names.add(frame_data.file_name)

    for file_name in video_feed_files_names:
        plt.figure(figsize=(10, 6))

        # NOTE(sw): This assumes that passed video commits are in desc order (so the most recent video commit comes first)
        current_commit = video_commits[0]
        current_commit_id = current_commit.commit_id[:8]
        current_frame_info: Optional[List[FrameInfo]] = None
        for video_frame_data in current_commit.video_frame_data:
            if video_frame_data.file_name == file_name:
                current_frame_info = video_frame_data.frame_info

        if not current_frame_info:
            _logger.warning(f"No frame data for video '{file_name}' in commit '{current_commit_id}'")
            plt.close()
            continue

        timestamps = [f.timestamp for f in current_frame_info]
        actual_steering = [f.actual_steering_angle for f in current_frame_info]
        plt.plot(timestamps, actual_steering, label="Actual Steering Angle", linestyle="-", color="blue")

        calculated_steering = [f.calculated_steering_angle for f in current_frame_info]
        plt.plot(timestamps, calculated_steering, label="Calculated Steering Angle", linestyle="--", color="green")

        prev_commit = video_commits[1]
        prev_commit_id = prev_commit.commit_id[:8]
        prev_frame_info: Optional[List[FrameInfo]] = None
        for video_frame_data in prev_commit.video_frame_data:
            if video_frame_data.file_name == file_name:
                prev_frame_info = video_frame_data.frame_info

        if not prev_frame_info:
            _logger.warning(f"No frame data for video '{file_name}' in previous commit '{prev_commit_id}'")
        else:
            prev_timestamps = [f.timestamp for f in prev_frame_info]
            if prev_timestamps != timestamps:
                _logger.warning(f"Timestamp mismatch for '{file_name}' between commits. Using current commit timestamps")
                prev_timestamps = timestamps

            prev_calculated = [f.calculated_steering_angle for f in prev_frame_info]
            plt.plot(
                prev_timestamps, 
                prev_calculated,  # NOTE(sw): this could technically be padded/aligned to match current "calculated_steering"
                label=f"Previous Calculated Steering Angle (Commit {prev_commit_id})", 
                linestyle=":", 
                color="red"
            )

        plt.xlabel("Timestamp (microseconds)")
        plt.ylabel("Steering Angle")
        plt.title(f"Steering Angle Comparison for {file_name}")
        plt.legend()
        plt.grid(True)
        plt.tight_layout()
        #path was throwing for me
        safe_file_name = Path(file_name).name.replace(".", "_")
        output_path = os.path.join(output_dir, f"commit_comparison_{safe_file_name}_{video_commits[0].commit_id[:8]}.png")
        plt.savefig(output_path, dpi=300, bbox_inches="tight")
        plt.close()

        _logger.info(f"Plot saved to {output_path}")

