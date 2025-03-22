# RebelCAD - Advanced CAD System

RebelCAD is an advanced CAD system that provides powerful tools for 3D modeling, simulation, and analysis. This repository contains the core components of RebelCAD, including the Finite Element Analysis (FEA) system.

```
  _____      _          _  _____          _____  
 |  __ \    | |        | |/ ____|   /\   |  __ \ 
 | |__) |___| |__   ___| | |       /  \  | |  | |
 |  _  // _ \ '_ \ / _ \ | |      / /\ \ | |  | |
 | | \ \  __/ |_) |  __/ | |____ / ____ \| |__| |
 |_|  \_\___|_.__/ \___|_|\_____/_/    \_\_____/ 
                                                 
 Advanced CAD System - FEA Simulation Engine
```

## Features

- **3D Modeling**: Create and edit complex 3D models
- **Simulation**: Analyze the behavior of models under various conditions
- **Finite Element Analysis**: Perform structural analysis on 3D models
- **Material Library**: Access a wide range of engineering materials
- **Result Visualization**: View and analyze simulation results

## Finite Element Analysis (FEA) System

The FEA system is a core component of RebelCAD that provides structural analysis capabilities. It allows users to analyze the mechanical behavior of 3D models under various loading conditions.

### FEA Features

- Linear static analysis
- Support for various element types (tetrahedral, hexahedral)
- Material library with common engineering materials
- Boundary conditions (fixed, displacement)
- Loads (point force, pressure, body force)
- Result visualization (displacements, stresses, strains)

For more details, see the [FEA System Documentation](docs/simulation/FEA_System.md).

## Building RebelCAD

### Prerequisites

- CMake 3.15 or higher
- C++17 compatible compiler (MSVC, GCC, Clang)
- Visual Studio 2022 (Windows) or equivalent IDE
- PowerShell 5.0+ (for build scripts on Windows)

### Building on Windows

#### Using the Build Tools Menu

The easiest way to build RebelCAD is to use the build tools menu:

```
build_tools.bat
```

This will open a menu-driven interface that allows you to:
- Build specific components
- Run tests
- Debug incrementally
- Manage build logs
- And more

#### Using Build Scripts

For more control, you can use the PowerShell build scripts directly:

```powershell
# Build a specific component
.\scripts\build_component.ps1 -Component simulation -BuildType Debug

# Build only modified files
.\scripts\rebuild_modified.ps1 -Component simulation

# Filter build logs
.\scripts\filter_logs.ps1 -BuildCommand "cmake --build build --config Debug"
```

See [scripts/README.md](scripts/README.md) for more details on the available scripts.

#### Manual Build

You can also build manually:

```
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A win32
cmake --build . --config Release
```

### Building on Linux/macOS

1. Clone the repository
2. Run the build script:
   ```
   ./build.sh
   ```
   
   Or manually:
   ```
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```

### Modular CMake Structure for AI-Assisted Development

RebelCAD uses a modular CMake structure that allows building and testing individual components independently. This is particularly useful for AI-assisted development and debugging.

#### Building Specific Modules

You can build specific modules by setting the corresponding CMake options:

```bash
# Configure with specific modules enabled
cmake -B build -DBUILD_SIMULATION=ON -DBUILD_MODELING=OFF -DBUILD_UI=OFF

# Build only the simulation module
cmake --build build --target simulation
```

#### Building Specific Submodules

For more granular control, you can build specific submodules:

```bash
# Configure with specific submodules enabled
cmake -B build -DBUILD_SIMULATION=ON -DBUILD_SIMULATION_FEA=ON -DBUILD_SIMULATION_THERMAL=OFF

# Build only the FEA submodule
cmake --build build --target SimulationFEA
```

#### Running Specific Tests

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
```

#### AI-Assisted Development Workflow

RebelCAD includes scripts to streamline the AI-assisted development workflow:

```bash
# Windows
ai_debug.bat -module simulation -submodule fea -clean

# Linux/macOS
./ai_debug.sh --module simulation --submodule fea --clean
```

For more details, see the [AI-Assisted Development](docs/AI_Assisted_Development.md) guide.

## Running the FEA Test

After building the project, you can run the FEA test program to see the FEA system in action:

### Windows
```
bin\Release\fea_test.exe
```

### Linux/macOS
```
./bin/fea_test
```

The test program creates a simple mesh, applies materials, boundary conditions, and loads, then solves the FEA problem and displays the results.

## Project Structure

```
RebelCAD/
├── include/              # Header files
│   ├── core/             # Core functionality
│   └── simulation/       # Simulation and FEA
├── src/                  # Source files
│   ├── core/             # Core implementation
│   └── simulation/       # Simulation and FEA implementation
│       ├── fea/          # FEA submodule
│       ├── thermal/      # Thermal analysis submodule
│       ├── dynamic/      # Dynamic analysis submodule
│       └── nonlinear/    # Non-linear analysis submodule
├── docs/                 # Documentation
│   ├── BuildAndDebugStrategy.md  # Build and debug strategy documentation
│   ├── CMake_Strategy.md         # CMake modular structure strategy
│   └── AI_Assisted_Development.md # AI-assisted development guide
├── scripts/              # Build and debug scripts
│   ├── build_component.ps1       # Build specific components
│   ├── filter_logs.ps1           # Filter and manage build logs
│   ├── analyze_log_chunks.ps1    # Analyze large log files
│   ├── test_component.ps1        # Run tests for specific components
│   ├── debug_incremental.ps1     # Debug tests incrementally
│   ├── run_tests_by_category.ps1 # Run tests by category
│   ├── rotate_logs.ps1           # Rotate and archive logs
│   ├── ai_debug_workflow.ps1     # AI-assisted debugging workflow
│   └── README.md                 # Script documentation
├── tests/                # Test files
│   ├── simulation/       # Simulation tests
│   │   ├── fea/          # FEA tests
│   │   ├── thermal/      # Thermal analysis tests
│   │   ├── dynamic/      # Dynamic analysis tests
│   │   └── nonlinear/    # Non-linear analysis tests
│   ├── integration/      # Integration tests
│   └── benchmark/        # Benchmark tests
├── build_tools.bat       # Build tools menu interface
├── build.bat             # Windows build script
├── build.sh              # Linux/macOS build script
├── ai_debug.bat          # AI-assisted debugging batch file
├── ai_debug.sh           # AI-assisted debugging shell script
└── CMakeLists.txt        # Main CMake configuration
```

## Debugging and Testing

RebelCAD includes a comprehensive strategy for debugging and testing, with a focus on handling large error logs and outputs.

### Testing

Tests are categorized as SMALL, MEDIUM, or LARGE, and can be run selectively:

```powershell
# Run all simulation tests
.\scripts\test_component.ps1 -Component simulation

# Run only small tests
.\scripts\run_tests_by_category.ps1 -Category SMALL
```

### Debugging

For debugging complex issues, RebelCAD provides tools for incremental debugging:

```powershell
# Debug tests incrementally, saving state
.\scripts\debug_incremental.ps1 -Component simulation -SaveState
```

### Log Management

Large build logs can be managed with specialized tools:

```powershell
# Analyze a large log file in chunks
.\scripts\analyze_log_chunks.ps1 -LogFile "logs/build_log.txt"

# Rotate and archive logs
.\scripts\rotate_logs.ps1 -Compress
```

For more details, see the [Build and Debug Strategy](docs/BuildAndDebugStrategy.md) document.

## Contributing

Contributions to RebelCAD are welcome! Please follow the standard GitHub workflow:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

## License

RebelCAD is licensed under the MIT License. See the LICENSE file for details.
