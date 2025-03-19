# Prismatic Joint

## Overview
A prismatic joint, also known as a sliding joint, allows linear translation along a single axis while constraining all other degrees of freedom. This joint is commonly used in mechanical assemblies for creating linear slides, pistons, and other translational mechanisms.

## Features
- Linear translation along a specified axis
- Configurable travel limits
- Maintains component orientation during translation
- Reference point-based positioning
- Real-time position updates

## Usage

### Creating a Prismatic Joint
```cpp
// Create components to be joined
auto component1 = std::make_shared<Component>();
auto component2 = std::make_shared<Component>();

// Define translation axis and reference point
glm::vec3 axis(1.0f, 0.0f, 0.0f);  // Translation along X-axis
glm::vec3 point(0.0f, 0.0f, 0.0f);  // Origin as reference point

// Create the prismatic joint
PrismaticJoint joint(component1, component2, axis, point);
```

### Setting Linear Limits
```cpp
// Set minimum and maximum travel distances
joint.setLinearLimits(-10.0, 10.0);  // Allow ±10 units of travel
```

### Translating Components
```cpp
// Translate component2 along the joint axis
bool success = joint.translate(5.0);  // Move 5 units in the positive direction
if (!success) {
    // Translation failed (likely due to limits)
}

// Get current displacement
double currentPos = joint.getCurrentDisplacement();
```

## Technical Details

### Constructor Parameters
- `component1`: First component to be joined (fixed reference)
- `component2`: Second component to be joined (moves along axis)
- `axis`: Direction vector for translation (will be normalized)
- `point`: Reference point for the joint

### Member Functions
- `setLinearLimits(minDistance, maxDistance)`: Set travel limits
- `getCurrentDisplacement()`: Get current translation distance
- `translate(distance)`: Move component2 by specified distance
- `update()`: Update joint constraints and component positions
- `getAxis()`: Get normalized translation axis
- `getReferencePoint()`: Get joint reference point

## Examples

### Creating a Sliding Mechanism
```cpp
// Create a slider that moves horizontally
PrismaticJoint slider(base, movingPart, 
                     glm::vec3(1.0f, 0.0f, 0.0f),  // X-axis movement
                     base->getPosition());          // Use base position as reference

// Limit travel to prevent derailing
slider.setLinearLimits(0.0, 100.0);  // 100 units of positive travel

// Move the slider
slider.translate(50.0);  // Move halfway
```

### Creating a Piston Mechanism
```cpp
// Create a vertical piston
PrismaticJoint piston(cylinder, rod,
                      glm::vec3(0.0f, 1.0f, 0.0f),  // Y-axis movement
                      cylinder->getPosition());      // Use cylinder position as reference

// Set stroke length
piston.setLinearLimits(-5.0, 5.0);  // ±5 units of travel

// Actuate the piston
piston.translate(5.0);   // Extend fully
piston.translate(-10.0); // Retract fully
```

## Best Practices

1. **Axis Selection**
   - Choose a clear, logical axis for translation
   - Use axis-aligned translations when possible for simpler understanding
   - Normalize input vectors (done automatically by the joint)

2. **Reference Point**
   - Choose a meaningful reference point (usually on component1)
   - Consider using component centers or connection points
   - Keep reference points consistent across related joints

3. **Travel Limits**
   - Always set appropriate limits to prevent unrealistic movement
   - Consider physical constraints of the assembly
   - Use limits to ensure safety and prevent interference

4. **Performance**
   - Update joint positions only when necessary
   - Consider using the joint's current displacement for animations
   - Cache transformed positions for complex assemblies

## Common Issues and Solutions

### Issue: Joint Not Moving
- Check if travel limits are set correctly
- Verify axis vector is not zero
- Ensure components are properly initialized

### Issue: Unexpected Movement
- Verify reference point location
- Check axis direction
- Confirm component hierarchies

### Issue: Performance Problems
- Minimize unnecessary updates
- Use efficient transformation calculations
- Consider implementing position caching

## See Also
- [Component Positioner](ComponentPositioner.md)
- [Fixed Joint](FixedJoint.md)
- [Revolute Joint](RevoluteJoint.md)
