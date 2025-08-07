#include "http_request.h"
#include <sstream>
#include <algorithm>
#include <cctype>

HttpRequest::HttpRequest() 
    : method_(Method::UNKNOWN), version_("HTTP/1.1"), is_valid_(false) {
}

HttpRequest::HttpRequest(const std::string& raw_request) 
    : method_(Method::UNKNOWN), version_("HTTP/1.1"), is_valid_(false) {
    parse(raw_request);
}

bool HttpRequest::parse(const std::string& raw_request) {
    if (raw_request.empty()) {
        is_valid_ = false;
        return false;
    }

    std::istringstream stream(raw_request);
    std::string line;
    
    // Parse request line
    if (!std::getline(stream, line)) {
        is_valid_ = false;
        return false;
    }
    
    // Remove carriage return if present
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
    
    std::istringstream request_line(line);
    std::string method_str, path_with_query, version;
    
    if (!(request_line >> method_str >> path_with_query >> version)) {
        is_valid_ = false;
        return false;
    }
    
    // Basic validation
    if (method_str.empty() || path_with_query.empty() || version.empty()) {
        is_valid_ = false;
        return false;
    }
    
    // Validate HTTP version format
    if (version.find("HTTP/") != 0) {
        is_valid_ = false;
        return false;
    }
    
    method_ = stringToMethod(method_str);
    version_ = version;
    
    // Parse path and query parameters
    size_t query_pos = path_with_query.find('?');
    if (query_pos != std::string::npos) {
        path_ = path_with_query.substr(0, query_pos);
        std::string query_string = path_with_query.substr(query_pos + 1);
        parseQueryParams(query_string);
    } else {
        path_ = path_with_query;
    }
    
    // URL decode the path
    path_ = urlDecode(path_);
    
    // Parse headers
    std::vector<std::string> header_lines;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        if (line.empty()) {
            break; // End of headers
        }
        
        header_lines.push_back(line);
    }
    
    parseHeaders(header_lines);
    
    // Parse body
    std::ostringstream body_stream;
    while (std::getline(stream, line)) {
        body_stream << line << "\n";
    }
    body_ = body_stream.str();
    
    // Remove trailing newline if present
    if (!body_.empty() && body_.back() == '\n') {
        body_.pop_back();
    }
    
    is_valid_ = true;
    return true;
}

std::string HttpRequest::getHeader(const std::string& name) const {
    auto it = headers_.find(name);
    return (it != headers_.end()) ? it->second : "";
}

void HttpRequest::setHeader(const std::string& name, const std::string& value) {
    headers_[name] = value;
}

std::string HttpRequest::getQueryParam(const std::string& name) const {
    auto it = query_params_.find(name);
    return (it != query_params_.end()) ? it->second : "";
}

std::string HttpRequest::methodToString() const {
    switch (method_) {
        case Method::GET:     return "GET";
        case Method::POST:    return "POST";
        case Method::PUT:     return "PUT";
        case Method::DELETE:  return "DELETE";
        case Method::HEAD:    return "HEAD";
        case Method::OPTIONS: return "OPTIONS";
        case Method::PATCH:   return "PATCH";
        default:              return "UNKNOWN";
    }
}

HttpRequest::Method HttpRequest::stringToMethod(const std::string& method_str) {
    std::string upper_method = method_str;
    std::transform(upper_method.begin(), upper_method.end(), upper_method.begin(), ::toupper);
    
    if (upper_method == "GET")         return Method::GET;
    if (upper_method == "POST")        return Method::POST;
    if (upper_method == "PUT")         return Method::PUT;
    if (upper_method == "DELETE")      return Method::DELETE;
    if (upper_method == "HEAD")        return Method::HEAD;
    if (upper_method == "OPTIONS")     return Method::OPTIONS;
    if (upper_method == "PATCH")       return Method::PATCH;
    
    return Method::UNKNOWN;
}

size_t HttpRequest::getContentLength() const {
    std::string length_str = getHeader("Content-Length");
    if (length_str.empty()) {
        return 0;
    }
    
    try {
        return std::stoull(length_str);
    } catch (const std::exception&) {
        return 0;
    }
}

std::string HttpRequest::getContentType() const {
    return getHeader("Content-Type");
}

void HttpRequest::parseQueryParams(const std::string& query_string) {
    std::istringstream stream(query_string);
    std::string pair;
    
    while (std::getline(stream, pair, '&')) {
        size_t eq_pos = pair.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = urlDecode(pair.substr(0, eq_pos));
            std::string value = urlDecode(pair.substr(eq_pos + 1));
            query_params_[key] = value;
        } else {
            query_params_[urlDecode(pair)] = "";
        }
    }
}

void HttpRequest::parseHeaders(const std::vector<std::string>& header_lines) {
    for (const auto& line : header_lines) {
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string name = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            
            // Trim whitespace
            name.erase(0, name.find_first_not_of(" \t"));
            name.erase(name.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            headers_[name] = value;
        }
    }
}

std::string HttpRequest::urlDecode(const std::string& encoded) {
    std::string decoded;
    decoded.reserve(encoded.length());
    
    for (size_t i = 0; i < encoded.length(); ++i) {
        if (encoded[i] == '%' && i + 2 < encoded.length()) {
            std::string hex = encoded.substr(i + 1, 2);
            try {
                char ch = static_cast<char>(std::stoi(hex, nullptr, 16));
                decoded += ch;
                i += 2;
            } catch (const std::exception&) {
                decoded += encoded[i];
            }
        } else if (encoded[i] == '+') {
            decoded += ' ';
        } else {
            decoded += encoded[i];
        }
    }
    
    return decoded;
}
