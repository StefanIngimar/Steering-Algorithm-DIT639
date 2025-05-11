# Builder stage
FROM --platform=linux/amd64 ubuntu:20.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libopencv-dev=4.2.0+dfsg-5 \
    libspdlog-dev \
    && rm -rf /var/lib/apt/lists/*

ADD . /opt/sources
WORKDIR /opt/sources

RUN rm -rf build && \
    mkdir build && \
    cd build && \
    cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/tmp .. && \
    make && make install

# Runtime stage
FROM --platform=linux/amd64 ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    libopencv-core4.2=4.2.0+dfsg-5 \
    libopencv-imgproc4.2=4.2.0+dfsg-5 \
    libopencv-highgui4.2=4.2.0+dfsg-5 \
    libopencv-dnn4.2=4.2.0+dfsg-5 \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/bin

COPY --from=builder /tmp/bin/nutmeg .
COPY --from=builder /opt/sources/res /usr/bin/res
COPY --from=builder /usr/lib/x86_64-linux-gnu/libspdlog.so* /usr/lib/
COPY --from=builder /opt/sources/external/onnxruntime/lib/libonnxruntime.so* /usr/lib/

ENTRYPOINT ["/usr/bin/nutmeg"]