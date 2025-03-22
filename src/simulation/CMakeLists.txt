# Simulation module CMakeLists.txt

# Add source files for the LinearStaticSolver
set(LINEAR_STATIC_SOURCES
    LinearStaticSolver.cpp
)

# Add header files for the LinearStaticSolver
set(LINEAR_STATIC_HEADERS
    ${CMAKE_SOURCE_DIR}/include/simulation/LinearStaticSolver.h
)

# Create the simulation meta-library
add_library(simulation INTERFACE)

# Add submodules based on build options
if(BUILD_SIMULATION_FEA)
    add_subdirectory(fea)
    target_link_libraries(simulation INTERFACE SimulationFEA)
endif()

if(BUILD_SIMULATION_THERMAL)
    add_subdirectory(thermal)
    target_link_libraries(simulation INTERFACE SimulationThermal)
endif()

if(BUILD_SIMULATION_DYNAMIC)
    add_subdirectory(dynamic)
    target_link_libraries(simulation INTERFACE SimulationDynamic)
endif()

if(BUILD_SIMULATION_NONLINEAR)
    add_subdirectory(nonlinear)
    target_link_libraries(simulation INTERFACE SimulationNonLinear)
endif()

# Create the LinearStaticSolver library
add_library(SimulationLinearStatic STATIC ${LINEAR_STATIC_SOURCES} ${LINEAR_STATIC_HEADERS})

# Set include directories for the LinearStaticSolver library
target_include_directories(SimulationLinearStatic PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Link against required libraries
target_link_libraries(SimulationLinearStatic PUBLIC
    core
    SimulationFEA
)

# Add LinearStaticSolver to the simulation meta-library
target_link_libraries(simulation INTERFACE SimulationLinearStatic)

# Install the LinearStaticSolver library
install(TARGETS SimulationLinearStatic
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install the header files
install(FILES ${LINEAR_STATIC_HEADERS}
    DESTINATION include/simulation
)
