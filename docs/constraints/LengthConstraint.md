# Length Constraint

The Length Constraint ensures that geometric entities maintain a specific length. This constraint is fundamental for precise technical drawing and CAD operations.

## Features

- Enforces exact lengths on lines
- Maintains length when moving endpoints
- Handles degenerate cases gracefully
- Provides precise error reporting
- Supports configurable tolerance

## Usage

### Basic Line Length Constraint

```cpp
// Create a length constraint for a line from (0,0) to (3,4) with length 5
auto constraint = LengthConstraint({0, 0}, {3, 4}, 5.0);

// Check if the constraint is satisfied
if (constraint.isSatisfied()) {
    // Line is the correct length
}

// Enforce the constraint (adjusts end point to match target length)
constraint.enforce();
```

### Modifying Constrained Lines

```cpp
// Update target length
constraint.setTargetLength(6.0);

// Move line while maintaining constraint
constraint.setStartPoint({1, 1});
constraint.enforce(); // Adjusts end point to maintain length

// Get current length for verification
double length = constraint.getCurrentLength();
```

### Error Handling

```cpp
// Get deviation from target length
double error = constraint.getError();

// Check if constraint is valid
if (constraint.isValid()) {
    // Constraint can be enforced
}
```

## Technical Details

### Constructor Parameters

- `start_point`: 2D coordinates of line start point {x, y}
- `end_point`: 2D coordinates of line end point {x, y}
- `target_length`: Desired length of the line (must be positive)
- `tolerance`: Maximum allowed deviation from target length (default: 1e-6)

### Error Handling

The constraint throws `std::invalid_argument` for:
- Non-positive target lengths
- Non-positive tolerance values

### Performance Optimization

The constraint implements caching for:
- Current length calculation
- Error computation

Cache is automatically invalidated when:
- Start/end points are modified
- Target length is changed

## Future Extensions

The Length Constraint system is designed to be extended to support:
- Arc length constraints
- Circle diameter constraints
- Polyline total length constraints

## Best Practices

1. Always check `isValid()` before enforcing constraints
2. Use appropriate tolerance for your application's precision needs
3. Handle constraint enforcement failures gracefully
4. Consider performance impact when modifying constrained entities frequently

## Example Workflow

```cpp
try {
    // Create constraint
    LengthConstraint constraint({0, 0}, {3, 4}, 5.0);
    
    // Verify initial state
    if (!constraint.isSatisfied()) {
        // Enforce correct length
        if (!constraint.enforce()) {
            // Handle enforcement failure
        }
    }
    
    // Modify as needed
    constraint.setStartPoint({1, 1});
    constraint.enforce();
    
} catch (const std::invalid_argument& e) {
    // Handle invalid parameters
}
