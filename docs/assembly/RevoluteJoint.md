# Revolute Joint

## Overview

A revolute joint, also known as a pin or hinge joint, allows rotation about a single axis while constraining all other degrees of freedom. This joint type is commonly used in mechanical assemblies for creating hinges, pivots, and other rotational mechanisms.

## Features

- Single-axis rotation with configurable axis and pivot point
- Optional angular limits to constrain rotation range
- Automatic angle normalization to [-π, π]
- Maintains component relationships during rotation
- Efficient transformation calculations using quaternions

## Usage

### Creating a Revolute Joint

```cpp
// Create components to be joined
auto component1 = std::make_shared<Component>();
auto component2 = std::make_shared<Component>();

// Define rotation axis and pivot point
glm::vec3 axis(0.0f, 1.0f, 0.0f);  // Y-axis rotation
glm::vec3 pivot(0.0f, 0.0f, 0.0f);  // Origin pivot point

// Create the joint
RevoluteJoint joint(component1, component2, axis, pivot);
```

### Setting Angular Limits

```cpp
// Limit rotation to ±90 degrees
joint.setAngularLimits(-glm::pi<double>() / 2.0, glm::pi<double>() / 2.0);

// Remove limits
joint.setAngularLimits(-glm::pi<double>(), glm::pi<double>());
```

### Rotating the Joint

```cpp
// Rotate by 45 degrees (π/4 radians)
bool success = joint.rotate(glm::pi<double>() / 4.0);

// Check current angle
double angle = joint.getCurrentAngle();
```

## Technical Details

### Transformation Process

1. The joint maintains a fixed component (component1) and a moving component (component2)
2. Rotation is performed around the specified axis through the pivot point
3. The transformation process:
   - Translates to pivot point
   - Applies rotation
   - Translates back to original position

### Implementation Notes

- Uses GLM for efficient 3D mathematics
- Quaternion-based rotation for numerical stability
- Automatic angle normalization prevents accumulation errors
- Component transforms are updated automatically

## Best Practices

1. **Axis Definition**
   - Use normalized vectors for the rotation axis
   - Choose a meaningful axis based on the desired motion

2. **Pivot Point Selection**
   - Place the pivot point at the intended center of rotation
   - Consider the geometric relationship between components

3. **Angular Limits**
   - Set appropriate limits to prevent unrealistic motion
   - Consider the mechanical constraints of your assembly

4. **Performance**
   - Minimize unnecessary rotations
   - Use the update() method only when needed

## Example: Creating a Door Hinge

```cpp
// Create door and frame components
auto frame = std::make_shared<Component>();
auto door = std::make_shared<Component>();

// Define vertical axis along frame edge
glm::vec3 hingeAxis(0.0f, 1.0f, 0.0f);
glm::vec3 hingePivot(1.0f, 0.0f, 0.0f);  // Edge of frame

// Create hinge joint
RevoluteJoint hinge(frame, door, hingeAxis, hingePivot);

// Limit door swing to 0-100 degrees
hinge.setAngularLimits(0.0, glm::radians(100.0));

// Open door 90 degrees
hinge.rotate(glm::pi<double>() / 2.0);
```

## Error Handling

The RevoluteJoint class includes robust error checking:

- Validates component pointers during construction
- Ensures rotation axis is non-zero
- Verifies angular limits are valid
- Returns success/failure status for rotation operations

## Related Components

- [ComponentPositioner](ComponentPositioner.md)
- [FixedJoint](FixedJoint.md)

## Future Enhancements

- Dynamic friction simulation
- Spring/damper behavior
- Animation support
- Constraint-based motion planning
