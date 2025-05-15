FROM alpine:3.21 AS builder

ENV TMPDIR=/opt/tmp

RUN mkdir -p /opt/tmp && chmod 1777 /opt/tmp

RUN apk add --no-cache \
    build-base \
    cmake \
    git \
    opencv-dev \
    pkgconfig \
    && mkdir -p /usr/lib

ADD . /opt/sources
WORKDIR /opt/sources

RUN rm -rf build && \
    mkdir build && \
    cd build && \
    cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/tmp .. && \
    make -j$(nproc) && make install

FROM alpine:3.21

RUN apk add --no-cache \
    opencv \
    libstdc++

WORKDIR /usr/bin

COPY --from=builder /tmp/bin/nutmeg .
COPY --from=builder /opt/sources/res /usr/bin/res

ENTRYPOINT ["/usr/bin/nutmeg"]
