# Cylindrical Joint

## Overview

A cylindrical joint is a mechanical constraint that allows both rotation around and translation along a common axis between two components. This joint type has two degrees of freedom:

1. Rotation around the axis
2. Translation along the axis

Common real-world examples include:
- Telescoping mechanisms
- Rotating and sliding doors
- Some types of pistons
- Adjustable-height chair posts

## Usage

```cpp
// Create components
auto component1 = std::make_shared<Component>();
auto component2 = std::make_shared<Component>();

// Define joint parameters
glm::vec3 axis(0.0f, 1.0f, 0.0f);  // Y-axis
glm::vec3 position(0.0f, 0.0f, 0.0f);  // Origin

// Create the joint
CylindricalJoint joint(component1, component2, axis, position);

// Control rotation
joint.SetRotationAngle(glm::radians(45.0f));  // 45-degree rotation

// Control translation
joint.SetTranslation(5.0f);  // 5 units along the axis
```

## API Reference

### Constructor

```cpp
CylindricalJoint(
    std::shared_ptr<Component> component1,
    std::shared_ptr<Component> component2,
    const glm::vec3& axis,
    const glm::vec3& position
);
```

Creates a new cylindrical joint between two components.

Parameters:
- `component1`: First component to be joined
- `component2`: Second component to be joined
- `axis`: The axis of rotation and translation (will be normalized)
- `position`: The position of the joint in world space

### Methods

#### SetRotationAngle
```cpp
void SetRotationAngle(float angle);
```
Sets the rotation angle around the joint axis in radians.

#### SetTranslation
```cpp
void SetTranslation(float distance);
```
Sets the translation distance along the joint axis.

#### GetRotationAngle
```cpp
float GetRotationAngle() const;
```
Returns the current rotation angle in radians.

#### GetTranslation
```cpp
float GetTranslation() const;
```
Returns the current translation distance.

#### GetAxis
```cpp
glm::vec3 GetAxis() const;
```
Returns the normalized joint axis vector.

#### GetPosition
```cpp
glm::vec3 GetPosition() const;
```
Returns the joint position in world space.

## Implementation Details

The cylindrical joint maintains the following constraints:

1. Component 1 remains fixed at the joint position
2. Component 2 can:
   - Rotate around the specified axis
   - Translate along the specified axis
   
The joint uses quaternions for rotation to avoid gimbal lock and ensure smooth motion.

## Best Practices

1. Always normalize the axis vector before creating the joint
2. Consider using angular limits for restricted motion
3. Validate component transforms after joint operations
4. Use appropriate units (radians for angles, world units for translation)

## Performance Considerations

- The joint performs minimal calculations during updates
- Transform matrices are only recalculated when rotation or translation changes
- Memory footprint is minimal, storing only essential vectors and angles

## Error Handling

The joint validates its configuration and will throw a `std::runtime_error` if:
- Either component pointer is null
- The axis vector is zero-length
