FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

# TODO - double check if all these dependencies are even neccessary. I just started adding things trying to make this work.
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libglib2.0-dev \
    wget \
    unzip \
    libopencv-dev \
    libprotobuf-dev \
    libprotoc-dev \
    protobuf-compiler \
    git \
    pkg-config \
    python3-opencv \
    libomp-dev

# ONNX runtime
WORKDIR /opt
RUN wget https://github.com/microsoft/onnxruntime/releases/download/v1.17.0/onnxruntime-linux-x64-1.17.0.tgz && \
    tar -xvzf onnxruntime-linux-x64-1.17.0.tgz && \
    rm onnxruntime-linux-x64-1.17.0.tgz

# ONNX env stuff
ENV ONNXRUNTIME_DIR=/opt/onnxruntime-linux-x64-1.17.0
ENV LD_LIBRARY_PATH=${ONNXRUNTIME_DIR}/lib:$LD_LIBRARY_PATH

ADD . /opt/sources
COPY src/color_cones_yolov5n.onnx /usr/share/models/color_cones_yolov5n.onnx
WORKDIR /opt/sources
RUN ls -l /opt/onnxruntime-linux-x64-1.17.0/lib
# build
RUN mkdir build && cd build && \
    cmake -D CMAKE_BUILD_TYPE=Release \
          -D CMAKE_INSTALL_PREFIX=/usr \
          -D ONNXRUNTIME_INCLUDE_DIR=${ONNXRUNTIME_DIR}/include \
          -D ONNXRUNTIME_LIB_DIR=${ONNXRUNTIME_DIR}/lib .. && \
    make && make install

# Entry point
ENTRYPOINT ["/usr/bin/opendlv-video-hsv-inspector"]

