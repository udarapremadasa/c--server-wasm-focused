#pragma once

#ifndef BUILD_WASM

#include <string>
#include <atomic>
#include <functional>

class SocketServer {
public:
    using ConnectionHandler = std::function<void(int)>;

    SocketServer();
    ~SocketServer();

    bool bind(int port, const std::string& host = "0.0.0.0");
    bool listen(int backlog = 128);
    void accept(ConnectionHandler handler);
    void stop();
    
    bool isRunning() const { return is_running_; }
    int getPort() const { return port_; }
    const std::string& getHost() const { return host_; }

    // Socket options
    void setReuseAddress(bool reuse);
    void setNonBlocking(bool non_blocking);
    void setTimeout(int seconds);

private:
    int server_socket_;
    int port_;
    std::string host_;
    std::atomic<bool> is_running_;
    
    bool createSocket();
    void closeSocket();
};

#endif // BUILD_WASM
