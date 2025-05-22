#!/bin/bash

set -e

COMMIT_SHA=$(git rev-parse --short HEAD)
REC_DIR="/res/video_feeds"
OUT_DIR="/res/steering_data/comparison_csv"
PLOT_DIR="/res/steering_data/comparison_plots"
PYTHON_DIR="/python"

mkdir -p "$OUT_DIR"
mkdir -p "$PLOT_DIR"

echo "Process recordings for commit: $COMMIT_SHA"

echo "Building nutmeg image..."
docker build -f Dockerfile -t nutmeg .

for rec in $REC_DIR/*.rec; do
  base=$(basename "$rec" .rec)

  echo "Processing $base..."
  OUTPUT_SUBDIR="$OUT_DIR/$base/$COMMIT_SHA"

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
