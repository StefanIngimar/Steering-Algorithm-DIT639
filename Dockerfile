FROM registry.git.chalmers.se/courses/dit638/students/2025-group-04/opencv-dev:4.10 AS builder

ENV TMPDIR=/opt/tmp

RUN mkdir -p /opt/tmp && chmod 1777 /opt/tmp

ADD . /opt/sources
WORKDIR /opt/sources

RUN rm -rf build && \
    mkdir build && \
    cd build && \
    cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/tmp .. && \
    make -j$(nproc) && make install

FROM registry.git.chalmers.se/courses/dit638/students/2025-group-04/opencv:4.10

RUN apt-get update && apt-get install -y bash coreutils python3 python3-pip

WORKDIR /usr/bin

COPY --from=builder /tmp/bin/nutmeg .
COPY --from=builder /opt/sources/res /usr/bin/res

ENTRYPOINT ["/usr/bin/nutmeg"]
