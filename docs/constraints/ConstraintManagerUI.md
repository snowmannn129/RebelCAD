# Constraint Manager UI

The Constraint Manager UI provides a centralized interface for managing geometric and dimensional constraints in RebelCAD. This document explains how to use the Constraint Manager UI effectively.

## Features

### Auto-Constraint Detection

The Constraint Manager includes an automatic constraint detection system that can identify and suggest constraints while sketching.

- **Enable/Disable**: Toggle auto-constraint detection using the checkbox in the settings section
- **Sensitivity**: Adjust the detection sensitivity using the slider (when auto-constraint is enabled)
  - Higher values (closer to 1.0) make the system more sensitive to potential constraints
  - Lower values (closer to 0.1) require more precise matching
  - Default sensitivity is 0.5

### Constraint Filtering

The Constraint Manager allows you to filter which types of constraints are displayed:

- Parallel
- Perpendicular
- Concentric
- Tangent
- Length
- Angle
- Radius

Use the checkboxes in the Filters section to show/hide specific constraint types. This helps manage complex sketches by focusing on relevant constraints.

### Active Constraints List

The main section displays all currently active constraints:

- Each constraint shows its type and affected elements
- Expand a constraint to view and edit its parameters
- Remove constraints using the "Remove" button
- Invalid constraints (e.g., referencing deleted elements) are automatically cleaned up

## Usage

### Managing Constraints

1. **Adding Constraints**:
   - Constraints are typically added through sketch interactions
   - Auto-detected constraints appear with a visual indicator
   - Click to accept or dismiss suggested constraints

2. **Modifying Constraints**:
   - Click the constraint in the list to expand its properties
   - Adjust parameters as needed
   - Changes take effect immediately

3. **Removing Constraints**:
   - Use the "Remove" button next to each constraint
   - Invalid constraints are automatically removed during refresh

### Auto-Constraint Settings

1. **Enabling Auto-Detection**:
   - Toggle the "Enable Auto-Constraints" checkbox
   - When enabled, the system automatically suggests constraints while sketching

2. **Adjusting Sensitivity**:
   - Use the sensitivity slider to control detection threshold
   - Higher values detect more potential constraints
   - Lower values require more precise geometric relationships

### Filter Management

1. **Using Filters**:
   - Open the Filters section
   - Toggle checkboxes for different constraint types
   - Only selected constraint types will be shown in the list

2. **Common Filter Combinations**:
   - Show only dimensional constraints (Length, Angle, Radius)
   - Show only geometric constraints (Parallel, Perpendicular, etc.)
   - Custom combinations for specific modeling tasks

## Best Practices

1. **Auto-Constraint Usage**:
   - Start with default sensitivity (0.5)
   - Adjust based on sketch complexity
   - Disable for precise manual control

2. **Constraint Organization**:
   - Use filters to focus on relevant constraints
   - Regularly clean up unused constraints
   - Group related constraints logically

3. **Performance Considerations**:
   - Large numbers of constraints may impact performance
   - Use filters to manage complex sketches
   - Remove unnecessary constraints

## Integration

The Constraint Manager UI integrates with:
- Sketch tools
- Auto-constraint detection system
- Geometric constraint solver
- Undo/redo system

## Technical Details

- Built using Dear ImGui for efficient rendering
- Modular design for easy extension
- Supports all RebelCAD constraint types
- Real-time constraint validation and updates
