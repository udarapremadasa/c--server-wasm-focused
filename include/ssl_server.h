#pragma once

#ifdef ENABLE_SSL
#ifndef BUILD_WASM

#include <string>
#include <memory>
#include <openssl/ssl.h>
#include <openssl/err.h>

class SslServer {
public:
    SslServer();
    ~SslServer();

    bool initialize(const std::string& cert_file, const std::string& key_file);
    void cleanup();
    
    SSL_CTX* getContext() const { return ssl_context_; }
    
    // SSL connection handling
    SSL* createSslConnection(int socket);
    bool performHandshake(SSL* ssl);
    void closeSslConnection(SSL* ssl);
    
    // SSL I/O operations
    int sslRead(SSL* ssl, char* buffer, int size);
    int sslWrite(SSL* ssl, const char* data, int size);
    
    // Certificate and key management
    bool loadCertificate(const std::string& cert_file);
    bool loadPrivateKey(const std::string& key_file);
    bool verifyCertificate() const;
    
    // Configuration
    void setVerifyMode(int mode);
    void setCipherList(const std::string& ciphers);
    
    // Error handling
    std::string getLastError() const;

private:
    SSL_CTX* ssl_context_;
    bool is_initialized_;
    
    void initializeOpenSSL();
    void cleanupOpenSSL();
    std::string getSslError() const;
};

#endif // BUILD_WASM
#endif // ENABLE_SSL
