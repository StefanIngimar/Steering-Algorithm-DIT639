from datetime import datetime

from pydantic import BaseModel


class VideoFeedFrameDataBase(BaseModel):
    timestamp: int
    actual_steering_angle: float
    calculated_steering_angle: float
    car_video_feed_id: str


class VideoFeedFrameDataCreate(VideoFeedFrameDataBase):
    pass


class VideoFeedFrameDataRead(VideoFeedFrameDataBase):
    id: str
    created_at: datetime
    updated_at: datetime

    class Config:
        from_attributes = True

