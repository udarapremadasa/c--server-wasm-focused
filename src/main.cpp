#include "http_server.h"
#include "logger.h"
#include <iostream>
#include <csignal>
#include <memory>

#ifdef BUILD_WASM
#include <emscripten.h>

// Global server instance for WebAssembly
std::unique_ptr<HttpServer> g_server;

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void start_server() {
        if (!g_server) {
            g_server = std::make_unique<HttpServer>();
            
            // Add some default routes for demo
            g_server->get("/", [](const HttpRequest& req, HttpResponse& res) {
                res.setHtmlContent("<h1>Hello from WebAssembly HTTP Server!</h1>");
            });
            
            g_server->get("/api/status", [](const HttpRequest& req, HttpResponse& res) {
                res.setJsonContent("{\"status\":\"running\",\"platform\":\"webassembly\"}");
            });
        }
        
        g_server->start(8080, "0.0.0.0");
    }
    
    EMSCRIPTEN_KEEPALIVE
    void stop_server() {
        if (g_server) {
            g_server->stop();
        }
    }
    
    EMSCRIPTEN_KEEPALIVE
    const char* handle_request(const char* raw_request) {
        if (!g_server) {
            return "Server not initialized";
        }
        
        static std::string response_buffer;
        g_server->handleRequest(std::string(raw_request), response_buffer);
        return response_buffer.c_str();
    }
}

int main() {
    start_server();
    return 0;
}

#else

// Global server instance for signal handling
std::unique_ptr<HttpServer> g_server;

void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\nShutting down server..." << std::endl;
        if (g_server) {
            g_server->stop();
        }
        exit(0);
    }
}

int main(int argc, char* argv[]) {
    // Set up signal handling
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Configure logger
    Logger::getInstance().setLevel(Logger::Level::INFO);
    Logger::getInstance().enableConsoleOutput(true);
    
    // Create server
    g_server = std::make_unique<HttpServer>();
    
    // Configure server
    g_server->setThreadPoolSize(4);
    g_server->setMaxConnections(100);
    g_server->setTimeoutSeconds(30);
    
    // Add CORS middleware
    g_server->use([](const HttpRequest& req, HttpResponse& res) -> bool {
        res.enableCors();
        return true;
    });
    
    // Add logging middleware
    g_server->use([](const HttpRequest& req, HttpResponse& res) -> bool {
        LOG_INFO(req.methodToString() + " " + req.getPath());
        return true;
    });
    
    // Define routes
    g_server->get("/", [](const HttpRequest& req, HttpResponse& res) {
        res.setHtmlContent(R"(
            <!DOCTYPE html>
            <html>
            <head>
                <title>C++ HTTP Server</title>
                <style>
                    body { font-family: Arial, sans-serif; margin: 40px; }
                    .container { max-width: 800px; margin: 0 auto; }
                    .endpoint { background: #f5f5f5; padding: 10px; margin: 10px 0; border-radius: 5px; }
                </style>
            </head>
            <body>
                <div class="container">
                    <h1>C++ HTTP Server</h1>
                    <p>Welcome to the C++ HTTP server with WebAssembly support!</p>
                    
                    <h2>Available Endpoints:</h2>
                    <div class="endpoint"><strong>GET /</strong> - This page</div>
                    <div class="endpoint"><strong>GET /api/status</strong> - Server status</div>
                    <div class="endpoint"><strong>GET /api/hello</strong> - Hello message</div>
                    <div class="endpoint"><strong>POST /api/echo</strong> - Echo request body</div>
                    <div class="endpoint"><strong>GET /health</strong> - Health check</div>
                </div>
            </body>
            </html>
        )");
    });
    
    g_server->get("/api/status", [](const HttpRequest& req, HttpResponse& res) {
        res.setJsonContent(R"({
            "status": "running",
            "platform": "native",
            "version": "1.0.0",
            "timestamp": ")" + std::to_string(std::time(nullptr)) + R"("
        })");
    });
    
    g_server->get("/api/hello", [](const HttpRequest& req, HttpResponse& res) {
        std::string name = req.getQueryParam("name");
        if (name.empty()) {
            name = "World";
        }
        
        res.setJsonContent(R"({"message": "Hello, )" + name + R"(!"})");
    });
    
    g_server->post("/api/echo", [](const HttpRequest& req, HttpResponse& res) {
        res.setJsonContent(R"({"echo": ")" + req.getBody() + R"("})");
    });
    
    g_server->get("/health", [](const HttpRequest& req, HttpResponse& res) {
        res.setTextContent("OK");
    });
    
    // Serve static files from public directory
    g_server->serveStatic("/static", "./public");
    
    // Custom 404 handler
    g_server->setNotFoundHandler([](const HttpRequest& req, HttpResponse& res) {
        res.setStatusCode(HttpResponse::StatusCode::NOT_FOUND);
        res.setJsonContent(R"({"error": "Not Found", "path": ")" + req.getPath() + R"("})");
    });
    
    // Parse command line arguments
    int port = 8080;
    std::string host = "0.0.0.0";
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--host" && i + 1 < argc) {
            host = argv[++i];
        } else if (arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]\n";
            std::cout << "Options:\n";
            std::cout << "  --port <port>    Port to listen on (default: 8080)\n";
            std::cout << "  --host <host>    Host to bind to (default: 0.0.0.0)\n";
            std::cout << "  --help           Show this help message\n";
            return 0;
        }
    }
    
    // Start server
    if (!g_server->start(port, host)) {
        LOG_ERROR("Failed to start server");
        return 1;
    }
    
    return 0;
}

#endif
