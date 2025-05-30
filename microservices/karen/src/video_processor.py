import logging
import time

from sqlalchemy.orm import Session

from db.database import Sqlite 
from db.exceptions import DbException

from schemas.git_commit import GitCommitCreate
from schemas.car_video_feed import CarVideoFeedCreate

from crud.git_commit import get_or_create_git_commit
from crud.car_video_feed import get_or_create_car_video_feed

from steering.controller import SteeringDataControllerProtocol

_logger = logging.getLogger(__name__)


class VideoProcessor:
    def __init__(self, db: Sqlite, data_controller: SteeringDataControllerProtocol):
        self._db = db
        self._data_controller = data_controller

    def process(self, commit_id: str, video_file_name: str) -> None:
        with self._db.new_session() as session:
            try:
                _logger.info("Getting or creating git commit")
                git_commit, _ = get_or_create_git_commit(
                    db=session,
                    commit_data=GitCommitCreate(
                        commit_id=str(commit_id),
                    )
                )

                _logger.info("Getting or creating car video feed")
                car_video_feed, _ = get_or_create_car_video_feed(
                    db=session,
                    feed_data=CarVideoFeedCreate(
                        file_name=video_file_name,
                    ),
                    git_commit=git_commit,
                )
            except DbException as e:
                _logger.error(f"Error while inserting git commit and car video feed objects to the database. Cannot proceed. Error: {e}")
                return
            
            self._process_readings(session=session, video_feed_id=str(car_video_feed.id))
            
            # Flush remaining data after all data has been processed
            self._data_controller.flush_batch(session=session)

            _logger.debug(f"Done processing '{video_file_name}'")
            _logger.debug(f"Steering data received: {self._data_controller.get_collected_data_count()}")

    def _process_readings(self, session: Session, video_feed_id: str):
        _logger.info("Starting reading steering data")

        is_data_coming = True
        missing_data_count = 0
        max_missing_retries = 10
        # it would get stuck at times for some reason, so i added this checker to see if it was done
        while is_data_coming:
            frame_data = self._data_controller.collect(video_feed_id=video_feed_id)
    
            if frame_data is None:
                missing_data_count += 1
                if missing_data_count > max_missing_retries:
                    _logger.warning("No frame data after multiple attempts. Stopping.")
                    break
                time.sleep(0.1)
                continue
            else:
                missing_data_count = 0

            is_data_coming = frame_data.has_more_data

            if self._data_controller.should_flush():
                self._data_controller.flush_batch(session=session)

            time.sleep(0.1)
        

        _logger.info("Finished reading steering data")

