# Concentric Constraint

The Concentric Constraint ensures that two geometric entities (circles and/or arcs) share the same center point. This is a fundamental constraint in CAD systems for creating concentric geometry.

## Usage

```cpp
// Create two circles
auto circle1 = std::make_shared<Circle>(0.0f, 0.0f, 5.0f);
auto circle2 = std::make_shared<Circle>(1.0f, 1.0f, 3.0f);

// Create a concentric constraint between them
ConcentricConstraint constraint(circle1, circle2);

// Enforce the constraint
constraint.enforce();  // Centers will be moved to the midpoint (0.5, 0.5)
```

## Features

- Supports both Circle-Circle and Circle-Arc constraints
- Automatically calculates and enforces center point alignment
- Provides error measurement for constraint satisfaction
- Handles invalid/null entities gracefully

## API Reference

### Constructor

```cpp
ConcentricConstraint(Entity first, Entity second)
```

Creates a new concentric constraint between two entities (circles or arcs).

**Parameters:**
- `first`: First circle or arc entity
- `second`: Second circle or arc entity

**Throws:**
- `Error` if either entity is null

### Methods

#### `bool isSatisfied() const`

Checks if the constraint is currently satisfied (centers are aligned within tolerance).

**Returns:**
- `true` if entities are concentric
- `false` otherwise

#### `bool enforce()`

Enforces the concentric constraint by moving both entities' centers to their midpoint.

**Returns:**
- `true` if constraint was successfully enforced
- `false` if enforcement failed

#### `double getError() const`

Gets the current error magnitude (distance between centers).

**Returns:**
- Distance between entity centers (0 if perfectly concentric)
- `infinity` if constraint is invalid

#### `bool isValid() const`

Checks if the constraint can be enforced.

**Returns:**
- `true` if both entities exist and are valid
- `false` otherwise

## Examples

### Circle-Arc Constraint

```cpp
// Create a circle and an arc
auto circle = std::make_shared<Circle>(0.0f, 0.0f, 5.0f);
auto arc = std::make_shared<Arc>(1.0f, 1.0f, 3.0f, 0.0f, M_PI);

// Make them concentric
ConcentricConstraint constraint(circle, arc);
constraint.enforce();

// Centers are now at (0.5, 0.5)
```

### Checking Constraint Status

```cpp
ConcentricConstraint constraint(circle1, circle2);

// Check if already concentric
if (constraint.isSatisfied()) {
    std::cout << "Entities are concentric\n";
} else {
    std::cout << "Error: " << constraint.getError() << " units\n";
    constraint.enforce();
}
```

## Implementation Details

The constraint works by:
1. Validating both entities exist and are valid
2. Computing the current distance between centers
3. When enforcing, moving both entities to the midpoint of their centers
4. Using a small tolerance (1e-5) for floating-point comparisons

## Best Practices

- Always check `isValid()` before using the constraint
- Handle the case where `enforce()` returns false
- Consider the tolerance when checking `isSatisfied()`
- Use shared pointers to manage entity lifetime
