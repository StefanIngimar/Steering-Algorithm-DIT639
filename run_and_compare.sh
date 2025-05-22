#!/bin/bash

set -e

COMMIT_SHA=$(git rev-parse --short HEAD)
REC_DIR="res/video_feeds"
OUT_DIR="res/steering_data/comparison_csv"
PLOT_DIR="res/steering_data/comparison_plots"
PYTHON_DIR="python"

mkdir -p "$OUT_DIR"
mkdir -p "$PLOT_DIR"

echo "Process recordings for commit: $COMMIT_SHA"

docker_image_exists() {
  docker image inspect "$1" >/dev/null 2>&1
}

echo "Starting Opendlv Vehicle View"
docker run --rm -d --init --net=host --name=opendlv-vehicle-view \
  -v "$PWD/res/video_feeds:/opt/vehicle-view/recordings" \
  -v /var/run/docker.sock:/var/run/docker.sock \
  -p 8081:8081 chrberger/opendlv-vehicle-view:v0.0.64

echo "Checking h264-decoder image"
if ! docker_image_exists "h264decoder:v0.0.5"; then
  echo "Building h264-decoder image..."
  docker build https://github.com/chalmers-revere/opendlv-video-h264-decoder.git#v0.0.5 \
    -f Dockerfile -t h264decoder:v0.0.5
else
  echo "h264-decoder image already exists"
fi

echo "Starting h264-decoder..."
docker run --rm -d --net=host --ipc=host \
  -e DISPLAY="$DISPLAY" -v /tmp:/tmp \
  h264decoder:v0.0.5 --cid=253 --name=img

sleep 5

echo "The other two services are running"

echo "Waiting for shared memory '/tmp/img' to be ready..."

for i in {1..10}; do
  if [ -e /tmp/img ]; then
    echo "Shared memory ready."
    break
  fi
  sleep 1
done

if [ ! -e /tmp/img ]; then
  echo "ERROR: Shared memory '/tmp/img' not found after waiting."
  docker ps -a
  docker logs $(docker ps -q --filter name=h264-decoder) || true
  exit 1
fi

echo "Building nutmeg..."
docker build -f Dockerfile -t nutmeg .

for rec in $REC_DIR/*.rec; do
  base=$(basename "$rec" .rec)

  echo "Processing $base..."
  OUTPUT_SUBDIR="$(pwd)/$OUT_DIR/$base/$COMMIT_SHA"

  mkdir -p "$OUTPUT_SUBDIR"

  docker run --rm \
    -v "$(pwd)/$rec:/data/input.rec" \
    -v "$OUTPUT_SUBDIR:/data/output" \
    nutmeg:latest \
    --cid=253 --name=img \
    --input=/data/input.rec \
    --output-dir=/data/output \
    --width=640 --height=480 --verbose --generate_plot
done

echo "Generating plot for comparison..."
docker run --rm \
  -e CI_COMMIT_SHA=$COMMIT_SHA \
  -v "$(pwd)/res/steering_data/csv:/data/comparison_csv" \
  -v "$(pwd)/res/steering_data/plots:/data/comparison_plots" \
  -v "$(pwd)/python:/app/python" \
  python:3.11-slim \
  sh -c "pip install -r /app/python/requirements.txt && python3 /app/python/pipeline_plot.py"
