#include "core/Error.h"
#include "core/Log.h"

namespace rebel::core {

// This file is intentionally minimal as most of the Error functionality
// is implemented in the header file. This implementation file exists to
// provide a place for any future extensions to the error handling system.

// Example of how we might extend the error handling system in the future:
// void LogError(const Error& error) {
//     REBEL_LOG_ERROR("Error: {} (Code: {}, File: {}, Line: {})",
//                    error.what(),
//                    static_cast<int>(error.code()),
//                    error.file(),
//                    error.line());
// }

} // namespace rebel::core
