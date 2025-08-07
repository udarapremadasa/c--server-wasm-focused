# C++ HTTP Server with WebAssembly Support

A modern C++ HTTP/HTTPS server implementation with WebAssembly compilation support, built using Test-Driven Development (TDD) methodology.

## Features

- **HTTP/HTTPS Server**: Full-featured HTTP server with optional SSL/TLS support
- **WebAssembly Compatible**: Can be compiled to WebAssembly for use in Node.js applications
- **Thread Pool**: Efficient multi-threaded request handling
- **Route Management**: Express.js-like routing system with middleware support
- **Static File Serving**: Built-in static file server
- **Cross-Platform**: Works on Linux, macOS, and Windows
- **Test-Driven Development**: Comprehensive test suite using Google Test

## Requirements

### Native Build
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.16+
- OpenSSL (optional, for HTTPS support)

### WebAssembly Build
- Emscripten SDK
- CMake 3.16+

## Building

### Native Build

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the project
make -j$(nproc)

# Run tests
make test
```

### With SSL Support

```bash
# Configure with SSL enabled (default)
cmake -DENABLE_SSL=ON ..
make -j$(nproc)
```

### WebAssembly Build

```bash
# Source Emscripten environment
source /path/to/emsdk/emsdk_env.sh

# Create WebAssembly build directory
mkdir build-wasm && cd build-wasm

# Configure for WebAssembly
emcmake cmake -DBUILD_WASM=ON -DENABLE_SSL=OFF ..

# Build
emmake make -j$(nproc)
```

## Usage

### Standalone Server

```cpp
#include "http_server.h"

int main() {
    HttpServer server;
    
    // Add routes
    server.get("/", [](const HttpRequest& req, HttpResponse& res) {
        res.setHtmlContent("<h1>Hello, World!</h1>");
    });
    
    server.post("/api/data", [](const HttpRequest& req, HttpResponse& res) {
        // Process JSON data
        res.setJsonContent("{\"status\":\"received\"}");
    });
    
    // Add middleware
    server.use([](const HttpRequest& req, HttpResponse& res) -> bool {
        res.enableCors();
        return true; // Continue processing
    });
    
    // Serve static files
    server.serveStatic("/static", "./public");
    
    // Start server
    server.start(8080, "0.0.0.0");
    
    return 0;
}
```

### HTTPS Server

```cpp
#include "http_server.h"

int main() {
    HttpServer server;
    
    // Configure routes...
    
    // Start HTTPS server
    server.startHttps(8443, "cert.pem", "key.pem", "0.0.0.0");
    
    return 0;
}
```

### WebAssembly Integration

```javascript
// Node.js integration
const fs = require('fs');
const HttpServerModule = require('./httpserver.js');

HttpServerModule().then(Module => {
    // Start the server
    Module._start_server();
    
    // Handle requests
    const request = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    const response = Module.ccall('handle_request', 'string', ['string'], [request]);
    
    console.log(response);
    
    // Stop the server
    Module._stop_server();
});
```

## API Reference

### HttpServer Class

#### Routes
- `get(path, handler)` - Add GET route
- `post(path, handler)` - Add POST route
- `put(path, handler)` - Add PUT route
- `del(path, handler)` - Add DELETE route
- `head(path, handler)` - Add HEAD route
- `options(path, handler)` - Add OPTIONS route
- `patch(path, handler)` - Add PATCH route

#### Middleware
- `use(middleware)` - Add middleware function

#### Static Files
- `serveStatic(path, directory)` - Serve static files from directory

#### Server Control
- `start(port, host)` - Start HTTP server
- `startHttps(port, cert_file, key_file, host)` - Start HTTPS server
- `stop()` - Stop server
- `isRunning()` - Check if server is running

#### Configuration
- `setMaxConnections(count)` - Set maximum concurrent connections
- `setTimeoutSeconds(seconds)` - Set request timeout
- `setThreadPoolSize(size)` - Set thread pool size

### HttpRequest Class

#### Properties
- `getMethod()` - Get HTTP method
- `getPath()` - Get request path
- `getVersion()` - Get HTTP version
- `getBody()` - Get request body

#### Headers
- `getHeader(name)` - Get header value
- `getHeaders()` - Get all headers

#### Query Parameters
- `getQueryParam(name)` - Get query parameter
- `getQueryParams()` - Get all query parameters

### HttpResponse Class

#### Status
- `setStatusCode(code)` - Set HTTP status code
- `getStatusCode()` - Get current status code

#### Headers
- `setHeader(name, value)` - Set header
- `getHeader(name)` - Get header value

#### Content
- `setBody(content)` - Set response body
- `setJsonContent(json)` - Set JSON content with proper headers
- `setHtmlContent(html)` - Set HTML content with proper headers
- `setTextContent(text)` - Set plain text content
- `setFileContent(file_path)` - Serve file content

#### CORS
- `enableCors(origin)` - Enable CORS headers

## Testing

The project uses Google Test for unit testing. Tests are automatically built when `BUILD_TESTS` is enabled (default).

```bash
# Run all tests
cd build
make test

# Run specific test
./httpserver_tests --gtest_filter="HttpRequestTest.*"

# Run with verbose output
./httpserver_tests --gtest_filter="*" --gtest_verbose
```

## Project Structure

```
c-server/
├── CMakeLists.txt          # Main CMake configuration
├── include/                # Header files
│   ├── http_server.h
│   ├── http_request.h
│   ├── http_response.h
│   ├── socket_server.h
│   ├── thread_pool.h
│   ├── ssl_server.h
│   └── logger.h
├── src/                    # Source files
│   ├── main.cpp
│   ├── http_server.cpp
│   ├── http_request.cpp
│   ├── http_response.cpp
│   ├── socket_server.cpp
│   ├── thread_pool.cpp
│   ├── ssl_server.cpp
│   └── logger.cpp
├── tests/                  # Test files
│   ├── test_http_server.cpp
│   ├── test_http_request.cpp
│   ├── test_http_response.cpp
│   ├── test_socket_server.cpp
│   └── test_thread_pool.cpp
└── README.md
```

## WebAssembly Build Options

When building for WebAssembly, the following CMake options are available:

- `BUILD_WASM=ON` - Enable WebAssembly build mode
- `ENABLE_SSL=OFF` - Disable SSL (not supported in WebAssembly)
- `BUILD_TESTS=OFF` - Disable tests for WebAssembly build

## Contributing

1. Fork the repository
2. Create a feature branch
3. Write tests for new functionality
4. Implement the feature
5. Ensure all tests pass
6. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Future Enhancements

- [ ] HTTP/2 support
- [ ] Compression support (gzip, deflate)
- [ ] Request/response streaming
- [ ] WebSocket support
- [ ] Authentication middleware
- [ ] Rate limiting
- [ ] Caching mechanisms
- [ ] Database connection pooling
- [ ] Metrics and monitoring
- [ ] Docker containerization
