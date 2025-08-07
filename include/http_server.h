#pragma once

#include <functional>
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <atomic>

#include "http_request.h"
#include "http_response.h"

#ifndef BUILD_WASM
#include "socket_server.h"
#include "thread_pool.h"
#ifdef ENABLE_SSL
#include "ssl_server.h"
#endif
#endif

class HttpServer {
public:
    using RequestHandler = std::function<void(const HttpRequest&, HttpResponse&)>;
    using MiddlewareFunction = std::function<bool(const HttpRequest&, HttpResponse&)>;

    HttpServer();
    ~HttpServer();

    // Server lifecycle
    bool start(int port, const std::string& host = "0.0.0.0");
    void stop();
    bool isRunning() const { return is_running_; }

#ifdef ENABLE_SSL
    // SSL/TLS support
    bool startHttps(int port, const std::string& cert_file, const std::string& key_file, const std::string& host = "0.0.0.0");
    void setSslContext(const std::string& cert_file, const std::string& key_file);
#endif

    // Route management
    void get(const std::string& path, RequestHandler handler);
    void post(const std::string& path, RequestHandler handler);
    void put(const std::string& path, RequestHandler handler);
    void del(const std::string& path, RequestHandler handler); // 'delete' is a C++ keyword
    void head(const std::string& path, RequestHandler handler);
    void options(const std::string& path, RequestHandler handler);
    void patch(const std::string& path, RequestHandler handler);
    
    // Generic route handler
    void route(HttpRequest::Method method, const std::string& path, RequestHandler handler);
    
    // Middleware
    void use(MiddlewareFunction middleware);
    
    // Static file serving
    void serveStatic(const std::string& path, const std::string& directory);
    
    // Configuration
    void setMaxConnections(int max_connections);
    void setTimeoutSeconds(int timeout_seconds);
    void setThreadPoolSize(int size);
    
    // Error handlers
    void setNotFoundHandler(RequestHandler handler);
    void setErrorHandler(std::function<void(const std::exception&, const HttpRequest&, HttpResponse&)> handler);

#ifdef BUILD_WASM
    // WebAssembly specific methods
    void handleRequest(const std::string& raw_request, std::string& response_output);
    void processRequest(const HttpRequest& request, HttpResponse& response);
#endif

private:
    struct Route {
        HttpRequest::Method method;
        std::string path;
        RequestHandler handler;
        bool is_regex;
    };

    std::vector<Route> routes_;
    std::vector<MiddlewareFunction> middlewares_;
    std::unordered_map<std::string, std::string> static_paths_;
    
    RequestHandler not_found_handler_;
    std::function<void(const std::exception&, const HttpRequest&, HttpResponse&)> error_handler_;
    
    std::atomic<bool> is_running_;
    int port_;
    std::string host_;
    
#ifndef BUILD_WASM
    std::unique_ptr<SocketServer> socket_server_;
    std::unique_ptr<ThreadPool> thread_pool_;
#ifdef ENABLE_SSL
    std::unique_ptr<SslServer> ssl_server_;
    bool use_ssl_;
#endif
#endif

    int max_connections_;
    int timeout_seconds_;
    int thread_pool_size_;

    // Request processing
    void handleConnection(int client_socket);
    void processHttpRequest(const HttpRequest& request, HttpResponse& response);
    bool runMiddlewares(const HttpRequest& request, HttpResponse& response);
    bool matchRoute(const std::string& pattern, const std::string& path) const;
    void handleStaticFile(const std::string& file_path, HttpResponse& response);
    
    // Default handlers
    void defaultNotFoundHandler(const HttpRequest& request, HttpResponse& response);
    void defaultErrorHandler(const std::exception& e, const HttpRequest& request, HttpResponse& response);
};
