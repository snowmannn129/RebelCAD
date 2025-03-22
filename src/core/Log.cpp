#include "core/Log.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <vector>

namespace rebel::core {

std::unique_ptr<Logger> Logger::instance_ = nullptr;

Logger::Logger() {
    try {
        // Create console sink
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::trace);
        console_sink->set_pattern("[%^%l%$] %v");

        // Create file sink
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("RebelCAD.log", true);
        file_sink->set_level(spdlog::level::trace);
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

        // Create logger with both sinks
        std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
        logger_ = std::make_shared<spdlog::logger>("RebelCAD", sinks.begin(), sinks.end());
        logger_->set_level(spdlog::level::trace);
        
        spdlog::register_logger(logger_);
    }
    catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Log initialization failed: " << ex.what() << std::endl;
    }
}

Logger::~Logger() {
    spdlog::shutdown();
}

void Logger::initialize() {
    getInstance(); // Ensure the logger is created
}

Logger& Logger::getInstance() {
    if (!instance_) {
        instance_ = std::unique_ptr<Logger>(new Logger());
    }
    return *instance_;
}

void Logger::trace(const std::string& message) {
    logger_->trace(message);
}

void Logger::info(const std::string& message) {
    logger_->info(message);
}

void Logger::warn(const std::string& message) {
    logger_->warn(message);
}

void Logger::error(const std::string& message) {
    logger_->error(message);
}

void Logger::critical(const std::string& message) {
    logger_->critical(message);
}

} // namespace rebel::core
