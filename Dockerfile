##################################################
# Section 1: Build the application
# alpine image is only 5mb in size and can be used as base instead of ubuntu:24.04 https://hub.docker.com/_/alpine
FROM alpine:3.14 as builder
MAINTAINER Christian Berger christian.berger@gu.se
# less run commands than previous image. according to a stackoverflow user, run commands create a temporary container from the last image https://stackoverflow.com/questions/37837212/what-are-the-reasons-not-to-use-many-run-commands-in-a-dockerfile
RUN apk add --no-cache cmake build-base

WORKDIR /opt/sources
# replaced ADD with COPY since i was trying different things. dont think this affects size
COPY demo/ /opt/sources/
# statically link dependencies in the build phase. this is because the bundle uses scratch as base https://www.ianlewis.org/en/creating-smaller-docker-images-static-binaries
RUN rm -rf build && \
    mkdir build && \
    cd build && \
    cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_EXE_LINKER_FLAGS="-static" .. && \
    make && make test && \
    strip main && \
    cp main /tmp/main

#################################################
# Section 2: Bundle the application.
# scratch is the most minimal image in docker. the static build above can be used in the scratch bundle. https://stackoverflow.com/questions/47373889/what-is-dockers-scratch-image
FROM scratch
MAINTAINER Christian Berger christian.berger@gu.se
WORKDIR /opt
COPY --from=builder /tmp/main /opt/main
ENTRYPOINT ["/opt/main"]
