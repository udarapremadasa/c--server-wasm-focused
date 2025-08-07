#include "http_server.h"
#include "logger.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <regex>

#ifndef BUILD_WASM
#include <unistd.h>
#include <sys/socket.h>
#endif

HttpServer::HttpServer() 
    : is_running_(false), port_(0), host_("0.0.0.0"),
      max_connections_(100), timeout_seconds_(30), thread_pool_size_(std::thread::hardware_concurrency()) {
    
#ifndef BUILD_WASM
    socket_server_ = std::make_unique<SocketServer>();
    thread_pool_ = std::make_unique<ThreadPool>(thread_pool_size_);
#ifdef ENABLE_SSL
    use_ssl_ = false;
#endif
#endif

    // Set default handlers
    not_found_handler_ = [this](const HttpRequest& req, HttpResponse& res) {
        defaultNotFoundHandler(req, res);
    };
    
    error_handler_ = [this](const std::exception& e, const HttpRequest& req, HttpResponse& res) {
        defaultErrorHandler(e, req, res);
    };
}

HttpServer::~HttpServer() {
    stop();
}

bool HttpServer::start(int port, const std::string& host) {
    if (is_running_) {
        return false;
    }
    
    port_ = port;
    host_ = host;

#ifndef BUILD_WASM
    if (!socket_server_->bind(port, host)) {
        LOG_ERROR("Failed to bind to " + host + ":" + std::to_string(port));
        return false;
    }
    
    if (!socket_server_->listen()) {
        LOG_ERROR("Failed to listen on socket");
        return false;
    }
    
    thread_pool_->start();
    is_running_ = true;
    
#ifdef ENABLE_SSL
    if (use_ssl_) {
        LOG_INFO("HTTPS server started on " + host + ":" + std::to_string(port));
    } else {
        LOG_INFO("HTTP server started on " + host + ":" + std::to_string(port));
    }
#else
    LOG_INFO("HTTP server started on " + host + ":" + std::to_string(port));
#endif
    
    // Accept connections
    socket_server_->accept([this](int client_socket) {
        thread_pool_->enqueue([this, client_socket]() {
            handleConnection(client_socket);
        });
    });
#else
    is_running_ = true;
    LOG_INFO("WebAssembly HTTP server initialized");
#endif
    
    return true;
}

#ifdef ENABLE_SSL
bool HttpServer::startHttps(int port, const std::string& cert_file, const std::string& key_file, const std::string& host) {
#ifndef BUILD_WASM
    if (is_running_) {
        return false;
    }
    
    ssl_server_ = std::make_unique<SslServer>();
    if (!ssl_server_->initialize(cert_file, key_file)) {
        LOG_ERROR("Failed to initialize SSL server");
        return false;
    }
    
    use_ssl_ = true;
    return start(port, host);
#else
    LOG_ERROR("SSL not supported in WebAssembly build");
    return false;
#endif
}

void HttpServer::setSslContext(const std::string& cert_file, const std::string& key_file) {
#ifndef BUILD_WASM
    if (!ssl_server_) {
        ssl_server_ = std::make_unique<SslServer>();
    }
    ssl_server_->initialize(cert_file, key_file);
#endif
}
#endif

void HttpServer::stop() {
    if (!is_running_) {
        return;
    }
    
    is_running_ = false;
    
#ifndef BUILD_WASM
    if (socket_server_) {
        socket_server_->stop();
    }
    
    if (thread_pool_) {
        thread_pool_->stop();
    }
#endif
    
    LOG_INFO("HTTP server stopped");
}

void HttpServer::get(const std::string& path, RequestHandler handler) {
    route(HttpRequest::Method::GET, path, handler);
}

void HttpServer::post(const std::string& path, RequestHandler handler) {
    route(HttpRequest::Method::POST, path, handler);
}

void HttpServer::put(const std::string& path, RequestHandler handler) {
    route(HttpRequest::Method::PUT, path, handler);
}

void HttpServer::del(const std::string& path, RequestHandler handler) {
    route(HttpRequest::Method::DELETE, path, handler);
}

void HttpServer::head(const std::string& path, RequestHandler handler) {
    route(HttpRequest::Method::HEAD, path, handler);
}

void HttpServer::options(const std::string& path, RequestHandler handler) {
    route(HttpRequest::Method::OPTIONS, path, handler);
}

void HttpServer::patch(const std::string& path, RequestHandler handler) {
    route(HttpRequest::Method::PATCH, path, handler);
}

void HttpServer::route(HttpRequest::Method method, const std::string& path, RequestHandler handler) {
    Route route;
    route.method = method;
    route.path = path;
    route.handler = handler;
    route.is_regex = path.find('*') != std::string::npos || path.find('(') != std::string::npos;
    
    routes_.push_back(route);
}

void HttpServer::use(MiddlewareFunction middleware) {
    middlewares_.push_back(middleware);
}

void HttpServer::serveStatic(const std::string& path, const std::string& directory) {
    static_paths_[path] = directory;
}

void HttpServer::setMaxConnections(int max_connections) {
    max_connections_ = max_connections;
}

void HttpServer::setTimeoutSeconds(int timeout_seconds) {
    timeout_seconds_ = timeout_seconds;
}

void HttpServer::setThreadPoolSize(int size) {
    thread_pool_size_ = size;
#ifndef BUILD_WASM
    if (thread_pool_) {
        thread_pool_->resize(size);
    }
#endif
}

void HttpServer::setNotFoundHandler(RequestHandler handler) {
    not_found_handler_ = handler;
}

