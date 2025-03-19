# Bezier Curve Tool

The Bezier curve tool in RebelCAD provides powerful functionality for creating and manipulating parametric curves. Bezier curves are widely used in CAD for their intuitive control and smooth interpolation properties.

## Features

- Create curves of any degree (linear, quadratic, cubic, etc.)
- Add, remove, and modify control points
- Evaluate points and derivatives along the curve
- Split curves at arbitrary parameters
- Degree elevation and reduction
- Convex hull calculation
- Closest point computation

## Usage

### Creating a Bezier Curve

```cpp
// Create an empty curve
BezierCurve curve;

// Add control points
curve.addControlPoint(0.0, 0.0);  // Start point
curve.addControlPoint(1.0, 2.0);  // Control point
curve.addControlPoint(2.0, 0.0);  // End point
```

### Evaluating Points on the Curve

```cpp
// Get a point at parameter t (0 ≤ t ≤ 1)
auto point = curve.evaluatePoint(0.5);  // Point at middle of curve

// Generate points for rendering
auto points = curve.calculateCurvePoints(100);  // Get 100 points along curve
```

### Modifying Control Points

```cpp
// Move a control point
curve.moveControlPoint(1, 1.5, 2.5);  // Move second point to (1.5, 2.5)

// Remove a control point
curve.removeControlPoint(1);  // Remove second point
```

### Advanced Operations

```cpp
// Split curve at parameter t
auto [leftCurve, rightCurve] = curve.split(0.5);

// Elevate degree
auto higherDegree = curve.elevateDegree();

// Reduce degree (if possible within tolerance)
auto lowerDegree = curve.reduceDegree(1e-6);

// Find closest point on curve to target point
auto [closestPoint, parameter] = curve.findClosestPoint(1.0, 1.0);
```

### Curve Analysis

```cpp
// Get curve derivatives
auto firstDerivative = curve.evaluateFirstDerivative(0.5);
auto secondDerivative = curve.evaluateSecondDerivative(0.5);

// Calculate convex hull
auto hull = curve.calculateConvexHull();
```

### Visual Properties

```cpp
// Set curve color
curve.setColor(Color(1.0f, 0.0f, 0.0f));  // Red color
```

## Mathematical Background

Bezier curves are defined by their control points and use Bernstein polynomials as basis functions. A Bezier curve of degree n is defined as:

P(t) = Σ(i=0 to n) Bᵢⁿ(t)Pᵢ

where:
- t is the parameter (0 ≤ t ≤ 1)
- Pᵢ are the control points
- Bᵢⁿ(t) are the Bernstein polynomials

## Implementation Details

The BezierCurve class provides:

- Robust error handling for invalid parameters
- Efficient algorithms for curve operations
- Numerical stability in calculations
- Thread-safe operations

## Performance Considerations

- Curve evaluation is O(n) where n is the degree
- Splitting and degree operations are O(n²)
- Convex hull calculation is O(n log n)
- Memory usage is linear with the number of control points

## Best Practices

1. Keep curve degree as low as possible for performance
2. Use degree elevation/reduction judiciously
3. Validate parameter values (t) between 0 and 1
4. Consider using multiple lower-degree curves instead of one high-degree curve
5. Cache calculated points for repeated rendering

## Integration with Other Tools

The Bezier curve tool integrates seamlessly with other RebelCAD features:

- Constraint system for precise control
- Transformation tools for curve manipulation
- Sketching system for 2D design
- Surface generation for 3D modeling

## Error Handling

The implementation includes comprehensive error checking:

- Invalid parameter values
- Empty curve operations
- Insufficient control points for operations
- Numerical stability checks

## Future Enhancements

Planned improvements include:

- Curve fitting from points
- Intersection calculation
- Offset curve generation
- Advanced curve analysis tools
- Performance optimizations
