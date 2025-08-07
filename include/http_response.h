#pragma once

#include <string>
#include <unordered_map>

class HttpResponse {
public:
    enum class StatusCode {
        OK = 200,
        CREATED = 201,
        NO_CONTENT = 204,
        BAD_REQUEST = 400,
        UNAUTHORIZED = 401,
        FORBIDDEN = 403,
        NOT_FOUND = 404,
        METHOD_NOT_ALLOWED = 405,
        INTERNAL_SERVER_ERROR = 500,
        NOT_IMPLEMENTED = 501,
        SERVICE_UNAVAILABLE = 503
    };

    HttpResponse();
    explicit HttpResponse(StatusCode status_code);
    
    // Status operations
    void setStatusCode(StatusCode code) { status_code_ = code; }
    StatusCode getStatusCode() const { return status_code_; }
    std::string getStatusText() const;
    
    // Header operations
    void setHeader(const std::string& name, const std::string& value);
    std::string getHeader(const std::string& name) const;
    const std::unordered_map<std::string, std::string>& getHeaders() const { return headers_; }
    
    // Body operations
    void setBody(const std::string& body);
    void setBody(const char* body, size_t length);
    const std::string& getBody() const { return body_; }
    
    // Content type shortcuts
    void setJsonContent(const std::string& json);
    void setHtmlContent(const std::string& html);
    void setTextContent(const std::string& text);
    void setFileContent(const std::string& file_path);
    
    // CORS support
    void enableCors(const std::string& origin = "*");
    
    // Generate HTTP response string
    std::string toString() const;
    
    // Utility methods
    void setContentLength();
    bool isError() const;

private:
    StatusCode status_code_;
    std::unordered_map<std::string, std::string> headers_;
    std::string body_;
    std::string version_;
    
    std::string statusCodeToString(StatusCode code) const;
    std::string getMimeType(const std::string& file_extension) const;
};
