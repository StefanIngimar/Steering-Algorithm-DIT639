import time
import logging

from typing import Callable, Optional, Protocol, List

from sqlalchemy.orm import Session

from schemas.video_feed_frame_data import VideoFeedFrameDataCreate

from crud.video_feed_frame_data import bulk_create_frame_data

from steering.shared_memory import read_steering_data
from steering.schemas import SteeringData

_logger = logging.getLogger(__name__)


class SteeringDataControllerProtocol(Protocol):
    def collect(self, video_feed_id: str) -> Optional[SteeringData]:
        ...

    def should_flush(self) -> bool:
        ...

    def flush_batch(self, session: Session) -> None:
        ...

    def get_collected_data_count(self) -> int:
        ...


class SteeringDataController:
    def __init__(self, read_steering_callback: Callable[[], Optional[SteeringData]] = read_steering_data, flush_interval_sec: float = 5.0, max_batch_size: int = 100):
        self._read_data_callback = read_steering_callback

        self._batch: List[VideoFeedFrameDataCreate] = []
        self._last_steering_ts = None
        self._last_flush_time = time.time()
        self._steering_data_received = 0

        self._flush_interval_sec = flush_interval_sec
        self._batch_size = max_batch_size

    def collect(self, video_feed_id: str) -> Optional[SteeringData]:
        steering_data: Optional[SteeringData] = self._read_data_callback()
        if not steering_data:
            return None

        if steering_data.timestamp == 0:
            _logger.warning("Received timestamp with value '0'. Skipping that reading.")
            # NOTE(sw): kind of a hack... when timestamp is 0 it means that cpp is done sending values
            # to the shared memory. steering_data has to be returned early without being added to the batch
            # to signal that not more that is coming
            return steering_data 

        current_steering_ts = steering_data.timestamp
        if current_steering_ts == self._last_steering_ts:
            return None

        self._last_steering_ts = current_steering_ts
        frame_data = VideoFeedFrameDataCreate(
            timestamp=steering_data.timestamp,
            actual_steering_angle=steering_data.actual,
            calculated_steering_angle=steering_data.predicted,
            car_video_feed_id=str(video_feed_id)
        )
        self._batch.append(frame_data)
        self._steering_data_received += 1

        return steering_data 

    def should_flush(self) -> bool:
        current_time = time.time()
        return (current_time - self._last_flush_time >= self._flush_interval_sec) or len(self._batch) >= self._batch_size

    def flush_batch(self, session: Session) -> None:
        if self._batch:
            bulk_create_frame_data(db=session, frame_data=self._batch)
            _logger.debug(f"Inserted {len(self._batch)} records into the database")
            self._batch = []
            self._last_flush_time = time.time()

    def get_collected_data_count(self) -> int:
        return self._steering_data_received
