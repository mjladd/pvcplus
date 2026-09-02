# PVCplus - Phase Vocoder Coding toolkit
# Docker build environment

FROM ubuntu:22.04

LABEL maintainer="mjladd"
LABEL description="Phase Vocoder audio DSP toolkit"

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Fix GPG keys for Ubuntu repositories (common issue with ARM images)
RUN apt-get clean && \
    rm -rf /var/lib/apt/lists/* && \
    apt-get update --allow-insecure-repositories && \
    apt-get install -y --allow-unauthenticated ubuntu-keyring && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    libsndfile1-dev \
    zsh \
    git \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Copy source code
WORKDIR /src/PVCplus
COPY . .

# Fix hardcoded MacPorts paths in Makefiles
# Remove /opt/local/include from CFLAGS and /opt/local/lib from LDFLAGS
# (libsndfile is installed in standard system locations in the container)
RUN sed -i 's|-I/opt/local/include||g' legacy/pvc_lib/Makefile legacy/pvc_src/Makefile legacy/cmusic_gen/gen/Makefile && \
    sed -i 's|-L/opt/local/lib/||g' legacy/pvc_lib/Makefile legacy/pvc_src/Makefile

# Fix library linking order: libpvoc depends on libsndfile and libm
# On Linux, dependent libraries must come after the library that uses them
RUN sed -i 's/LDFLAGS.*=.*/LDFLAGS = -lpvoc -lsndfile -lm/' legacy/pvc_src/Makefile

# Add -fcommon flag to handle legacy C code with duplicate global definitions
# (required for GCC 10+ which defaults to -fno-common)
RUN sed -i 's/^CFLAGS =/CFLAGS = -fcommon/' legacy/cmusic_gen/lib/libran/Makefile

# Fix macOS-specific ranlib -s flag (Linux ranlib doesn't need it)
RUN sed -i 's/ranlib -s/ranlib/' legacy/pvc_lib/Makefile

# Build cmusic_gen libraries and generators
RUN cd legacy/cmusic_gen && make

# Build the PVC library
RUN cd legacy/pvc_lib && make clean && make

# Build all PVC tools (explicitly run 'make all' as lib: is the first target)
RUN cd legacy/pvc_src && make clean && make all
RUN cd legacy/pvc_src && make install

# Add bin directory to PATH (pvc_src Makefile installs to ../bin => legacy/bin)
ENV PATH="/src/PVCplus/legacy/bin:${PATH}"

# Create directories for audio input and output
RUN mkdir -p /audio/input /audio/output

# Declare volumes so hosts can mount local folders
VOLUME ["/audio/input", "/audio/output"]

# Set working directory to the parent for convenience
WORKDIR /audio

# Default command shows available tools
CMD ["sh", "-c", "echo 'PVCplus tools available:' && ls /src/PVCplus/legacy/bin && echo 'Input dir: /audio/input | Output dir: /audio/output'"]
