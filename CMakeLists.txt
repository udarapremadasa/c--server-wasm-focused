cmake_minimum_required(VERSION 3.16)

project(HttpServer 
    VERSION 1.0.0
    DESCRIPTION "A C++ HTTP/HTTPS server with WebAssembly support"
    LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compile options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Options
option(BUILD_TESTS "Build test suite" ON)
option(BUILD_WASM "Build for WebAssembly" OFF)
option(ENABLE_SSL "Enable SSL/TLS support" ON)

# Find packages
if(ENABLE_SSL AND NOT BUILD_WASM)
    find_package(OpenSSL REQUIRED)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/http_server.cpp
    src/http_request.cpp
    src/http_response.cpp
    src/socket_server.cpp
    src/thread_pool.cpp
    src/logger.cpp
)

# Add SSL sources if enabled
if(ENABLE_SSL AND NOT BUILD_WASM)
    list(APPEND SOURCES src/ssl_server.cpp)
endif()

# Create library
add_library(httpserver_lib ${SOURCES})

# Link libraries
if(ENABLE_SSL AND NOT BUILD_WASM)
    target_link_libraries(httpserver_lib OpenSSL::SSL OpenSSL::Crypto)
endif()

if(NOT BUILD_WASM)
    target_link_libraries(httpserver_lib pthread)
endif()

# Compile definitions
if(BUILD_WASM)
    target_compile_definitions(httpserver_lib PRIVATE BUILD_WASM=1)
endif()

if(ENABLE_SSL)
    target_compile_definitions(httpserver_lib PRIVATE ENABLE_SSL=1)
endif()

# Main executable
add_executable(httpserver src/main.cpp)
target_link_libraries(httpserver httpserver_lib)

# Compile definitions for main executable
if(BUILD_WASM)
    target_compile_definitions(httpserver PRIVATE BUILD_WASM=1)
endif()

if(ENABLE_SSL)
    target_compile_definitions(httpserver PRIVATE ENABLE_SSL=1)
endif()

# WebAssembly specific settings
if(BUILD_WASM)
    set_target_properties(httpserver PROPERTIES
        COMPILE_FLAGS "-s USE_PTHREADS=1 -pthread"
        LINK_FLAGS "-s USE_PTHREADS=1 -pthread -s EXPORTED_FUNCTIONS='[\"_main\", \"_start_server\", \"_stop_server\"]' -s EXPORTED_RUNTIME_METHODS='[\"ccall\", \"cwrap\"]' -s MODULARIZE=1 -s EXPORT_NAME='HttpServerModule'"
    )
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    
    # Use system-installed Google Test if available
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        # Download and build GoogleTest
        include(FetchContent)
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/v1.14.0.zip
            DOWNLOAD_EXTRACT_TIMESTAMP true
        )
        
        # For Windows: Prevent overriding the parent project's compiler/linker settings
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()
    
    # Test executable
    add_executable(
        httpserver_tests
        tests/test_http_request.cpp
        tests/test_http_response.cpp
        tests/test_http_server.cpp
        tests/test_socket_server.cpp
        tests/test_thread_pool.cpp
    )
    
    if(GTest_FOUND)
        target_link_libraries(
            httpserver_tests
            httpserver_lib
            GTest::gtest_main
        )
    else()
        target_link_libraries(
            httpserver_tests
            httpserver_lib
            gtest_main
            gmock_main
        )
    endif()
    
    if(GTest_FOUND)
        include(GoogleTest)
        gtest_discover_tests(httpserver_tests)
    else()
        add_test(NAME httpserver_tests COMMAND httpserver_tests)
    endif()
endif()

# Install rules
install(TARGETS httpserver DESTINATION bin)
install(TARGETS httpserver_lib DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
