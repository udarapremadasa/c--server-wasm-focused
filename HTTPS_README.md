# HTTPS Server Setup

This document explains how to run the C++ HTTP server with HTTPS support using self-signed certificates.

## Quick Start

1. **Generate SSL certificates and start HTTPS server:**
   ```bash
   ./start-https.sh
   ```

2. **Visit the server:**
   - Open your browser and go to: https://localhost:8443
   - You'll see a security warning due to the self-signed certificate
   - Click "Advanced" → "Proceed to localhost (unsafe)" to continue

## Manual Setup

### 1. Generate Self-Signed Certificates

```bash
./generate-certs.sh
```

This creates:
- `./certs/server.crt` - SSL certificate
- `./certs/server.key` - Private key

### 2. Build with SSL Support

```bash
mkdir -p build
cd build
cmake .. -DENABLE_SSL=ON
make -j$(nproc)
cd ..
```

### 3. Start HTTPS Server

```bash
# Default HTTPS port (8443)
./build/httpserver --https

# Custom port
./build/httpserver --https --port 443

# Custom certificate files
./build/httpserver --https --cert mycert.crt --key mykey.key
```

## Command Line Options

```
Usage: ./build/httpserver [options]
Options:
  --port <port>    Port to listen on (default: 8080 for HTTP, 8443 for HTTPS)
  --host <host>    Host to bind to (default: 0.0.0.0)
  --https          Enable HTTPS mode with SSL/TLS
  --cert <file>    SSL certificate file (default: ./certs/server.crt)
  --key <file>     SSL private key file (default: ./certs/server.key)
  --help           Show this help message

Examples:
  ./build/httpserver                    # Start HTTP server on port 8080
  ./build/httpserver --https             # Start HTTPS server on port 8443
  ./build/httpserver --https --port 443  # Start HTTPS server on port 443
```

## Testing HTTPS

### Using curl
```bash
# Test API endpoint (ignore SSL warnings with -k)
curl -k https://localhost:8443/api/status

# Test home page
curl -k https://localhost:8443/

# Test with parameters
curl -k https://localhost:8443/api/hello?name=HTTPS

# Test POST request
curl -k -X POST -H "Content-Type: application/json" \
     -d '{"test": "data"}' \
     https://localhost:8443/api/echo
```

### Using a Browser
1. Navigate to https://localhost:8443
2. Accept the security warning for the self-signed certificate
3. You should see the server welcome page

## Available Endpoints

- `GET /` - Server welcome page
- `GET /api/status` - Server status information
- `GET /api/hello?name=<name>` - Hello message
- `POST /api/echo` - Echo request body
- `GET /health` - Health check

## SSL Certificate Details

The self-signed certificate includes:
- **Subject**: CN=localhost
- **Valid for**: localhost, 127.0.0.1, ::1
- **Validity**: 365 days
- **Key size**: 2048 bits RSA

## Security Notes

⚠️ **Important**: The generated certificate is self-signed and should only be used for development and testing purposes. For production use, obtain a certificate from a trusted Certificate Authority.

## Troubleshooting

### SSL Handshake Errors
If you see SSL handshake failed errors in the logs, this is normal when:
- Testing with non-SSL clients
- Browser security scanners probe the server
- Invalid SSL requests are made

### Browser Security Warnings
Modern browsers will show security warnings for self-signed certificates. This is expected behavior. Click "Advanced" and proceed to continue.

### Port Permission Issues
If running on port 443 (standard HTTPS port), you may need root privileges:
```bash
sudo ./build/httpserver --https --port 443
```

## Files Created

- `generate-certs.sh` - Script to generate SSL certificates
- `start-https.sh` - Convenient script to start HTTPS server
- `certs/server.crt` - SSL certificate (after running generate-certs.sh)
- `certs/server.key` - Private key (after running generate-certs.sh)
- `certs/server.conf` - OpenSSL configuration file
