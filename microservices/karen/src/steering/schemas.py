from pydantic import BaseModel

class SteeringData(BaseModel):
    timestamp: int
    predicted: float
    actual: float
    has_more_data: bool
