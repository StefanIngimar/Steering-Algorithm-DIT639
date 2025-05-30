from typing import List 

from dataclasses import dataclass


@dataclass
class FrameInfo:
    timestamp: int 
    actual_steering_angle: float
    calculated_steering_angle: float


@dataclass
class FrameDataByVideo:
    file_name: str
    frame_info: List[FrameInfo]


@dataclass
class VideoCommit:
    commit_id: str
    video_frame_data: List[FrameDataByVideo]

