import uuid

from datetime import datetime, timezone

from sqlalchemy import BigInteger, Column, DateTime, String, ForeignKey, Float, Table
from sqlalchemy.orm import DeclarativeBase, relationship


class BaseModel(DeclarativeBase):
    __abstract__ = True

    id = Column(String(36), primary_key=True, default=lambda: str(uuid.uuid4()), unique=True, nullable=False)
    created_at = Column(DateTime, default=datetime.now(timezone.utc), nullable=False)
    updated_at = Column(DateTime, default=datetime.now(timezone.utc), onupdate=datetime.now(timezone.utc), nullable=False)


# many-to-many relationship between GitCommit and CarVideoFeed tables 
commit_video_feed = Table(
    "commit_video_feed",
    BaseModel.metadata,
    Column("git_commit_id", String(36), ForeignKey("git_commit.id"), primary_key=True),
    Column("car_video_feed_id", String(36), ForeignKey("car_video_feed.id"), primary_key=True),
)


class GitCommit(BaseModel):
    __tablename__ = "git_commit"

    commit_id = Column(String(50), unique=True, nullable=False)
    
    video_feeds = relationship(
        "CarVideoFeed",
        secondary=commit_video_feed,
        back_populates="git_commits",
    )


class CarVideoFeed(BaseModel):
    __tablename__ = "car_video_feed"

    file_name = Column(String(250), unique=True, nullable=False)

    git_commits = relationship(
        "GitCommit",
        secondary=commit_video_feed,
        back_populates="video_feeds",
    )
    frame_data = relationship("VideoFeedFrameData", back_populates="video_feed")


class VideoFeedFrameData(BaseModel):
    __tablename__ = "video_feed_frame_data"

    timestamp = Column(BigInteger, nullable=False)
    actual_steering_angle = Column(Float, nullable=False)
    calculated_steering_angle = Column(Float, nullable=False)

    car_video_feed_id = Column(String(36), ForeignKey("car_video_feed.id"), nullable=False)

    video_feed = relationship("CarVideoFeed", back_populates="frame_data")

