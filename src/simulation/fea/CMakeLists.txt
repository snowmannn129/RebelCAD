# FEA submodule CMakeLists.txt

# Add source files for the FEA library
set(FEA_SOURCES
    FEASystem.cpp
    Mesh.cpp
    BoundaryCondition.cpp
    Load.cpp
    Material.cpp
    # FEAResult implementation split into multiple files for better maintainability
    FEAResultStructures.cpp
    FEAResultCore.cpp
    FEAResultAccessors.cpp
    FEAResultStatistics.cpp
    FEAResultIO.cpp
    FEAResultModifiers.cpp
    FEAResultComputation.cpp
)

# Add header files for the FEA library
set(FEA_HEADERS
    ${CMAKE_SOURCE_DIR}/include/simulation/FEASystem.h
    ${CMAKE_SOURCE_DIR}/include/simulation/Mesh.h
    ${CMAKE_SOURCE_DIR}/include/simulation/BoundaryCondition.h
    ${CMAKE_SOURCE_DIR}/include/simulation/Load.h
    ${CMAKE_SOURCE_DIR}/include/simulation/Material.h
    ${CMAKE_SOURCE_DIR}/include/simulation/FEAResult.h
)

# Create the FEA library
add_library(SimulationFEA STATIC ${FEA_SOURCES} ${FEA_HEADERS})

# Set include directories for the FEA library
target_include_directories(SimulationFEA PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Link against required libraries
target_link_libraries(SimulationFEA PUBLIC
    core
)

# Add the FEA test executable
add_executable(fea_test FEATest.cpp)

# Link the test executable with the FEA library
target_link_libraries(fea_test PRIVATE
    SimulationFEA
)

# Set properties for the test executable
set_target_properties(fea_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Add the FEA system simple test executable
add_executable(fea_system_simple_test FEASystemSimpleTest.cpp)

# Link the simple test executable with the FEA library
target_link_libraries(fea_system_simple_test PRIVATE
    SimulationFEA
)

# Set properties for the simple test executable
set_target_properties(fea_system_simple_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Install the FEA library
install(TARGETS SimulationFEA
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install the header files
install(FILES ${FEA_HEADERS}
    DESTINATION include/simulation
)

# Install the test executables
install(TARGETS fea_test fea_system_simple_test
    RUNTIME DESTINATION bin
)