void HttpServer::setErrorHandler(std::function<void(const std::exception&, const HttpRequest&, HttpResponse&)> handler) {
    error_handler_ = handler;
}

#ifdef BUILD_WASM
void HttpServer::handleRequest(const std::string& raw_request, std::string& response_output) {
    HttpRequest request(raw_request);
    HttpResponse response;
    
    processRequest(request, response);
    response_output = response.toString();
}

void HttpServer::processRequest(const HttpRequest& request, HttpResponse& response) {
    processHttpRequest(request, response);
}
#endif

#ifndef BUILD_WASM
void HttpServer::handleConnection(int client_socket) {
    try {
        // Set socket timeout
        struct timeval timeout;
        timeout.tv_sec = timeout_seconds_;
        timeout.tv_usec = 0;
        setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        
#ifdef ENABLE_SSL
        SSL* ssl_connection = nullptr;
        if (use_ssl_ && ssl_server_) {
            ssl_connection = ssl_server_->createSslConnection(client_socket);
            if (!ssl_connection || !ssl_server_->performHandshake(ssl_connection)) {
                LOG_ERROR("SSL handshake failed");
                if (ssl_connection) {
                    ssl_server_->closeSslConnection(ssl_connection);
                }
                close(client_socket);
                return;
            }
        }
#endif
        
        // Read request
        char buffer[8192];
        std::string raw_request;
        
        ssize_t bytes_read;
        while (true) {
#ifdef ENABLE_SSL
            if (use_ssl_ && ssl_connection) {
                bytes_read = ssl_server_->sslRead(ssl_connection, buffer, sizeof(buffer) - 1);
            } else {
                bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
            }
#else
            bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
#endif
            
            if (bytes_read <= 0) {
                break;
            }
            
            buffer[bytes_read] = '\0';
            raw_request += buffer;
            
            // Check if we have a complete HTTP request
            if (raw_request.find("\r\n\r\n") != std::string::npos) {
                break;
            }
        }
        
        if (raw_request.empty()) {
#ifdef ENABLE_SSL
            if (ssl_connection) {
                ssl_server_->closeSslConnection(ssl_connection);
            }
#endif
            close(client_socket);
            return;
        }
        
        // Parse and process request
        HttpRequest request(raw_request);
        HttpResponse response;
        
        processHttpRequest(request, response);
        
        // Send response
        std::string response_str = response.toString();
        
#ifdef ENABLE_SSL
        if (use_ssl_ && ssl_connection) {
            ssl_server_->sslWrite(ssl_connection, response_str.c_str(), response_str.length());
            ssl_server_->closeSslConnection(ssl_connection);
        } else {
            send(client_socket, response_str.c_str(), response_str.length(), 0);
        }
#else
        send(client_socket, response_str.c_str(), response_str.length(), 0);
#endif
        
    } catch (const std::exception& e) {
        LOG_ERROR("Error handling connection: " + std::string(e.what()));
    }
    
    close(client_socket);
}
#endif

void HttpServer::processHttpRequest(const HttpRequest& request, HttpResponse& response) {
    try {
        // Run middlewares
        if (!runMiddlewares(request, response)) {
            return;
        }
        
        // Check static files first
        for (const auto& static_path : static_paths_) {
            if (request.getPath().find(static_path.first) == 0) {
                std::string file_path = static_path.second + request.getPath().substr(static_path.first.length());
                handleStaticFile(file_path, response);
                return;
            }
        }
        
        // Find matching route
        bool route_found = false;
        for (const auto& route : routes_) {
            if (route.method == request.getMethod() || route.method == HttpRequest::Method::OPTIONS) {
                if (matchRoute(route.path, request.getPath())) {
                    route.handler(request, response);
                    route_found = true;
                    break;
                }
            }
        }
        
        if (!route_found) {
            not_found_handler_(request, response);
        }
        
    } catch (const std::exception& e) {
        error_handler_(e, request, response);
    }
}

bool HttpServer::runMiddlewares(const HttpRequest& request, HttpResponse& response) {
    for (const auto& middleware : middlewares_) {
        if (!middleware(request, response)) {
            return false;
        }
    }
    return true;
}

bool HttpServer::matchRoute(const std::string& pattern, const std::string& path) const {
    if (pattern == path) {
        return true;
    }
    
    // Simple wildcard matching
    if (pattern.find('*') != std::string::npos) {
        std::string regex_pattern = pattern;
        std::replace(regex_pattern.begin(), regex_pattern.end(), '*', '.');
        regex_pattern += "*";
        
        try {
            std::regex regex(regex_pattern);
            return std::regex_match(path, regex);
        } catch (const std::exception&) {
            return false;
        }
    }
    
    return false;
}

void HttpServer::handleStaticFile(const std::string& file_path, HttpResponse& response) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        response.setStatusCode(HttpResponse::StatusCode::NOT_FOUND);
        response.setTextContent("File not found");
        return;
    }
    
    response.setFileContent(file_path);
}

void HttpServer::defaultNotFoundHandler(const HttpRequest& request, HttpResponse& response) {
    response.setStatusCode(HttpResponse::StatusCode::NOT_FOUND);
    response.setTextContent("404 Not Found: " + request.getPath());
}

void HttpServer::defaultErrorHandler(const std::exception& e, const HttpRequest& request, HttpResponse& response) {
    LOG_ERROR("Error processing request " + request.getPath() + ": " + e.what());
    response.setStatusCode(HttpResponse::StatusCode::INTERNAL_SERVER_ERROR);
    response.setTextContent("Internal Server Error");
}
