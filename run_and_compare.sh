#!/bin/bash
set -e

COMMIT_SHA=$(git rev-parse --short HEAD)
REC_DIR="res/video_feeds"
OUT_DIR="res/steering_data/comparison_csv"
PLOT_DIR="res/steering_data/comparison_plots"
PYTHON_DIR="python"
VIDEO_WIDTH=640
VIDEO_HEIGHT=480
SHM_HEX_KEY="0x696d67"

echo "Building rec2txt"
mkdir -p build && cd build
cmake .. -D CMAKE_BUILD_TYPE=Release
make rec2txt -j$(nproc)
cd ..

mkdir -p "$OUT_DIR" "$PLOT_DIR"
echo "Process recordings for commit: $COMMIT_SHA"

echo "Building nutmeg..."
docker build -f Dockerfile -t nutmeg .

for rec in $REC_DIR/*.rec; do
  base=$(basename "$rec" .rec)
  echo "Processing $base..."
  OUTPUT_SUBDIR="$(pwd)/$OUT_DIR/$base/$COMMIT_SHA"
  mkdir -p "$OUTPUT_SUBDIR"

  echo "Extracting ground steering data..."
  REC2TXT="./build/rec2txt"
  if [ ! -x "$REC2TXT" ]; then
    echo "Error: rec2txt not built. Run cmake and make first."
    exit 1
  fi

  "$REC2TXT" --rec="$rec" --output="$OUTPUT_SUBDIR/raw.txt"

  echo "Streaming frames from raw.txt to shared memory..."
  python3 scripts/stream_h264_frames_to_shm.py \
    --input="$OUTPUT_SUBDIR/raw.txt" \
    --width=$VIDEO_WIDTH \
    --height=$VIDEO_HEIGHT &
  PRODUCER_PID=$!

  echo "Waiting for shared memory token..."
  for i in {1..10}; do
    if ipcs -m | grep -q "$(printf '%d' $SHM_HEX_KEY)"; then
      echo "Shared memory token detected."
      break
    fi
    sleep 2
  done

  echo "Running nutmeg on $base..."
  docker run --rm \
    --ipc=host \
    -v "$OUTPUT_SUBDIR:/data/output" \
    -v /tmp:/tmp \
    nutmeg:latest \
    --cid=253 --name=img \
    --output-dir=/data/output \
    --width=$VIDEO_WIDTH --height=$VIDEO_HEIGHT --generate_plot

  echo "Killing frame producer..."
  kill $PRODUCER_PID || true
  wait $PRODUCER_PID || true

  echo "Cleaning up token file..."
  rm -f /tmp/img
done

echo "Generating plot for comparison..."
docker run --rm \
  -e CI_COMMIT_SHA=$COMMIT_SHA \
  -v "$(pwd)/res/steering_data/csv:/data/comparison_csv" \
  -v "$(pwd)/res/steering_data/plots:/data/comparison_plots" \
  -v "$(pwd)/python:/app/python" \
  python:3.11-slim \
  sh -c "pip install -r /app/python/requirements.txt && python3 /app/python/pipeline_plot.py"
