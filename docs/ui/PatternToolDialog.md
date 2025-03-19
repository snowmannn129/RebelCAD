# Pattern Tool Dialog

## Overview
The Pattern Tool Dialog provides a user interface for creating and manipulating patterns in RebelCAD. It supports three pattern types:
- Linear patterns (translation along a direction)
- Circular patterns (rotation around an axis)
- Mirror patterns (reflection across a plane)

## Features
- Real-time preview of pattern operations
- Parameter validation and error feedback
- Event-driven updates
- Memory-efficient parameter handling

## Technical Implementation

### Class Structure
```cpp
class PatternToolDialog {
    // Public interface for initialization and rendering
    // Protected methods for testing
    // Private implementation details
}
```

### Key Components

#### Pattern Type Selection
- Combo box for selecting pattern type
- Automatic parameter validation on type change
- Preview updates on selection change

#### Parameter Input
1. Linear Pattern
   - Instance count (min: 2)
   - Spacing distance
   - Direction vector (automatically normalized)

2. Circular Pattern
   - Instance count (min: 2)
   - Total angle (0-360 degrees)
   - Center point
   - Rotation axis (automatically normalized)

3. Mirror Pattern
   - Normal vector (automatically normalized)
   - Point on plane
   - Include original option

#### Preview System
- Real-time updates during parameter changes
- Event-based preview generation
- Automatic cleanup on dialog close

#### Error Handling
- Parameter validation
- Visual error feedback
- Clear error messages
- Event-based error reporting

### Memory Management
- Smart pointers for resource management
- Efficient parameter storage using union
- Automatic cleanup in destructor
- Event handle management

### Event System Integration
- PreviewUpdate event handling
- Error event handling
- Event unregistration in destructor

## Usage Example
```cpp
// Create and initialize dialog
auto patternTool = std::make_shared<PatternTool>();
auto dialog = std::make_unique<PatternToolDialog>();
dialog->initialize(patternTool);

// Show dialog
dialog->show();

// Handle updates through event system
// Dialog will automatically update preview and handle errors
```

## Testing
- Unit tests cover core functionality
- Event handling tests
- Parameter validation tests
- UI state management tests

## Performance Considerations
- Efficient parameter updates
- Preview caching
- Optimized vector normalization
- Memory-efficient design

## Future Improvements
1. Advanced Pattern Features
   - Variable spacing
   - Custom distributions
   - Pattern along curve
   - Pattern on surface

2. UI Enhancements
   - Direct manipulation in viewport
   - Advanced preview options
   - Custom pattern presets
   - Undo/redo support

3. Performance Optimizations
   - GPU-accelerated preview
   - Multi-threaded pattern generation
   - Advanced caching strategies
