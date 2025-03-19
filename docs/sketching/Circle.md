# Circle Tool Documentation

## Overview

The Circle tool in RebelCAD provides functionality for creating and manipulating 2D circles in sketches. A circle is defined by its center point and radius, offering essential capabilities for geometric construction and design.

## Features

### Basic Properties
- Center point (x, y coordinates)
- Radius
- Circumference calculation
- Area calculation

### Geometric Operations
- Translation (moving the circle)
- Point containment testing (both on circle and inside circle)
- Point generation at specified angles
- Intersection calculations with lines, arcs, and other circles
- Creation of concentric circles and arcs

## Usage

### Creating a Circle
```cpp
// Create a circle at (0,0) with radius 10
Circle circle(0.0f, 0.0f, 10.0f);
```

### Modifying a Circle
```cpp
// Move the circle
circle.translate(5.0f, -3.0f);

// Change the center point
circle.setCenter(10.0f, 10.0f);

// Change the radius
circle.setRadius(15.0f);
```

### Querying Circle Properties
```cpp
// Get circle properties
auto center = circle.getCenter();     // Returns {x, y}
float radius = circle.getRadius();
float circumference = circle.getCircumference();
float area = circle.getArea();

// Get a point on the circle at a specific angle
auto point = circle.getPointAtAngle(M_PI/4.0f); // 45 degrees
```

### Point Containment
```cpp
// Check if a point lies on the circle
bool onCircle = circle.containsPoint(x, y, tolerance);

// Check if a point lies inside the circle
bool inside = circle.containsPointInside(x, y);
```

### Geometric Operations
```cpp
// Find intersections with a line
Line line(x1, y1, x2, y2);
auto lineIntersections = circle.getLineIntersections(line);

// Find intersections with an arc
Arc arc(cx, cy, r, startAngle, endAngle);
auto arcIntersections = circle.getArcIntersections(arc);

// Find intersections with another circle
Circle other(cx, cy, r);
auto circleIntersections = circle.getCircleIntersections(other);

// Create a concentric circle
Circle concentric = circle.createConcentricCircle(newRadius);

// Create an arc from this circle
Arc arc = circle.createArc(startAngle, endAngle);
```

### Rendering
```cpp
// Render the circle with specified color and thickness
circle.render(graphicsSystem, color, thickness);

// Render with dash pattern
std::vector<float> dashPattern = {5.0f, 2.0f}; // 5 pixels on, 2 pixels off
circle.render(graphicsSystem, color, thickness, &dashPattern);
```

## Best Practices

1. **Parameter Validation**
   - Center coordinates must be finite numbers
   - Radius must be positive and finite
   - Invalid parameters will throw errors

2. **Numerical Precision**
   - Use appropriate tolerance values for point containment tests
   - Consider floating-point precision in geometric calculations
   - Intersection calculations handle edge cases and near-misses

3. **Performance Considerations**
   - Circle rendering automatically adjusts segment count based on radius
   - Intersection calculations are optimized for common cases
   - Use concentric circle creation instead of manual calculations

4. **Error Handling**
   - Constructor and setter methods validate input parameters
   - Invalid parameters (negative radius, infinite coordinates) throw errors
   - Use try-catch blocks when working with circles

## Integration with Other Tools

The Circle tool is designed to work seamlessly with other RebelCAD sketching tools:

- **Lines**: Calculate intersections between circles and lines
- **Arcs**: Create arcs from circles and calculate intersections
- **Constraints**: Apply geometric constraints to circle center and radius
- **Dimensions**: Measure and control circle radius and position

## Examples

### Creating a Pattern of Circles
```cpp
// Create a circular pattern of circles
float centerX = 0.0f, centerY = 0.0f;
float radius = 20.0f;
int count = 6;
float patternRadius = 50.0f;

for (int i = 0; i < count; ++i) {
    float angle = i * 2.0f * M_PI / count;
    float x = centerX + patternRadius * std::cos(angle);
    float y = centerY + patternRadius * std::sin(angle);
    Circle circle(x, y, radius);
    // Use the circle...
}
```

### Finding Tangent Points
```cpp
// Find points where a line from a point is tangent to the circle
Circle circle(0.0f, 0.0f, 10.0f);
float px = 20.0f, py = 0.0f;  // External point

// Calculate tangent points
// (Implementation depends on geometry calculation utilities)
// This would typically involve finding points where a line
// from the external point touches the circle
```

## See Also

- [Line Tool Documentation](Line.md)
- [Arc Tool Documentation](Arc.md)
- [Geometric Constraints Documentation](Constraints.md) (Coming Soon)
