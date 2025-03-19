# Arc Tool Documentation

## Overview

The Arc tool in RebelCAD enables users to create and manipulate circular arcs in 2D sketches. An arc is defined by its center point, radius, and angular range (start and end angles). This tool is essential for creating curved geometry in mechanical designs, architectural drawings, and other CAD applications.

## Features

### Basic Properties
- Center point (x, y coordinates)
- Radius
- Start angle (in radians)
- End angle (in radians)
- Sweep angle (angular extent)

### Geometric Operations
- Translation (moving the arc)
- Rotation (around center point)
- Point containment testing
- Intersection calculation with lines and other arcs
- Creation of concentric arcs

## Usage

### Creating an Arc
```cpp
// Create an arc at (0,0) with radius 10, from 0° to 90°
Arc arc(0.0f, 0.0f, 10.0f, 0.0f, M_PI/2.0f);
```

### Modifying an Arc
```cpp
// Move the arc
arc.translate(5.0f, -3.0f);

// Rotate the arc by 45 degrees
arc.rotate(M_PI/4.0f);

// Change the radius
arc.setRadius(15.0f);

// Modify the angular range
arc.setStartAngle(M_PI/6.0f);    // 30 degrees
arc.setEndAngle(2.0f*M_PI/3.0f); // 120 degrees
```

### Querying Arc Properties
```cpp
// Get arc properties
auto center = arc.getCenter();     // Returns {x, y}
float radius = arc.getRadius();
float sweep = arc.getSweepAngle();
float length = arc.getLength();

// Get points on the arc
auto start = arc.getStartPoint();  // Start point coordinates
auto end = arc.getEndPoint();      // End point coordinates
auto mid = arc.getMidpoint();      // Midpoint coordinates
```

### Geometric Operations
```cpp
// Check if a point lies on the arc
bool onArc = arc.containsPoint(x, y, tolerance);

// Find intersections with a line
Line line(x1, y1, x2, y2);
auto lineIntersections = arc.getLineIntersections(line);

// Find intersections with another arc
Arc otherArc(cx, cy, r, start, end);
auto arcIntersections = arc.getArcIntersections(otherArc);

// Create a concentric arc
Arc concentric = arc.createConcentricArc(newRadius);
```

### Rendering
```cpp
// Render the arc with specified color and thickness
arc.render(graphicsSystem, color, thickness);

// Render with dash pattern
std::vector<float> dashPattern = {5.0f, 2.0f}; // 5 pixels on, 2 pixels off
arc.render(graphicsSystem, color, thickness, &dashPattern);
```

## Best Practices

1. **Angle Conventions**
   - Angles are specified in radians
   - Positive angles go counterclockwise
   - Zero angle points along positive X-axis
   - Angles are automatically normalized to [0, 2π)

2. **Error Handling**
   - Constructor and setter methods validate input parameters
   - Invalid parameters (negative radius, infinite coordinates) throw errors
   - Use try-catch blocks when working with arcs

3. **Numerical Precision**
   - Use appropriate tolerance values for point containment tests
   - Consider floating-point precision in geometric calculations
   - Intersection calculations handle edge cases and near-misses

4. **Performance Considerations**
   - Arc rendering automatically adjusts segment count based on radius and sweep
   - Intersection calculations are optimized for common cases
   - Use concentric arc creation instead of manual calculations

## Integration with Other Tools

The Arc tool is designed to work seamlessly with other RebelCAD sketching tools:

- **Lines**: Calculate intersections between arcs and lines
- **Circles**: Create full circles using arcs with 2π sweep
- **Constraints**: Apply geometric constraints to arc endpoints and center
- **Dimensions**: Measure and control arc radius and angles

## Examples

### Creating a Fillet
```cpp
// Create a fillet between two lines using an arc
Line line1(0.0f, 0.0f, 10.0f, 0.0f);
Line line2(10.0f, 0.0f, 10.0f, 10.0f);
float filletRadius = 2.0f;

// Calculate fillet arc center and angles
// (Implementation depends on geometry calculation utilities)
Arc fillet(8.0f, 2.0f, filletRadius, 0.0f, M_PI/2.0f);
```

### Creating a Circular Pattern
```cpp
// Create a pattern of arcs around a center point
float centerX = 0.0f, centerY = 0.0f;
float radius = 10.0f;
int count = 8;

for (int i = 0; i < count; ++i) {
    float startAngle = i * 2.0f * M_PI / count;
    float endAngle = startAngle + M_PI / 4.0f;  // 45-degree arc
    Arc arc(centerX, centerY, radius, startAngle, endAngle);
    // Use the arc...
}
```

## See Also

- [Line Tool Documentation](Line.md)
- [Circle Tool Documentation](Circle.md) (Coming Soon)
- [Geometric Constraints Documentation](Constraints.md) (Coming Soon)
