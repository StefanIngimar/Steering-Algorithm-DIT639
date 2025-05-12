FROM ubuntu:22.04 AS builder

ARG TARGETARCH

ENV TMPDIR=/opt/tmp 
ENV DEBIAN_FRONTEND=noninteractive

RUN mkdir -p /opt/tmp && chmod 1777 /opt/tmp

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    build-essential \
    cmake \
    libopencv-dev \
    libspdlog-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* /var/cache/apt/archives/* /var/lib/dpkg/*-old

ADD . /opt/sources
WORKDIR /opt/sources

RUN set -eux; \
    ARCH_DIR=""; \
    case "$TARGETARCH" in \
        amd64) ARCH_DIR="amd64";; \
        arm) ARCH_DIR="armv7";; \
        *) echo "Unsupported architecture: $TARGETARCH" && exit 1;; \
    esac && \
    mkdir -p external/onnxruntime/include && \
    cp -r external/onnxruntime/${ARCH_DIR}/include/* external/onnxruntime/include && \
    cp -r external/onnxruntime/${ARCH_DIR}/lib external/onnxruntime/lib

RUN rm -rf build && \
    mkdir build && \
    cd build && \
    cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/tmp .. && \
    make && make install

FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    libopencv-core4.5 \
    libopencv-imgproc4.5 \
    libopencv-highgui4.5 \
    libopencv-dnn4.5 \
    libstdc++6 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* /var/cache/apt/archives/* /var/lib/dpkg/*-old

WORKDIR /usr/bin

COPY --from=builder /tmp/bin/nutmeg .
COPY --from=builder /opt/sources/res /usr/bin/res
COPY --from=builder /usr/lib/x86_64-linux-gnu/libspdlog.so* /usr/lib/
COPY --from=builder /opt/sources/external/onnxruntime/lib/libonnxruntime.so* /usr/lib/

ENTRYPOINT ["/usr/bin/nutmeg"]
