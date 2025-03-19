# Fixed Joint Constraint

## Overview

The Fixed Joint constraint is a fundamental assembly constraint that completely locks all degrees of freedom (both translation and rotation) between two components. This creates a rigid connection where the components move and rotate together as if they were a single solid body.

## Features

- Locks all 6 degrees of freedom (3 translational + 3 rotational)
- Maintains relative position and orientation between components
- Automatic validation of constraint satisfaction
- Serialization support for save/load operations
- Efficient transform calculations
- Configurable tolerance settings

## Usage

### Creating a Fixed Joint

```cpp
// Create a fixed joint between two components
uint64_t component1Id = 1;  // ID of first component
uint64_t component2Id = 2;  // ID of second component
glm::vec3 connectionPoint(1.0f, 0.0f, 0.0f);  // Point where components connect

auto joint = std::make_unique<FixedJoint>(component1Id, component2Id, connectionPoint);
```

### Updating Component Positions

```cpp
// When components move, update the joint
glm::mat4 component1Transform = /* ... */;  // Current transform of component 1
glm::mat4 component2Transform = /* ... */;  // Current transform of component 2

if (!joint->update(component1Transform, component2Transform)) {
    // Handle constraint violation
}
```

### Validating Constraint

```cpp
// Check if constraint is satisfied within tolerance
float tolerance = 0.001f;  // Maximum allowed deviation in model units
if (!joint->validate(tolerance)) {
    // Handle invalid constraint state
}
```

## Technical Details

### Connection Point

The connection point represents the location in 3D space where the joint is established. This point is used as a reference for:
- Initial constraint setup
- Constraint validation
- Transform calculations

### Relative Transform

The joint maintains a relative transformation matrix between the two components that:
- Captures the initial spatial relationship
- Enables validation of constraint satisfaction
- Supports proper component movement

### Serialization

The joint supports JSON serialization for:
- Component IDs
- Connection point
- Relative transform matrix
- All necessary state data

## Integration with Assembly System

The Fixed Joint works in conjunction with the ComponentPositioner system to:
1. Establish initial component positions
2. Maintain proper relationships during assembly operations
3. Validate assembly constraints
4. Support assembly save/load operations

## Performance Considerations

- Efficient matrix operations for transform calculations
- Minimal memory footprint
- Optimized constraint validation
- Fast serialization/deserialization

## Error Handling

The Fixed Joint includes robust error handling for:
- Invalid component IDs
- Self-referential joints
- Constraint violations
- Serialization errors
- Transform calculation errors

## Best Practices

1. Always validate joint constraints after component movements
2. Use appropriate tolerance values for your specific use case
3. Handle constraint violations appropriately in your application
4. Maintain proper component IDs and references
5. Consider performance implications in large assemblies

## Future Enhancements

- Support for temporary constraint relaxation
- Enhanced validation visualization
- Performance optimizations for large assemblies
- Additional serialization formats
- Integration with constraint solving system
