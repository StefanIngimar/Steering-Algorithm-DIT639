#!/bin/bash

cd build || {
  echo "Error: Failed to change to build directory"
  exit 1
}

cmake .. || {
  echo "Error: CMake configuration failed"
  exit 1
}

make || {
  echo "Error: Make failed"
  exit 1
}

./perfy || {
  echo "Error: Failed to run perfy"
  exit 1
}
