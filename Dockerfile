# =============================================================================
# Stage 1: Build — compiles SudharshanMart and all vcpkg dependencies.
# =============================================================================
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
ENV VCPKG_ROOT=/opt/vcpkg
ENV VCPKG_DISABLE_METRICS=1
ENV VCPKG_MAX_CONCURRENCY=2
ENV CMAKE_BUILD_PARALLEL_LEVEL=2

# Build tools plus the portability headers required to build vcpkg ports.
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        ninja-build \
        git \
        curl \
        wget \
        zip \
        unzip \
        tar \
        pkg-config \
        ca-certificates \
        autoconf \
        automake \
        libtool \
        bison \
        flex \
        python3 \
        perl \
        file \
    && rm -rf /var/lib/apt/lists/*

# Install a pinned vcpkg checkout and disable telemetry.
RUN git clone https://github.com/microsoft/vcpkg.git "$VCPKG_ROOT" \
    && git -C "$VCPKG_ROOT" checkout "${VCPKG_COMMIT:-master}" \
    && "$VCPKG_ROOT/bootstrap-vcpkg.sh" -disableMetrics

WORKDIR /app

# Dependency manifest and the custom Release triplet it references.
COPY vcpkg.json ./
COPY triplets/ ./triplets/

# Resolve the manifest in Release-only static mode.
RUN "$VCPKG_ROOT/vcpkg" install \
        --triplet=x64-linux-release \
        --overlay-triplets=/app/triplets \
        --clean-after-build

# Application sources.
COPY . .

# Configure with the vcpkg toolchain, reusing the pre-installed dependency tree.
RUN cmake -S . -B build -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
        -DVCPKG_TARGET_TRIPLET=x64-linux-release \
        -DVCPKG_OVERLAY_TRIPLETS=/app/triplets \
        -DVCPKG_INSTALLED_DIR=/app/vcpkg_installed \
        -DVCPKG_MANIFEST_MODE=ON \
        -DVCPKG_INSTALL_OPTIONS=--clean-after-build \
    && cmake --build build --target sudharshanmart --parallel 2

# =============================================================================
# Stage 2: Runtime — minimal image containing only the self-contained binary.
# =============================================================================
FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive

# curl is used by the health check; tini reaps children and forwards signals.
# util-linux (provides setpriv) and libstdc++6 are already present in base.
RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates \
        libstdc++6 \
        tini \
        curl \
    && rm -rf /var/lib/apt/lists/* \
    && groupadd --system --gid 1001 appuser \
    && useradd --system --uid 1001 --gid appuser \
        --home-dir /opt/sudharshanmart --shell /usr/sbin/nologin appuser

WORKDIR /opt/sudharshanmart

# Compiled binary and runtime assets.
COPY --from=builder /app/build/sudharshanmart /opt/sudharshanmart/bin/sudharshanmart
COPY --from=builder /app/public /opt/sudharshanmart/public
COPY --from=builder /app/db /opt/sudharshanmart/db

# Entrypoint (normalize line endings to be safe on Windows checkouts).
COPY docker/entrypoint.sh /usr/local/bin/entrypoint.sh
RUN sed -i 's/\r$//' /usr/local/bin/entrypoint.sh \
    && chmod +x /usr/local/bin/entrypoint.sh \
    && mkdir -p /var/data \
    && chown -R appuser:appuser /opt/sudharshanmart /var/data

# Runtime defaults; Render overrides PORT and (optionally) DB_PATH.
ENV PORT=8080
ENV DB_PATH=/var/data/sudharshanmart.db
ENV AI_CHATBOT_PROVIDER=mock

EXPOSE 8080

HEALTHCHECK --interval=30s --timeout=5s --start-period=20s --retries=3 \
    CMD curl -fsS "http://127.0.0.1:${PORT}/api/v1/health" || exit 1

ENTRYPOINT ["/usr/bin/tini", "--", "/usr/local/bin/entrypoint.sh"]
CMD ["/opt/sudharshanmart/bin/sudharshanmart"]
