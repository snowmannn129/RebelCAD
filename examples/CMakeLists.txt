# Examples CMakeLists.txt

# Add the NonLinearSolverExample executable
add_executable(non_linear_solver_example NonLinearSolverExample.cpp)

# Link against the simulation library
target_link_libraries(non_linear_solver_example PRIVATE
    simulation
)

# Set properties for the example executable
set_target_properties(non_linear_solver_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Install the example executable
install(TARGETS non_linear_solver_example
    RUNTIME DESTINATION bin
)
