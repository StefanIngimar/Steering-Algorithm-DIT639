DEBUG = False

DATABASE_URL = "sqlite:///./data/app.db" if not DEBUG else "sqlite:///:memory:"

# This has to match what nutmeg is using for communication
STEERING_SHM_KEY = 0x123e89
STEERING_SEM_KEY = 0x654321 
STEERING_DATA_SIZE_BYTES = 17
