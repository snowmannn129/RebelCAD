# Sweep Tool

The Sweep Tool creates 3D geometry by moving a 2D profile along a specified path. This tool is essential for creating complex shapes like pipes, cables, moldings, and other path-driven geometries.

## Features

- Create swept geometry from any 2D profile along a 3D path
- Control profile orientation during sweep
- Apply scaling along the sweep path
- Add twist to the swept geometry
- Use guide rails for advanced orientation control

## Usage

### Basic Sweep

```cpp
// Create a sweep tool instance
RebelCAD::Modeling::SweepTool sweepTool;

// Set the profile (2D curve) to sweep
sweepTool.SetProfile(profile);

// Set the path to sweep along
sweepTool.SetPath(path);

// Execute with default options
sweepTool.Execute();
```

### Advanced Options

```cpp
// Configure sweep options
RebelCAD::Modeling::SweepTool::SweepOptions options;

// Set orientation mode
options.orientationMode = RebelCAD::Modeling::SweepTool::OrientationMode::PathNormal;

// Add twist along the path
options.SetTwistAngle(45.0f);  // 45 degree twist

// Scale the profile along the path
options.SetScaleFactors(1.0f, 2.0f);  // Start normal, end at 2x size

// Execute with custom options
sweepTool.Execute(options);
```

### Using Guide Rails

```cpp
// Create and configure the sweep tool
RebelCAD::Modeling::SweepTool sweepTool;
sweepTool.SetProfile(profile);
sweepTool.SetPath(path);

// Add a guide rail for orientation control
sweepTool.SetGuideRail(guideRail);

// Configure options to use guide rail
RebelCAD::Modeling::SweepTool::SweepOptions options;
options.orientationMode = RebelCAD::Modeling::SweepTool::OrientationMode::GuideRail;

// Execute the sweep
sweepTool.Execute(options);
```

## Preview and Validation

The tool supports previewing the sweep result before committing:

```cpp
// Preview the sweep
sweepTool.Preview(graphicsSystem);

// If the preview looks good, execute
sweepTool.Execute();

// Or cancel if needed
sweepTool.Cancel();
```

## Error Handling

The tool provides detailed error messages for common issues:

- Invalid or null profile/path
- Missing guide rail when GuideRail orientation mode is selected
- Invalid profile/path dimensions
- Preview system errors

## Implementation Notes

The SweepTool uses the following process to create swept geometry:

1. Validates input profile and path
2. Samples points along the path
3. Creates transformation matrices at each point
4. Transforms and scales profile according to options
5. Generates surface/solid geometry
6. Applies twist if specified
7. Handles orientation based on selected mode

## Best Practices

- Ensure profile is a closed 2D curve for solid geometry
- Use PathNormal orientation for most cases
- Add guide rails when specific orientation control is needed
- Preview complex sweeps before executing
- Keep twist angles moderate for better results
- Use appropriate scale factors to avoid self-intersection

## Limitations

- Profile must be a valid 2D curve
- Path must be a valid 3D curve
- Guide rail must be a valid 3D curve
- Twist angles should be between -360 and 360 degrees
- Scale factors should be positive non-zero values
