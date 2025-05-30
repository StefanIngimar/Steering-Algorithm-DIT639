#!/bin/bash

SCRIPT_DIR=$(dirname "$(realpath "$0")")
cd "$SCRIPT_DIR" || exit 1

mkdir -p build
cd build || exit 1

# use the right cmake
cmake .. || {
  echo "Error: CMake configuration failed"
  exit 1
}
make || {
  echo "Error: Make failed"
  exit 1
}

# correct path to the binary
./perfy || {
  echo "Error: Failed to run perfy"
  exit 1
}
