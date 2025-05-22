#!/bin/bash

# Detect platform
PLATFORM=$(uname)

docker_image_exists() {
    docker image inspect "$1" > /dev/null 2>&1
}

# macOS-specific setup for XQuartz
if [ "$PLATFORM" == "Darwin" ]; then
    echo "Starting XQuartz..."
    open -a XQuartz
    sleep 2  # Give XQuartz time to initialize

    # Grant access from local Docker containers using system DISPLAY
    DISPLAY=:0 xhost + 127.0.0.1 > /dev/null

    # Set DISPLAY to the standard value used by XQuartz
    export DISPLAY=host.docker.internal:0

    echo "macOS detected — DISPLAY set to $DISPLAY and XQuartz access granted"
fi


echo "Starting Opendlv Vehicle View"
if [ "$PLATFORM" == "Linux" ]; then
    # Linux: use host networking
    docker run --rm -d --init --net=host --name=opendlv-vehicle-view \
        -v "$PWD/res/video_feeds:/opt/vehicle-view/recordings" \
        -v /var/run/docker.sock:/var/run/docker.sock \
        -p 8081:8081 chrberger/opendlv-vehicle-view:v0.0.64
else
    # macOS: port forwarding only
    docker run --rm -d --init --net=bridge --name=opendlv-vehicle-view \
        -v "$PWD/res/video_feeds:/opt/vehicle-view/recordings" \
        -v /var/run/docker.sock:/var/run/docker.sock \
        -p 8081:8081 \
        chrberger/opendlv-vehicle-view:v0.0.64
fi

echo "Checking h264-decoder image"
if ! docker_image_exists "h264decoder:v0.0.5"; then
    echo "Building h264-decoder image..."
    docker build https://github.com/chalmers-revere/opendlv-video-h264-decoder.git#v0.0.5 \
        -f Dockerfile -t h264decoder:v0.0.5
else
    echo "h264-decoder image already exists"
fi

echo "Starting h264-decoder..."
if [ "$PLATFORM" == "Linux" ]; then
    docker run --rm -d --net=host --ipc=host \
        -e DISPLAY="$DISPLAY" -v /tmp:/tmp \
        h264decoder:v0.0.5 --cid=253 --name=img
else
    docker run --rm -ti --net=bridge --ipc=host \
        -e DISPLAY="$DISPLAY" -v /tmp:/tmp \
        h264decoder:v0.0.5 --cid=253 --name=img --verbose

fi

echo "All services are running"