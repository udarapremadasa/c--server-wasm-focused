#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <memory>

class Logger {
public:
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        FATAL
    };

    static Logger& getInstance();
    
    void setLevel(Level level);
    void setOutputFile(const std::string& filename);
    void enableConsoleOutput(bool enable);
    
    void log(Level level, const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void fatal(const std::string& message);

private:
    Logger() = default;
    ~Logger() = default;
    
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    Level current_level_ = Level::INFO;
    std::unique_ptr<std::ofstream> file_stream_;
    bool console_output_ = true;
    std::mutex log_mutex_;
    
    std::string levelToString(Level level) const;
    std::string getCurrentTimestamp() const;
    void writeLog(const std::string& formatted_message);
};

// Convenience macros
#define LOG_DEBUG(msg) Logger::getInstance().debug(msg)
#define LOG_INFO(msg) Logger::getInstance().info(msg)
#define LOG_WARNING(msg) Logger::getInstance().warning(msg)
#define LOG_ERROR(msg) Logger::getInstance().error(msg)
#define LOG_FATAL(msg) Logger::getInstance().fatal(msg)
