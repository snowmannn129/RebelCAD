# RadiusConstraint

The RadiusConstraint is used to enforce a specific radius value on circles and arcs in RebelCAD sketches.

## Overview

The RadiusConstraint ensures that a circle or arc maintains a specified radius value. This constraint is useful for:
- Defining exact sizes of circular shapes
- Maintaining consistent dimensions across multiple circles/arcs
- Parametric modeling where circle/arc sizes need to be controlled

## Usage

### Creating a Radius Constraint

```cpp
// For circles
auto circle = std::make_shared<Circle>(Point(0, 0), 5.0);
RadiusConstraint circleConstraint(circle, 10.0); // Constrain to 10 units radius

// For arcs
auto arc = std::make_shared<Arc>(Point(0, 0), 5.0, 0, M_PI);
RadiusConstraint arcConstraint(arc, 10.0); // Constrain to 10 units radius
```

### Enforcing the Constraint

The constraint can be enforced at any time by calling the `enforce()` method:

```cpp
if (constraint.isValid()) {
    constraint.enforce();
}
```

### Checking Constraint Status

Several methods are available to check the constraint's status:

```cpp
// Check if the constraint is valid (non-null shape, positive radius)
bool valid = constraint.isValid();

// Check if the constraint is currently satisfied
bool satisfied = constraint.isSatisfied();

// Get the current error (difference between actual and target radius)
double error = constraint.getError();
```

## Important Considerations

1. **Validation**
   - The constraint requires a non-null circle or arc
   - The target radius must be positive (greater than 0)
   - Invalid constraints will return false for `isValid()`

2. **Tolerance**
   - The constraint uses a small tolerance value (1e-6) when checking satisfaction
   - Values within this tolerance are considered equal

3. **Error Handling**
   - `getError()` returns infinity for invalid constraints
   - `enforce()` returns false if the constraint is invalid or cannot be satisfied

## Best Practices

1. **Always Check Validity**
   ```cpp
   if (constraint.isValid()) {
       // Safe to use the constraint
   }
   ```

2. **Error Monitoring**
   ```cpp
   double error = constraint.getError();
   if (error > threshold) {
       // Handle large deviations
   }
   ```

3. **Constraint Satisfaction**
   ```cpp
   if (!constraint.isSatisfied()) {
       bool success = constraint.enforce();
       // Handle enforcement result
   }
   ```

## Integration with Other Constraints

The RadiusConstraint can be used alongside other geometric constraints. When multiple constraints affect the same shape:

1. Consider the order of constraint enforcement
2. Verify that constraints don't conflict
3. Use a constraint solver for complex constraint systems

## Performance Considerations

- The constraint performs minimal calculations
- Enforcement is O(1) as it directly sets the radius
- Memory overhead is minimal, storing only the shape reference and target radius
