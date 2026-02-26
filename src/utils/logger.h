#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <ctime>

enum class LogLevel {
    DEBUG,
    INFO,
    ERROR
};

class Logger {
public:
    static Logger& instance();

    void set_level(LogLevel level);
    void set_file(const std::string& filepath);

    void debug(const std::string& msg);
    void info(const std::string& msg);
    void error(const std::string& msg);

    void log(LogLevel level, const std::string& msg);

private:
    Logger();
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string get_timestamp();
    std::string level_to_string(LogLevel level);

    LogLevel min_level_;
    std::ofstream file_;
    std::mutex mutex_;
};

#define LOG_DEBUG(msg) Logger::instance().debug(msg)
#define LOG_INFO(msg) Logger::instance().info(msg)
#define LOG_ERROR(msg) Logger::instance().error(msg)
