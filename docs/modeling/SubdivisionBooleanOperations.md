# Subdivision Boolean Operations

The SubdivisionBooleanOperations class provides functionality to perform boolean operations (union, subtract, intersect) between subdivision surfaces while maintaining sharp features and proper topology.

## Overview

Boolean operations on subdivision surfaces require special handling to:
- Preserve sharp features at intersection boundaries
- Maintain proper topology after the operation
- Optimize the resulting mesh for further subdivision
- Support real-time preview of operations

## Usage

```cpp
// Create two subdivision surfaces
auto cube = SubdivisionSurface::Create(cubeVertices, cubeEdges, cubeFaces);
auto sphere = SubdivisionSurface::Create(sphereVertices, sphereEdges, sphereFaces);

// Configure boolean operation parameters
BooleanParams params;
params.maintain_features = true;  // Preserve sharp features
params.optimize_result = true;    // Optimize resulting topology
params.intersection_tolerance = 1e-6f;  // Precision for intersection detection

// Perform boolean operation
auto result = SubdivisionBooleanOperations::PerformOperation(
    cube,
    sphere,
    BooleanOperationType::Union,
    params
);

// Preview operation in graphics system
auto graphics = GraphicsSystem::getInstance();
SubdivisionBooleanOperations::PreviewOperation(
    cube,
    sphere,
    BooleanOperationType::Union,
    graphics,
    params
);
```

## Features

### Sharp Feature Preservation
- Automatically detects and preserves sharp features at intersection boundaries
- Maintains consistent edge flow around intersections
- Supports variable sharpness based on intersection angles

### Topology Management
- Handles complex intersection cases
- Maintains manifold topology
- Optimizes mesh structure for subdivision

### Performance Optimization
- Efficient intersection detection using spatial partitioning
- Memory-efficient mesh representation
- Support for multi-threaded processing

### Preview System
- Real-time preview of boolean operations
- Interactive manipulation of operands
- Visual feedback for intersection regions

## Technical Details

### Boolean Operation Process

1. Conversion to Solid Body
   - Subdivide input surfaces to desired level
   - Convert to solid body representation
   - Prepare for intersection detection

2. Intersection Detection
   - Use spatial partitioning for efficiency
   - Find intersection points and curves
   - Create new topology elements

3. Boolean Operation
   - Union: Combine outer shells
   - Subtract: Remove intersecting regions
   - Intersect: Keep only common regions

4. Feature Preservation
   - Mark intersection edges as sharp
   - Preserve input surface sharp features
   - Maintain proper topology

5. Optimization
   - Remove small/degenerate features
   - Optimize vertex positions
   - Prepare for further subdivision

### Implementation Notes

- Uses half-edge data structure for topology operations
- Employs robust geometric predicates for intersection
- Handles numerical precision issues
- Maintains UV coordinates and other attributes

## Performance Considerations

- Memory usage depends on input complexity
- Intersection detection is the primary bottleneck
- Consider preview mode for interactive work
- Use appropriate subdivision levels

## Example Applications

1. Mechanical Design
   - Part modeling
   - Assembly operations
   - Feature creation

2. Artistic Modeling
   - Character creation
   - Organic forms
   - Conceptual design

3. Architecture
   - Building components
   - Space planning
   - Form studies

## Best Practices

1. Input Preparation
   - Use appropriate subdivision levels
   - Clean up input geometry
   - Mark important features

2. Operation Selection
   - Choose appropriate operation type
   - Consider alternative approaches
   - Plan for subsequent operations

3. Performance Optimization
   - Use preview mode during design
   - Adjust tolerance parameters
   - Consider final use case

4. Quality Control
   - Verify topology after operation
   - Check for unwanted artifacts
   - Validate feature preservation
