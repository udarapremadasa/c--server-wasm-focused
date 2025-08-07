#include "http_response.h"
#include <sstream>
#include <fstream>
#include <unordered_map>

HttpResponse::HttpResponse() 
    : status_code_(StatusCode::OK), version_("HTTP/1.1") {
}

HttpResponse::HttpResponse(StatusCode status_code) 
    : status_code_(status_code), version_("HTTP/1.1") {
}

std::string HttpResponse::getStatusText() const {
    return statusCodeToString(status_code_);
}

void HttpResponse::setHeader(const std::string& name, const std::string& value) {
    headers_[name] = value;
}

std::string HttpResponse::getHeader(const std::string& name) const {
    auto it = headers_.find(name);
    return (it != headers_.end()) ? it->second : "";
}

void HttpResponse::setBody(const std::string& body) {
    body_ = body;
    setContentLength();
}

void HttpResponse::setBody(const char* body, size_t length) {
    body_.assign(body, length);
    setContentLength();
}

void HttpResponse::setJsonContent(const std::string& json) {
    setHeader("Content-Type", "application/json; charset=utf-8");
    setBody(json);
}

void HttpResponse::setHtmlContent(const std::string& html) {
    setHeader("Content-Type", "text/html; charset=utf-8");
    setBody(html);
}

void HttpResponse::setTextContent(const std::string& text) {
    setHeader("Content-Type", "text/plain; charset=utf-8");
    setBody(text);
}

void HttpResponse::setFileContent(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        status_code_ = StatusCode::NOT_FOUND;
        setTextContent("File not found");
        return;
    }
    
    // Get file extension for MIME type
    size_t dot_pos = file_path.find_last_of('.');
    std::string extension;
    if (dot_pos != std::string::npos) {
        extension = file_path.substr(dot_pos + 1);
    }
    
    std::string mime_type = getMimeType(extension);
    setHeader("Content-Type", mime_type);
    
    // Read file content
    std::ostringstream content;
    content << file.rdbuf();
    setBody(content.str());
}

void HttpResponse::enableCors(const std::string& origin) {
    setHeader("Access-Control-Allow-Origin", origin);
    setHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
}

std::string HttpResponse::toString() const {
    std::ostringstream response;
    
    // Status line
    response << version_ << " " << static_cast<int>(status_code_) << " " << getStatusText() << "\r\n";
    
    // Headers
    for (const auto& header : headers_) {
        response << header.first << ": " << header.second << "\r\n";
    }
    
    // Empty line to separate headers from body
    response << "\r\n";
    
    // Body
    response << body_;
    
    return response.str();
}

void HttpResponse::setContentLength() {
    setHeader("Content-Length", std::to_string(body_.length()));
}

bool HttpResponse::isError() const {
    int code = static_cast<int>(status_code_);
    return code >= 400;
}

std::string HttpResponse::statusCodeToString(StatusCode code) const {
    static const std::unordered_map<StatusCode, std::string> status_texts = {
        {StatusCode::OK, "OK"},
        {StatusCode::CREATED, "Created"},
        {StatusCode::NO_CONTENT, "No Content"},
        {StatusCode::BAD_REQUEST, "Bad Request"},
        {StatusCode::UNAUTHORIZED, "Unauthorized"},
        {StatusCode::FORBIDDEN, "Forbidden"},
        {StatusCode::NOT_FOUND, "Not Found"},
        {StatusCode::METHOD_NOT_ALLOWED, "Method Not Allowed"},
        {StatusCode::INTERNAL_SERVER_ERROR, "Internal Server Error"},
        {StatusCode::NOT_IMPLEMENTED, "Not Implemented"},
        {StatusCode::SERVICE_UNAVAILABLE, "Service Unavailable"}
    };
    
    auto it = status_texts.find(code);
    return (it != status_texts.end()) ? it->second : "Unknown";
}

std::string HttpResponse::getMimeType(const std::string& file_extension) const {
    static const std::unordered_map<std::string, std::string> mime_types = {
        {"html", "text/html"},
        {"htm", "text/html"},
        {"css", "text/css"},
        {"js", "application/javascript"},
        {"json", "application/json"},
        {"xml", "application/xml"},
        {"txt", "text/plain"},
        {"png", "image/png"},
        {"jpg", "image/jpeg"},
        {"jpeg", "image/jpeg"},
        {"gif", "image/gif"},
        {"svg", "image/svg+xml"},
        {"ico", "image/x-icon"},
        {"pdf", "application/pdf"},
        {"zip", "application/zip"},
        {"mp4", "video/mp4"},
        {"mp3", "audio/mpeg"},
        {"wav", "audio/wav"},
        {"wasm", "application/wasm"}
    };
    
    auto it = mime_types.find(file_extension);
    return (it != mime_types.end()) ? it->second : "application/octet-stream";
}
