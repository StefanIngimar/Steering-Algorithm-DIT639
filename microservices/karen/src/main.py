import argparse

from db.database import Sqlite 
import os
from core.logger import setup_logging
import time
from steering.controller import SteeringDataController
from plotting.commit_comparison import generate_plots
from video_processor import VideoProcessor

def wait_for_ready_signal(ready_path, timeout=60):
    print("[KAREN] Waiting for producer to finish..")
    for _ in range(timeout):
        if os.path.exists(ready_path):
            print("[KAREN] Detected .ready file. Proceeding to genereate plots")
            return True
        time.sleep(1)
    raise TimeoutError("Timeout waiting for .ready signal file")

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

    ready_path = "/res/.ready"
    wait_for_ready_signal(ready_path)

    setup_logging()

    db = Sqlite()
    data_controller = SteeringDataController()
    processor = VideoProcessor(db=db, data_controller=data_controller)

    processor.process(commit_id=args.commit_id, video_file_name=args.video_file)

    print(">>>>> Finished processing, about to generate plots")
    
    with db.new_session() as session:
        generate_plots(session=session)

    os.remove(ready_path)


if __name__ == "__main__":
    main()

