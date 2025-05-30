import logging

from typing import Tuple, List

from sqlalchemy import desc
from sqlalchemy.exc import SQLAlchemyError
from sqlalchemy.orm import Session

from db.models import GitCommit, CarVideoFeed, VideoFeedFrameData, commit_video_feed
from db.exceptions import DbException

from schemas.git_commit import GitCommitCreate

from crud.dataclasses import FrameInfo, FrameDataByVideo, VideoCommit

_logger = logging.getLogger(__name__)


def get_or_create_git_commit(db: Session, commit_data: GitCommitCreate) -> Tuple[GitCommit, bool]:
    """
    Get git commit based on the commit ID or create a new one if it does not exist.

    Returns:
        Tuple[GitCommit, bool]:
            - GitCommit: git commit db object either newly created or retrieved from db
            - bool: flag specifying if the object was created or retrieved

    Raises:
        DbException: when any of the db operations fails
    """
    try:
        git_commit = db.query(GitCommit).filter(GitCommit.commit_id == commit_data.commit_id).first()
        if git_commit:
            return git_commit, False

        git_commit = GitCommit(commit_id=commit_data.commit_id)
        db.add(git_commit)
        db.commit()
        db.refresh(git_commit)

        return git_commit, True

    except SQLAlchemyError as e:
        _logger.error(f"Error in get_or_create_git_commit: {e}")
        db.rollback()
        raise DbException()


def get_last_two_commits_data(db: Session) -> List[VideoCommit]:
    """
    Retrieve data for the two most recent Git commits and their frame data for each video feed.

    Returns:
        List of VideoCommit objects, each containing frame data for associated video feeds.
    """

    commits = (
        db.query(GitCommit)
        .order_by(desc(GitCommit.created_at))
        .limit(2)
        .all()
    )

    if not commits:
        _logger.warning("No commits were found in the database")
        return []

    if len(commits) < 2:
        _logger.warning(f"Only {len(commits)} commit(s) found, but at least 2 are expected")

    video_commits = []
    for commit in commits:
        frame_data_by_videos = []
        video_feeds = (
            db.query(CarVideoFeed)
            .join(commit_video_feed)
            .filter(commit_video_feed.c.git_commit_id == commit.id)
            .all()
        )
        for video_feed in video_feeds:
            frame_data: List[VideoFeedFrameData] = (
                db.query(VideoFeedFrameData)
                .filter(VideoFeedFrameData.car_video_feed_id == video_feed.id)
                .order_by(VideoFeedFrameData.timestamp)
                .all()
            )

            if not frame_data:
                _logger.debug(f"No frame data found for video feed '{video_feed.file_name}' and commit '{commit.commit_id}'")
                continue

            frame_data_by_videos.append(FrameDataByVideo(
                file_name=str(video_feed.file_name),
                frame_info=[
                    FrameInfo(
                        timestamp=frame.timestamp,
                        actual_steering_angle=frame.actual_steering_angle,
                        calculated_steering_angle=frame.calculated_steering_angle,
                    ) for frame in frame_data
                ]
            ))

        video_commits.append(VideoCommit(
            commit_id=str(commit.commit_id),
            video_frame_data=frame_data_by_videos,
        ))

    return video_commits

