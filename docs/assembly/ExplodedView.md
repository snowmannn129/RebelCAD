# Exploded View

The exploded view feature in RebelCAD allows users to visualize assembly components in an expanded state, making it easier to understand the assembly structure and create technical documentation.

## Overview

The exploded view system provides:
- Global enable/disable control
- Adjustable explosion factor (0.0 - 1.0)
- Per-component offset vectors
- Hierarchical offset accumulation

## Usage

### Enable/Disable Exploded View

```cpp
AssemblyTree tree;
// Enable exploded view
tree.setExplodedViewEnabled(true);
// Disable exploded view
tree.setExplodedViewEnabled(false);
```

### Set Explosion Factor

The explosion factor controls the overall magnitude of the explosion effect:

```cpp
// Set explosion to 50%
tree.setExplosionFactor(0.5f);
```

### Component Offsets

Each component can have its own explosion offset vector:

```cpp
// Move component up by 10 units
tree.setComponentExplodeOffset("component1", glm::vec3(0.0f, 10.0f, 0.0f));

// Get current offset
auto offset = tree.getComponentExplodeOffset("component1");
if (offset) {
    // Use offset...
}
```

### Calculate Exploded Positions

Get the final world position for a component in exploded view:

```cpp
auto position = tree.getExplodedPosition("component1");
if (position) {
    // Use position...
}
```

### Reset Exploded View

Reset all explosion settings to default:

```cpp
tree.resetExplodedView();
```

## Technical Details

### Hierarchical Offset Calculation

The final exploded position of a component is calculated by:
1. Starting from the component
2. Accumulating explosion offsets up the parent chain
3. Applying the global explosion factor
4. Applying per-component explosion factors

For example:
```
Final Position = Σ(component_offset * explosion_factor * component_factor)
```

### Performance Considerations

- Explosion offsets are cached per component
- Position calculations are performed on-demand
- Memory overhead is minimal (one vec3 per component)

## Best Practices

1. **Offset Directions**
   - Use logical explosion directions based on assembly structure
   - Consider component relationships when setting offsets
   - Maintain consistent explosion patterns

2. **Explosion Factor**
   - Use smaller factors for dense assemblies
   - Adjust based on model scale
   - Consider viewport size

3. **Component Organization**
   - Group related components
   - Use consistent offset patterns for similar sub-assemblies
   - Consider assembly/disassembly sequence

## Example Workflow

1. Create assembly structure
2. Enable exploded view
3. Set component offsets
4. Adjust explosion factor
5. Get exploded positions for rendering
6. Disable exploded view when done

## API Reference

### AssemblyTree Methods

| Method | Description |
|--------|-------------|
| `setExplodedViewEnabled(bool)` | Enable/disable exploded view |
| `isExplodedViewEnabled()` | Check if exploded view is active |
| `setExplosionFactor(float)` | Set global explosion factor (0.0-1.0) |
| `getExplosionFactor()` | Get current explosion factor |
| `setComponentExplodeOffset(id, vec3)` | Set component's explosion offset |
| `getComponentExplodeOffset(id)` | Get component's explosion offset |
| `getExplodedPosition(id)` | Get final world position in exploded view |
| `resetExplodedView()` | Reset all explosion settings |

## See Also

- [Assembly Tree](AssemblyTree.md)
- [Component Positioning](ComponentPositioner.md)
- [Assembly Visualization](AssemblyVisualization.md)
