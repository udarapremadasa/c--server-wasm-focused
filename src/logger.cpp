#include "logger.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::setLevel(Level level) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    current_level_ = level;
}

void Logger::setOutputFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    file_stream_ = std::make_unique<std::ofstream>(filename, std::ios::app);
}

void Logger::enableConsoleOutput(bool enable) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    console_output_ = enable;
}

void Logger::log(Level level, const std::string& message) {
    if (level < current_level_) {
        return;
    }
    
    std::string timestamp = getCurrentTimestamp();
    std::string level_str = levelToString(level);
    
    std::ostringstream formatted;
    formatted << "[" << timestamp << "] [" << level_str << "] " << message;
    
    writeLog(formatted.str());
}

void Logger::debug(const std::string& message) {
    log(Level::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(Level::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(Level::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(Level::ERROR, message);
}

void Logger::fatal(const std::string& message) {
    log(Level::FATAL, message);
}

std::string Logger::levelToString(Level level) const {
    switch (level) {
        case Level::DEBUG:   return "DEBUG";
        case Level::INFO:    return "INFO";
        case Level::WARNING: return "WARNING";
        case Level::ERROR:   return "ERROR";
        case Level::FATAL:   return "FATAL";
        default:             return "UNKNOWN";
    }
}

std::string Logger::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return oss.str();
}

void Logger::writeLog(const std::string& formatted_message) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    if (console_output_) {
        std::cout << formatted_message << std::endl;
    }
    
    if (file_stream_ && file_stream_->is_open()) {
        *file_stream_ << formatted_message << std::endl;
        file_stream_->flush();
    }
}
