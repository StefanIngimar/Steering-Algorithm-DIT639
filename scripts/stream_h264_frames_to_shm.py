import argparse
import base64
import re
import cv2
import numpy as np
import sysv_ipc
import time
import tempfile
import os

def extract_h264_to_file(rec_txt_path):
    pattern = re.compile(r'data: \(base64\)\s+([A-Za-z0-9+/=\n\r]+)')
    with open(rec_txt_path, 'r') as f:
        content = f.read()

    matches = pattern.findall(content)
    if not matches:
        raise ValueError("No base64 H264 frames found.")

    tmp_file = tempfile.NamedTemporaryFile(delete=False, suffix='.h264')
    with open(tmp_file.name, 'wb') as out_f:
        for encoded in matches:
            clean_b64 = encoded.replace('\n', '').replace('\r', '').strip()
            try:
                raw_frame = base64.b64decode(clean_b64)
                if len(raw_frame) < 10000:
                    continue
                out_f.write(raw_frame)
            except base64.binascii.Error:
                continue

    return tmp_file.name

def stream_frames(video_path, shm_key, width, height, delay=0.033):
    token_path = "/tmp/img"
    with open(token_path, "w") as token:
        token.write("token")

    shape = (height, width, 3)
    frame_size = int(np.prod(shape))
    shm = sysv_ipc.SharedMemory(shm_key, sysv_ipc.IPC_CREAT, mode=0o666, size=frame_size)

    cap = cv2.VideoCapture(video_path)
    if not cap.isOpened():
        print(f"Error: Cannot open video file {video_path}")
        return

    frame_count = 0
    while cap.isOpened():
        ret, frame = cap.read()
        if not ret:
            print("End of video or read failure.")
            break

        frame = cv2.resize(frame, (width, height))
        shm.write(frame.tobytes())
        frame_count += 1
        time.sleep(delay)

    print(f"Streamed {frame_count} frames to shared memory.")
    cap.release()
    shm.detach()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True, help="Path to .rec.txt file")
    parser.add_argument("--width", type=int, required=True)
    parser.add_argument("--height", type=int, required=True)
    args = parser.parse_args()

    shm_key = 0x12600af
    h264_path = extract_h264_to_file(args.input)
    try:
        stream_frames(h264_path, shm_key, args.width, args.height)
    finally:
        if os.path.exists(h264_path):
            os.remove(h264_path)

if __name__ == "__main__":
    main()

