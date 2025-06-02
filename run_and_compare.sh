#!/bin/bash

set -euo pipefail

COMMIT_ID=$(git rev-parse --short HEAD)
REC_DIR="microservices/perfy_producer/res"
REC_FILES=("$REC_DIR"/*.rec)

STEERING_SHM_DECIMAL=1193737
STEERING_SEM_DECIMAL=6636321 # 0x654321

echo "[*] Test creating shared memory token file"
touch /tmp/img

# Build Nutmeg once outside the loop
echo "[*] Building Nutmeg image once..."
docker build -f Dockerfile -t nutmeg .

echo "[*] Building services..."
git clone https://github.com/chalmers-revere/opendlv-video-h264-decoder.git
docker build -f opendlv-video-h264-decoder/Dockerfile -t h264decoder:v0.0.5 opendlv-video-h264-decoder

echo "[*] Installing Python dependencies..."
pip install -r microservices/perfy_producer/requirements.txt
pip install -r microservices/karen/requirements.txt

echo "[*] Installing perfy_bridge..."
mkdir -p microservices/perfy_bridge/build
cd microservices/perfy_bridge/build
cmake .. || {
  echo "Error: CMake configuration failed"
  exit 1
}

make || {
  echo "Error: Make failed"
  exit 1
}

cd - >/dev/null

xhost +local: || true

for rec_file in "${REC_FILES[@]}"; do
  echo "===== Processing: $rec_file ====="

  echo "[*] Starting shared memory services..."
  chmod +x scripts/run_services.sh
  ./scripts/run_services.sh

  echo "[*] Launching Cyber-Perfy Producer..."
  python3 microservices/perfy_producer/main.py --file "$rec_file" &
  PRODUCER_PID=$!

  echo "[*] Starting Karen with $rec_file and commit ID: $COMMIT_ID"
  PYTHONPATH=microservices/karen/src \
    python3 microservices/karen/src/main.py \
    --commit_id "$COMMIT_ID" --video_file "$rec_file"

  echo "[*] Launching Nutmeg container..."
  docker run --rm --name nutmeg_container --net=host --ipc=host \
    -e DISPLAY="${DISPLAY:-:0}" -v /tmp:/tmp \
    nutmeg:latest --cid=253 --name=img \
    --width=640 --height=480 --analyze &
  NUTMEG_PID=$!

  sleep 5

  echo "[*] Launching Cyber-Perfy Bridge..."
  ./microservices/perfy_bridge/build/perfy &
  BRIDGE_PID=$!

  kill $PRODUCER_PID 2>/dev/null || true
  kill $BRIDGE_PID 2>/dev/null || true
  docker stop nutmeg_container >/dev/null 2>&1 || true

  echo "[*] Stopping shared memory services..."
  chmod +x scripts/stop_services.sh
  ./scripts/stop_services.sh

  echo "===== Done with: $rec_file ====="
done

xhost - || true
