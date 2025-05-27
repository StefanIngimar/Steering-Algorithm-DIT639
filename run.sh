#!/bin/bash

xhost +local:

echo "Building nutmeg image..."
docker build -f Dockerfile -t nutmeg .

echo "Starting nutmeg..."
docker run --rm --net=host --ipc=host \
  -e DISPLAY="$DISPLAY" -v /tmp:/tmp \
  nutmeg:latest --cid=253 --name=img \
  --width=640 --height=480 --verbose

xhost -
