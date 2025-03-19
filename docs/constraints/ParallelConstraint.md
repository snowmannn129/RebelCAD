# Parallel Constraint

The `ParallelConstraint` class provides functionality to enforce and maintain parallel relationships between two line segments in the RebelCAD sketching system.

## Overview

The parallel constraint ensures that two line segments remain parallel to each other within a specified tolerance. When enforced, it will adjust the second line to be parallel with the first line while maintaining the second line's length and midpoint position.

## Usage

```cpp
#include "constraints/ParallelConstraint.h"

// Create two lines
std::array<double, 2> line1_start = {0, 0};
std::array<double, 2> line1_end = {1, 0};
std::array<double, 2> line2_start = {0, 1};
std::array<double, 2> line2_end = {1, 2};

// Create a parallel constraint
RebelCAD::Constraints::ParallelConstraint constraint(
    line1_start, line1_end,
    line2_start, line2_end,
    1e-6  // tolerance in radians (optional)
);

// Check if lines are parallel
bool isParallel = constraint.isSatisfied();

// Enforce parallelism if needed
if (!isParallel) {
    constraint.enforce();
}
```

## Constructor Parameters

- `line1_start`: First point of the first line `{x, y}`
- `line1_end`: Second point of the first line `{x, y}`
- `line2_start`: First point of the second line `{x, y}`
- `line2_end`: Second point of the second line `{x, y}`
- `tolerance`: Maximum allowed angular deviation in radians (default: 1e-6)

## Methods

### `bool isSatisfied() const`
Checks if the lines are parallel within the specified tolerance.
- Returns: `true` if lines are parallel, `false` otherwise

### `bool enforce()`
Attempts to make the lines parallel by rotating the second line around its midpoint.
- Returns: `true` if constraint was successfully enforced, `false` if it failed

### `double getError() const`
Gets the angular difference between the lines in radians.
- Returns: The absolute angle between the lines, or infinity if lines are invalid

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
3. When enforcing, rotating the second line around its midpoint to match the angle of the first line

The enforcement preserves:
- The length of both lines
- The midpoint position of the second line
- The position and orientation of the first line

## Error Handling

The constraint handles several edge cases:
- Zero-length lines: Returns false for `isValid()` and `isSatisfied()`
- Invalid points: Properly handles NaN and infinite values
- Numerical precision: Uses a tolerance value to account for floating-point errors

## Example: Enforcing Parallelism

```cpp
// Create two non-parallel lines
std::array<double, 2> line1_start = {0, 0};
std::array<double, 2> line1_end = {1, 0};     // Horizontal line
std::array<double, 2> line2_start = {0, 1};
std::array<double, 2> line2_end = {1, 1.5};   // Angled line

// Create constraint
auto constraint = RebelCAD::Constraints::ParallelConstraint(
    line1_start, line1_end,
    line2_start, line2_end
);

// Check initial state
std::cout << "Initially parallel: " << constraint.isSatisfied() << "\n";
std::cout << "Initial angle difference: " << constraint.getError() << " radians\n";

// Enforce parallelism
if (constraint.enforce()) {
    std::cout << "Constraint enforced successfully\n";
    std::cout << "New angle difference: " << constraint.getError() << " radians\n";
    
    // Get the adjusted line points
    auto new_start = constraint.getLine2Start();
    auto new_end = constraint.getLine2End();
}
```

## Performance Considerations

The constraint operations are computationally lightweight, involving only basic trigonometric calculations. The memory footprint is minimal, storing only four points (eight doubles) and a tolerance value.
