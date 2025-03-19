# TangentConstraint

## Overview

The TangentConstraint ensures that geometric entities (lines and circles) maintain a tangent relationship. This constraint is essential for creating precise technical drawings and mechanical designs where components need to touch at exactly one point.

## Features

- Supports line-to-circle tangency
- Supports circle-to-circle tangency (both external and internal)
- Configurable tolerance for numerical stability
- Automatic constraint enforcement
- Error reporting for constraint violations
- Performance optimization through caching
- Robust error handling for invalid configurations

## Usage

### Line-Circle Tangency

```cpp
// Create a tangent constraint between a line and circle
TangentConstraint constraint(
    {x1, y1},  // line start point
    {x2, y2},  // line end point
    {cx, cy},  // circle center
    radius     // circle radius
);
```

### Circle-Circle Tangency

```cpp
// Create a tangent constraint between two circles
TangentConstraint constraint(
    {cx1, cy1},  // first circle center
    radius1,     // first circle radius
    {cx2, cy2},  // second circle center
    radius2      // second circle radius
);
```

## Configuration

### Tolerance

The constraint accepts an optional tolerance parameter to control the precision of tangency checking:

```cpp
// Create constraint with custom tolerance
TangentConstraint constraint(
    {x1, y1}, {x2, y2},    // line points
    {cx, cy}, radius,      // circle parameters
    0.001                  // custom tolerance (default is 1e-6)
);
```

## Methods

### isValid()

Checks if the constraint configuration is valid (non-degenerate):
- Lines must have non-zero length
- Circles must have positive radii
- Circles cannot be coincident

```cpp
if (constraint.isValid()) {
    // Constraint is valid and can be enforced
}
```

### isSatisfied()

Checks if the tangency condition is met within the specified tolerance:

```cpp
if (constraint.isSatisfied()) {
    // Entities are tangent
}
```

### enforce()

Attempts to modify the entities to satisfy the tangency condition:

```cpp
if (constraint.enforce()) {
    // Tangency was successfully enforced
} else {
    // Tangency could not be achieved
}
```

### getError()

Returns the current deviation from perfect tangency:

```cpp
double error = constraint.getError();
// 0.0 indicates perfect tangency
// Larger values indicate greater deviation
```

## Edge Cases

### Invalid Configurations

The constraint will throw std::invalid_argument for:
- Zero or negative radii
- Degenerate lines (zero length)
- Coincident circles
- Impossible tangency configurations

### Special Cases

The constraint handles various tangency configurations:
- External circle-circle tangency (circles touch on outside)
- Internal circle-circle tangency (one circle inside another)
- Multiple possible tangent points (selects most appropriate)

## Performance Considerations

- Uses efficient geometric calculations
- Implements caching of error values and tangent points
- Automatically invalidates cache when geometry changes
- Configurable tolerance allows balancing precision vs. performance
- Optimized calculations to minimize redundant computations

## Integration

The TangentConstraint integrates with RebelCAD's constraint system:
- Can be combined with other geometric constraints
- Participates in constraint solving systems
- Supports undo/redo operations
- Updates automatically with geometry modifications

## Example Workflow

```cpp
// Create geometric entities
Line line({0,0}, {1,0});
Circle circle({0,1}, 1.0);

// Create tangency constraint
TangentConstraint constraint(
    line.start(), line.end(),
    circle.center(), circle.radius()
);

// Verify and enforce constraint
if (constraint.isValid()) {
    if (!constraint.isSatisfied()) {
        if (constraint.enforce()) {
            // Tangency achieved
        }
    }
}
```

## Best Practices

1. Always check isValid() before attempting to enforce constraints
2. Use appropriate tolerance values for your application
3. Handle enforcement failures gracefully
4. Consider performance impact in constraint systems
5. Maintain proper error handling for invalid configurations
6. Let the cache system optimize performance automatically

## Implementation Details

### Caching System

The constraint implements an intelligent caching system that:
- Caches error values to avoid recalculation
- Caches tangent points for reuse in enforcement
- Automatically invalidates cache when geometry changes
- Provides thread-safe const correctness

### Error Handling

Robust error handling includes:
- Constructor validation of input parameters
- Detection of degenerate geometries
- Handling of coincident circles
- Validation of tangency possibility
- Clear error messages for debugging

### Numerical Stability

The implementation ensures numerical stability through:
- Configurable tolerance values
- Normalized vector calculations
- Stable geometric algorithms
- Proper handling of edge cases

## See Also

- [ParallelConstraint](ParallelConstraint.md)
- [PerpendicularConstraint](PerpendicularConstraint.md)
- [ConcentricConstraint](ConcentricConstraint.md)
