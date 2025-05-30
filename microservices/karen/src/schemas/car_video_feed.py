from datetime import datetime

from pydantic import BaseModel, Field


class CarVideoFeedBase(BaseModel):
    file_name: str = Field(max_length=250)


class CarVideoFeedCreate(CarVideoFeedBase):
    pass


class CarVideoFeedRead(CarVideoFeedBase):
    id: str
    created_at: datetime
    updated_at: datetime

    class Config:
        from_attributes = True

