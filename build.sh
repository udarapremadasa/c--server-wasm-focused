#!/bin/bash

# Build script for C++ HTTP Server

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Default values
BUILD_TYPE="Release"
BUILD_TESTS="ON"
ENABLE_SSL="ON"
BUILD_WASM="OFF"
CLEAN_BUILD="OFF"
NUM_CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        --wasm)
            BUILD_WASM="ON"
            ENABLE_SSL="OFF"  # SSL not supported in WebAssembly
            print_warning "WebAssembly build: SSL disabled"
            shift
            ;;
        --no-ssl)
            ENABLE_SSL="OFF"
            shift
            ;;
        --no-tests)
            BUILD_TESTS="OFF"
            shift
            ;;
        --clean)
            CLEAN_BUILD="ON"
            shift
            ;;
        --jobs)
            NUM_CORES="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  --debug       Build in debug mode"
            echo "  --release     Build in release mode (default)"
            echo "  --wasm        Build for WebAssembly"
            echo "  --no-ssl      Disable SSL support"
            echo "  --no-tests    Disable building tests"
            echo "  --clean       Clean build directory before building"
            echo "  --jobs N      Use N parallel jobs (default: $NUM_CORES)"
            echo "  --help        Show this help message"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Determine build directory
if [[ "$BUILD_WASM" == "ON" ]]; then
    BUILD_DIR="build-wasm"
    print_status "Building for WebAssembly"
else
    BUILD_DIR="build"
    print_status "Building for native platform"
fi

# Clean build directory if requested
if [[ "$CLEAN_BUILD" == "ON" ]]; then
    print_status "Cleaning build directory: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

print_status "Build configuration:"
echo "  Build Type: $BUILD_TYPE"
echo "  SSL Support: $ENABLE_SSL"
echo "  Tests: $BUILD_TESTS"
echo "  WebAssembly: $BUILD_WASM"
echo "  Parallel Jobs: $NUM_CORES"

# Configure with CMake
print_status "Configuring with CMake..."

CMAKE_ARGS=(
    "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
    "-DBUILD_TESTS=$BUILD_TESTS"
    "-DENABLE_SSL=$ENABLE_SSL"
    "-DBUILD_WASM=$BUILD_WASM"
)

if [[ "$BUILD_WASM" == "ON" ]]; then
    # Check if Emscripten is available
    if ! command -v emcmake &> /dev/null; then
        print_error "Emscripten not found. Please install and source the Emscripten SDK."
        print_error "Download from: https://emscripten.org/docs/getting_started/downloads.html"
        exit 1
    fi
    
    emcmake cmake "${CMAKE_ARGS[@]}" ..
else
    cmake "${CMAKE_ARGS[@]}" ..
fi

# Build the project
print_status "Building project..."

if [[ "$BUILD_WASM" == "ON" ]]; then
    emmake make -j"$NUM_CORES"
else
    make -j"$NUM_CORES"
fi

# Run tests if enabled and not building for WebAssembly
if [[ "$BUILD_TESTS" == "ON" && "$BUILD_WASM" == "OFF" ]]; then
    print_status "Running tests..."
    if make test; then
        print_status "All tests passed!"
    else
        print_error "Some tests failed!"
        exit 1
    fi
fi

# Print build results
print_status "Build completed successfully!"

if [[ "$BUILD_WASM" == "ON" ]]; then
    print_status "WebAssembly files generated:"
    ls -la *.wasm *.js 2>/dev/null || print_warning "No WebAssembly files found"
else
    print_status "Executable created:"
    ls -la httpserver 2>/dev/null || print_warning "No executable found"
fi

print_status "Build artifacts are in: $(pwd)"
