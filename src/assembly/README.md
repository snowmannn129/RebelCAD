# Assembly Module

This directory contains the implementation of the assembly module for RebelCAD. The assembly module provides functionality for creating and manipulating assemblies of components, applying constraints, and simulating motion.

## Overview

The assembly module is a core component of RebelCAD that allows users to create complex mechanical assemblies from individual parts. It provides tools for:

- Managing component hierarchies
- Positioning and orienting components
- Applying constraints between components
- Detecting interference between components
- Simulating motion of assemblies
- Generating exploded views
- Analyzing assembly properties

## Key Components

### Component Class

The `Component` class represents a single component in an assembly. It can be a part or a sub-assembly, and it has a position and orientation in 3D space. Components can be organized in a hierarchical structure, with parent-child relationships.

Key features of the Component class:
- Position and orientation management
- Hierarchical structure (parent-child relationships)
- Transformation matrix handling
- Component naming and identification

### Assembly Constraints

The `constraints` directory contains various constraint types that can be applied between components in an assembly. Constraints restrict the relative motion between components, allowing for the creation of realistic mechanical assemblies.

Basic constraints include:
- Mate constraints (coincident, concentric, etc.)
- Align constraints (parallel, perpendicular, etc.)
- Distance constraints
- Angle constraints

Advanced constraints (in the `constraints/advanced` directory) include:
- Gear constraints
- Cam constraints
- Path constraints
- Belt/chain constraints
- Screw constraints

### Assembly Tree

The `AssemblyTree` class manages the overall structure of an assembly, including:
- Component hierarchy
- Constraint management
- Assembly operations (add/remove components, apply constraints)
- Assembly analysis (interference detection, mass properties)
- Assembly visualization (exploded views, section views)

### Motion Simulation

The assembly module includes functionality for simulating the motion of assemblies, including:
- Kinematic simulation
- Dynamic simulation (with forces and torques)
- Motion path visualization
- Motion analysis

## Usage Example

```cpp
// Create an assembly
auto assembly = std::make_shared<AssemblyTree>("MyAssembly");

// Create components
auto base = std::make_shared<Component>("Base");
auto arm = std::make_shared<Component>("Arm");
auto gear1 = std::make_shared<Component>("Gear1");
auto gear2 = std::make_shared<Component>("Gear2");

// Add components to the assembly
assembly->addComponent(base);
assembly->addComponent(arm, base); // arm is a child of base
assembly->addComponent(gear1, base); // gear1 is a child of base
assembly->addComponent(gear2, arm); // gear2 is a child of arm

// Position components
base->setPosition(Eigen::Vector3d(0.0, 0.0, 0.0));
arm->setPosition(Eigen::Vector3d(0.0, 0.0, 1.0));
gear1->setPosition(Eigen::Vector3d(1.0, 0.0, 0.0));
gear2->setPosition(Eigen::Vector3d(1.0, 0.0, 1.0));

// Apply constraints
auto revolute = std::make_shared<RevoluteJoint>(base, arm, Eigen::Vector3d(0.0, 0.0, 1.0));
auto gear = std::make_shared<GearConstraint>(gear1, gear2, Eigen::Vector3d(0.0, 0.0, 1.0), Eigen::Vector3d(0.0, 0.0, 1.0), 2.0);

assembly->addConstraint(revolute);
assembly->addConstraint(gear);

// Simulate motion
assembly->simulateMotion(1.0); // Simulate for 1 second

// Check for interference
bool hasInterference = assembly->checkInterference();

// Generate exploded view
assembly->generateExplodedView(1.5); // Explode with factor 1.5
```

## Future Enhancements

- **Constraint Solver**: Implement a comprehensive constraint solver for handling multiple constraints simultaneously
- **Interference Detection**: Enhance interference detection with more sophisticated algorithms
- **Motion Analysis**: Add more advanced motion analysis tools
- **Assembly Patterns**: Add support for component patterns (linear, circular, etc.)
- **Assembly Animation**: Add animation capabilities for assemblies
- **Assembly Export**: Add export capabilities for assemblies to other formats (e.g., STEP, IGES)
- **Assembly Documentation**: Add tools for generating assembly documentation (e.g., bill of materials, assembly instructions)
