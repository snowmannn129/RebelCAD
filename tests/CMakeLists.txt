enable_testing()

# Add required headers
include_directories(${CMAKE_SOURCE_DIR}/external/glad/include)

# Find Google Test and Google Benchmark
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.11.0
)

FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
)

FetchContent_MakeAvailable(googletest benchmark)
include_directories(${GTEST_INCLUDE_DIRS})

# Add required definitions
if(MSVC)
    add_definitions(
        -D_USE_MATH_DEFINES
        -DGLAD_GLAPI_EXPORT
        -DGLAD_GLAPI_EXPORT_BUILD
    )
endif()

# Core tests executable
add_executable(core_tests
    core/LogTests.cpp
    core/ErrorTests.cpp
    core/MemoryPoolTests.cpp
    core/BlockAllocatorTests.cpp
    core/SmartResourceManagerTests.cpp
    core/MemoryLeakDetectorTests.cpp
    core/ComponentTests.cpp
    core/SmartPtrTests.cpp
    core/EventSystemTests.cpp
    core/FileIOTests.cpp
    core/ThreadingTests.cpp
    graphics/GraphicsSystemTests.cpp
    graphics/ShaderTests.cpp
    graphics/SceneNodeTest.cpp
    graphics/GraphicsErrorTest.cpp
    ui/MainWindowTests.cpp
    ui/WindowManagerTests.cpp
    ui/ContextMenuManagerTests.cpp
    ui/ToolbarManagerTests.cpp
    ui/WorkspaceManagerTests.cpp
    ui/DockingManagerTest.cpp
    ui/InputManagerTest.cpp
    ui/StyleManagerTest.cpp
    ui/widgets/ButtonTest.cpp
    ui/widgets/TextInputTest.cpp
    ui/widgets/CheckboxTest.cpp
    ui/widgets/RadioButtonTest.cpp
    ui/widgets/DropdownTest.cpp
    ui/widgets/ListViewTest.cpp
    ui/layouts/GridLayoutTest.cpp
    ui/layouts/FlowLayoutTest.cpp
    ui/layouts/StackLayoutTest.cpp
    ui/layouts/DockLayoutTest.cpp
    ui/layouts/FlexLayoutTest.cpp
    sketching/LineTests.cpp
    sketching/ArcTests.cpp
    sketching/CircleTests.cpp
    constraints/ParallelConstraintTests.cpp
    constraints/PerpendicularConstraintTests.cpp
    constraints/TangentConstraintTests.cpp
    constraints/ConstraintManagerUITests.cpp
    modeling/NurbsSurfaceTests.cpp
    modeling/HalfEdgeMeshTests.cpp
    modeling/PatchSurfaceTests.cpp
    modeling/SubdivisionSurfaceTests.cpp
    modeling/SubdivisionBooleanOperationsTests.cpp
    modeling/BooleanToolTests.cpp
    modeling/PatternToolTests.cpp
    modeling/HalfEdgeTests.cpp
    modeling/patterns/PatternSpacingTests.cpp
    modeling/patterns/CurvePatternTests.cpp
    assembly/ComponentPositionerTests.cpp
    assembly/FixedJointTests.cpp
    assembly/RevoluteJointTests.cpp
    assembly/PrismaticJointTests.cpp
    ui/PatternToolDialogTests.cpp
)

# Find OpenGL
find_package(OpenGL REQUIRED)

target_link_libraries(core_tests
    PRIVATE
    RebelCADCore
    gtest
    gtest_main
    ${OPENGL_LIBRARIES}
    glfw
    glad
    imgui
    nlohmann_json::nlohmann_json
    glm
    dbghelp
    libzstd_static
)

if(Vulkan_FOUND)
    target_link_libraries(core_tests PRIVATE Vulkan::Vulkan ${SHADERC_LIB})
endif()

# Enable memory tracking in tests
target_compile_definitions(core_tests PRIVATE
    $<$<CONFIG:Debug>:REBEL_MEMORY_TRACKING_ENABLED>
)

# Include directories
target_include_directories(core_tests
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${GTEST_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/external/glad/include
)

if(Vulkan_FOUND)
    target_include_directories(core_tests PRIVATE
        ${VULKAN_SDK_PATH}/Include
        ${SHADERC_INCLUDE_DIR}
    )
endif()


# Set working directory for tests
set_target_properties(core_tests PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Add tests to CTest
add_test(NAME CoreTests COMMAND core_tests)

# Memory benchmarks executable
add_executable(memory_benchmarks
    core/MemoryBenchmarks.cpp
)

target_link_libraries(memory_benchmarks
    PRIVATE
    RebelCADCore
    benchmark::benchmark
    benchmark::benchmark_main
)

target_include_directories(memory_benchmarks
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Set working directory for benchmarks
set_target_properties(memory_benchmarks PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# UI benchmarks executable
add_executable(ui_benchmarks
    ui/widgets/ListViewBenchmarks.cpp
)

target_link_libraries(ui_benchmarks
    PRIVATE
    RebelCADCore
    benchmark::benchmark
    benchmark::benchmark_main
    imgui
    glfw
    glad
    nlohmann_json::nlohmann_json
)

target_include_directories(ui_benchmarks
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/glad/include
)

# Set working directory for UI benchmarks
set_target_properties(ui_benchmarks PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Add UI benchmarks to CTest with configuration
add_test(
    NAME UIBenchmarks 
    COMMAND ui_benchmarks
    CONFIGURATIONS Debug Release
)
set_tests_properties(UIBenchmarks PROPERTIES
    ENVIRONMENT "GTEST_COLOR=1"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
)
