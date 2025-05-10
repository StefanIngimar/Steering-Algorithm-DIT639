FROM alpine:3.21 AS builder

RUN apk add --no-cache \
    build-base \
    cmake \
    opencv-dev \
    ca-certificates

ADD . /opt/sources
WORKDIR /opt/sources
RUN rm -rf build && \
    mkdir build && \
    cd build && \
    cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/tmp .. && \
    make && make install

FROM alpine:3.21

RUN apk add --no-cache \
    opencv \
    libstdc++

WORKDIR /usr/bin

COPY --from=builder /opt/sources/res /usr/bin/res
COPY --from=builder /tmp/bin/nutmeg .

ENTRYPOINT ["/usr/bin/nutmeg"]
