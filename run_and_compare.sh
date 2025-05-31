#!/bin/bash

set -euo pipefail

COMMIT_ID=$(git rev-parse --short HEAD)
REC_DIR="microservices/perfy_producer/res"
REC_FILES=("$REC_DIR"/*.rec)

# Build Nutmeg once outside the loop
echo "[*] Building Nutmeg image once..."
docker build -f Dockerfile -t nutmeg .

echo "[*] Building services..."
docker build https://github.com/chalmers-revere/opendlv-video-h264-decoder.git -f Dockerfile -t h264decoder:v0.0.5

echo "[*] Installing Python dependencies..."
pip install -r microservices/perfy_producer/requirements.txt
pip install -r microservices/karen/requirements.txt

xhost +local: || true

for rec_file in "${REC_FILES[@]}"; do
  echo "===== Processing: $rec_file ====="

  echo "[*] Starting shared memory services..."
  chmod +x scripts/run_services.sh
  ./scripts/run_services.sh

  echo "[*] Launching Nutmeg container..."
  docker run --rm --name nutmeg_container --net=host --ipc=host \
    -e DISPLAY="$DISPLAY" -v /tmp:/tmp \
    nutmeg:latest --cid=253 --name=img \
    --width=640 --height=480 --analyze &
  NUTMEG_PID=$!

  echo "[*] Launching Cyber-Perfy Producer..."
  python3 microservices/perfy_producer/main.py --file "$rec_file" &
  PRODUCER_PID=$!

  echo "[*] Launching Cyber-Perfy Bridge..."
  chmod +x microservices/perfy_bridge/run.sh
  ./microservices/perfy_bridge/run.sh &
  BRIDGE_PID=$!

  echo "[*] Waiting briefly for shared memory setup..."
  sleep 3

  echo "[*] Starting Karen with $rec_file and commit ID: $COMMIT_ID"
  PYTHONPATH=microservices/karen/src \
    python3 microservices/karen/src/main.py \
    --commit_id "$COMMIT_ID" --video_file "$rec_file"

  echo "[*] Karen completed. Cleaning up..."

  kill $PRODUCER_PID 2>/dev/null || true
  kill $BRIDGE_PID 2>/dev/null || true
  docker stop nutmeg_container >/dev/null 2>&1 || true

  echo "[*] Stopping shared memory services..."
  chmod +x scripts/stop_services.sh
  ./scripts/stop_services.sh

  echo "===== Done with: $rec_file ====="
done

xhost - || true
