enable_testing()

# Add test categories
set(TEST_CATEGORIES
    SMALL
    MEDIUM
    LARGE
    UNIT
    INTEGRATION
    BENCHMARK
    SIMULATION
    SIMULATION_FEA
    SIMULATION_THERMAL
    SIMULATION_DYNAMIC
    SIMULATION_NONLINEAR
    CORE
    MODELING
    UI
    SKETCHING
    ASSEMBLY
    CONSTRAINTS
    GRAPHICS
)

# Add required headers
include_directories(${CMAKE_SOURCE_DIR}/external/glad/include)

# Find Google Test and Google Benchmark
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.11.0
)

# Add Google Benchmark if benchmark tests are enabled
if(BUILD_BENCHMARK_TESTS)
    FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.7.1
    )
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable benchmark testing" FORCE)
    FetchContent_MakeAvailable(benchmark)
endif()

FetchContent_MakeAvailable(googletest)
include_directories(${GTEST_INCLUDE_DIRS})

# Add required definitions
if(MSVC)
    add_definitions(
        -D_USE_MATH_DEFINES
        -DGLAD_GLAPI_EXPORT
        -DGLAD_GLAPI_EXPORT_BUILD
    )
endif()

# Find required packages
find_package(OpenGL REQUIRED)

# Try to find nlohmann_json
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    # If not found, we'll assume it's included in the main project
    message(STATUS "nlohmann_json package not found, assuming it's included in the main project")
endif()

# Only build tests if the corresponding module is enabled
if(BUILD_UNIT_TESTS)
    # Add subdirectories for unit tests
    if(BUILD_SIMULATION)
        add_subdirectory(simulation)
    endif()
    
    if(BUILD_MODELING AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modeling/CMakeLists.txt")
        add_subdirectory(modeling)
    endif()
    
    if(BUILD_UI AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ui/CMakeLists.txt")
        add_subdirectory(ui)
    endif()
    
    if(BUILD_SKETCHING AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/sketching/CMakeLists.txt")
        add_subdirectory(sketching)
    endif()
    
    if(BUILD_ASSEMBLY AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assembly/CMakeLists.txt")
        add_subdirectory(assembly)
    endif()
    
    if(BUILD_CONSTRAINTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/constraints/CMakeLists.txt")
        add_subdirectory(constraints)
    endif()
    
    if(BUILD_GRAPHICS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/graphics/CMakeLists.txt")
        add_subdirectory(graphics)
    endif()
endif()

if(BUILD_INTEGRATION_TESTS)
    # Add subdirectory for integration tests
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/integration/CMakeLists.txt")
        add_subdirectory(integration)
    endif()
endif()

if(BUILD_BENCHMARK_TESTS)
    # Add subdirectory for benchmark tests
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/benchmark/CMakeLists.txt")
        add_subdirectory(benchmark)
    endif()
endif()

# Simulation tests executable (for backward compatibility)
if(BUILD_SIMULATION AND BUILD_UNIT_TESTS)
    add_executable(simulation_tests
        simulation/MaterialTests.cpp
        simulation/BoundaryConditionTests.cpp
        simulation/LoadTests.cpp
        simulation/FEASystemTests.cpp
    )
    
    target_link_libraries(simulation_tests
        PRIVATE
        simulation
        core
        gtest
        gtest_main
        ${OPENGL_LIBRARIES}
        glfw
        glad
        imgui
        glm
        dbghelp
        libzstd_static
    )
    
    # Add nlohmann_json if found
    if(nlohmann_json_FOUND)
        target_link_libraries(simulation_tests PRIVATE nlohmann_json::nlohmann_json)
    endif()
    
    # Include directories
    target_include_directories(simulation_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${GTEST_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/external/glad/include
    )
    
    # Set working directory for simulation tests
    set_target_properties(simulation_tests PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
    
    # Add simulation tests to CTest
    add_test(NAME SimulationTests COMMAND simulation_tests)
    
    # Define test category for each test
    set_property(TEST SimulationTests PROPERTY LABELS "MEDIUM;SIMULATION;UNIT")
endif()
