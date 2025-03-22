# Finite Element Analysis (FEA) System

The FEA system is a core component of RebelCAD that provides structural analysis capabilities. It allows users to analyze the mechanical behavior of 3D models under various loading conditions.

## Overview

The FEA system consists of several key components:

- **FEASystem**: The main entry point for the FEA functionality. It manages the overall simulation process.
- **Material**: Defines the material properties used in the analysis.
- **Mesh**: Represents the discretized geometry for analysis.
- **BoundaryCondition**: Defines constraints on the model.
- **Load**: Defines forces and pressures applied to the model.
- **FEAResult**: Contains the results of the analysis.

## Features

- Linear static analysis with a robust finite element solver
- Support for various element types (tetrahedral, hexahedral)
- Material library with common engineering materials
- Boundary conditions (fixed, displacement)
- Loads (point force, pressure, body force)
- Result visualization (displacements, stresses, strains)
- Efficient sparse matrix representation and solution

## Building the FEA System

The FEA system is built as part of the RebelCAD application. To build it:

1. Make sure you have CMake (version 3.15 or higher) and a C++17 compatible compiler installed.
2. Navigate to the RebelCAD root directory.
3. Create a build directory:
   ```bash
   mkdir build
   cd build
   ```
4. Configure the project:
   ```bash
   cmake ..
   ```
5. Build the project:
   ```bash
   cmake --build .
   ```

## Running the FEA Test

The FEA system includes a test program that demonstrates its functionality. To run it:

1. Build the project as described above.
2. Run the test program:
   ```bash
   ./bin/fea_test
   ```

The test program creates a simple mesh, applies materials, boundary conditions, and loads, then solves the FEA problem and displays the results.

## Using the FEA System

Here's a simple example of how to use the FEA system in your code:

```cpp
#include "simulation/FEASystem.h"
#include "simulation/Material.h"
#include "simulation/Mesh.h"
#include "simulation/BoundaryCondition.h"
#include "simulation/Load.h"
#include "simulation/FEAResult.h"

using namespace rebel::simulation;

// Initialize the FEA system
auto& feaSystem = FEASystem::getInstance();
feaSystem.initialize();

// Set FEA settings
FEASettings settings;
settings.solverType = SolverType::Linear;
settings.elementType = ElementType::Tetra;
settings.meshRefinementLevel = 2;
settings.convergenceTolerance = 1e-6;
settings.maxIterations = 1000;
feaSystem.setSettings(settings);

// Create a mesh
auto mesh = feaSystem.createMesh("my_geometry", ElementType::Tetra, 2);

// Register materials
auto steel = Material::createIsotropic(
    "Steel",
    210.0e9,    // Young's modulus (Pa)
    0.3,        // Poisson's ratio
    7850.0      // Density (kg/m³)
);
feaSystem.registerMaterial(steel);

// Set material for elements
int groupId = mesh->createElementGroup("Default", steel);
for (size_t i = 0; i < mesh->getElementCount(); ++i) {
    mesh->addElementToGroup(groupId, static_cast<int>(i));
}

// Add boundary conditions
auto fixedBC = BoundaryCondition::createDisplacement(
    "fixed",
    "Left",
    DisplacementDirection::XYZ,
    0.0
);
feaSystem.addBoundaryCondition(fixedBC);

// Add loads
auto pressureLoad = Load::createSurfacePressure(
    "pressure",
    "Right",
    1.0e6  // 1 MPa
);
feaSystem.addLoad(pressureLoad);

// Solve the FEA problem with progress reporting
auto result = feaSystem.solve([](float progress) {
    std::cout << "Progress: " << (progress * 100.0f) << "%" << std::endl;
});

// Access results
double maxDisplacement = result->getMaxDisplacementMagnitude();
double maxStress = result->getMaxStress(StressComponent::VonMises);

// Export results
feaSystem.exportResults(result, "fea_results.json");

// Shutdown the FEA system
feaSystem.shutdown();
```

## Advanced Usage

For more advanced usage, refer to the API documentation for each class. The FEA system provides a wide range of functionality for complex structural analysis scenarios.

### Linear Static Solver

The FEA system includes a robust linear static solver that efficiently solves structural analysis problems. The solver:

1. Assembles the global stiffness matrix using sparse matrix representation for memory efficiency
2. Applies boundary conditions using the penalty method
3. Solves the system of equations using a direct sparse solver
4. Computes displacements, stresses, strains, and reaction forces
5. Provides detailed progress reporting during the solution process

The solver supports various element types, including tetrahedral and hexahedral elements, and can handle complex geometries with a large number of degrees of freedom.

## Future Enhancements

- Nonlinear analysis
- Dynamic analysis
- Thermal analysis
- Contact analysis
- Optimization
- Parallel processing for improved performance
