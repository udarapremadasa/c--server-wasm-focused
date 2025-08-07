#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class HttpRequest {
public:
    enum class Method {
        GET,
        POST,
        PUT,
        DELETE,
        HEAD,
        OPTIONS,
        PATCH,
        UNKNOWN
    };

    HttpRequest();
    explicit HttpRequest(const std::string& raw_request);
    
    // Parse raw HTTP request
    bool parse(const std::string& raw_request);
    
    // Getters
    Method getMethod() const { return method_; }
    const std::string& getPath() const { return path_; }
    const std::string& getVersion() const { return version_; }
    const std::string& getBody() const { return body_; }
    
    // Header operations
    std::string getHeader(const std::string& name) const;
    void setHeader(const std::string& name, const std::string& value);
    const std::unordered_map<std::string, std::string>& getHeaders() const { return headers_; }
    
    // Query parameters
    std::string getQueryParam(const std::string& name) const;
    const std::unordered_map<std::string, std::string>& getQueryParams() const { return query_params_; }
    
    // Utility methods
    std::string methodToString() const;
    static Method stringToMethod(const std::string& method_str);
    bool isValid() const { return is_valid_; }
    
    // Content handling
    size_t getContentLength() const;
    std::string getContentType() const;

private:
    Method method_;
    std::string path_;
    std::string version_;
    std::string body_;
    std::unordered_map<std::string, std::string> headers_;
    std::unordered_map<std::string, std::string> query_params_;
    bool is_valid_;
    
    void parseQueryParams(const std::string& query_string);
    void parseHeaders(const std::vector<std::string>& header_lines);
    std::string urlDecode(const std::string& encoded);
};
