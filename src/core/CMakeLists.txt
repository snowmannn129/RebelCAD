# Core module CMakeLists.txt

# Add header files for the core library
set(CORE_HEADERS
    ${CMAKE_SOURCE_DIR}/include/core/Log.h
)

# Create the core library (header-only for now)
add_library(core INTERFACE)

# Set include directories for the core library
target_include_directories(core INTERFACE
    ${CMAKE_SOURCE_DIR}/include
)

# Install the header files
install(FILES ${CORE_HEADERS}
    DESTINATION include/core
)
