#!/bin/bash

DEFAULT_SRC_DIR="../src"
DEFAULT_BUILD_DIR="build"

SRC_DIR="${1:-$DEFAULT_SRC_DIR}"
BUILD_DIR="${2:-$DEFAULT_BUILD_DIR}"

if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory: $BUILD_DIR"
    mkdir -p "$BUILD_DIR"
fi

cd "$BUILD_DIR" || exit 1

echo "Running cmake for source directory: $SRC_DIR"
cmake "$SRC_DIR"

if [ $? -eq 0 ]; then
    echo "CMake has succeeded. Running make..."
    make 
else
    echo "CMake failed. Exiting."
    exit 1
fi

