#include <gtest/gtest.h>
#include "socket_server.h"

#ifndef BUILD_WASM

class SocketServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        server = std::make_unique<SocketServer>();
    }
    
    void TearDown() override {
        if (server) {
            server->stop();
        }
    }
    
    std::unique_ptr<SocketServer> server;
};

TEST_F(SocketServerTest, InitialState) {
    EXPECT_FALSE(server->isRunning());
    EXPECT_EQ(server->getPort(), 0);
    EXPECT_EQ(server->getHost(), "");
}

TEST_F(SocketServerTest, BindToValidPort) {
    // Test binding to a high port number to avoid permission issues
    EXPECT_TRUE(server->bind(9999, "127.0.0.1"));
    EXPECT_EQ(server->getPort(), 9999);
    EXPECT_EQ(server->getHost(), "127.0.0.1");
}

TEST_F(SocketServerTest, BindToInvalidPort) {
    // Test binding to an invalid port
    EXPECT_FALSE(server->bind(-1, "127.0.0.1"));
    EXPECT_FALSE(server->bind(0, "invalid_host"));
}

TEST_F(SocketServerTest, ListenWithoutBind) {
    // Should fail if not bound first
    EXPECT_FALSE(server->listen());
}

TEST_F(SocketServerTest, BindAndListen) {
    EXPECT_TRUE(server->bind(9998, "127.0.0.1"));
    EXPECT_TRUE(server->listen());
    EXPECT_TRUE(server->isRunning());
}

TEST_F(SocketServerTest, Stop) {
    EXPECT_TRUE(server->bind(9997, "127.0.0.1"));
    EXPECT_TRUE(server->listen());
    EXPECT_TRUE(server->isRunning());
    
    server->stop();
    EXPECT_FALSE(server->isRunning());
}

TEST_F(SocketServerTest, SocketOptions) {
    EXPECT_TRUE(server->bind(9996, "127.0.0.1"));
    
    // Test that socket option methods don't crash
    server->setReuseAddress(true);
    server->setNonBlocking(false);
    server->setTimeout(30);
    
    EXPECT_TRUE(server->listen());
}

TEST_F(SocketServerTest, MultipleBindAttempts) {
    EXPECT_TRUE(server->bind(9995, "127.0.0.1"));
    EXPECT_TRUE(server->listen());
    
    // Create another server and try to bind to the same port
    auto server2 = std::make_unique<SocketServer>();
    EXPECT_FALSE(server2->bind(9995, "127.0.0.1"));
}

// Note: Testing the accept() method would require a more complex setup
// with actual client connections, which is better suited for integration tests

#endif // BUILD_WASM
