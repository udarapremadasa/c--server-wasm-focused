#include <gtest/gtest.h>
#include "http_response.h"

class HttpResponseTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(HttpResponseTest, DefaultConstructor) {
    HttpResponse response;
    
    EXPECT_EQ(response.getStatusCode(), HttpResponse::StatusCode::OK);
    EXPECT_EQ(response.getStatusText(), "OK");
    EXPECT_FALSE(response.isError());
    EXPECT_TRUE(response.getBody().empty());
}

TEST_F(HttpResponseTest, StatusCodeConstructor) {
    HttpResponse response(HttpResponse::StatusCode::NOT_FOUND);
    
    EXPECT_EQ(response.getStatusCode(), HttpResponse::StatusCode::NOT_FOUND);
    EXPECT_EQ(response.getStatusText(), "Not Found");
    EXPECT_TRUE(response.isError());
}

TEST_F(HttpResponseTest, SetStatusCode) {
    HttpResponse response;
    
    response.setStatusCode(HttpResponse::StatusCode::CREATED);
    EXPECT_EQ(response.getStatusCode(), HttpResponse::StatusCode::CREATED);
    EXPECT_EQ(response.getStatusText(), "Created");
    EXPECT_FALSE(response.isError());
    
    response.setStatusCode(HttpResponse::StatusCode::INTERNAL_SERVER_ERROR);
    EXPECT_EQ(response.getStatusCode(), HttpResponse::StatusCode::INTERNAL_SERVER_ERROR);
    EXPECT_TRUE(response.isError());
}

TEST_F(HttpResponseTest, HeaderOperations) {
    HttpResponse response;
    
    response.setHeader("Content-Type", "application/json");
    EXPECT_EQ(response.getHeader("Content-Type"), "application/json");
    
    response.setHeader("Custom-Header", "Custom-Value");
    EXPECT_EQ(response.getHeader("Custom-Header"), "Custom-Value");
    
    EXPECT_EQ(response.getHeader("Nonexistent-Header"), "");
}

TEST_F(HttpResponseTest, BodyOperations) {
    HttpResponse response;
    
    std::string test_body = "Hello, World!";
    response.setBody(test_body);
    EXPECT_EQ(response.getBody(), test_body);
    EXPECT_EQ(response.getHeader("Content-Length"), std::to_string(test_body.length()));
    
    // Test with const char* and length
    const char* test_data = "Binary data";
    response.setBody(test_data, 6); // Only first 6 characters
    EXPECT_EQ(response.getBody(), "Binary");
}

TEST_F(HttpResponseTest, ContentTypeShortcuts) {
    HttpResponse response;
    
    // Test JSON content
    response.setJsonContent("{\"test\": true}");
    EXPECT_EQ(response.getHeader("Content-Type"), "application/json; charset=utf-8");
    EXPECT_EQ(response.getBody(), "{\"test\": true}");
    
    // Test HTML content
    response.setHtmlContent("<h1>Hello</h1>");
    EXPECT_EQ(response.getHeader("Content-Type"), "text/html; charset=utf-8");
    EXPECT_EQ(response.getBody(), "<h1>Hello</h1>");
    
    // Test text content
    response.setTextContent("Plain text");
    EXPECT_EQ(response.getHeader("Content-Type"), "text/plain; charset=utf-8");
    EXPECT_EQ(response.getBody(), "Plain text");
}

TEST_F(HttpResponseTest, CorsSupport) {
    HttpResponse response;
    
    response.enableCors();
    EXPECT_EQ(response.getHeader("Access-Control-Allow-Origin"), "*");
    EXPECT_EQ(response.getHeader("Access-Control-Allow-Methods"), "GET, POST, PUT, DELETE, OPTIONS");
    EXPECT_EQ(response.getHeader("Access-Control-Allow-Headers"), "Content-Type, Authorization");
    
    response.enableCors("https://example.com");
    EXPECT_EQ(response.getHeader("Access-Control-Allow-Origin"), "https://example.com");
}

TEST_F(HttpResponseTest, ToString) {
    HttpResponse response;
    response.setStatusCode(HttpResponse::StatusCode::OK);
    response.setHeader("Content-Type", "text/plain");
    response.setBody("Hello, World!");
    
    std::string response_str = response.toString();
    
    // Check that the response contains the expected components
    EXPECT_NE(response_str.find("HTTP/1.1 200 OK"), std::string::npos);
    EXPECT_NE(response_str.find("Content-Type: text/plain"), std::string::npos);
    EXPECT_NE(response_str.find("Content-Length: 13"), std::string::npos);
    EXPECT_NE(response_str.find("Hello, World!"), std::string::npos);
    
    // Check that headers and body are separated by \r\n\r\n
    EXPECT_NE(response_str.find("\r\n\r\n"), std::string::npos);
}

TEST_F(HttpResponseTest, ErrorStatusCodes) {
    HttpResponse response;
    
    // Test client error codes
    response.setStatusCode(HttpResponse::StatusCode::BAD_REQUEST);
    EXPECT_TRUE(response.isError());
    
    response.setStatusCode(HttpResponse::StatusCode::UNAUTHORIZED);
    EXPECT_TRUE(response.isError());
    
    response.setStatusCode(HttpResponse::StatusCode::FORBIDDEN);
    EXPECT_TRUE(response.isError());
    
    response.setStatusCode(HttpResponse::StatusCode::NOT_FOUND);
    EXPECT_TRUE(response.isError());
    
    // Test server error codes
    response.setStatusCode(HttpResponse::StatusCode::INTERNAL_SERVER_ERROR);
    EXPECT_TRUE(response.isError());
    
    response.setStatusCode(HttpResponse::StatusCode::NOT_IMPLEMENTED);
    EXPECT_TRUE(response.isError());
    
    // Test success codes
    response.setStatusCode(HttpResponse::StatusCode::OK);
    EXPECT_FALSE(response.isError());
    
    response.setStatusCode(HttpResponse::StatusCode::CREATED);
    EXPECT_FALSE(response.isError());
    
    response.setStatusCode(HttpResponse::StatusCode::NO_CONTENT);
    EXPECT_FALSE(response.isError());
}
