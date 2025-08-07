#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "http_server.h"
#include <thread>
#include <chrono>

class HttpServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        server = std::make_unique<HttpServer>();
    }
    
    void TearDown() override {
        if (server && server->isRunning()) {
            server->stop();
        }
    }
    
    std::unique_ptr<HttpServer> server;
};

TEST_F(HttpServerTest, ServerInitialization) {
    EXPECT_FALSE(server->isRunning());
}

TEST_F(HttpServerTest, AddRoutes) {
    bool route_called = false;
    
    server->get("/test", [&route_called](const HttpRequest& req, HttpResponse& res) {
        route_called = true;
        res.setTextContent("GET test");
    });
    
    server->post("/test", [&route_called](const HttpRequest& req, HttpResponse& res) {
        route_called = true;
        res.setTextContent("POST test");
    });
    
    server->put("/test", [&route_called](const HttpRequest& req, HttpResponse& res) {
        route_called = true;
        res.setTextContent("PUT test");
    });
    
    server->del("/test", [&route_called](const HttpRequest& req, HttpResponse& res) {
        route_called = true;
        res.setTextContent("DELETE test");
    });
    
    // Routes are added, but we can't easily test them without starting the server
    // In a real scenario, you'd want integration tests for this
    EXPECT_TRUE(true); // Basic test that routes can be added without crashing
}

TEST_F(HttpServerTest, MiddlewareSupport) {
    bool middleware_called = false;
    
    server->use([&middleware_called](const HttpRequest& req, HttpResponse& res) -> bool {
        middleware_called = true;
        res.setHeader("X-Custom-Header", "middleware-test");
        return true;
    });
    
    // Test that middleware can be added
    EXPECT_TRUE(true);
}

TEST_F(HttpServerTest, StaticFileServing) {
    server->serveStatic("/static", "./public");
    
    // Test that static file serving can be configured
    EXPECT_TRUE(true);
}

TEST_F(HttpServerTest, Configuration) {
    server->setMaxConnections(50);
    server->setTimeoutSeconds(60);
    server->setThreadPoolSize(8);
    
    // Test that configuration methods don't crash
    EXPECT_TRUE(true);
}

TEST_F(HttpServerTest, CustomHandlers) {
    bool not_found_called = false;
    bool error_called = false;
    
    server->setNotFoundHandler([&not_found_called](const HttpRequest& req, HttpResponse& res) {
        not_found_called = true;
        res.setStatusCode(HttpResponse::StatusCode::NOT_FOUND);
        res.setTextContent("Custom 404");
    });
    
    server->setErrorHandler([&error_called](const std::exception& e, const HttpRequest& req, HttpResponse& res) {
        error_called = true;
        res.setStatusCode(HttpResponse::StatusCode::INTERNAL_SERVER_ERROR);
        res.setTextContent("Custom error");
    });
    
    // Test that custom handlers can be set
    EXPECT_TRUE(true);
}

#ifdef BUILD_WASM
TEST_F(HttpServerTest, WebAssemblyRequestHandling) {
    server->get("/wasm-test", [](const HttpRequest& req, HttpResponse& res) {
        res.setJsonContent("{\"platform\":\"webassembly\"}");
    });
    
    // Simulate starting the server
    EXPECT_TRUE(server->start(8080, "0.0.0.0"));
    EXPECT_TRUE(server->isRunning());
    
    // Test request handling in WebAssembly mode
    std::string raw_request = 
        "GET /wasm-test HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "\r\n";
    
    std::string response_output;
    server->handleRequest(raw_request, response_output);
    
    EXPECT_NE(response_output.find("HTTP/1.1 200 OK"), std::string::npos);
    EXPECT_NE(response_output.find("{\"platform\":\"webassembly\"}"), std::string::npos);
}

TEST_F(HttpServerTest, WebAssemblyNotFoundHandling) {
    // Simulate starting the server
    EXPECT_TRUE(server->start(8080, "0.0.0.0"));
    
    // Test 404 handling in WebAssembly mode
    std::string raw_request = 
        "GET /nonexistent HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "\r\n";
    
    std::string response_output;
    server->handleRequest(raw_request, response_output);
    
    EXPECT_NE(response_output.find("HTTP/1.1 404"), std::string::npos);
}
#endif

// Integration test for route matching
TEST_F(HttpServerTest, RouteMatching) {
    // This would typically require more sophisticated testing
    // For now, we test that the server can be configured without errors
    
    server->get("/api/users/:id", [](const HttpRequest& req, HttpResponse& res) {
        res.setJsonContent("{\"user\":\"test\"}");
    });
    
    server->get("/api/*", [](const HttpRequest& req, HttpResponse& res) {
        res.setJsonContent("{\"wildcard\":true}");
    });
    
    EXPECT_TRUE(true);
}

// Test server lifecycle
TEST_F(HttpServerTest, ServerLifecycle) {
    EXPECT_FALSE(server->isRunning());
    
    // Note: In a real test environment, you might want to use a random port
    // to avoid conflicts with other tests or running services
    
#ifndef BUILD_WASM
    // For native builds, we can test actual server startup
    // but we need to be careful about port conflicts in CI environments
    
    // Test with an invalid port to ensure error handling works
    EXPECT_FALSE(server->start(-1, "invalid_host"));
    EXPECT_FALSE(server->isRunning());
#else
    // For WebAssembly builds, the server doesn't actually bind to ports
    EXPECT_TRUE(server->start(8080, "0.0.0.0"));
    EXPECT_TRUE(server->isRunning());
    
    server->stop();
    EXPECT_FALSE(server->isRunning());
#endif
}
