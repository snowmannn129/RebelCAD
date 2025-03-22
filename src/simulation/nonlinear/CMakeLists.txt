# NonLinear submodule CMakeLists.txt

# Add source files for the NonLinear library
set(NONLINEAR_SOURCES
    NonLinearSolver.cpp
)

# Add header files for the NonLinear library
set(NONLINEAR_HEADERS
    ${CMAKE_SOURCE_DIR}/include/simulation/NonLinearSolver.h
)

# Create the NonLinear library
add_library(SimulationNonLinear STATIC ${NONLINEAR_SOURCES} ${NONLINEAR_HEADERS})

# Set include directories for the NonLinear library
target_include_directories(SimulationNonLinear PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Link against required libraries
target_link_libraries(SimulationNonLinear PUBLIC
    core
    SimulationFEA
)

# Add the NonLinear test executable
add_executable(non_linear_solver_test NonLinearSolverTest.cpp)

# Link the test executable with the NonLinear library
target_link_libraries(non_linear_solver_test PRIVATE
    SimulationNonLinear
)

# Set properties for the test executable
set_target_properties(non_linear_solver_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Install the NonLinear library
install(TARGETS SimulationNonLinear
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install the header files
install(FILES ${NONLINEAR_HEADERS}
    DESTINATION include/simulation
)

# Install the test executables
install(TARGETS non_linear_solver_test
    RUNTIME DESTINATION bin
)
