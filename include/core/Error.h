#pragma once

#include <string>
#include <stdexcept>

namespace rebel::core {

/**
 * @enum ErrorCode
 * @brief Error codes for RebelCAD.
 * 
 * This enum defines the error codes that can be used to identify
 * different types of errors in the RebelCAD application.
 */
enum class ErrorCode {
    None = 0,                 ///< No error
    Unknown,                  ///< Unknown error
    InvalidArgument,          ///< Invalid argument provided to a function
    InvalidOperation,         ///< Invalid operation attempted
    OutOfMemory,              ///< Out of memory error
    FileNotFound,             ///< File not found error
    FileIOError,              ///< File I/O error
    ShaderCompilationFailed,  ///< Shader compilation failed
    GraphicsError,            ///< Graphics system error
    SystemError               ///< System error
    // Add more error codes as needed
};

/**
 * @class Error
 * @brief Exception class for RebelCAD errors.
 * 
 * The Error class is used to represent errors that occur in the RebelCAD
 * application. It extends std::runtime_error and adds additional information
 * such as an error code, file name, and line number.
 */
class Error : public std::runtime_error {
public:
    /**
     * @brief Constructs a new Error object.
     * 
     * @param code The error code.
     * @param message The error message.
     * @param file The source file where the error occurred.
     * @param line The line number where the error occurred.
     */
    Error(ErrorCode code, const std::string& message, const char* file = __FILE__, int line = __LINE__)
        : std::runtime_error(message),
          code_(code),
          file_(file),
          line_(line) {}

    /**
     * @brief Gets the error code.
     * @return The error code.
     */
    ErrorCode code() const { return code_; }
    
    /**
     * @brief Gets the source file where the error occurred.
     * @return The source file name.
     */
    const char* file() const { return file_; }
    
    /**
     * @brief Gets the line number where the error occurred.
     * @return The line number.
     */
    int line() const { return line_; }

private:
    ErrorCode code_;    ///< The error code
    const char* file_;  ///< The source file where the error occurred
    int line_;          ///< The line number where the error occurred
};

// Convenience macro for throwing errors
#define REBEL_THROW_ERROR(code, message) \
    throw rebel::core::Error(rebel::core::ErrorCode::code, message)

} // namespace rebel::core
