# Simulation tests CMakeLists.txt

# Only build submodule tests if the corresponding submodule is enabled
if(BUILD_SIMULATION_FEA)
    # FEA tests
    add_executable(simulation_fea_tests
        MaterialTests.cpp
        BoundaryConditionTests.cpp
        LoadTests.cpp
        FEASystemTests.cpp
    )
    
    target_link_libraries(simulation_fea_tests
        PRIVATE
        SimulationFEA
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
        target_link_libraries(simulation_fea_tests PRIVATE nlohmann_json::nlohmann_json)
    endif()
    
    # Include directories
    target_include_directories(simulation_fea_tests
        PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${GTEST_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/external/glad/include
    )
    
    # Set working directory for FEA tests
    set_target_properties(simulation_fea_tests PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
    
    # Add FEA tests to CTest
    add_test(NAME SimulationFEATests COMMAND simulation_fea_tests)
    
    # Define test category for each test
    set_property(TEST SimulationFEATests PROPERTY LABELS "MEDIUM;SIMULATION;SIMULATION_FEA;UNIT")
endif()

if(BUILD_SIMULATION_THERMAL)
    # Thermal tests
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ThermalTests.cpp")
        add_executable(simulation_thermal_tests
            ThermalTests.cpp
        )
        
        target_link_libraries(simulation_thermal_tests
            PRIVATE
            SimulationThermal
            core
            gtest
            gtest_main
            ${OPENGL_LIBRARIES}
        )
        
        # Include directories
        target_include_directories(simulation_thermal_tests
            PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${GTEST_INCLUDE_DIRS}
        )
        
        # Set working directory for Thermal tests
        set_target_properties(simulation_thermal_tests PROPERTIES
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        )
        
        # Add Thermal tests to CTest
        add_test(NAME SimulationThermalTests COMMAND simulation_thermal_tests)
        
        # Define test category for each test
        set_property(TEST SimulationThermalTests PROPERTY LABELS "MEDIUM;SIMULATION;SIMULATION_THERMAL;UNIT")
    endif()
endif()

if(BUILD_SIMULATION_DYNAMIC)
    # Dynamic tests
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/DynamicTests.cpp")
        add_executable(simulation_dynamic_tests
            DynamicTests.cpp
        )
        
        target_link_libraries(simulation_dynamic_tests
            PRIVATE
            SimulationDynamic
            core
            gtest
            gtest_main
        )
        
        # Include directories
        target_include_directories(simulation_dynamic_tests
            PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${GTEST_INCLUDE_DIRS}
        )
        
        # Set working directory for Dynamic tests
        set_target_properties(simulation_dynamic_tests PROPERTIES
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        )
        
        # Add Dynamic tests to CTest
        add_test(NAME SimulationDynamicTests COMMAND simulation_dynamic_tests)
        
        # Define test category for each test
        set_property(TEST SimulationDynamicTests PROPERTY LABELS "MEDIUM;SIMULATION;SIMULATION_DYNAMIC;UNIT")
    endif()
endif()

if(BUILD_SIMULATION_NONLINEAR)
    # NonLinear tests
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/NonLinearTests.cpp")
        add_executable(simulation_nonlinear_tests
            NonLinearTests.cpp
        )
        
        target_link_libraries(simulation_nonlinear_tests
            PRIVATE
            SimulationNonLinear
            core
            gtest
            gtest_main
        )
        
        # Include directories
        target_include_directories(simulation_nonlinear_tests
            PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${GTEST_INCLUDE_DIRS}
        )
        
        # Set working directory for NonLinear tests
        set_target_properties(simulation_nonlinear_tests PROPERTIES
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        )
        
        # Add NonLinear tests to CTest
        add_test(NAME SimulationNonLinearTests COMMAND simulation_nonlinear_tests)
        
        # Define test category for each test
        set_property(TEST SimulationNonLinearTests PROPERTY LABELS "MEDIUM;SIMULATION;SIMULATION_NONLINEAR;UNIT")
    endif()
endif()

# Add integration tests for simulation module
if(BUILD_INTEGRATION_TESTS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/integration/SimulationIntegrationTests.cpp")
        add_executable(simulation_integration_tests
            integration/SimulationIntegrationTests.cpp
        )
        
        target_link_libraries(simulation_integration_tests
            PRIVATE
            simulation
            core
            gtest
            gtest_main
        )
        
        # Include directories
        target_include_directories(simulation_integration_tests
            PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${GTEST_INCLUDE_DIRS}
        )
        
        # Set working directory for integration tests
        set_target_properties(simulation_integration_tests PROPERTIES
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        )
        
        # Add integration tests to CTest
        add_test(NAME SimulationIntegrationTests COMMAND simulation_integration_tests)
        
        # Define test category for each test
        set_property(TEST SimulationIntegrationTests PROPERTY LABELS "LARGE;SIMULATION;INTEGRATION")
    endif()
endif()

# Add benchmark tests for simulation module
if(BUILD_BENCHMARK_TESTS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/benchmark/SimulationBenchmarks.cpp")
        add_executable(simulation_benchmarks
            benchmark/SimulationBenchmarks.cpp
        )
        
        target_link_libraries(simulation_benchmarks
            PRIVATE
            simulation
            core
            benchmark::benchmark
            benchmark::benchmark_main
        )
        
        # Include directories
        target_include_directories(simulation_benchmarks
            PRIVATE
            ${CMAKE_SOURCE_DIR}/include
        )
        
        # Set working directory for benchmark tests
        set_target_properties(simulation_benchmarks PROPERTIES
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        )
        
        # Add benchmark tests to CTest
        add_test(NAME SimulationBenchmarks COMMAND simulation_benchmarks)
        
        # Define test category for each test
        set_property(TEST SimulationBenchmarks PROPERTY LABELS "LARGE;SIMULATION;BENCHMARK")
    endif()
endif()
