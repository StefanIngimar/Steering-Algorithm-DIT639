from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker, Session

from core.settings import DATABASE_URL

from db.models import BaseModel


class Sqlite:
    _initialised = False

    def __init__(self):
        if Sqlite._initialised:
            raise Exception("Database should be initialized only once")

        self._engine = create_engine(
            url=DATABASE_URL,
            connect_args={
                "check_same_thread": False,
            }
        )
        self._session = sessionmaker(
            autocommit=False,
            autoflush=False,
            bind=self._engine
        )
        BaseModel.metadata.create_all(self._engine)

        Sqlite._initialised = True

    def new_session(self) -> Session:
        return self._session()

