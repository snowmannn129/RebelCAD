# Dynamic submodule CMakeLists.txt

# Add source files for the Dynamic library
set(DYNAMIC_SOURCES
    DynamicSolver.cpp
)

# Add header files for the Dynamic library
set(DYNAMIC_HEADERS
    ${CMAKE_SOURCE_DIR}/include/simulation/DynamicSolver.h
)

# Create the Dynamic library
add_library(SimulationDynamic STATIC ${DYNAMIC_SOURCES} ${DYNAMIC_HEADERS})

# Set include directories for the Dynamic library
target_include_directories(SimulationDynamic PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Link against required libraries
target_link_libraries(SimulationDynamic PUBLIC
    core
    SimulationFEA
)

# Add the Dynamic test executable
add_executable(dynamic_solver_test DynamicSolverTest.cpp)

# Link the test executable with the Dynamic library
target_link_libraries(dynamic_solver_test PRIVATE
    SimulationDynamic
)

# Set properties for the test executable
set_target_properties(dynamic_solver_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Install the Dynamic library
install(TARGETS SimulationDynamic
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install the header files
install(FILES ${DYNAMIC_HEADERS}
    DESTINATION include/simulation
)

# Install the test executables
install(TARGETS dynamic_solver_test
    RUNTIME DESTINATION bin
)
