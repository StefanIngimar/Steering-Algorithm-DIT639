import logging
import sys


def setup_logging() -> None:
    root_logger = logging.getLogger()
    root_logger.setLevel(logging.DEBUG)

    root_logger.handlers.clear()

    console_handler = logging.StreamHandler(sys.stdout)
    console_handler.setFormatter(
        logging.Formatter(
            "[PRODUCER] [%(levelname)s] [%(asctime)s] %(name)s: %(message)s"
        )
    )
    console_handler.setLevel(logging.DEBUG)
    root_logger.addHandler(console_handler)
