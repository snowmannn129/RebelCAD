# Perpendicular Constraint

The `PerpendicularConstraint` class provides functionality to enforce and maintain perpendicular (90-degree) relationships between two line segments in the RebelCAD sketching system.

## Overview

The perpendicular constraint ensures that two line segments remain perpendicular to each other within a specified tolerance. When enforced, it will adjust the second line to be perpendicular to the first line while maintaining the second line's length and midpoint position.

## Usage

```cpp
#include "constraints/PerpendicularConstraint.h"

// Create two lines
std::array<double, 2> line1_start = {0, 0};
std::array<double, 2> line1_end = {1, 0};     // Horizontal line
std::array<double, 2> line2_start = {1, 0};
std::array<double, 2> line2_end = {1, 1};     // Vertical line

// Create a perpendicular constraint
RebelCAD::Constraints::PerpendicularConstraint constraint(
    line1_start, line1_end,
    line2_start, line2_end,
    1e-6  // tolerance in radians (optional)
);

// Check if lines are perpendicular
bool isPerpendicular = constraint.isSatisfied();

// Enforce perpendicularity if needed
if (!isPerpendicular) {
    constraint.enforce();
}
```

## Constructor Parameters

- `line1_start`: First point of the first line `{x, y}`
- `line1_end`: Second point of the first line `{x, y}`
- `line2_start`: First point of the second line `{x, y}`
- `line2_end`: Second point of the second line `{x, y}`
- `tolerance`: Maximum allowed angular deviation from 90 degrees in radians (default: 1e-6)

## Methods

### `bool isSatisfied() const`
Checks if the lines are perpendicular within the specified tolerance.
- Returns: `true` if lines are perpendicular (90 degrees), `false` otherwise

### `bool enforce()`
Attempts to make the lines perpendicular by rotating the second line around its midpoint.
- Returns: `true` if constraint was successfully enforced, `false` if it failed

### `double getError() const`
Gets the angular deviation from 90 degrees in radians.
- Returns: The absolute difference between the actual angle and 90 degrees, or infinity if lines are invalid

### `bool isValid() const`
Checks if both lines have non-zero length.
- Returns: `true` if both lines are valid, `false` otherwise

### Getter Methods
- `getLine1Start()`: Returns the start point of the first line
- `getLine1End()`: Returns the end point of the first line
- `getLine2Start()`: Returns the start point of the second line
- `getLine2End()`: Returns the end point of the second line

## Implementation Details

The constraint works by:
1. Calculating the angle of each line relative to the x-axis
2. Computing the angular difference between the lines
3. When enforcing, rotating the second line around its midpoint to achieve a 90-degree angle with the first line

The enforcement preserves:
- The length of both lines
- The midpoint position of the second line
- The position and orientation of the first line

## Error Handling

The constraint handles several edge cases:
- Zero-length lines: Returns false for `isValid()` and `isSatisfied()`
- Invalid points: Properly handles NaN and infinite values
- Numerical precision: Uses a tolerance value to account for floating-point errors

## Example: Enforcing Perpendicularity

```cpp
// Create two non-perpendicular lines
std::array<double, 2> line1_start = {0, 0};
std::array<double, 2> line1_end = {1, 0};     // Horizontal line
std::array<double, 2> line2_start = {1, 0};
std::array<double, 2> line2_end = {2, 0.5};   // 45-degree line

// Create constraint
auto constraint = RebelCAD::Constraints::PerpendicularConstraint(
    line1_start, line1_end,
    line2_start, line2_end
);

// Check initial state
std::cout << "Initially perpendicular: " << constraint.isSatisfied() << "\n";
std::cout << "Initial deviation from 90°: " << constraint.getError() << " radians\n";

// Enforce perpendicularity
if (constraint.enforce()) {
    std::cout << "Constraint enforced successfully\n";
    std::cout << "New deviation from 90°: " << constraint.getError() << " radians\n";
    
    // Get the adjusted line points
    auto new_start = constraint.getLine2Start();
    auto new_end = constraint.getLine2End();
}
```

## Performance Considerations

The constraint operations are computationally lightweight, involving only basic trigonometric calculations. The memory footprint is minimal, storing only four points (eight doubles) and a tolerance value.
