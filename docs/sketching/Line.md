# Line Class Documentation

## Overview

The `Line` class represents a 2D line segment in the RebelCAD sketching system. It provides comprehensive functionality for creating, manipulating, and analyzing line segments, including geometric operations and styling options for rendering.

## Features

### Basic Operations

- Create lines between two points
- Get/Set start and end points
- Calculate line length
- Translate (move) the line
- Rotate the line around its start point

### Geometric Operations

- Calculate midpoint
- Find intersection with another line
- Calculate angle between lines
- Check if a point lies on the line
- Create parallel lines at specified distances
- Create perpendicular lines through points

### Styling and Rendering

- Customizable line color
- Adjustable line thickness
- Support for dashed line patterns

## Usage Examples

### Creating a Line

```cpp
// Create a line from (0,0) to (10,10)
Line line(0.0f, 0.0f, 10.0f, 10.0f);
```

### Modifying Line Points

```cpp
// Change start point
line.setStartPoint(1.0f, 1.0f);

// Change end point
line.setEndPoint(11.0f, 11.0f);
```

### Geometric Calculations

```cpp
// Get line length
float length = line.getLength();

// Get midpoint
auto midpoint = line.getMidpoint();

// Check if point lies on line (with default tolerance)
bool contains = line.containsPoint(5.0f, 5.0f);

// Find intersection with another line
Line other(0.0f, 10.0f, 10.0f, 0.0f);
auto intersection = line.getIntersection(other);
if (intersection) {
    float x = (*intersection)[0];
    float y = (*intersection)[1];
}

// Calculate angle between lines (in radians)
float angle = line.getAngleTo(other);
```

### Creating Related Lines

```cpp
// Create parallel line 5 units to the left
Line parallel = line.createParallelLine(5.0f);

// Create perpendicular line through point (5,5)
Line perpendicular = line.createPerpendicularLine(5.0f, 5.0f);
```

### Transformations

```cpp
// Move line by vector (dx,dy)
line.translate(10.0f, 5.0f);

// Rotate line 90 degrees (π/2 radians) around start point
line.rotate(M_PI / 2.0f);
```

### Rendering with Style

```cpp
auto graphics = std::make_shared<Graphics::GraphicsSystem>();

// Basic rendering with color
line.render(graphics, Graphics::Color::Blue);

// Rendering with custom thickness
line.render(graphics, Graphics::Color::Red, 2.0f);

// Rendering with dash pattern
std::vector<float> dashPattern = {5.0f, 2.0f}; // 5 units on, 2 units off
line.render(graphics, Graphics::Color::Black, 1.0f, &dashPattern);
```

## Error Handling

The Line class includes robust error checking:

- Validates all coordinate inputs
- Throws `Error` for invalid coordinates (NaN, infinity)
- Enforces reasonable coordinate bounds
- Handles edge cases in geometric calculations

## Best Practices

1. Always check return values from intersection calculations
2. Use appropriate tolerances for point containment checks
3. Consider numerical precision in geometric calculations
4. Handle potential errors when modifying line endpoints
5. Use const references when passing Line objects as parameters

## Performance Considerations

- Geometric calculations are optimized for common cases
- Caching of frequently used values (length, slope)
- Efficient intersection testing with bounding box checks
- Minimal memory footprint (only stores essential data)

## Integration with Graphics System

The Line class integrates seamlessly with RebelCAD's graphics system:
- Supports hardware-accelerated rendering
- Compatible with both OpenGL and Vulkan backends
- Efficient batch rendering of multiple lines
- Support for different line styles and patterns
