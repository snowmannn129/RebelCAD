# Multi-Monitor Support in RebelCAD

RebelCAD provides comprehensive support for multi-monitor setups, allowing you to efficiently utilize multiple displays in your CAD workflow.

## Features

- **Monitor Detection**: Automatically detects all connected monitors and their properties
- **Window Management**: Move and arrange RebelCAD windows across multiple displays
- **Per-Monitor DPI Awareness**: Proper scaling on monitors with different DPI settings
- **Dynamic Monitor Changes**: Handles monitor connection/disconnection events gracefully

## Using Multi-Monitor Features

### Via Menu

1. Navigate to `View > Monitors` in the main menu
2. Choose from the following options:
   - Select a specific monitor to move the window to
   - Choose window mode (Centered/Maximized) for the current monitor

### Window Behaviors

- **Window Movement**: Windows can be freely moved between monitors
- **Window State**: Each window remembers its position and state per monitor
- **DPI Handling**: UI elements automatically scale when moving between monitors with different DPI settings

### Monitor-Specific Features

- **Primary Monitor**: Identified with a checkmark in the monitors menu
- **Window Modes**:
  - Centered: Places window in the center of the selected monitor
  - Maximized: Maximizes window on the selected monitor

## Technical Details

### Monitor Information

For each connected monitor, RebelCAD tracks:
- Physical resolution
- Work area (accounting for taskbar/dock)
- Refresh rate
- DPI scaling factor
- Monitor name and identification

### Window Management

- Windows maintain proper aspect ratios when moving between monitors
- Automatic repositioning if a monitor becomes unavailable
- Smooth transitions when moving between displays

## Best Practices

1. **Window Organization**:
   - Keep tool palettes on secondary monitors
   - Use main modeling viewport on your primary display
   - Arrange reference windows on additional displays

2. **Performance Considerations**:
   - Each additional window may impact performance
   - Consider monitor refresh rates when positioning animation previews

3. **Display Settings**:
   - Ensure all monitors are properly calibrated
   - Match color settings across displays for consistent visualization
   - Consider monitor orientation for optimal workspace layout

## Troubleshooting

### Common Issues

1. **Window Positioning**:
   - If a window becomes unreachable, use View > Monitors > Primary Monitor to reset its position
   - Use Window Mode > Centered to ensure visibility

2. **Display Detection**:
   - If a monitor is not detected, try disconnecting and reconnecting it
   - Ensure your graphics drivers are up to date

3. **DPI Scaling**:
   - If UI elements appear too large/small, check your system's display scaling settings
   - Restart RebelCAD after making system display changes

### Support

For additional help with multi-monitor setups:
- Check the RebelCAD user manual
- Visit the support forum
- Contact technical support with your monitor configuration details

## Future Enhancements

Planned improvements for multi-monitor support:
- Per-monitor color profiles
- Customizable window layouts per monitor
- Enhanced touch and pen input support for compatible displays
- Monitor-specific workspace configurations
