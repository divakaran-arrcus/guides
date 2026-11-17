# mdg-lab: the memory-debugging lab toolchain. Build: docker build -t mdg-lab - < Dockerfile
# The base is pinned by digest so every capture comes from the same userland.
FROM ubuntu:24.04@sha256:73b999393d07f2bf65a40373778a44dc9a6d54ad7718d29247d8ff9413c4d6a8

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      build-essential clang-18 libclang-rt-18-dev gdb valgrind rr heaptrack \
      libc6-dbg binutils file procps \
 && rm -rf /var/lib/apt/lists/* \
 && update-alternatives --install /usr/bin/clang clang /usr/bin/clang-18 100

WORKDIR /lab
