# Auto Constraint Detection

The Auto Constraint Detection system automatically analyzes spatial relationships between sketch elements and suggests appropriate geometric constraints. This feature helps users by automatically identifying and applying common constraints, reducing the time needed for manual constraint application.

## Usage

```cpp
// Create an auto constraint detector with default settings
RebelCAD::AutoConstraintDetector detector;

// Or with custom configuration
RebelCAD::AutoConstraintDetector::Config config;
config.parallelTolerance = 0.02;      // More lenient parallel detection
config.concentricTolerance = 0.0005;  // Stricter concentric detection
RebelCAD::AutoConstraintDetector detector(config);

// Detect constraints for a set of sketch elements
std::vector<std::shared_ptr<SketchElement>> elements = /* your sketch elements */;
auto suggestedConstraints = detector.detectConstraints(elements);
```

## Supported Relationships

The detector can identify the following geometric relationships:

1. **Line-Line Relationships**
   - Parallel lines
   - Perpendicular lines
   - Equal length lines
   - Horizontal/vertical alignment

2. **Circle-Circle Relationships**
   - Concentric circles
   - Tangent circles
   - Equal radius circles

3. **Circle-Arc Relationships**
   - Concentric circle-arc pairs
   - Tangent circle-arc pairs
   - Equal radius circle-arc pairs

4. **Arc-Arc Relationships**
   - Concentric arcs
   - Tangent arcs
   - Equal radius arcs

## Configuration Parameters

The `Config` struct allows fine-tuning of the detection tolerances:

| Parameter | Default | Description |
|-----------|---------|-------------|
| parallelTolerance | 0.01 | Maximum angle difference (in radians) for parallel detection |
| perpendicularTolerance | 0.01 | Maximum deviation from 90° (in radians) |
| concentricTolerance | 0.001 | Maximum center point distance for concentricity |
| tangentTolerance | 0.001 | Maximum deviation from perfect tangency |
| horizontalTolerance | 0.01 | Maximum angle from horizontal (in radians) |
| verticalTolerance | 0.01 | Maximum angle from vertical (in radians) |
| equalLengthTolerance | 0.001 | Maximum length difference for equality |
| equalRadiusTolerance | 0.001 | Maximum radius difference for equality |

## Best Practices

1. **Tolerance Selection**
   - Start with default tolerances
   - Adjust based on your specific needs and drawing scale
   - Tighter tolerances reduce false positives but may miss intended relationships
   - Looser tolerances catch more relationships but may suggest unwanted constraints

2. **Performance Considerations**
   - The detector analyzes all possible pairs of elements
   - Consider limiting the number of elements analyzed at once for large sketches
   - Use the detector strategically, such as when:
     * A new element is added to the sketch
     * Elements are moved or modified
     * User requests automatic constraint detection

3. **User Interface Integration**
   - Highlight suggested constraints before applying them
   - Allow users to accept/reject individual suggestions
   - Provide visual feedback for detected relationships
   - Consider adding a "sensitivity" slider in the UI to adjust tolerances

## Example Workflow

1. User draws several sketch elements
2. Auto constraint detection is triggered (automatically or manually)
3. System analyzes element relationships
4. Suggested constraints are displayed to the user
5. User reviews and accepts desired constraints
6. Selected constraints are applied to the sketch

## Error Handling

The detector includes robust error handling:
- Invalid or null elements are safely ignored
- Numerical stability is maintained through appropriate tolerances
- Edge cases (e.g., zero-length lines) are properly handled

## Future Enhancements

Planned improvements include:
- Support for additional geometric relationships
- Machine learning-based relationship detection
- Performance optimizations for large sketches
- Integration with constraint solving system
- Smart constraint suggestion based on user patterns
