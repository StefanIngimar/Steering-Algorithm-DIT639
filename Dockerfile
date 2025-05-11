FROM ubuntu:22.04 AS builder

ENV TMPDIR=/opt/tmp 
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libopencv-dev \
    libspdlog-dev \
    && rm -rf /var/lib/apt/lists/*

ADD . /opt/sources
WORKDIR /opt/sources

RUN rm -rf build && \
    mkdir build && \
    cd build && \
    cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/tmp .. && \
    make && make install

FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    libopencv-core4.5 \
    libopencv-imgproc4.5 \
    libopencv-highgui4.5 \
    libopencv-dnn4.5 \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/bin

COPY --from=builder /tmp/bin/nutmeg .
COPY --from=builder /opt/sources/res /usr/bin/res
COPY --from=builder /usr/lib/x86_64-linux-gnu/libspdlog.so* /usr/lib/
COPY --from=builder /opt/sources/external/onnxruntime/lib/libonnxruntime.so* /usr/lib/

ENTRYPOINT ["/usr/bin/nutmeg"]
