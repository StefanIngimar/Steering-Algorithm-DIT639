import os

DEBUG = False

if DEBUG:
    DATABASE_URL = "sqlite:///:memory:"
else:
    BASE_DIR = os.path.dirname(os.path.abspath(__file__))
    DATABASE_URL = f"sqlite:///{os.path.abspath(os.path.join(BASE_DIR, '../../data/app.db'))}"

# This has to match what nutmeg is using for communication
STEERING_SHM_KEY = 0x123e89
STEERING_SEM_KEY = 0x654321 
STEERING_DATA_SIZE_BYTES = 17
