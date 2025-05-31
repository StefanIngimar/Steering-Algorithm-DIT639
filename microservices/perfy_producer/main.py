import time
import struct
import logging

import sysv_ipc
import argparse

import cluonDataStructures_pb2
import opendlv_standard_message_set_v0_9_9_pb2

from enum import Enum

from logger import setup_logging


SHM_KEY = 0x696D67

FRAME_WIDTH = 640
FRAME_HEIGHT = 480

MAX_H264_FRAME_SIZE = 200 * 1024
# Header format:
# frame availability flag    termination flag     incoming content length      timestamp      steering angle
#       [ 1 byte ]                [1 byte]                [4 bytes]            [8 bytes]         [8 bytes]
HEADER_SIZE = 1 + 1 + 4 + 8 + 8


class EnvelopeDataType(Enum):
    GROUND_STEERING_ID = 1090
    IMAGE_READING_ID = 1055


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--file", required=True, help="Path to .rec file")
    args = parser.parse_args()
    file_name = args.file

    setup_logging()
    logger = logging.getLogger(__name__)

    buffer = b""
    envelope_header_length = 5
    was_envelope_header_consumed = False

    expected_bytes = 0
    read_ground_steering_angles = 0

    loop_delay = 1 / 10

    logger.info(f"Starting shared memory with key '{SHM_KEY}'")
    shared_frames = 0
    shared_memory_size = HEADER_SIZE + MAX_H264_FRAME_SIZE
    shared_memory = sysv_ipc.SharedMemory(
        SHM_KEY, sysv_ipc.IPC_CREAT, mode=0o666, size=shared_memory_size
    )

    latest_steering = 0

    logger.info(f"Started processing and sharing file '{file_name}'")
    with open(file_name, "rb") as file:
        while True:
            byte = file.read(1)
            if not byte:
                logger.info("Byte not found. Breaking from the main loop")
                break
            buffer += byte

            if was_envelope_header_consumed:
                if len(buffer) >= expected_bytes:
                    envelope = cluonDataStructures_pb2.cluon_data_Envelope()
                    envelope.ParseFromString(buffer[:expected_bytes])

                    frame_ts = (
                        envelope.sent.seconds * 1_000_000 + envelope.sent.microseconds
                    )

                    if envelope.dataType == EnvelopeDataType.GROUND_STEERING_ID.value:
                        reading = opendlv_standard_message_set_v0_9_9_pb2.opendlv_proxy_GroundSteeringReading()
                        reading.ParseFromString(envelope.serializedData)
                        if reading.HasField("groundSteering"):
                            read_ground_steering_angles += 1
                            latest_steering = reading.groundSteering

                    elif envelope.dataType == EnvelopeDataType.IMAGE_READING_ID.value:
                        image_msg = opendlv_standard_message_set_v0_9_9_pb2.opendlv_proxy_ImageReading()
                        image_msg.ParseFromString(envelope.serializedData)

                        if image_msg.fourcc.lower() == "h264":
                            encoded_bytes = image_msg.data
                            encoded_size = len(encoded_bytes)

                            if MAX_H264_FRAME_SIZE >= encoded_size:
                                # wait for the consumer to clear the 'read frame' flag
                                while shared_memory.read(1)[0] == 1:
                                    time.sleep(0.001)

                                # clear shared memory first
                                shared_memory.write(b"\x00" * encoded_size)

                                # the format is: big-endian, two unsigned chars (B), unsigned int (I), unsigned long long (Q), and double (d)
                                header = struct.pack(
                                    ">BBIQd",
                                    1,  # indicating that the frame is ready
                                    0,  # indicating not to terminate
                                    encoded_size,
                                    frame_ts,
                                    latest_steering,
                                )

                                shared_memory.write(header, offset=0)
                                shared_memory.write(encoded_bytes, offset=HEADER_SIZE)

                                shared_frames += 1
                                if shared_frames % 100 == 0:
                                    logger.debug(
                                        f"{shared_frames} frames shared... and counting"
                                    )

                                time.sleep(loop_delay)
                        else:
                            logger.warning(
                                f"Unsupported fourcc value. Expected 'h264' but received '{image_msg.fourcc}'"
                            )

                    buffer = buffer[expected_bytes:]
                    expected_bytes = 0
                    was_envelope_header_consumed = False

            else:
                while len(buffer) >= envelope_header_length:
                    byte0 = buffer[0]
                    byte1 = buffer[1]

                    if byte0 == 0x0D and byte1 == 0xA4:
                        # Read uint32_t and convert to little endian.
                        v = struct.unpack("<L", buffer[1:5])
                        # The second byte belongs to the header of an Envelope.
                        expected_bytes = v[0] >> 8
                        # Remove header.
                        buffer = buffer[5:]
                        was_envelope_header_consumed = True
                        break
                    else:
                        buffer = buffer[1:]
                        logger.error(
                            f"Failed to consume header from Envelope. Expected 0x0D ({0x0D}) and 0xA4 ({0xA4}) as first two bytes but got {byte0} and {byte1} instead"
                        )

    # wait for the consumer to clear the 'read frame' flag
    while shared_memory.read(1)[0] == 1:
        time.sleep(0.001)

    # TODO: lets use struct.pack for this
    shared_memory.write(b"\x00", offset=0)
    shared_memory.write(b"\x01", offset=1)
    shared_memory.detach()

    logger.info(f"Done. Sent '{shared_frames}' frames")
    logger.debug(f"Found ground steering angles: {read_ground_steering_angles}")


if __name__ == "__main__":
    main()
