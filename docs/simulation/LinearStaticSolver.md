# Linear Static Solver

The Linear Static Solver is a key component of the RebelCAD Finite Element Analysis (FEA) system. It provides a robust and efficient solution for linear static structural analysis problems.

## Overview

The Linear Static Solver implements the finite element method for linear static analysis. It assembles the global stiffness matrix and load vector, applies boundary conditions, solves the system of equations, and computes the results (displacements, stresses, strains, and reaction forces).

## Features

- Efficient assembly of the global stiffness matrix using sparse matrix representation
- Application of boundary conditions using the penalty method
- Solution of the system of equations using a direct sparse solver
- Computation of displacements, stresses, strains, and reaction forces
- Support for various element types (tetrahedral, hexahedral)
- Comprehensive error handling and progress reporting

## Implementation Details

### Stiffness Matrix Assembly

The solver assembles the global stiffness matrix by computing the element stiffness matrices for each element in the mesh and assembling them into a global sparse matrix. The element stiffness matrices are computed using numerical integration (Gauss quadrature) and the finite element shape functions.

### Boundary Condition Application

Boundary conditions are applied using the penalty method, which modifies the stiffness matrix and load vector to enforce the prescribed displacements. This method is robust and easy to implement, and it works well for a wide range of problems.

### System Solution

The system of equations is solved using a direct sparse solver (SparseLU from the Eigen library). This solver is efficient for problems with a moderate number of degrees of freedom and provides accurate results.

### Result Computation

After solving the system of equations, the solver computes the displacements, stresses, strains, and reaction forces. The stresses and strains are computed at the element level using the element shape functions and the displacement solution.

## Usage

The Linear Static Solver is used internally by the FEASystem class. You don't need to interact with it directly in most cases. Instead, you can use the FEASystem class to set up and solve your FEA problems.

Here's an example of how to use the FEASystem class with the Linear Static Solver:

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

// Set FEA settings to use the Linear Static Solver
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

If you need to use the Linear Static Solver directly, you can create an instance of the LinearStaticSolver class and call its solve method:

```cpp
#include "simulation/LinearStaticSolver.h"
#include "simulation/FEASystem.h"
#include "simulation/Material.h"
#include "simulation/Mesh.h"
#include "simulation/BoundaryCondition.h"
#include "simulation/Load.h"
#include "simulation/FEAResult.h"

using namespace rebel::simulation;

// Create a mesh
auto mesh = Mesh::createFromGeometry("my_geometry", ElementType::Tetra, 2);

// Create boundary conditions
std::vector<std::shared_ptr<BoundaryCondition>> boundaryConditions;
auto fixedBC = BoundaryCondition::createDisplacement(
    "fixed",
    "Left",
    DisplacementDirection::XYZ,
    0.0
);
boundaryConditions.push_back(fixedBC);

// Create loads
std::vector<std::shared_ptr<Load>> loads;
auto pressureLoad = Load::createSurfacePressure(
    "pressure",
    "Right",
    1.0e6  // 1 MPa
);
loads.push_back(pressureLoad);

// Create solver settings
FEASettings settings;
settings.solverType = SolverType::Linear;
settings.elementType = ElementType::Tetra;
settings.meshRefinementLevel = 2;
settings.convergenceTolerance = 1e-6;
settings.maxIterations = 1000;

// Create solver
LinearStaticSolver solver(settings);

// Solve the problem
auto result = solver.solve(mesh, boundaryConditions, loads, [](float progress) {
    std::cout << "Progress: " << (progress * 100.0f) << "%" << std::endl;
});

// Access results
double maxDisplacement = result->getMaxDisplacementMagnitude();
double maxStress = result->getMaxStress(StressComponent::VonMises);
```

## Performance Considerations

The Linear Static Solver is designed to be efficient for problems with a moderate number of degrees of freedom (up to a few hundred thousand). For larger problems, you may need to use a more advanced solver or enable parallel processing.

The solver uses sparse matrix representation to minimize memory usage, and it employs efficient algorithms for matrix assembly and solution. However, the performance of the solver depends on the size and complexity of the problem, as well as the available computational resources.

## Future Enhancements

Future enhancements to the Linear Static Solver may include:

- Parallel processing for improved performance
- Iterative solvers for large-scale problems
- Adaptive mesh refinement
- Improved error estimation and convergence criteria
- Support for more element types and material models
