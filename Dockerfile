##################################################
# Section 1: Build the application
FROM alpine:3.14 AS builder
LABEL maintainer="Stefan Ingimarsson stefanla@student.chalmers.se"
RUN apk add --no-cache cmake build-base

WORKDIR /opt/sources
COPY demo/ /opt/sources/
RUN rm -rf build && \
    mkdir build && \
    cd build && \
    cmake -D CMAKE_BUILD_TYPE=Release .. && \
    make && make test && \
    strip main && \
    cp main /tmp/main

#################################################
# Section 2: Bundle the application.
FROM scratch
LABEL maintainer="Stefan Ingimarsson stefanla@student.chalmers.se"
WORKDIR /opt
COPY --from=builder /tmp/main /opt/main
ENTRYPOINT ["/opt/main"]
