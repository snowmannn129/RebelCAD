# RebelCAD CMake Strategy for AI-Assisted Development

## Overview

This document outlines the CMake strategy for RebelCAD that enables AI-assisted development and debugging. The key goals are:

1. Allow building and testing individual modules or functions independently
2. Avoid building the entire program for every change
3. Keep everything modular, testable, and easy to integrate
4. Enable AI to focus on specific parts of the codebase

## Implementation Strategy

### 1. Library-Based Architecture

Each module and submodule will be implemented as its own CMake target using `add_library`:

- **Core Libraries**: Essential functionality shared across the system
- **Module Libraries**: Major functional areas (simulation, modeling, UI, etc.)
- **Submodule Libraries**: Smaller, focused components within modules

This approach allows:
- Building individual pieces independently
- Reusing components across executables/tests
- Reducing build scope for debugging

### 2. Granular Build Options

CMake options will control which components are built:

- **Module-level options**: Control entire modules (e.g., `BUILD_SIMULATION`)
- **Submodule-level options**: Control specific components (e.g., `BUILD_SIMULATION_FEA`)
- **Feature-level options**: Control specific features (e.g., `BUILD_SIMULATION_THERMAL`)

### 3. Test Executables Per Component

Each component will have dedicated test executables:

- **Unit tests**: Test individual functions and classes
- **Integration tests**: Test interactions between components
- **Benchmark tests**: Measure performance

### 4. CTest Integration

All tests will be registered with CTest, allowing:

- Running specific test categories
- Filtering tests by name or label
- Generating test reports

### 5. Object Libraries for Fine-Grained Control

For components that require fine-grained control, we'll use OBJECT libraries:

- Compile source files without linking
- Combine object files into larger libraries as needed
- Avoid redundant compilation

## Directory Structure

```
RebelCAD/
├── CMakeLists.txt                  # Top-level CMake file
├── include/                        # Public headers
│   ├── core/                       # Core headers
│   ├── simulation/                 # Simulation headers
│   └── ...
├── src/                            # Source code
│   ├── core/                       # Core implementation
│   │   ├── CMakeLists.txt          # Core module CMake
│   │   └── ...
│   ├── simulation/                 # Simulation implementation
│   │   ├── CMakeLists.txt          # Simulation module CMake
│   │   ├── fea/                    # FEA submodule
│   │   │   ├── CMakeLists.txt      # FEA submodule CMake
│   │   │   └── ...
│   │   ├── thermal/                # Thermal submodule
│   │   │   ├── CMakeLists.txt      # Thermal submodule CMake
│   │   │   └── ...
│   │   └── ...
│   └── ...
└── tests/                          # Tests
    ├── CMakeLists.txt              # Tests CMake
    ├── core/                       # Core tests
    │   ├── CMakeLists.txt          # Core tests CMake
    │   └── ...
    ├── simulation/                 # Simulation tests
    │   ├── CMakeLists.txt          # Simulation tests CMake
    │   ├── fea/                    # FEA tests
    │   │   ├── CMakeLists.txt      # FEA tests CMake
    │   │   └── ...
    │   └── ...
    └── ...
```

## AI Debugging Workflow

1. AI identifies the specific component to modify
2. AI builds and tests only that component:
   ```bash
   cmake --build build --target SimulationFEA
   cmake --build build --target SimulationFEATests
   ctest -R "SimulationFEA"
   ```
3. AI verifies changes work correctly before integrating with the larger system
4. AI can run integration tests to ensure compatibility:
   ```bash
   ctest -L "INTEGRATION"
   ```

## Implementation Plan

1. Restructure the top-level CMakeLists.txt to add submodule options
2. Reorganize source directories to match the new structure
3. Create CMakeLists.txt files for each submodule
4. Update test structure to match the new organization
5. Add CTest labels for better test filtering
