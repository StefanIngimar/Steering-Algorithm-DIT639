from typing import Iterable

from sqlalchemy.orm import Session

from db.models import VideoFeedFrameData

from schemas.video_feed_frame_data import VideoFeedFrameDataCreate


def bulk_create_frame_data(db: Session, frame_data: Iterable[VideoFeedFrameDataCreate]) -> None:
    if not frame_data:
        return

    db.bulk_insert_mappings(
        VideoFeedFrameData,
        [frame.model_dump() for frame in frame_data]
    )
    db.commit()

