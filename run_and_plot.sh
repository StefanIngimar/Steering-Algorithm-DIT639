#!/bin/bash

xhost +local:

mkdir -p res/steering_data/csv
mkdir -p res/steering_data/plots

echo "Starting nutmeg..."
docker run --rm --net=host --ipc=host \
  -e DISPLAY="$DISPLAY" -v /tmp:/tmp \
  -v "$(pwd)/res/steering_data/csv:/data/csv" \
  nutmeg:latest --cid=253 --name=img \
  --width=640 --height=480 --verbose --generate_plot

xhost -

echo "Generating plot..."
docker run --rm \
  -v "$(pwd)/res/steering_data/csv:/data/csv" \
  -v "$(pwd)/res/steering_data/plots:/data/plots" \
  -v "$(pwd)/python:/app/python" \
  python:3.11-slim \
  sh -c "pip install -r /app/python/requirements.txt && python3 /app/python/plot.py"
