import sysv_ipc
import os

DEBUG = False

if DEBUG:
    DATABASE_URL = "sqlite:///:memory:"
else:
    BASE_DIR = os.path.dirname(os.path.abspath(__file__))
    DATABASE_URL = f"sqlite:///{os.path.abspath(os.path.join(BASE_DIR, '../../data/app.db'))}"

#DEBUG = False

#DATABASE_URL = "sqlite:///./data/app.db" if not DEBUG else "sqlite:///:memory:"

# This has to match what nutmeg is using for communication
TOKEN_FILE = "/tmp/img"
PROJECT_ID_SHM = ord('a')
PROJECT_ID_SEM = ord('b')

if not os.path.exists(TOKEN_FILE):
    open(TOKEN_FILE, 'a').close()

STEERING_SHM_KEY = sysv_ipc.ftok(TOKEN_FILE, PROJECT_ID_SHM)
STEERING_SEM_KEY = sysv_ipc.ftok(TOKEN_FILE, PROJECT_ID_SEM)
#STEERING_SHM_KEY = 0x123e89
#STEERING_SEM_KEY = 0x654321 
STEERING_DATA_SIZE_BYTES = 17
