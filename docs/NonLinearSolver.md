# NonLinearSolver

## Overview

The `NonLinearSolver` class is a component of the RebelCAD Finite Element Analysis (FEA) system that provides capabilities for solving non-linear structural analysis problems. Non-linear analysis is essential for accurately modeling structures that exhibit large deformations, material non-linearity, or contact interactions.

## Features

- Multiple non-linear solution methods:
  - Newton-Raphson method
  - Modified Newton-Raphson method
  - Quasi-Newton method
  - Arc-length method
- Support for different types of non-linearity:
  - Geometric non-linearity (large deformations)
  - Material non-linearity (plasticity, hyperelasticity, etc.)
  - Contact non-linearity
  - Combined non-linearity
- Adaptive load stepping for improved convergence
- Line search capabilities for enhanced robustness
- Progress reporting during solution

## Usage

### Basic Usage

```cpp
#include "simulation/NonLinearSolver.h"
#include "simulation/FEASystem.h"

// Get the FEA system instance
FEASystem& feaSystem = FEASystem::getInstance();

// Create a mesh
std::shared_ptr<Mesh> mesh = feaSystem.createMesh("model_geometry", ElementType::Tetra, 2);

// Create boundary conditions and loads
std::vector<std::shared_ptr<BoundaryCondition>> boundaryConditions;
std::vector<std::shared_ptr<Load>> loads;

// Configure the non-linear solver
NonLinearSolverSettings settings;
settings.method = NonLinearSolverMethod::NewtonRaphson;
settings.nonLinearityType = NonLinearityType::Geometric;
settings.convergenceTolerance = 1e-6;
settings.maxIterations = 100;

// Create the solver
NonLinearSolver solver(feaSystem.getSettings(), settings);

// Solve the problem
std::shared_ptr<FEAResult> result = solver.solve(mesh, boundaryConditions, loads);

// Process the results
if (result) {
    // Access displacements, stresses, etc.
    const auto& displacements = result->getDisplacements();
    const auto& stresses = result->getStresses();
}
```

### With Progress Reporting

```cpp
// Define a progress callback
auto progressCallback = [](float progress) {
    std::cout << "Solution progress: " << (progress * 100.0f) << "%" << std::endl;
};

// Solve with progress reporting
std::shared_ptr<FEAResult> result = solver.solve(mesh, boundaryConditions, loads, progressCallback);
```

## Solution Methods

### Newton-Raphson Method

The Newton-Raphson method is the most commonly used method for solving non-linear problems. It provides quadratic convergence when the solution is close to the correct value. The method works by:

1. Applying the load incrementally
2. For each load increment:
   - Computing the tangent stiffness matrix
   - Computing the residual force vector
   - Solving for the displacement increment
   - Updating the displacements
   - Checking for convergence
   - Repeating until convergence or maximum iterations

### Modified Newton-Raphson Method

The Modified Newton-Raphson method is similar to the standard Newton-Raphson method, but it reuses the tangent stiffness matrix for multiple iterations. This reduces computational cost per iteration but may require more iterations to converge.

### Quasi-Newton Method

The Quasi-Newton method approximates the tangent stiffness matrix using information from previous iterations. This can be more efficient than the Newton-Raphson method for large problems, as it avoids the expensive computation of the tangent stiffness matrix at each iteration.

### Arc-Length Method

The Arc-length method is particularly useful for problems with limit points or snap-through behavior. It controls both the load and displacement increments, allowing the solution to track the equilibrium path beyond limit points.

## Non-Linearity Types

### Geometric Non-Linearity

Geometric non-linearity occurs when a structure undergoes large deformations, causing the stiffness to change with the deformation. This is handled by:

- Using the Green-Lagrange strain tensor instead of the infinitesimal strain tensor
- Computing the geometric stiffness matrix
- Updating the configuration during the solution process

### Material Non-Linearity

Material non-linearity occurs when the material behavior is not linearly elastic. Examples include:

- Plasticity
- Hyperelasticity
- Viscoelasticity
- Damage

The solver handles material non-linearity by:

- Using appropriate constitutive models
- Computing the material tangent matrix
- Updating the material state during the solution process

### Contact Non-Linearity

Contact non-linearity occurs when parts of the structure come into contact with each other or with external objects. This is handled by:

- Detecting contact
- Applying contact constraints
- Computing contact forces
- Updating the contact state during the solution process

## Implementation Details

The `NonLinearSolver` class is implemented using the following components:

- `NonLinearSolverSettings`: Configuration settings for the solver
- `NonLinearSolverMethod`: Enumeration of available solution methods
- `NonLinearityType`: Enumeration of non-linearity types
- Solver methods: `solveNewtonRaphson`, `solveModifiedNewtonRaphson`, `solveQuasiNewton`, `solveArcLength`

The solver uses the Eigen library for matrix and vector operations, and it integrates with the RebelCAD FEA system for mesh management, material properties, boundary conditions, and loads.

## Future Enhancements

Planned enhancements for the `NonLinearSolver` include:

- Additional solution methods (e.g., BFGS, dynamic relaxation)
- Enhanced convergence criteria
- Parallel computation for improved performance
- Support for more complex material models
- Advanced contact algorithms
- Coupling with thermal and fluid analyses

## References

1. Bathe, K.J. (2006). Finite Element Procedures. Prentice Hall.
2. Crisfield, M.A. (1991). Non-linear Finite Element Analysis of Solids and Structures. Wiley.
3. Belytschko, T., Liu, W.K., Moran, B. (2000). Nonlinear Finite Elements for Continua and Structures. Wiley.
