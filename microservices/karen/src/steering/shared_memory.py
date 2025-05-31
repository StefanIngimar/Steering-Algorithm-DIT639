import struct
import logging
import sysv_ipc
import time

from typing import Optional

from core.settings import STEERING_SHM_KEY, STEERING_SEM_KEY, STEERING_DATA_SIZE_BYTES

from steering.schemas import SteeringData

_logger = logging.getLogger(__name__)


def read_steering_data(timeout: float = 30.0) -> Optional[SteeringData]:
    """
    Read steering data from shared memory, blocking until shared memory and semaphore are accessible.
    Returns 'None'' if shared memory is not accessible after specified timeout.
    """

    log_interval_sec = 3
    last_log_time = -log_interval_sec

    start_time = time.time()
    while True:
        try:
            semaphore = sysv_ipc.Semaphore(STEERING_SEM_KEY, flags=0)
            shared_memory = sysv_ipc.SharedMemory(STEERING_SHM_KEY, flags=0)

            semaphore.acquire()
            try:
                raw_data = shared_memory.read(STEERING_DATA_SIZE_BYTES)
                timestamp, predicted, actual, has_more_data = struct.unpack(
                    "<qffb", raw_data
                )

                return SteeringData(
                    timestamp=timestamp,
                    predicted=predicted,
                    actual=actual,
                    has_more_data=bool(has_more_data),
                )
            finally:
                semaphore.release()
        except sysv_ipc.ExistentialError:
            current_time = time.time()
            elapsed = current_time - start_time
            if elapsed >= timeout:
                _logger.error(
                    f"Shared memory of semaphore with key {hex(STEERING_SHM_KEY)}/{hex(STEERING_SEM_KEY)} does not exist"
                )
                return None

            if current_time - last_log_time >= 3:
                _logger.debug("Shared memory/semaphore is not accessible, retrying...")
                last_log_time = current_time

            time.sleep(0.1)
        except sysv_ipc.Error as e:
            _logger.error(f"Failed to access shared memory for semaphore: {e}")
            return None
        except struct.error as e:
            _logger.error(f"Failed to unpack shared memory data: {e}")
            return None
        except Exception as e:
            _logger.error(f"Unexpected error while reading steering data: {e}")
            return None
