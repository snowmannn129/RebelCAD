cmake_minimum_required(VERSION 3.15)
project(RebelCAD VERSION 0.1.0 LANGUAGES CXX)

# Log level options
set(REBELCAD_LOG_LEVEL "INFO" CACHE STRING "Log level (ERROR, WARNING, INFO, DEBUG, VERBOSE)")
set_property(CACHE REBELCAD_LOG_LEVEL PROPERTY STRINGS ERROR WARNING INFO DEBUG VERBOSE)

# Component-specific build options
option(BUILD_SIMULATION "Build simulation module" ON)
option(BUILD_MODELING "Build modeling module" ON)
option(BUILD_UI "Build UI module" ON)
option(BUILD_SKETCHING "Build sketching module" ON)
option(BUILD_ASSEMBLY "Build assembly module" ON)
option(BUILD_CONSTRAINTS "Build constraints module" ON)
option(BUILD_GRAPHICS "Build graphics module" ON)
option(BUILD_TESTS "Build tests" ON)

# Submodule-specific build options (only active if parent module is enabled)
# Simulation submodules
option(BUILD_SIMULATION_FEA "Build FEA submodule" ON)
option(BUILD_SIMULATION_THERMAL "Build thermal analysis submodule" ON)
option(BUILD_SIMULATION_DYNAMIC "Build dynamic analysis submodule" ON)
option(BUILD_SIMULATION_NONLINEAR "Build non-linear analysis submodule" ON)

# Modeling submodules
option(BUILD_MODELING_GEOMETRY "Build geometry submodule" ON)
option(BUILD_MODELING_TOPOLOGY "Build topology submodule" ON)
option(BUILD_MODELING_OPERATIONS "Build modeling operations submodule" ON)

# UI submodules
option(BUILD_UI_WIDGETS "Build UI widgets submodule" ON)
option(BUILD_UI_DIALOGS "Build UI dialogs submodule" ON)
option(BUILD_UI_VIEWPORT "Build UI viewport submodule" ON)

# Test categories
option(BUILD_UNIT_TESTS "Build unit tests" ON)
option(BUILD_INTEGRATION_TESTS "Build integration tests" ON)
option(BUILD_BENCHMARK_TESTS "Build benchmark tests" ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add compiler flags
if(MSVC)
    # MSVC flags
    add_compile_options(/W4 /MP)
else()
    # GCC/Clang flags
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/eigen
)

# Add subdirectories
add_subdirectory(src/core)

if(BUILD_SIMULATION)
    add_subdirectory(src/simulation)
endif()

if(BUILD_MODELING)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/modeling/CMakeLists.txt")
        add_subdirectory(src/modeling)
    endif()
endif()

if(BUILD_UI)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/CMakeLists.txt")
        add_subdirectory(src/ui)
    endif()
endif()

if(BUILD_SKETCHING)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/sketching/CMakeLists.txt")
        add_subdirectory(src/sketching)
    endif()
endif()

if(BUILD_ASSEMBLY)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/assembly/CMakeLists.txt")
        add_subdirectory(src/assembly)
    endif()
endif()

if(BUILD_CONSTRAINTS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/constraints/CMakeLists.txt")
        add_subdirectory(src/constraints)
    endif()
endif()

if(BUILD_GRAPHICS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/CMakeLists.txt")
        add_subdirectory(src/graphics)
    endif()
endif()

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

add_subdirectory(examples)

# Define log level as a preprocessor macro
add_definitions(-DREBELCAD_LOG_LEVEL_${REBELCAD_LOG_LEVEL})

# Print configuration summary
message(STATUS "")
message(STATUS "RebelCAD Configuration Summary")
message(STATUS "------------------------------")
message(STATUS "Version:        ${PROJECT_VERSION}")
message(STATUS "Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Compiler:   ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ Standard:   ${CMAKE_CXX_STANDARD}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Log level:      ${REBELCAD_LOG_LEVEL}")
message(STATUS "")
message(STATUS "Components:")
message(STATUS "  Core:         ON (required)")
message(STATUS "  Simulation:   ${BUILD_SIMULATION}")
if(BUILD_SIMULATION)
  message(STATUS "    FEA:        ${BUILD_SIMULATION_FEA}")
  message(STATUS "    Thermal:    ${BUILD_SIMULATION_THERMAL}")
  message(STATUS "    Dynamic:    ${BUILD_SIMULATION_DYNAMIC}")
  message(STATUS "    NonLinear:  ${BUILD_SIMULATION_NONLINEAR}")
endif()
message(STATUS "  Modeling:     ${BUILD_MODELING}")
if(BUILD_MODELING)
  message(STATUS "    Geometry:   ${BUILD_MODELING_GEOMETRY}")
  message(STATUS "    Topology:   ${BUILD_MODELING_TOPOLOGY}")
  message(STATUS "    Operations: ${BUILD_MODELING_OPERATIONS}")
endif()
message(STATUS "  UI:           ${BUILD_UI}")
if(BUILD_UI)
  message(STATUS "    Widgets:    ${BUILD_UI_WIDGETS}")
  message(STATUS "    Dialogs:    ${BUILD_UI_DIALOGS}")
  message(STATUS "    Viewport:   ${BUILD_UI_VIEWPORT}")
endif()
message(STATUS "  Sketching:    ${BUILD_SKETCHING}")
message(STATUS "  Assembly:     ${BUILD_ASSEMBLY}")
message(STATUS "  Constraints:  ${BUILD_CONSTRAINTS}")
message(STATUS "  Graphics:     ${BUILD_GRAPHICS}")
message(STATUS "  Tests:        ${BUILD_TESTS}")
if(BUILD_TESTS)
  message(STATUS "    Unit:       ${BUILD_UNIT_TESTS}")
  message(STATUS "    Integration:${BUILD_INTEGRATION_TESTS}")
  message(STATUS "    Benchmark:  ${BUILD_BENCHMARK_TESTS}")
endif()
message(STATUS "")
