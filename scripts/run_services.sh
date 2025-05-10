#!/bin/bash

docker_image_exists() {
    docker image insepect "$1" > /dev/null 2>&1
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

wait

echo "All services are running"
