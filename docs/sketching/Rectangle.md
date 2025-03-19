# Rectangle Tool

The Rectangle tool in RebelCAD allows users to create and manipulate rectangular shapes in 2D sketching mode.

## Features

- Create rectangles by specifying two corner points
- Automatic normalization of coordinates (order of points doesn't matter)
- Calculate rectangle properties (width, height, area, perimeter)
- Point containment testing
- Color customization

## Usage

### Creating a Rectangle

To create a rectangle, specify two corner points:

```cpp
Rectangle rect(x1, y1, x2, y2);
```

The coordinates will be automatically normalized so that:
- `(x1, y1)` becomes the top-left corner
- `(x2, y2)` becomes the bottom-right corner

### Properties

The following properties can be accessed:

- `getWidth()`: Returns the width of the rectangle
- `getHeight()`: Returns the height of the rectangle
- `getArea()`: Returns the area of the rectangle
- `getPerimeter()`: Returns the perimeter of the rectangle
- `getFirstCorner()`: Returns the coordinates of the first corner as `std::array<double, 2>`
- `getSecondCorner()`: Returns the coordinates of the second corner as `std::array<double, 2>`

### Color Management

The rectangle's color can be customized:

```cpp
// Create a rectangle with a specific color
Color blue(0, 0, 1);
Rectangle rect(0, 0, 5, 5, blue);

// Change the color later
Color red(1, 0, 0);
rect.setColor(red);
```

### Point Containment

Test if a point lies inside the rectangle:

```cpp
Rectangle rect(0, 0, 5, 5);
bool isInside = rect.containsPoint(2.5, 2.5); // Returns true
```

## Best Practices

1. When creating rectangles, consider the coordinate system orientation:
   - X increases from left to right
   - Y increases from top to bottom

2. For better precision, use double-precision floating-point coordinates

3. Always check return values when testing point containment

## Integration with Other Tools

The Rectangle tool can be used in combination with other sketching tools:
- As a bounding box for other shapes
- For creating regular patterns
- As a base for more complex geometric constructions

## Performance Considerations

The Rectangle implementation is optimized for:
- Fast point containment testing
- Efficient memory usage
- Quick property calculations

## Future Enhancements

Planned features for future versions:
- Rotation support
- Constraint system integration
- Pattern creation tools
- Advanced transformation operations
