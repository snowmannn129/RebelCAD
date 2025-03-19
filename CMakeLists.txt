cmake_minimum_required(VERSION 3.20)
project(RebelCAD VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
include(FetchContent)

# Find optional packages
find_package(OpenSSL)
find_package(ZLIB)
find_package(Vulkan)

if(NOT Vulkan_FOUND)
    message(WARNING "Vulkan not found - some features will be disabled")
endif()

if(NOT OPENSSL_FOUND)
    message(WARNING "OpenSSL not found - some features will be disabled")
endif()

if(NOT ZLIB_FOUND)
    message(WARNING "ZLIB not found - some features will be disabled")
endif()

# zstd
FetchContent_Declare(
    zstd
    GIT_REPOSITORY https://github.com/facebook/zstd.git
    GIT_TAG v1.5.5
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
)
FetchContent_GetProperties(zstd)
if(NOT zstd_POPULATED)
    FetchContent_Populate(zstd)
    add_subdirectory(${zstd_SOURCE_DIR}/build/cmake ${zstd_BINARY_DIR})
endif()

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
FetchContent_MakeAvailable(glfw)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)

# JSON
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

# spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0
)
FetchContent_MakeAvailable(spdlog)

# Google Test
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Google Benchmark
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
)
set(BENCHMARK_ENABLE_TESTING OFF)
FetchContent_MakeAvailable(benchmark)

# GLAD
add_library(glad STATIC
    external/glad/src/glad.c
)
target_include_directories(glad PUBLIC
    external/glad/include
)

# ImGui
add_library(imgui STATIC
    external/imgui/imgui.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends
)

if(Vulkan_FOUND)
    target_include_directories(imgui PUBLIC ${Vulkan_INCLUDE_DIRS})
endif()
target_link_libraries(imgui PUBLIC
    glfw
    glad
)

# Compiler options
if(MSVC)
    add_compile_options(/W4 /WX)
    add_definitions(
        -D_USE_MATH_DEFINES
        -DGLAD_GLAPI_EXPORT
        -DGLAD_GLAPI_EXPORT_BUILD
    )
else()
    add_compile_options(-Wall -Wextra -Werror)
endif()

# RebelCAD Core Library
add_library(RebelCADCore STATIC
    src/core/BlockAllocator.cpp
    src/core/EventSystem.cpp
    src/core/FileIO.cpp
    src/core/Log.cpp
    src/graphics/CameraController.cpp
    src/graphics/Color.cpp
    src/graphics/Frustum.cpp
    src/graphics/GraphicsSystem.cpp
    src/graphics/ObjectPicker.cpp
    src/graphics/OpenGLShader.cpp
    src/graphics/PreviewRenderer.cpp
    src/graphics/RayCasting.cpp
    src/graphics/SceneGraph.cpp
    src/graphics/SceneNode.cpp
    src/graphics/SelectionManager.cpp
    src/graphics/Shader.cpp
    src/graphics/SpatialPartitioning.cpp
    src/graphics/Viewport.cpp
    src/graphics/ViewportLayout.cpp
    src/graphics/ViewSyncManager.cpp
    src/graphics/VulkanShader.cpp
    src/ui/PatternToolDialog.cpp
    src/ui/DockingManager.cpp
    src/ui/InputManager.cpp
    src/ui/StyleManager.cpp
    src/ui/widgets/Button.cpp
    src/ui/widgets/TextInput.cpp
    src/ui/widgets/Checkbox.cpp
    src/ui/widgets/RadioButton.cpp
    src/ui/widgets/Dropdown.cpp
    src/ui/layouts/GridLayout.cpp
    src/ui/layouts/FlowLayout.cpp
    src/ui/layouts/StackLayout.cpp
    src/ui/layouts/DockLayout.cpp
    src/ui/layouts/FlexLayout.cpp
    src/modeling/HalfEdge.cpp
    src/modeling/PatternTool.cpp
    src/modeling/patterns/PatternSpacing.cpp
)

# Enable debug symbols
set_target_properties(RebelCADCore PROPERTIES
    DEBUG_POSTFIX "d"
    COMPILE_PDB_NAME "RebelCADCore"
    COMPILE_PDB_NAME_DEBUG "RebelCADCored"
)
target_include_directories(RebelCADCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends
    ${json_SOURCE_DIR}/include
    ${zstd_SOURCE_DIR}/lib
    ${zstd_SOURCE_DIR}/lib/zstd
)

# Enable memory tracking in debug builds
target_compile_definitions(RebelCADCore PUBLIC
    $<$<CONFIG:Debug>:REBEL_MEMORY_TRACKING_ENABLED>
    $<$<CONFIG:Debug>:REBEL_DEBUG>
)

if(Vulkan_FOUND)
    target_include_directories(RebelCADCore PUBLIC ${Vulkan_INCLUDE_DIRS})
endif()

if(OPENSSL_FOUND)
    target_include_directories(RebelCADCore PUBLIC ${OPENSSL_INCLUDE_DIR})
    target_link_libraries(RebelCADCore PUBLIC OpenSSL::SSL OpenSSL::Crypto)
    target_compile_definitions(RebelCADCore PUBLIC HAVE_OPENSSL)
endif()

target_link_libraries(RebelCADCore PUBLIC
    glm::glm
    nlohmann_json::nlohmann_json
    libzstd_static
    glad
    glfw
    spdlog::spdlog
)

if(ZLIB_FOUND)
    target_link_libraries(RebelCADCore PUBLIC ZLIB::ZLIB)
    target_compile_definitions(RebelCADCore PUBLIC HAVE_ZLIB)
endif()

# Enable GLM experimental features
target_compile_definitions(RebelCADCore PUBLIC GLM_ENABLE_EXPERIMENTAL)

# Main Application
add_executable(RebelCAD
    src/main.cpp
)
target_link_libraries(RebelCAD PRIVATE
    RebelCADCore
    imgui
)

# Tests
enable_testing()
add_subdirectory(tests)

# Install rules
install(TARGETS RebelCAD
    RUNTIME DESTINATION bin
)
