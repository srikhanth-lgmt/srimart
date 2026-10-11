
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
ENV VCPKG_MAX_CONCURRENCY=2
ENV CMAKE_BUILD_PARALLEL_LEVEL=2

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake ninja-build git curl zip unzip tar pkg-config \
    ca-certificates autoconf automake libtool bison flex python3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

RUN git clone https://github.com/microsoft/vcpkg.git /opt/vcpkg \
    && /opt/vcpkg/bootstrap-vcpkg.sh -disableMetrics

ENV VCPKG_ROOT=/opt/vcpkg

COPY vcpkg.json ./

# Install project dependencies with limited concurrency
RUN /opt/vcpkg/vcpkg install --triplet x64-linux

COPY . .

# Build the application with limited parallel jobs
RUN cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DVCPKG_TARGET_TRIPLET=x64-linux \
    && cmake --build build --config Release --parallel 2

FROM ubuntu:24.04 AS runtime

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates libsqlite3-0 libsodium23 libcurl4 libssl3 libuuid1 \
    libjsoncpp25 libbrotli1 libzstd1 libnghttp2-14 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /opt/sudharshanmart

COPY --from=builder /app/build/sudharshanmart /opt/sudharshanmart/bin/sudharshanmart
COPY --from=builder /app/public /opt/sudharshanmart/public
COPY --from=builder /app/db /opt/sudharshanmart/db
COPY --from=builder /app/vcpkg_installed/x64-linux/lib/ /opt/sudharshanmart/lib/

ENV LD_LIBRARY_PATH=/opt/sudharshanmart/lib
ENV PORT=8080
ENV DB_PATH=/var/data/sudharshanmart.db
ENV AI_CHATBOT_PROVIDER=mock

EXPOSE 8080

ENTRYPOINT ["/opt/sudharshanmart/bin/sudharshanmart"]
