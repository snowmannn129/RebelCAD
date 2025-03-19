# LoftTool

The LoftTool creates 3D solid or surface geometry by transitioning between two or more profile sections. It supports various options for controlling the loft operation, including guide curves and different transition types.

## Features

- Create lofted geometry between multiple profile sections
- Support for both ruled (linear) and smooth transitions
- Guide curves to control the loft path
- Customizable start and end tangency conditions
- Preview functionality with real-time visualization
- Support for closed loft operations

## Usage

```cpp
// Create a LoftTool instance
LoftTool loftTool;

// Add profile sections (must be closed contours)
auto profile1 = std::make_shared<Sketching::Spline>();
profile1->addControlPoint(0, 0);
profile1->addControlPoint(1, 0);
profile1->addControlPoint(1, 1);
profile1->addControlPoint(0, 1);
profile1->addControlPoint(0, 0);  // Close the contour

auto profile2 = std::make_shared<Sketching::Spline>();
profile2->addControlPoint(0, 0);
profile2->addControlPoint(2, 0);
profile2->addControlPoint(2, 2);
profile2->addControlPoint(0, 2);
profile2->addControlPoint(0, 0);  // Close the contour

loftTool.AddProfile(profile1);
loftTool.AddProfile(profile2);

// Optional: Add guide curves for controlling the loft path
auto guide = std::make_shared<Sketching::Spline>();
guide->addControlPoint(0, 0);
guide->addControlPoint(1, 1);
guide->addControlPoint(2, 1);
loftTool.AddGuideCurve(guide);

// Configure loft options
LoftTool::LoftOptions options;
options.transitionType = LoftTool::TransitionType::Smooth;  // or TransitionType::Ruled
options.startCondition = LoftTool::TangencyCondition::Normal;  // or Natural/Custom
options.endCondition = LoftTool::TangencyCondition::Natural;
options.closed = false;  // Set to true to connect last profile back to first

// Preview the loft result
auto graphics = std::make_shared<Graphics::GraphicsSystem>();
loftTool.Preview(graphics);

// Execute the loft operation
auto result = loftTool.Execute(options);
if (result.code() != rebel::core::ErrorCode::None) {
    // Handle error
    std::cerr << "Loft operation failed: " << result.message() << std::endl;
}
```

## Transition Types

### Ruled
- Linear interpolation between corresponding points
- Faster computation
- Suitable for simple transitions between similar profiles

### Smooth
- Uses guide curves and tangency conditions for smooth transitions
- Better quality for complex shapes
- Supports various tangency controls at start/end

## Tangency Conditions

### Natural
- Default tangency based on guide curves or profile geometry
- Provides smooth transitions without explicit control

### Normal
- Uses profile normal vectors for tangency
- Useful for maintaining perpendicular transitions

### Custom
- User-defined tangent directions
- Maximum control over transition shape

## Guide Curves

Guide curves control the path and shape of the loft transition. They can be used to:
- Define non-linear paths between profiles
- Control surface curvature
- Maintain specific geometric relationships

## Error Handling

The tool provides detailed error messages for common issues:
- Invalid profiles (non-closed contours)
- Insufficient number of profiles
- Incompatible profile point counts
- Guide curve validation failures

## Preview System

The preview functionality allows real-time visualization of the loft result:
- Semi-transparent preview mesh
- Interactive updates when parameters change
- Helps verify the result before committing

## Performance Considerations

- Profile point count affects mesh density and performance
- Guide curves add computational overhead for smooth transitions
- Closed lofts require additional geometry calculations
- Preview system optimized for real-time feedback

## Best Practices

1. Ensure profiles are closed contours
2. Use similar point counts across profiles for better results
3. Add guide curves for complex transitions
4. Preview results before final execution
5. Consider using ruled transitions for simple, linear cases
6. Validate profile geometry before lofting
7. Use appropriate tangency conditions for desired shape

## Implementation Details

The LoftTool uses:
- Catmull-Rom splines for smooth interpolation
- Profile normal calculation for tangency control
- Efficient mesh generation algorithms
- OpenGL/Vulkan for preview rendering
- Smart pointer management for resource handling
