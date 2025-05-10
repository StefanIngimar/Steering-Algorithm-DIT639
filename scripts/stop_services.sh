#!/bin/bash

echo "Stopping all containers..."

docker stop opendlv-vehicle-view 2>/dev/null || true

docker stop $(docker ps -q --filter ancestor=h264decoder:v0.0.5) 2>/dev/null || true

echo "All containers stopped."
