#include "socket_server.h"

#ifndef BUILD_WASM

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <stdexcept>

SocketServer::SocketServer() 
    : server_socket_(-1), port_(0), is_running_(false) {
}

SocketServer::~SocketServer() {
    stop();
}

bool SocketServer::createSocket() {
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ < 0) {
        return false;
    }
    
    // Set socket to reuse address
    setReuseAddress(true);
    
    return true;
}

bool SocketServer::bind(int port, const std::string& host) {
    if (!createSocket()) {
        return false;
    }
    
    port_ = port;
    host_ = host;
    
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (host == "0.0.0.0" || host.empty()) {
        server_addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
            closeSocket();
            return false;
        }
    }
    
    if (::bind(server_socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        closeSocket();
        return false;
    }
    
    return true;
}

bool SocketServer::listen(int backlog) {
    if (server_socket_ < 0) {
        return false;
    }
    
    if (::listen(server_socket_, backlog) < 0) {
        return false;
    }
    
    is_running_ = true;
    return true;
}

void SocketServer::accept(ConnectionHandler handler) {
    if (!is_running_ || server_socket_ < 0) {
        return;
    }
    
    while (is_running_) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_socket = ::accept(server_socket_, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket < 0) {
            if (is_running_) {
                // Only report error if we're still supposed to be running
                continue;
            }
            break;
        }
        
        if (is_running_) {
            handler(client_socket);
        } else {
            close(client_socket);
            break;
        }
    }
}

void SocketServer::stop() {
    is_running_ = false;
    closeSocket();
}

void SocketServer::closeSocket() {
    if (server_socket_ >= 0) {
        close(server_socket_);
        server_socket_ = -1;
    }
}

void SocketServer::setReuseAddress(bool reuse) {
    if (server_socket_ < 0) {
        return;
    }
    
    int option = reuse ? 1 : 0;
    setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
}

void SocketServer::setNonBlocking(bool non_blocking) {
    if (server_socket_ < 0) {
        return;
    }
    
    int flags = fcntl(server_socket_, F_GETFL, 0);
    if (flags < 0) {
        return;
    }
    
    if (non_blocking) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    
    fcntl(server_socket_, F_SETFL, flags);
}

void SocketServer::setTimeout(int seconds) {
    if (server_socket_ < 0) {
        return;
    }
    
    struct timeval timeout;
    timeout.tv_sec = seconds;
    timeout.tv_usec = 0;
    
    setsockopt(server_socket_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(server_socket_, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
}

#endif // BUILD_WASM
