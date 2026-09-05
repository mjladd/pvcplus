# PVCplus - Phase Vocoder Coding toolkit
#
# Multi-stage build. Default target (`docker build .`) is `runtime`: a small
# image with only the built legacy C tools and their runtime library.
# `--target dev` builds the interactive devcontainer image instead.

# ---- legacy-build: compile the legacy C toolkit with CMake ----
FROM debian:bookworm-slim AS legacy-build

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        pkg-config \
        libsndfile1-dev \
    && rm -rf /var/lib/apt/lists/*

COPY legacy /src/legacy

RUN cmake -S /src/legacy -B /build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build /build -j"$(nproc)" \
    && cmake --install /build --prefix /opt/pvc-legacy

# ---- dev: interactive devcontainer image (adds a shell + dev tools) ----
FROM legacy-build AS dev

RUN apt-get update && apt-get install -y --no-install-recommends \
        zsh \
        git \
        curl \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

ENV PATH="/opt/pvc-legacy/bin:${PATH}"
WORKDIR /workspaces/docker-pvcplus
CMD ["zsh"]

# ---- rust-build: placeholder for the Rust CLI (wired up in Phase 2+) ----
FROM rust:1-bookworm AS rust-build

WORKDIR /src
# COPY rust /src/rust
# RUN cargo build --release --locked --manifest-path rust/Cargo.toml

# ---- runtime: minimal image with just the built tools ----
FROM debian:bookworm-slim AS runtime

RUN apt-get update && apt-get install -y --no-install-recommends \
        libsndfile1 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=legacy-build /opt/pvc-legacy /opt/pvc-legacy
# COPY --from=rust-build /src/rust/target/release/pvc /usr/local/bin/pvc

ENV PATH="/opt/pvc-legacy/bin:${PATH}"

RUN mkdir -p /audio/input /audio/output
VOLUME ["/audio/input", "/audio/output"]
WORKDIR /audio

# ENTRYPOINT ["pvc"]   # switch on once the Rust CLI exists (Phase 2+)
CMD ["sh", "-c", "echo 'pvc-legacy tools available in /opt/pvc-legacy/bin:' && ls /opt/pvc-legacy/bin && echo 'Input dir: /audio/input | Output dir: /audio/output'"]
