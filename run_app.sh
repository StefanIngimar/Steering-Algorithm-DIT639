#!/bin/bash

docker_image_exists() {
  docker image inspect "$1" >/dev/null 2>&1
}

xhost +local:

echo "Checking nutmeg image..."
if ! docker_image_exists "nutmeg:latest"; then
  echo "Building nutmeg image..."
  docker build -f Dockerfile -t nutmeg .
else
  echo "nutmeg image already exists."
fi

mkdir -p res/steering_data/csv
mkdir -p res/steering_data/plots

echo "Starting nutmeg..."
docker run --net=host --ipc=host \
  -e DISPLAY="$DISPLAY" -v /tmp:/tmp \
  -v "$(pwd)/res/steering_data/csv:/data/csv" \
  nutmeg:latest --cid=253 --name=img \
  --width=640 --height=480 --verbose

echo "check the path:"
ls -l res/steering_data

xhost -

echo "Generating plot..."
docker run --rm \
  -v "$(pwd)/res/steering_data/csv:/data/csv" \
  -v "$(pwd)/res/steering_data/plots:/data/plots" \
  -v "$(pwd)/python:/app/python" \
  python:3.11-slim \
  sh -c "pip install -r /app/python/requirements.txt && python3 /app/python/plot.py"
