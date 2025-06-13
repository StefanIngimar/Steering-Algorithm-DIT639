import logging 

from typing import Tuple

from sqlalchemy.exc import SQLAlchemyError
from sqlalchemy.orm import Session

from db.models import CarVideoFeed, GitCommit
from db.exceptions import DbException

from schemas.car_video_feed import CarVideoFeedCreate

_logger = logging.getLogger(__name__)


def get_or_create_car_video_feed(db: Session, feed_data: CarVideoFeedCreate, git_commit: GitCommit) -> Tuple[CarVideoFeed, bool]:
    """
    Get car video feed based on the file name or create a new one if it does not exist.

    Returns:
        Tuple[CarVideoFeed, bool]:
            - CarVideoFeed: car video feed db object either newly created or retrieved from db
            - bool: flag specifying if the object was created or retrieved

    Raises:
        DbException: when any of the db operations fails
    """
    try:
        video_feed = db.query(CarVideoFeed).filter(CarVideoFeed.file_name == feed_data.file_name).first()
        if video_feed:
            if git_commit not in video_feed.git_commits:
                video_feed.git_commits.append(git_commit)
                db.commit()

            return video_feed, False

        video_feed = CarVideoFeed(
            file_name=feed_data.file_name,
        )
        db.add(video_feed)
        video_feed.git_commits.append(git_commit)
        db.commit()
        db.refresh(video_feed)
        
        return video_feed, True
    except SQLAlchemyError as e:
        _logger.error(f"Error in get_or_create_car_video_feed: {e}")
        db.rollback()
        raise DbException()

