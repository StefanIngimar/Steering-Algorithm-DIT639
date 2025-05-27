import time
import sysv_ipc
import numpy as np
import cv2
import av
from io import BytesIO
import threading
import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "../python")))
from opendlv_proxy import GroundSteeringReading  # Generated via cluon-tool

SHM_KEY = 0x696d67
FRAME_WIDTH = 640
FRAME_HEIGHT = 480
MAX_H264_FRAME_SIZE = 200 * 1024
HEADER_SIZE = 1 + 1 + 4  # frame ready flag, termination flag, content length


def encode_frame_to_h264(frame: np.ndarray) -> bytes:
    buffer = BytesIO()
    output = av.open(buffer, mode="w", format="h264")
    stream = output.add_stream("h264", rate=30)
    stream.width = frame.shape[1]
    stream.height = frame.shape[0]
    stream.pix_fmt = "yuv420p"
    frame_av = av.VideoFrame.from_ndarray(frame, format="bgr24")

    for packet in stream.encode(frame_av):
        output.mux(packet)
    for packet in stream.encode(None):
        output.mux(packet)
    output.close()
    return buffer.getvalue()


def listen_groundsteering():
    session = cluon.OD4Session(cid=253)

    def on_data(env):
        if env.dataTypeID() == 1100:  # Replace with correct ID if different
            msg = GroundSteeringReading()
            msg.deserialize(env.data())
            print(f"[steering] actualSteeringAngle: {msg.actualSteeringAngle()}")

    session.dataAvailable += on_data
    session.run()


def main():
    file_name = "./res/CID-140-recording-2020-03-18_144821-selection.rec"
    capture = cv2.VideoCapture(filename=file_name)
    if not capture.isOpened():
        raise IOError(f"Cannot open video file at: {file_name}")

    fps = capture.get(cv2.CAP_PROP_FPS)
    loop_delay = 1 / fps if fps > 0 else 1 / 30

    shared_memory_size = HEADER_SIZE + MAX_H264_FRAME_SIZE
    shared_memory = sysv_ipc.SharedMemory(SHM_KEY, sysv_ipc.IPC_CREAT, mode=0o666, size=shared_memory_size)

    # start steering listener in a thread
    steering_thread = threading.Thread(target=listen_groundsteering, daemon=True)
    steering_thread.start()

    shared_frames = 0
    while capture.isOpened():
        while shared_memory.read(1)[0] == 1:
            time.sleep(0.001)

        ret, frame = capture.read()
        if not ret:
            break

        frame = cv2.resize(frame, (FRAME_WIDTH, FRAME_HEIGHT))
        encoded_bytes = encode_frame_to_h264(frame)
        encoded_size = len(encoded_bytes)

        if encoded_size > MAX_H264_FRAME_SIZE:
            print(f"Frame too large: {encoded_size} bytes")
            continue

        shared_memory.write(b'\x00' * encoded_size)  # zeroing out prior data
        shared_memory.write(b'\x01', offset=0)
        shared_memory.write(b'\x00', offset=1)
        shared_memory.write(encoded_size.to_bytes(4, 'big'), offset=2)
        shared_memory.write(encoded_bytes, offset=HEADER_SIZE)

        shared_frames += 1
        if shared_frames % 100 == 0:
            print(f"{shared_frames} frames shared...")

        time.sleep(loop_delay)

    while shared_memory.read(1)[0] == 1:
        time.sleep(0.001)

    shared_memory.write(b'\x00', offset=0)
    shared_memory.write(b'\x01', offset=1)
    shared_memory.detach()
    capture.release()
    print(f"Done. Sent {shared_frames} frames")


if __name__ == "__main__":
    main()

