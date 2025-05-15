#!/bin/bash

docker_image_exists() {
  docker image insepect "$1" >/dev/null 2>&1
}

xhost +local:

echo "Checking nutmeg image..."
if ! docker_image_exists "nutmeg:latest"; then
  echo "Building nutmeg image..."
  docker build -f Dockerfile -t nutmeg .
else
  echo "nutmeg image already exists."
fi

mkdir -p res/steering_data

echo "Starting nutmeg..."
docker run --net=host --ipc=host \
  -e DISPLAY="$DISPLAY" -v /tmp:/tmp \
  -v "$(pwd)/res/steering_data:/usr/bin/res/steering_data" \
  nutmeg:latest --cid=253 --name=img \
  --width=640 --height=480 --verbose

xhost -

echo "Generating plot..."
docker run --rm \
  -v "$(pwd)/res/steering_data:/app/data" \
  -v "$(pwd)/python:/app/python" \
  python:3.11-slim \
  sh -c "pip install -r /app/python/requirements.txt && python3 /app/python/plot.py"
