# PVCplus - Phase Vocoder Coding toolkit
#
# Multi-stage build. Default target (`docker build .`) is `runtime`: a small
# image with `pvc` as its entrypoint, plus the legacy C tools reachable
# through `pvc legacy <tool> <flags>`. `--target dev` builds the interactive
# devcontainer image instead (Rust toolchain, C build tools, and a shell).

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

# ---- rust-build: compile the pvc CLI ----
FROM rust:1-bookworm AS rust-build

WORKDIR /src
COPY rust /src/rust
RUN cargo build --release --locked --manifest-path rust/Cargo.toml -p pvc-cli

# ---- dev: interactive devcontainer image (Rust + C toolchains, a shell) ----
FROM rust:1-bookworm AS dev

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        pkg-config \
        libsndfile1-dev \
        zsh \
        git \
        curl \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY --from=legacy-build /opt/pvc-legacy /opt/pvc-legacy

ENV PATH="/opt/pvc-legacy/bin:${PATH}"
WORKDIR /workspaces/docker-pvcplus
CMD ["zsh"]

# ---- runtime: minimal image with pvc plus the legacy tools ----
FROM debian:bookworm-slim AS runtime

RUN apt-get update && apt-get install -y --no-install-recommends \
        libsndfile1 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=legacy-build /opt/pvc-legacy /opt/pvc-legacy
COPY --from=rust-build /src/rust/target/release/pvc /usr/local/bin/pvc

ENV PATH="/opt/pvc-legacy/bin:${PATH}"

RUN mkdir -p /audio/input /audio/output
VOLUME ["/audio/input", "/audio/output"]
WORKDIR /audio

# `docker run pvcplus <legacy-tool> <flags>` (the pre-Phase-4.4 invocation
# style) no longer reaches a legacy tool directly - use
# `docker run pvcplus legacy <legacy-tool> <flags>` instead, or the
# legacy-runtime target/`:legacy` tag below. See docs/getting-started.md
# and docs/migration.md.
ENTRYPOINT ["pvc"]
CMD ["--help"]

# ---- legacy-runtime: legacy tools only, no pvc, for anyone who wants
# the pre-Phase-4.4 direct-by-name invocation style
# (`docker run ghcr.io/mjladd/pvcplus:legacy plainpv ...`) ----
FROM debian:bookworm-slim AS legacy-runtime

RUN apt-get update && apt-get install -y --no-install-recommends \
        libsndfile1 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=legacy-build /opt/pvc-legacy /opt/pvc-legacy

ENV PATH="/opt/pvc-legacy/bin:${PATH}"

RUN mkdir -p /audio/input /audio/output
VOLUME ["/audio/input", "/audio/output"]
WORKDIR /audio

CMD ["sh", "-c", "echo 'pvc-legacy tools available in /opt/pvc-legacy/bin:' && ls /opt/pvc-legacy/bin && echo 'Input dir: /audio/input | Output dir: /audio/output'"]
