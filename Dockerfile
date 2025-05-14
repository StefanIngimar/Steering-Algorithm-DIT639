FROM ubuntu:22.04 AS builder

ENV TMPDIR=/opt/tmp 
ENV DEBIAN_FRONTEND=noninteractive

RUN mkdir -p /opt/tmp && chmod 1777 /opt/tmp

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    build-essential \
    cmake \
    git \
    wget \
    unzip \
    pkg-config \
    libjpeg-dev \
    libpng-dev \
    libtiff-dev \
    libavcodec-dev \
    libavformat-dev \
    libswscale-dev \
    libv4l-dev \
    libxvidcore-dev \
    libx264-dev \
    libgtk-3-dev \
    libatlas-base-dev \
    gfortran \
    python3-dev \
    libspdlog-dev \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /opt
RUN git clone --branch 4.10.0 --depth 1 https://github.com/opencv/opencv.git && \
    git clone --branch 4.10.0 --depth 1 https://github.com/opencv/opencv_contrib.git

RUN mkdir -p /opt/opencv/build && cd /opt/opencv/build && \
    cmake -D CMAKE_BUILD_TYPE=Release \
          -D CMAKE_INSTALL_PREFIX=/opt/opencv-install \
          -D OPENCV_EXTRA_MODULES_PATH=/opt/opencv_contrib/modules \
          -D BUILD_EXAMPLES=OFF \
          -D BUILD_TESTS=OFF \
          -D BUILD_PERF_TESTS=OFF \
          .. && \
    make -j"$(nproc)" && \
    make install

ADD . /opt/sources
WORKDIR /opt/sources

RUN rm -rf build && \
    mkdir build && \
    cd build && \
    cmake -D CMAKE_BUILD_TYPE=Release \
          -D CMAKE_PREFIX_PATH=/opt/opencv-install \
          -D OpenCV_DIR=/opt/opencv-install/lib/cmake/opencv4 \
          -D CMAKE_INSTALL_PREFIX=/tmp .. && \
    make && make install

FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    libgtk-3-0 \
    libstdc++6 \
    libjpeg-turbo8 \
    libpng16-16 \
    libtiff5 \
    libavcodec58 \
    libavformat58 \
    libswscale5 \
    libv4l-0 \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

COPY --from=builder /opt/opencv-install /opt/opencv-install
ENV LD_LIBRARY_PATH=/opt/opencv-install/lib:$LD_LIBRARY_PATH

WORKDIR /usr/bin
COPY --from=builder /tmp/bin/nutmeg .
COPY --from=builder /opt/sources/res /usr/bin/res
COPY --from=builder /usr/lib/libspdlog.so* /usr/lib/

ENTRYPOINT ["/usr/bin/nutmeg"]
