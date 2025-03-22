#pragma once

#include <string>
#include <iostream>

namespace rebel {

/**
 * @brief Simple logging utility for the RebelCAD application.
 * 
 * This class provides static methods for logging messages at different
 * severity levels: info, warning, error, and debug.
 */
class Log {
public:
    /**
     * @brief Log an informational message.
     * 
     * @param message The message to log.
     */
    static void info(const std::string& message) {
        std::cout << "[INFO] " << message << std::endl;
    }
    
    /**
     * @brief Log a warning message.
     * 
     * @param message The message to log.
     */
    static void warning(const std::string& message) {
        std::cout << "[WARNING] " << message << std::endl;
    }
    
    /**
     * @brief Log an error message.
     * 
     * @param message The message to log.
     */
    static void error(const std::string& message) {
        std::cerr << "[ERROR] " << message << std::endl;
    }
    
    /**
     * @brief Log a debug message.
     * 
     * @param message The message to log.
     */
    static void debug(const std::string& message) {
        std::cout << "[DEBUG] " << message << std::endl;
    }
};

// Convenience macros for logging
#define REBEL_LOG_INFO(message) rebel::Log::info(message)
#define REBEL_LOG_WARNING(message) rebel::Log::warning(message)
#define REBEL_LOG_ERROR(message) rebel::Log::error(message)
#define REBEL_LOG_DEBUG(message) rebel::Log::debug(message)

} // namespace rebel
