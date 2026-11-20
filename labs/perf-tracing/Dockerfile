# ptg-lab: the performance & tracing lab toolchain. Build: docker build -t ptg-lab - < Dockerfile
# Debian 12 because its perf (linux-perf 6.1) matches a 6.1 kernel; the base is pinned by digest.
FROM debian:12@sha256:268fb24c39bca92d326dd83ea3abebe4a3f7751c804ffef23bb6d857dd343ff8

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      build-essential systemtap-sdt-dev linux-perf bpftrace python3 libc6-dbg \
      procps util-linux binutils \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /lab
