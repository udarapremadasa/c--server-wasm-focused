#include "ssl_server.h"

#ifdef ENABLE_SSL
#ifndef BUILD_WASM

#include <iostream>

SslServer::SslServer() : ssl_context_(nullptr), is_initialized_(false) {
}

SslServer::~SslServer() {
    cleanup();
}

bool SslServer::initialize(const std::string& cert_file, const std::string& key_file) {
    initializeOpenSSL();
    
    // Create SSL context
    ssl_context_ = SSL_CTX_new(TLS_server_method());
    if (!ssl_context_) {
        return false;
    }
    
    // Set minimum protocol version
    SSL_CTX_set_min_proto_version(ssl_context_, TLS1_2_VERSION);
    
    // Load certificate and private key
    if (!loadCertificate(cert_file) || !loadPrivateKey(key_file)) {
        cleanup();
        return false;
    }
    
    // Verify private key
    if (!verifyCertificate()) {
        cleanup();
        return false;
    }
    
    is_initialized_ = true;
    return true;
}

void SslServer::cleanup() {
    if (ssl_context_) {
        SSL_CTX_free(ssl_context_);
        ssl_context_ = nullptr;
    }
    
    cleanupOpenSSL();
    is_initialized_ = false;
}

SSL* SslServer::createSslConnection(int socket) {
    if (!is_initialized_ || !ssl_context_) {
        return nullptr;
    }
    
    SSL* ssl = SSL_new(ssl_context_);
    if (!ssl) {
        return nullptr;
    }
    
    if (SSL_set_fd(ssl, socket) != 1) {
        SSL_free(ssl);
        return nullptr;
    }
    
    return ssl;
}

bool SslServer::performHandshake(SSL* ssl) {
    if (!ssl) {
        return false;
    }
    
    int result = SSL_accept(ssl);
    return result == 1;
}

void SslServer::closeSslConnection(SSL* ssl) {
    if (ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
}

int SslServer::sslRead(SSL* ssl, char* buffer, int size) {
    if (!ssl) {
        return -1;
    }
    
    return SSL_read(ssl, buffer, size);
}

int SslServer::sslWrite(SSL* ssl, const char* data, int size) {
    if (!ssl) {
        return -1;
    }
    
    return SSL_write(ssl, data, size);
}

bool SslServer::loadCertificate(const std::string& cert_file) {
    if (!ssl_context_) {
        return false;
    }
    
    int result = SSL_CTX_use_certificate_file(ssl_context_, cert_file.c_str(), SSL_FILETYPE_PEM);
    return result == 1;
}

bool SslServer::loadPrivateKey(const std::string& key_file) {
    if (!ssl_context_) {
        return false;
    }
    
    int result = SSL_CTX_use_PrivateKey_file(ssl_context_, key_file.c_str(), SSL_FILETYPE_PEM);
    return result == 1;
}

bool SslServer::verifyCertificate() const {
    if (!ssl_context_) {
        return false;
    }
    
    int result = SSL_CTX_check_private_key(ssl_context_);
    return result == 1;
}

void SslServer::setVerifyMode(int mode) {
    if (ssl_context_) {
        SSL_CTX_set_verify(ssl_context_, mode, nullptr);
    }
}

void SslServer::setCipherList(const std::string& ciphers) {
    if (ssl_context_) {
        SSL_CTX_set_cipher_list(ssl_context_, ciphers.c_str());
    }
}

std::string SslServer::getLastError() const {
    return getSslError();
}

void SslServer::initializeOpenSSL() {
    SSL_load_error_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();
}

void SslServer::cleanupOpenSSL() {
    EVP_cleanup();
    ERR_free_strings();
}

std::string SslServer::getSslError() const {
    BIO* bio = BIO_new(BIO_s_mem());
    ERR_print_errors(bio);
    
    char* data;
    long len = BIO_get_mem_data(bio, &data);
    
    std::string error(data, len);
    BIO_free(bio);
    
    return error;
}

#endif // BUILD_WASM
#endif // ENABLE_SSL
