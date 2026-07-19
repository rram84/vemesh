# VEMesh CI image
#
# Rebuild and push when Boost/PMP versions change:
#   docker buildx build --platform linux/amd64 -t rram84/vemesh-ci:latest -f .ci/Dockerfile --push .
#
# Contains: Ubuntu 24.04, GCC 13, CMake, Boost 1.87, Eigen3, PMP 3.0.

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV BOOST_VERSION=1.87.0
ENV PMP_VERSION=3.0.0

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        g++ \
        git \
        ca-certificates \
        wget \
        cmake \
        libeigen3-dev \
    && rm -rf /var/lib/apt/lists/*

# Boost (built from source; not available in apt at this version)
RUN set -eux; \
    BOOST_UNDERSCORE=$(echo ${BOOST_VERSION} | tr . _); \
    cd /tmp; \
    wget -q https://archives.boost.io/release/${BOOST_VERSION}/source/boost_${BOOST_UNDERSCORE}.tar.gz; \
    tar xzf boost_${BOOST_UNDERSCORE}.tar.gz; \
    cd boost_${BOOST_UNDERSCORE}; \
    ./bootstrap.sh --prefix=/opt/boost; \
    ./b2 install --prefix=/opt/boost -j"$(nproc)" link=static,shared variant=release; \
    cd /; \
    rm -rf /tmp/boost_*

# PMP — no tests, no examples, no viewers (no GLFW/OpenGL pulled in)
#
# The 3.0.0 git tag still carries a stale `project(... VERSION 2.0.1)` and PMP
# reads its installed package version from an (unset) PMP_VERSION variable, so a
# plain build reports itself as 2.0.1 in pmpConfigVersion.cmake. Pass the version
# explicitly so the installed package identifies as ${PMP_VERSION}.
RUN set -eux; \
    git clone --branch ${PMP_VERSION} --depth 1 https://github.com/pmp-library/pmp-library.git /tmp/pmp; \
    cmake -S /tmp/pmp -B /tmp/pmp/build \
        -DCMAKE_BUILD_TYPE=Release \
        -DPMP_VERSION=${PMP_VERSION} \
        -DCMAKE_INSTALL_PREFIX=/opt/pmp \
        -DPMP_BUILD_EXAMPLES=OFF \
        -DPMP_BUILD_TESTS=OFF \
        -DPMP_BUILD_DOCS=OFF \
        -DPMP_BUILD_VIS=OFF \
        -DPMP_STRICT_COMPILATION=OFF; \
    cmake --build /tmp/pmp/build -j"$(nproc)"; \
    cmake --install /tmp/pmp/build; \
    rm -rf /tmp/pmp

ENV CMAKE_PREFIX_PATH=/opt/pmp:/opt/boost