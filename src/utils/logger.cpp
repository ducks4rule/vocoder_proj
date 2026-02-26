#include "utils/logger.h"
#include <iostream>
#include <sstream>
#include <iomanip>

Logger::Logger() : min_level_(LogLevel::DEBUG) {
}

Logger::~Logger() {
    if (file_.is_open()) {
        file_.close();
    }
}

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::set_level(LogLevel level) {
    min_level_ = level;
}

void Logger::set_file(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_.is_open()) {
        file_.close();
    }
    file_.open(filepath, std::ios::app);
}

void Logger::debug(const std::string& msg) {
    log(LogLevel::DEBUG, msg);
}

void Logger::info(const std::string& msg) {
    log(LogLevel::INFO, msg);
}

void Logger::error(const std::string& msg) {
    log(LogLevel::ERROR, msg);
}

void Logger::log(LogLevel level, const std::string& msg) {
    if (level < min_level_) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    std::ostringstream oss;
    oss << "[" << get_timestamp() << "] [" << level_to_string(level) << "] " << msg;

    std::string line = oss.str();

    if (file_.is_open()) {
        file_ << line << std::endl;
        file_.flush();
    }

    if (level == LogLevel::ERROR) {
        std::cerr << line << std::endl;
    }
}

std::string Logger::get_timestamp() {
    time_t now = time(nullptr);
    tm* local = localtime(&now);

    std::ostringstream oss;
    oss << std::put_time(local, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string Logger::level_to_string(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}
