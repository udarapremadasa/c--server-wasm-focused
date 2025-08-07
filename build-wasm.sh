#!/bin/bash

# WebAssembly build script for C++ HTTP Server

set -e

print_status() {
    echo -e "\033[0;32m[INFO]\033[0m $1"
}

print_error() {
    echo -e "\033[0;31m[ERROR]\033[0m $1"
}

# Check if Emscripten is available
if ! command -v emcmake &> /dev/null; then
    print_error "Emscripten not found!"
    echo "Please install Emscripten SDK from: https://emscripten.org/docs/getting_started/downloads.html"
    echo ""
    echo "Installation steps:"
    echo "1. git clone https://github.com/emscripten-core/emsdk.git"
    echo "2. cd emsdk"
    echo "3. ./emsdk install latest"
    echo "4. ./emsdk activate latest"
    echo "5. source ./emsdk_env.sh"
    exit 1
fi

print_status "Building for WebAssembly..."

# Create WebAssembly build directory
mkdir -p build-wasm
cd build-wasm

# Configure for WebAssembly
print_status "Configuring with Emscripten..."
emcmake cmake -DBUILD_WASM=ON -DENABLE_SSL=OFF -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release ..

# Build
print_status "Building WebAssembly module..."
emmake make -j$(nproc)

print_status "WebAssembly build completed!"
print_status "Generated files:"
ls -la *.wasm *.js 2>/dev/null || print_error "No WebAssembly files found"

echo ""
echo "To use in Node.js:"
echo "1. Copy httpserver.js and httpserver.wasm to your Node.js project"
echo "2. Use the example in examples/nodejs/"
