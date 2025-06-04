import argparse
import logging

from db.database import Sqlite 
from core.logger import setup_logging
from steering.controller import SteeringDataController
from plotting.commit_comparison import generate_plots
from video_processor import VideoProcessor


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Generate a steering angle comparison plot for current git commit and specific video file"
    )
    parser.add_argument(
        "--commit_id", type=str, required=True, help="Git commit ID"
    )
    parser.add_argument(
        "--video_file", type=str, required=True, help="Video file name that is currently used by the image processing application"
    )
    args = parser.parse_args()

    setup_logging()
    logger = logging.getLogger(__name__)

    db = Sqlite()
    data_controller = SteeringDataController()
    processor = VideoProcessor(db=db, data_controller=data_controller)

    processor.process(commit_id=args.commit_id, video_file_name=args.video_file)
    
    logger.info("Finished processing, about to generate plots")
    with db.new_session() as session:
        generate_plots(session=session)

if __name__ == "__main__":
    main()

