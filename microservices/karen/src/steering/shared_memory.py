import struct
import logging
import sysv_ipc
import time

from typing import Optional

from core.settings import STEERING_SHM_KEY, STEERING_SEM_KEY, STEERING_DATA_SIZE_BYTES

from steering.schemas import SteeringData

_logger = logging.getLogger(__name__)


class SteeringSharedMemory:
    def __init__(self, timeout: float = 30.0):
        self._shared_memory = None
        self._semaphore = None

        self._timeout = timeout

        self._attach_to_shared_memory()

    def read_steering_data(self) -> Optional[SteeringData]:
        """
        Read steering data from shared memory, blocking until shared memory and semaphore are accessible.
        Returns 'None'' if shared memory is not accessible after specified timeout.
        """

        if not self._shared_memory:
            _logger.error("Shared memory not attached")
            return None

        if not self._semaphore:
            _logger.error("Semaphore not attached")
            return None

        try:
            self._semaphore.acquire()
            try:
                raw_data = self._shared_memory.read(STEERING_DATA_SIZE_BYTES)
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
                self._semaphore.release()
        except sysv_ipc.ExistentialError:
            _logger.error(
                f"Shared memory or semaphore with key {hex(STEERING_SHM_KEY)}/{hex(STEERING_SEM_KEY)} no longer exists"
            )
            return None
        except sysv_ipc.Error as e:
            _logger.error(f"Failed to access shared memory or semaphore: {e}")
            return None
        except struct.error as e:
            _logger.error(f"Failed to unpack shared memory data: {e}")
            return None
        except Exception as e:
            _logger.error(f"Unexpected error while reading steering data: {e}")
            return None

    def cleanup(self) -> None:
        """
        Detach from shared memory and semaphore.
        """

        if self._semaphore is not None:
            self._semaphore = None

        if self._shared_memory is not None:
            try:
                self._shared_memory.detach()
                _logger.debug("Detached from shared memory")
            except sysv_ipc.ExistentialError:
                _logger.debug("Shared memory already detached or removed")
            self._shared_memory = None

    def _attach_to_shared_memory(self) -> None:
        start_time = time.time()
        log_interval_sec = 3
        last_log_time = -log_interval_sec
        
        is_initialized = False
        while not is_initialized:
            try:
                self._shared_memory = sysv_ipc.SharedMemory(STEERING_SHM_KEY, flags=0)
                self._semaphore = sysv_ipc.Semaphore(STEERING_SEM_KEY, flags=0)

                is_initialized = True

                _logger.info(f"Successfully attached to shared memory and semaphore with key {hex(STEERING_SHM_KEY)}/{hex(STEERING_SEM_KEY)}")
            except sysv_ipc.ExistentialError:
                elapsed = time.time() - start_time
                if elapsed >= self._timeout:
                    raise sysv_ipc.ExistentialError(
                        f"Shared memory or semaphore with key {hex(STEERING_SHM_KEY)}/{hex(STEERING_SEM_KEY)} not available after {self._timeout}s"
                    )

                if time.time() - last_log_time >= log_interval_sec:
                    _logger.debug("Shared memory/semaphore not accessible, retrying...")
                    last_log_time = time.time()

                time.sleep(0.1)

