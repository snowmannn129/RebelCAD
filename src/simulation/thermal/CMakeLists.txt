# Thermal submodule CMakeLists.txt

# Add source files for the Thermal library
set(THERMAL_SOURCES
    ThermalSolver.cpp
    ThermalSolverCore.cpp
    ThermalSolverElements.cpp
    ThermalSolverShapeFunctions.cpp
    ThermalSolverBoundaryConditions.cpp
    ThermalSolverSolvers.cpp
    ThermalSolverResults.cpp
    ThermalSolverAssembly.cpp
    ThermalSolverTimeIntegration.cpp
    ThermalBoundaryConditions.cpp
    ThermalLoads.cpp
    ThermalResult.cpp
)

# Add header files for the Thermal library
set(THERMAL_HEADERS
    ${CMAKE_SOURCE_DIR}/include/simulation/ThermalSolver.h
    ${CMAKE_SOURCE_DIR}/include/simulation/ThermalBoundaryConditions.h
    ${CMAKE_SOURCE_DIR}/include/simulation/ThermalLoads.h
    ${CMAKE_SOURCE_DIR}/include/simulation/ThermalResult.h
)

# Create the Thermal library
add_library(SimulationThermal STATIC ${THERMAL_SOURCES} ${THERMAL_HEADERS})

# Set include directories for the Thermal library
target_include_directories(SimulationThermal PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Link against required libraries
target_link_libraries(SimulationThermal PUBLIC
    core
    SimulationFEA
)

# Add the Thermal test executable
add_executable(thermal_test ThermalTest.cpp)

# Link the test executable with the Thermal library
target_link_libraries(thermal_test PRIVATE
    SimulationThermal
)

# Set properties for the test executable
set_target_properties(thermal_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Install the Thermal library
install(TARGETS SimulationThermal
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install the header files
install(FILES ${THERMAL_HEADERS}
    DESTINATION include/simulation
)

# Install the test executables
install(TARGETS thermal_test
    RUNTIME DESTINATION bin
)
