import time

import sysv_ipc
import numpy as np

import cv2
import av
import os
from io import BytesIO

SHM_KEY = 0x696d67

FRAME_WIDTH = 640
FRAME_HEIGHT = 480

MAX_H264_FRAME_SIZE = 200 * 1024
# Header format:
# frame availability flag    termination flag     incoming content length
#       [ 1 byte ]                [1 byte]                [4 bytes]
HEADER_SIZE = 1 + 1 + 4
BASE_DIR = os.path.dirname(os.path.abspath(__file__))

def encode_frame_to_h264(frame: np.ndarray) -> bytes:
    buffer = BytesIO()

    output = av.open(buffer, mode="w", format="h264")

    stream = output.add_stream("h264", rate=30)

    stream.width = frame.shape[1]
    stream.height = frame.shape[0]

    stream.pix_fmt = "yuv420p"

    frame_av = av.VideoFrame.from_ndarray(frame, format="bgr24")
    packets = stream.encode(frame_av)

    for packet in packets:
        output.mux(packet)

    packets = stream.encode(None)
    for packet in packets:
        output.mux(packet)

    output.close()
    return buffer.getvalue()


def main() -> None:
    file_name = os.path.join(BASE_DIR, "../../res/video_feeds/CID-140-recording-2020-03-18_144821-selection.rec")

    capture = cv2.VideoCapture(filename=file_name)
    if not capture.isOpened():
        raise IOError(f"Cannot open video file at: {file_name}")

    fps = capture.get(cv2.CAP_PROP_FPS)
    loop_delay = 1 / fps if fps > 0 else 1/30 

    shared_memory_size = HEADER_SIZE + MAX_H264_FRAME_SIZE
    shared_memory = sysv_ipc.SharedMemory(SHM_KEY, sysv_ipc.IPC_CREAT, mode=0o666, size=shared_memory_size)
    
    shared_frames = 0
    while capture.isOpened():
        # wait for the consumer to clear the 'read frame' flag
        while shared_memory.read(1)[0] == 1:
            time.sleep(0.001)

        ret, frame = capture.read()
        if not ret:
            print("Frame not received. Leaving the main loop")
            break

        if frame is None:
            print("Received empty frame... huh?")
            continue
        
        frame = cv2.resize(frame, (FRAME_WIDTH, FRAME_HEIGHT))
        encoded_bytes = encode_frame_to_h264(frame=frame)
        encoded_size = len(encoded_bytes)

        if encoded_size > MAX_H264_FRAME_SIZE:
            print(f"Frame too large for shared memory: {encoded_size} bytes")
            continue

        # clear shared memory first
        shared_memory.write(b'\x00' * encoded_size)

        shared_memory.write(b'\x01', offset=0) # 1 indicating that the frame is ready
        shared_memory.write(b'\x00', offset=1) # 0 indicating not to terminate
        shared_memory.write(encoded_size.to_bytes(4, 'big'), offset=2)
        shared_memory.write(encoded_bytes, offset=HEADER_SIZE)

        shared_frames += 1
        if shared_frames % 100 == 0:
            print(f"{shared_frames} frames shared... and counting")

        time.sleep(loop_delay)

    # wait for the consumer to clear the 'read frame' flag
    while shared_memory.read(1)[0] == 1:
        time.sleep(0.001)

    shared_memory.write(b'\x00', offset=0)
    shared_memory.write(b'\x01', offset=1)

    shared_memory.detach()
    capture.release()

    print(f"Done. Sent '{shared_frames}' frames")


if __name__ == "__main__":
    main()


