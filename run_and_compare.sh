#!/bin/bash

COMMIT_ID=$(git rev-parse --short HEAD)
REC_DIR="microservices/perfy_producer/res"
REC_FILES=("$REC_DIR"/*.rec)

for rec_file in "${REC_FILES[@]}"; do
  echo "===== Processing: $rec_file ====="

  echo "[*] Starting services..."
  chmod +x scripts/run_services.sh
  ./scripts/run_services.sh

  xhost +local:

  echo "[*] Installing dependencies..."
  pip install -r microservices/perfy_producer/requirements.txt
  pip install -r microservices/karen/requirements.txt

  echo "[*] Starting Cyber-Perfy Producer with $rec_file"
  python3 microservices/perfy_producer/main.py --file "$rec_file" &

  echo "[*] Starting Nutmeg..."
  docker run --rm --net=host --ipc=host \
    -e DISPLAY="$DISPLAY" -v /tmp:/tmp \
    nutmeg:latest --cid=253 --name=img \
    --width=640 --height=480 &

  echo "[*] Starting Cyber-Perfy bridge..."
  chmod +x microservices/perfy_bridge/run.sh
  ./microservices/perfy_bridge/run.sh &

  echo "[*] Sleeping briefly to allow shared memory to initialize..."
  sleep 3

  echo "[*] Starting Karen with $rec_file and commit ID: $COMMIT_ID"
  PYTHONPATH=microservices/karen/src \
    python3 microservices/karen/src/main.py \
    --commit_id "$COMMIT_ID" --video_file "$rec_file"

  echo "[*] Waiting for services to finish..."
  wait

  xhost -

  echo "[*] Stopping services..."
  chmod +x scripts/stop_services.sh
  ./scripts/stop_services.sh

  echo "===== Done with: $rec_file ====="
done
