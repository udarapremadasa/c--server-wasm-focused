#!/bin/bash

# Script to start the HTTPS server with self-signed certificates

set -e

echo "🔒 Starting HTTPS server setup..."

# Check if certificates exist
if [ ! -f "./certs/server.crt" ] || [ ! -f "./certs/server.key" ]; then
    echo "📜 SSL certificates not found. Generating self-signed certificates..."
    ./generate-certs.sh
    echo ""
fi

# Check if server binary exists
if [ ! -f "./build/httpserver" ]; then
    echo "🔨 Server binary not found. Building project..."
    if [ ! -d "./build" ]; then
        mkdir build
    fi
    
    cd build
    cmake .. -DENABLE_SSL=ON
    make -j$(nproc)
    cd ..
    echo "✅ Build completed!"
    echo ""
fi

# Parse command line arguments
PORT=8443
HOST="0.0.0.0"
EXTRA_ARGS=""

while [[ $# -gt 0 ]]; do
    case $1 in
        --port)
            PORT="$2"
            shift 2
            ;;
        --host)
            HOST="$2"
            shift 2
            ;;
        *)
            EXTRA_ARGS="$EXTRA_ARGS $1"
            shift
            ;;
    esac
done

echo "🚀 Starting HTTPS server..."
echo "📍 Host: $HOST"
echo "🔌 Port: $PORT"
echo "📜 Certificate: ./certs/server.crt"
echo "🔑 Private key: ./certs/server.key"
echo ""
echo "⚠️  Your browser will show a security warning for the self-signed certificate."
echo "    Click 'Advanced' → 'Proceed to localhost (unsafe)' to continue."
echo ""
echo "🌐 Server URL: https://$([ "$HOST" = "0.0.0.0" ] && echo "localhost" || echo "$HOST"):$PORT"
echo ""
echo "Press Ctrl+C to stop the server"
echo "----------------------------------------"

# Start the server
exec ./build/httpserver --https --port "$PORT" --host "$HOST" $EXTRA_ARGS
