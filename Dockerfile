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
        autoconf-archive \
        automake \
        libtool \
        bison \
        flex \
        python3 \
        perl \
        file \
        libpq-dev \
    && rm -rf /var/lib/apt/lists/*

# Pin the vcpkg tool + registry to the exact baseline declared in vcpkg.json.
ARG VCPKG_BASELINE=9e593bb18ea69cc5095e012465dcd675a822ed0d

# Install the pinned vcpkg checkout and disable telemetry.
RUN git clone https://github.com/microsoft/vcpkg.git "$VCPKG_ROOT" \
    && git -C "$VCPKG_ROOT" checkout "$VCPKG_BASELINE" \
    && "$VCPKG_ROOT/bootstrap-vcpkg.sh" -disableMetrics

WORKDIR /app

# Dependency manifest and the custom Release triplet it references.
COPY vcpkg.json ./
COPY triplets/ ./triplets/

# Resolve the manifest in Release-only static mode. The wrapper reprints the
# underlying vcpkg error so it is visible at the end of the Render build log.
COPY docker/install-deps.sh /usr/local/bin/install-deps.sh
RUN sed -i 's/\r$//' /usr/local/bin/install-deps.sh \
    && chmod +x /usr/local/bin/install-deps.sh \
    && /usr/local/bin/install-deps.sh

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
# libpq5 provides libpq.so at runtime for the PostgreSQL (Neon) backend.
RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates \
        libstdc++6 \
        libpq5 \
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

# Runtime defaults; Render must provide DATABASE_URL for the Neon database.
# Local Docker Compose explicitly overrides REQUIRE_POSTGRES to keep SQLite usable.
ENV PORT=8080
ENV DB_PATH=/var/data/sudharshanmart.db
ENV REQUIRE_POSTGRES=true
ENV AI_CHATBOT_PROVIDER=mock

EXPOSE 8080

HEALTHCHECK --interval=30s --timeout=5s --start-period=20s --retries=3 \
    CMD curl -fsS "http://127.0.0.1:${PORT}/api/v1/health" || exit 1

ENTRYPOINT ["/usr/bin/tini", "--", "/usr/local/bin/entrypoint.sh"]
CMD ["/opt/sudharshanmart/bin/sudharshanmart"]
