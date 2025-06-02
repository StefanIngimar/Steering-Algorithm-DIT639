FROM registry.git.chalmers.se/courses/dit638/students/2025-group-04/opencv-dev:4.10 AS builder

WORKDIR /app

COPY . /app

RUN rm -rf build && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make -j$(nproc) && \
    make install

FROM registry.git.chalmers.se/courses/dit638/students/2025-group-04/opencv:4.10

WORKDIR /usr/bin

COPY --from=builder /app/build/perfy . 

ENTRYPOINT ["/usr/bin/perfy"]

