# Assembly Constraints

This directory contains the implementation of the assembly constraint system for RebelCAD. The system is responsible for enforcing geometric relationships between components in an assembly.

## Overview

The assembly constraint system consists of the following components:

1. **AssemblyConstraint**: Base class for all assembly constraints. It defines the interface that all specific assembly constraints must implement.

2. **ConstraintSolver**: Class responsible for solving a system of assembly constraints. It can handle multiple constraints simultaneously, resolve conflicts, and find a valid configuration that satisfies all constraints.

3. **Specific Constraints**: Various specific constraint types that implement the AssemblyConstraint interface, such as:
   - Mate constraint
   - Align constraint
   - Angle constraint
   - Distance constraint
   - Gear constraint
   - Cam constraint
   - Path constraint
   - Gear-rack constraint

## ConstraintSolver

The ConstraintSolver class is the core of the assembly constraint system. It is responsible for solving a system of assembly constraints to find a valid configuration that satisfies all constraints.

### Features

- **Multiple Solving Algorithms**: The solver supports multiple algorithms for solving constraints, including:
  - Sequential: Enforces constraints one by one in a specific order.
  - Relaxation: Uses a relaxation method to iteratively converge to a solution.

- **Constraint Management**: The solver provides methods for adding, removing, and clearing constraints.

- **Degrees of Freedom Analysis**: The solver can analyze the degrees of freedom in the system to determine if it is over-constrained, under-constrained, or properly constrained.

- **Progress Reporting**: The solver supports progress reporting during the solving process.

### Usage

```cpp
// Create a constraint solver
ConstraintSolver solver;

// Add constraints
solver.addConstraint(std::make_shared<MateConstraint>(component1, component2));
solver.addConstraint(std::make_shared<AngleConstraint>(component1, component2, 90.0));

// Solve the constraints
ConstraintSolver::Result result = solver.solve();

// Check the result
if (result.success) {
    std::cout << "Constraints solved successfully in " << result.iterations << " iterations." << std::endl;
    std::cout << "Final error: " << result.error << std::endl;
} else {
    std::cout << "Failed to solve constraints." << std::endl;
    std::cout << "Unsatisfied constraints:" << std::endl;
    for (const auto& name : result.unsatisfiedConstraints) {
        std::cout << "  " << name << std::endl;
    }
}
```

## Advanced Assembly Constraints

The advanced assembly constraints extend the basic assembly constraints with additional functionality for complex mechanical systems, including motion simulation and force/torque calculation.

### Features

- **Motion Simulation**: Advanced constraints can simulate motion over time, updating component positions and orientations based on physical relationships.

- **Force and Torque Calculation**: Constraints can calculate the forces and torques generated during motion, enabling realistic physical simulation.

- **Constraint Enforcement**: Constraints can enforce geometric relationships between components, correcting any violations.

- **Customizable Profiles**: Some constraints support custom profile functions, allowing for a wide variety of shapes and behaviors.

- **Mechanical Efficiency**: Some constraints model energy losses through a mechanical efficiency parameter.

### Implementation

The advanced assembly constraints are implemented using a modular, object-oriented approach:

1. **Base Classes**: Robust base classes with clear interfaces.

2. **Inheritance Hierarchy**: Inheritance is used to extend basic constraints with advanced functionality.

3. **Composition**: Complex constraints are built from simpler components.

4. **Template Method Pattern**: The template method pattern is used for common constraint operations.

5. **Strategy Pattern**: The strategy pattern is used for customizable behavior (e.g., cam profiles).

## Future Work

The following enhancements are planned for the assembly constraint system:

1. **Additional Advanced Constraints**:
   - Belt/chain constraint for modeling belt or chain drives
   - Screw constraint for modeling screw mechanisms

2. **Comprehensive Constraint Solver**: Implement a more comprehensive constraint solver for handling multiple constraints simultaneously, including:
   - Redundant constraint detection
   - Conflicting constraint detection
   - Prioritized constraint solving (completed)

3. **Interference Detection**: Implement interference detection for assembly components.

4. **Exploded View Generation**: Add functionality to generate exploded views of assemblies.

5. **Enhanced Motion Analysis**: Add more advanced motion analysis tools.
