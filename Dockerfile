FROM ubuntu:24.04 AS base

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    gcc \
    g++ \
    clang \
    clang-tidy \
    cmake \
    ninja-build \
    cppcheck \
    git \
    ca-certificates \
    libgl1-mesa-dev \
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    libxext-dev \
    libwayland-dev \
    libxkbcommon-dev \
    libasound2-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /project
COPY . .

# GCC Debug build
FROM base AS build-gcc-debug
RUN cmake -B build/gcc-debug -G Ninja \
    -DCMAKE_C_COMPILER=gcc \
    -DCMAKE_BUILD_TYPE=Debug \
    && cmake --build build/gcc-debug

# GCC Release build
FROM base AS build-gcc-release
RUN cmake -B build/gcc-release -G Ninja \
    -DCMAKE_C_COMPILER=gcc \
    -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build/gcc-release

# Clang Debug build
FROM base AS build-clang-debug
RUN cmake -B build/clang-debug -G Ninja \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_BUILD_TYPE=Debug \
    && cmake --build build/clang-debug

# Default target: GCC Release
FROM build-gcc-release
