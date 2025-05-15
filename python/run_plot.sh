#!/bin/bash
docker run --rm -v "$(pwd)/data:/app/res/steering_data" -v "$(pwd)/python:/app/python" python:3.11-slim \
  python3 /app/python/plot.py
