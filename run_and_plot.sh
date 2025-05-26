#!/bin/bash

set -e # Exit on error
xhost +local:

# Ensure output directories exist
mkdir -p res/steering_data/csv
mkdir -p res/steering_data/plots

echo "1. Extracting actual steering from example.rec..."
python3 scripts/python3/printContentFromRecFile.py ./res/video_feeds/CID-140-recording-2020-03-18_144821-selection.rec res/steering_data/csv/actual.csv

echo "2. Starting Python frame producer (main.py)..."
python3 scripts/python3/main.py &
PY_PID=$!

echo "3. Waiting 2s to allow shared memory setup..."
sleep 2

echo "4. Starting Nutmeg container..."
docker run --rm --net=host --ipc=host \
  -e DISPLAY="$DISPLAY" -v /tmp:/tmp \
  -v "$(pwd)/res/steering_data/csv:/data/csv" \
  nutmeg:latest --cid=253 --name=img \
  --width=640 --height=480 --generate_plot &
NUTMEG_PID=$!

echo "5. Waiting 2s before starting C++ bridge..."
sleep 2

echo "6. Starting C++ bridge..."
(cd scripts/cpp && ./run.sh) &
BRIDGE_PID=$!

echo "7. Waiting for bridge and Nutmeg to finish..."
wait $BRIDGE_PID
wait $NUTMEG_PID
kill $PY_PID

xhost -

echo "8. Merging actual.csv and inferred.csv into combined.csv..."
python3 scripts/python3/merge_csv.py

echo "✅ Done. Combined CSV written to: res/steering_data/csv/combined.csv"
