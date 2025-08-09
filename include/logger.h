/**
 * @file logger.h
 * @brief Simple logger abstraction with configurable log levels.
 *
 * Provides a lightweight logging system that supports different log levels
 * (error, warn, info, debug) with filtering capabilities.
 *
 * @author Nick Ingargiola
 * @date 2025-01-11
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>

/**
 * @enum LogLevel
 * @brief Enumeration of supported log levels in order of severity.
 */
enum class LogLevel {
    ERROR = 0,  ///< Error messages only
    WARN = 1,   ///< Warning and error messages  
    INFO = 2,   ///< Info, warning, and error messages
    DEBUG = 3   ///< All messages including debug
};

/**
 * @class Logger
 * @brief Simple logger class with level-based message filtering.
 *
 * Messages are only output if their level is at or above the configured threshold.
 * All output goes to stderr to separate from application output on stdout.
 */
class Logger {
private:
    LogLevel currentLevel = LogLevel::WARN;  ///< Current log level threshold

public:
    /**
     * @brief Set the current log level threshold.
     * @param level New log level (only messages at this level or higher will be shown)
     */
    void setLevel(LogLevel level) {
        currentLevel = level;
    }

    /**
     * @brief Set log level from string representation.
     * @param levelStr String representation ("error", "warn", "info", "debug")
     * @return true if level was valid and set, false otherwise
     */
    bool setLevel(const std::string& levelStr) {
        if (levelStr == "error") {
            currentLevel = LogLevel::ERROR;
            return true;
        } else if (levelStr == "warn") {
            currentLevel = LogLevel::WARN;
            return true;
        } else if (levelStr == "info") {
            currentLevel = LogLevel::INFO;
            return true;
        } else if (levelStr == "debug") {
            currentLevel = LogLevel::DEBUG;
            return true;
        }
        return false;
    }

    /**
     * @brief Get current log level.
     * @return Current LogLevel
     */
    LogLevel getLevel() const {
        return currentLevel;
    }

    /**
     * @brief Log an error message.
     * @param message Message to log
     */
    void error(const std::string& message) {
        if (currentLevel >= LogLevel::ERROR) {
            std::cerr << "ERROR: " << message << std::endl;
        }
    }

    /**
     * @brief Log a warning message.
     * @param message Message to log
     */
    void warn(const std::string& message) {
        if (currentLevel >= LogLevel::WARN) {
            std::cerr << "WARN: " << message << std::endl;
        }
    }

    /**
     * @brief Log an info message.
     * @param message Message to log
     */
    void info(const std::string& message) {
        if (currentLevel >= LogLevel::INFO) {
            std::cerr << "INFO: " << message << std::endl;
        }
    }

    /**
     * @brief Log a debug message.
     * @param message Message to log
     */
    void debug(const std::string& message) {
        if (currentLevel >= LogLevel::DEBUG) {
            std::cerr << "DEBUG: " << message << std::endl;
        }
    }
};

/**
 * @brief Global logger instance.
 * 
 * Provides a convenient singleton-like access pattern while keeping
 * the implementation simple and avoiding complex initialization issues.
 */
extern Logger g_logger;

#endif // LOGGER_H