# AI-Assisted Development with RebelCAD's Modular CMake Structure

This guide explains how to use RebelCAD's modular CMake structure for AI-assisted development and debugging. The structure allows building and testing individual components independently, which is particularly useful for AI-assisted development.

## Building Specific Modules

You can build specific modules by setting the corresponding CMake options:

```bash
# Configure with specific modules enabled
cmake -B build -DBUILD_SIMULATION=ON -DBUILD_MODELING=OFF -DBUILD_UI=OFF

# Build only the simulation module
cmake --build build --target simulation
```

## Building Specific Submodules

For more granular control, you can build specific submodules:

```bash
# Configure with specific submodules enabled
cmake -B build -DBUILD_SIMULATION=ON -DBUILD_SIMULATION_FEA=ON -DBUILD_SIMULATION_THERMAL=OFF

# Build only the FEA submodule
cmake --build build --target SimulationFEA
```

## Running Specific Tests

You can run specific tests using CTest:

```bash
# Run all tests
ctest --test-dir build

# Run only simulation tests
ctest --test-dir build -L SIMULATION

# Run only FEA tests
ctest --test-dir build -L SIMULATION_FEA

# Run only unit tests
ctest --test-dir build -L UNIT

# Run only integration tests
ctest --test-dir build -L INTEGRATION

# Run only benchmark tests
ctest --test-dir build -L BENCHMARK
```

## AI Debugging Workflow

Here's a typical workflow for AI-assisted debugging:

1. Identify the specific component to modify
2. Build only that component:
   ```bash
   cmake --build build --target SimulationFEA
   ```
3. Run tests for that component:
   ```bash
   ctest --test-dir build -R "SimulationFEATests"
   ```
4. Make changes to the component
5. Rebuild and retest:
   ```bash
   cmake --build build --target SimulationFEA
   ctest --test-dir build -R "SimulationFEATests"
   ```
6. Once the component works correctly, run integration tests:
   ```bash
   ctest --test-dir build -L "INTEGRATION"
   ```

## Available Modules and Submodules

### Core Module
- Always built (required)

### Simulation Module (`BUILD_SIMULATION`)
- FEA Submodule (`BUILD_SIMULATION_FEA`)
- Thermal Submodule (`BUILD_SIMULATION_THERMAL`)
- Dynamic Submodule (`BUILD_SIMULATION_DYNAMIC`)
- NonLinear Submodule (`BUILD_SIMULATION_NONLINEAR`)

### Modeling Module (`BUILD_MODELING`)
- Geometry Submodule (`BUILD_MODELING_GEOMETRY`)
- Topology Submodule (`BUILD_MODELING_TOPOLOGY`)
- Operations Submodule (`BUILD_MODELING_OPERATIONS`)

### UI Module (`BUILD_UI`)
- Widgets Submodule (`BUILD_UI_WIDGETS`)
- Dialogs Submodule (`BUILD_UI_DIALOGS`)
- Viewport Submodule (`BUILD_UI_VIEWPORT`)

### Other Modules
- Sketching Module (`BUILD_SKETCHING`)
- Assembly Module (`BUILD_ASSEMBLY`)
- Constraints Module (`BUILD_CONSTRAINTS`)
- Graphics Module (`BUILD_GRAPHICS`)

## Test Categories

Tests are categorized by:

- Size: `SMALL`, `MEDIUM`, `LARGE`
- Type: `UNIT`, `INTEGRATION`, `BENCHMARK`
- Module: `SIMULATION`, `CORE`, `MODELING`, etc.
- Submodule: `SIMULATION_FEA`, `SIMULATION_THERMAL`, etc.

You can use these categories with CTest's `-L` option to run specific test groups.

## Example: Debugging the FEA System

```bash
# Configure with only FEA enabled
cmake -B build -DBUILD_SIMULATION=ON -DBUILD_SIMULATION_FEA=ON -DBUILD_SIMULATION_THERMAL=OFF -DBUILD_SIMULATION_DYNAMIC=OFF -DBUILD_SIMULATION_NONLINEAR=OFF -DBUILD_MODELING=OFF -DBUILD_UI=OFF -DBUILD_SKETCHING=OFF -DBUILD_ASSEMBLY=OFF -DBUILD_CONSTRAINTS=OFF -DBUILD_GRAPHICS=OFF

# Build only the FEA submodule
cmake --build build --target SimulationFEA

# Run only FEA tests
ctest --test-dir build -R "SimulationFEATests"
```

This approach allows for faster iteration cycles during development and debugging, as you only need to build and test the specific component you're working on.
