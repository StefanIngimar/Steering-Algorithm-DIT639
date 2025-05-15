#!/bin/bash

docker_image_exists() {
    docker image inspect "$1" > /dev/null 2>&1
}

xhost +local:

echo "Checking nutmeg image..."
if ! docker_image_exists "nutmeg:latest"; then
    echo "Building nutmeg image..."
    docker build -f Dockerfile -t nutmeg .
else
    echo "nutmeg image already exists."
fi

echo "Starting nutmeg..."
docker run --net=host --ipc=host \
    -e DISPLAY="$DISPLAY" -v /tmp:/tmp \
    nutmeg:latest --cid=253 --name=img \
    --width=640 --height=480 --verbose

xhost -
