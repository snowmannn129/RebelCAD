# RebelCAD

## Overview

RebelCAD is a comprehensive CAD/3D modeling software component of the RebelSUITE ecosystem. It provides powerful tools for engineering design, 3D modeling, and technical drawing.

## Features

- **3D Modeling**: Create and manipulate complex 3D models
- **Technical Drawing**: Generate precise technical drawings
- **Assembly Design**: Design and manage multi-part assemblies
- **Parametric Modeling**: Create models with parametric constraints
- **Simulation**: Perform basic structural and thermal simulations
- **Import/Export**: Support for standard CAD file formats

## Technology Stack

- **Core**: C++ with modern C++17/20 features
- **Graphics**: OpenGL/DirectX for rendering
- **UI**: Custom UI framework with Qt integration
- **Math**: Robust computational geometry library
- **Physics**: Integrated physics simulation engine

## Directory Structure

```
RebelCAD/
├── src/                 # Source code
├── include/             # Header files
├── tests/               # Unit and integration tests
├── docs/                # Documentation
├── external/            # External dependencies
├── CMakeLists.txt       # Build configuration
├── .github/             # GitHub workflows and templates
├── .gitignore           # Git ignore file
└── README.md            # This file
```

## Integration with RebelSUITE

RebelCAD is designed to work seamlessly with other RebelSUITE components:

- **RebelENGINE**: Export models for use in game development
- **RebelCODE**: Script-driven model generation and automation
- **RebelFLOW**: Workflow automation for CAD operations
- **RebelDESK**: Integrated development environment for CAD scripting

## Building from Source

### Prerequisites

- CMake 3.15+
- C++17 compatible compiler (MSVC 2019+, GCC 9+, or Clang 10+)
- OpenGL 4.5+ or DirectX 11+
- Qt 6.2+ (optional, for advanced UI features)

### Build Steps

```bash
# Clone the repository
git clone https://github.com/snowmannn129/RebelCAD.git
cd RebelCAD

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
cmake --build . --config Release

# Run tests
ctest -C Release
```

## Contributing

Contributions to RebelCAD are welcome! Please see our [Contributing Guide](.github/CONTRIBUTING.md) for more information.

## License

RebelCAD is licensed under the [MIT License](LICENSE).

## Contact

For questions or support, please contact the RebelSUITE team at [support@rebelsuite.com](mailto:support@rebelsuite.com).
