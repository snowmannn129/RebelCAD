# Pattern Tool

The PatternTool class provides functionality for creating patterns of 3D bodies using various transformation methods. It supports linear, circular, and mirror patterns with real-time preview capabilities.

## Overview

Pattern tools are essential for creating repetitive geometry in CAD applications. The PatternTool supports:
- Linear patterns along any direction
- Circular patterns around any axis
- Mirror patterns across any plane
- Real-time preview of pattern operations
- Topology preservation during transformations

## Usage

```cpp
// Create a linear pattern
LinearPatternParams linearParams;
linearParams.direction = glm::dvec3(1.0, 0.0, 0.0);  // X-axis direction
linearParams.spacing = 2.0;                          // 2 units apart
linearParams.count = 3;                             // Create 3 instances
linearParams.includeOriginal = true;                // Include original body

auto linearInstances = PatternTool::CreateLinearPattern(body, linearParams);

// Create a circular pattern
CircularPatternParams circularParams;
circularParams.axis = glm::dvec3(0.0, 1.0, 0.0);    // Y-axis rotation
circularParams.center = glm::dvec3(0.0);            // Around origin
circularParams.angle = glm::pi<double>();           // 180 degrees
circularParams.count = 4;                           // Create 4 instances
circularParams.includeOriginal = true;              // Include original body

auto circularInstances = PatternTool::CreateCircularPattern(body, circularParams);

// Create a mirror pattern
MirrorPatternParams mirrorParams;
mirrorParams.normal = glm::dvec3(1.0, 0.0, 0.0);    // Mirror across YZ plane
mirrorParams.point = glm::dvec3(0.0);              // Through origin
mirrorParams.includeOriginal = true;                // Include original body

auto mirrorInstances = PatternTool::CreateMirrorPattern(body, mirrorParams);

// Preview a pattern operation
auto graphics = GraphicsSystem::getInstance();
PatternTool::PreviewPattern(
    body,
    PatternType::Linear,
    &linearParams,
    graphics
);
```

## Features

### Linear Pattern
- Pattern along any direction vector
- Configurable spacing between instances
- Variable instance count
- Option to include original body

### Circular Pattern
- Pattern around any rotation axis
- Configurable center point
- Variable angle and instance count
- Option to include original body

### Mirror Pattern
- Mirror across any plane
- Plane defined by normal and point
- Proper normal transformation
- Option to include original body

### Preview System
- Real-time visualization
- Interactive feedback
- Efficient preview geometry generation
- Support for large patterns

## Technical Details

### Transformation Process

1. Linear Pattern
   - Translate along normalized direction vector
   - Scale translation by spacing and instance index
   - Apply transformation to vertices and normals

2. Circular Pattern
   - Translate to origin
   - Apply rotation around axis
   - Translate back to original position
   - Transform vertices and normals

3. Mirror Pattern
   - Calculate reflection matrix from normal
   - Apply translation to reflection plane
   - Perform reflection
   - Transform vertices and normals

### Implementation Notes

- Uses GLM for transformation mathematics
- Preserves vertex attributes (normals, UVs)
- Handles topology consistently
- Optimized preview generation
- Robust parameter validation

## Performance Considerations

- Memory usage scales with instance count
- Preview mode uses simplified geometry
- Efficient transformation calculations
- Optimized vertex attribute handling

## Example Applications

1. Mechanical Design
   - Bolt patterns
   - Gear teeth
   - Structural elements

2. Architectural Design
   - Window arrays
   - Column sequences
   - Symmetrical features

3. Pattern Creation
   - Regular arrays
   - Radial distributions
   - Symmetrical designs

## Best Practices

1. Pattern Setup
   - Use normalized direction vectors
   - Set appropriate spacing
   - Consider performance with large counts
   - Validate input parameters

2. Preview Usage
   - Use preview for interactive editing
   - Verify pattern before creation
   - Check for intersections
   - Consider view orientation

3. Performance Optimization
   - Limit instance count for previews
   - Use appropriate spacing
   - Consider memory usage
   - Cache results when possible

4. Error Handling
   - Validate input parameters
   - Check for degenerate cases
   - Handle transformation errors
   - Verify output topology

## Limitations

- No direct support for nested patterns
- Preview mode may simplify geometry
- Memory usage scales with instance count
- Performance impact with large counts

## Future Enhancements

1. Pattern Types
   - Rectangular arrays
   - Curve-driven patterns
   - Surface patterns
   - Custom distribution patterns

2. Performance
   - Instance pooling
   - GPU-accelerated preview
   - Lazy instance creation
   - Memory optimization

3. Features
   - Pattern editing
   - Dynamic updates
   - Pattern constraints
   - Pattern relationships
