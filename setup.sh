#!/bin/bash

# Setup script for C++ HTTP Server development environment

set -e

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${GREEN}[✓]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[!]${NC} $1"
}

print_error() {
    echo -e "${RED}[✗]${NC} $1"
}

print_header() {
    echo "=================================="
    echo "  C++ HTTP Server Setup"
    echo "=================================="
    echo ""
}

# Check if running on supported OS
check_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        print_status "Detected Linux environment"
        OS="linux"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        print_status "Detected macOS environment"
        OS="macos"
    else
        print_error "Unsupported operating system: $OSTYPE"
        exit 1
    fi
}

# Install dependencies based on OS
install_dependencies() {
    print_status "Installing dependencies..."
    
    if [[ "$OS" == "linux" ]]; then
        # Update package list
        sudo apt update
        
        # Install build tools and libraries
        sudo apt install -y \
            build-essential \
            cmake \
            libssl-dev \
            libgtest-dev \
            git \
            curl \
            wget
            
    elif [[ "$OS" == "macos" ]]; then
        # Check if Homebrew is installed
        if ! command -v brew &> /dev/null; then
            print_error "Homebrew not found. Please install it first:"
            echo "  /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
            exit 1
        fi
        
        # Install dependencies with Homebrew
        brew install cmake openssl googletest
    fi
    
    print_status "Dependencies installed successfully"
}

# Test the build
test_build() {
    print_status "Testing native build..."
    
    # Build in release mode
    if ./build.sh --release --clean; then
        print_status "Native build successful"
    else
        print_error "Native build failed"
        exit 1
    fi
    
    # Test the executable
    if [[ -f "build/httpserver" ]]; then
        print_status "HTTP server executable created successfully"
        
        # Quick test of help option
        if ./build/httpserver --help > /dev/null; then
            print_status "HTTP server executable works correctly"
        else
            print_warning "HTTP server executable may have issues"
        fi
    else
        print_error "HTTP server executable not found"
        exit 1
    fi
}

# Setup Node.js environment (optional)
setup_nodejs() {
    print_status "Setting up Node.js environment..."
    
    # Check if Node.js is installed
    if ! command -v node &> /dev/null; then
        print_warning "Node.js not found. Please install Node.js to run WebAssembly examples."
        print_warning "Download from: https://nodejs.org/"
        return
    fi
    
    # Check if npm is installed
    if ! command -v npm &> /dev/null; then
        print_warning "npm not found. Please install npm to manage Node.js packages."
        return
    fi
    
    # Install Node.js dependencies for examples
    cd examples/nodejs
    if npm install; then
        print_status "Node.js dependencies installed"
    else
        print_warning "Failed to install Node.js dependencies"
    fi
    cd ../..
}

# Setup Emscripten (optional)
setup_emscripten() {
    print_status "Checking Emscripten setup..."
    
    if command -v emcc &> /dev/null; then
        print_status "Emscripten is already installed"
        return
    fi
    
    print_warning "Emscripten not found. WebAssembly compilation will not be available."
    echo ""
    echo "To install Emscripten:"
    echo "1. git clone https://github.com/emscripten-core/emsdk.git"
    echo "2. cd emsdk"
    echo "3. ./emsdk install latest"
    echo "4. ./emsdk activate latest"
    echo "5. source ./emsdk_env.sh"
    echo ""
}

# Main setup function
main() {
    print_header
    
    check_os
    install_dependencies
    test_build
    setup_nodejs
    setup_emscripten
    
    echo ""
    print_status "Setup completed successfully!"
    echo ""
    echo "Next steps:"
    echo "1. Build and test: ./build.sh"
    echo "2. Run server: ./build/httpserver"
    echo "3. Run tests: ./build.sh && cd build && make test"
    echo "4. For WebAssembly: ./build-wasm.sh (requires Emscripten)"
    echo "5. Try Node.js example: cd examples/nodejs && npm start"
    echo ""
}

# Run if script is executed directly
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi
