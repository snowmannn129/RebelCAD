# Angle Constraint

The Angle Constraint ensures that two lines maintain a specific angle between them. This constraint is useful for creating precise geometric relationships in sketches, such as 45-degree angles, perpendicular lines, or any other specific angular relationship.

## Usage

```cpp
// Create two lines
auto line1 = std::make_shared<Line>(Point{0, 0}, Point{2, 0});
auto line2 = std::make_shared<Line>(Point{0, 0}, Point{1, 1});

// Create an angle constraint of 45 degrees between the lines
auto angleConstraint = std::make_shared<AngleConstraint>(line1, line2, 45.0);

// Apply the constraint
if (angleConstraint->apply()) {
    // Constraint was successfully applied
} else {
    // Failed to apply constraint (e.g., lines are parallel)
}
```

## Features

- Maintains a specific angle between two lines
- Angle is measured counterclockwise from the first line to the second line
- Automatically handles angle normalization (0-360 degrees)
- Provides validation to ensure the constraint is satisfied
- Handles edge cases like parallel lines

## Constructor

```cpp
AngleConstraint(std::shared_ptr<Line> line1, std::shared_ptr<Line> line2, double angle)
```

### Parameters

- `line1`: First line for angle measurement
- `line2`: Second line for angle measurement
- `angle`: Desired angle in degrees (must be between 0 and 360)

### Exceptions

- Throws `std::invalid_argument` if either line is null
- Throws `std::invalid_argument` if angle is not in the valid range [0, 360)

## Methods

### getAngle

```cpp
double getAngle() const
```

Returns the current target angle in degrees.

### setAngle

```cpp
void setAngle(double angle)
```

Sets a new target angle. Throws `std::invalid_argument` if angle is not in the valid range.

### validate

```cpp
bool validate() const override
```

Checks if the current angle between the lines matches the constraint within a small tolerance.

### apply

```cpp
bool apply() override
```

Attempts to satisfy the constraint by rotating the second line around the intersection point. Returns `true` if successful, `false` if the constraint cannot be applied (e.g., parallel lines).

## Best Practices

1. **Intersection Point**: The constraint works best when the lines intersect. While it can handle non-intersecting lines, the behavior might be less intuitive.

2. **Angle Range**: Always use angles between 0 and 360 degrees. The constraint automatically normalizes angles to this range.

3. **Error Handling**: Always check the return value of `apply()` to ensure the constraint was successfully applied.

4. **Performance**: The constraint uses efficient vector mathematics for angle calculations and rotations.

## Example: Creating a 45-degree Corner

```cpp
// Create a horizontal line
auto horizontal = std::make_shared<Line>(Point{0, 0}, Point{2, 0});

// Create a second line starting from the end of the first
auto diagonal = std::make_shared<Line>(Point{2, 0}, Point{3, 0});

// Create a 45-degree constraint
auto angleConstraint = std::make_shared<AngleConstraint>(horizontal, diagonal, 45.0);

// Apply the constraint to create the corner
if (angleConstraint->apply()) {
    // The diagonal line is now at 45 degrees from the horizontal
}
```

## Related Constraints

- Length Constraint: Often used in combination with angle constraints to create precise geometric shapes
- Parallel Constraint: For creating parallel lines (equivalent to 0 or 180-degree angles)
- Perpendicular Constraint: For creating right angles (equivalent to 90-degree angles)
