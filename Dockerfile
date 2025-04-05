##################################################
# Section 1: Build the application
FROM alpine:3.14 as builder
MAINTAINER Christian Berger christian.berger@gu.se
RUN apk add --no-cache cmake build-base

WORKDIR /opt/sources
COPY demo/ /opt/sources/
RUN rm -rf build && \
    mkdir build && \
    cd build && \
    cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_EXE_LINKER_FLAGS="-static" .. && \
    make && make test && \
    strip main && \
    cp main /tmp/main

#################################################
# Section 2: Bundle the application.
FROM scratch
MAINTAINER Christian Berger christian.berger@gu.se
WORKDIR /opt
COPY --from=builder /tmp/main /opt/main
ENTRYPOINT ["/opt/main"]
