from datetime import datetime

from pydantic import BaseModel, Field


class GitCommitBase(BaseModel):
    commit_id: str = Field(max_length=50)


class GitCommitCreate(GitCommitBase):
    ...


class GitCommitRead(GitCommitBase):
    id: str
    created_at: datetime
    created_at: datetime

    class Config:
        from_attributes = True

