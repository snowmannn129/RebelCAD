# Assembly Tree

The Assembly Tree manages the hierarchical structure of components in RebelCAD assemblies. It provides functionality for organizing components in a parent-child relationship, enabling complex assembly structures while maintaining proper relationships between components.

## Features

- Hierarchical component organization
- Parent-child relationship management
- Component visibility control
- Circular reference prevention
- Automatic restructuring on component removal

## Usage

### Creating Components

```cpp
// Create root level component
auto engine = tree.addComponent("engine", "Engine Assembly");

// Create child components
auto cylinder = tree.addComponent("cylinder", "Cylinder", "engine");
auto piston = tree.addComponent("piston", "Piston", "engine");
```

### Managing Components

```cpp
// Move component to new parent
tree.moveComponent("piston", "cylinder");

// Remove component (children moved to root)
tree.removeComponent("engine");

// Find component by ID
auto component = tree.findComponent("piston");

// Control visibility
component->setVisible(false);
```

### Traversing the Tree

```cpp
// Get root components
const auto& roots = tree.getRootComponents();

// Access children
for (const auto& child : component->getChildren()) {
    // Process child component
}

// Access parent
if (auto parent = component->getParent().lock()) {
    // Process parent component
}
```

## Best Practices

1. **Unique IDs**: Always ensure component IDs are unique within the assembly.
2. **Parent-Child Relationships**: Consider the logical hierarchy when structuring assemblies.
3. **Memory Management**: Use the provided smart pointers (Ptr and WeakPtr) to handle component references.
4. **Visibility Control**: Use visibility flags to control component display without removing them.

## Error Handling

The AssemblyTree provides several safety features:

- Returns nullptr when adding duplicate components
- Prevents circular references in the hierarchy
- Automatically restructures when removing components
- Uses weak pointers to prevent memory leaks

## Performance Considerations

- Component lookup is O(1) using hash map
- Tree operations (add, remove, move) are O(1) for direct operations
- Tree traversal is O(n) where n is number of affected nodes

## Integration with Other Systems

The AssemblyTree is designed to work seamlessly with:

- Component Positioning System
- Joint Management
- Assembly Constraints
- Visualization System

## Example: Complex Assembly

```cpp
// Create main assembly
auto assembly = tree.addComponent("main", "Main Assembly");

// Add subassemblies
auto subAssembly1 = tree.addComponent("sub1", "Sub Assembly 1", "main");
auto subAssembly2 = tree.addComponent("sub2", "Sub Assembly 2", "main");

// Add components to subassemblies
auto component1 = tree.addComponent("comp1", "Component 1", "sub1");
auto component2 = tree.addComponent("comp2", "Component 2", "sub1");
auto component3 = tree.addComponent("comp3", "Component 3", "sub2");

// Reorganize structure
tree.moveComponent("comp2", "sub2");  // Move to different subassembly

// Hide subassembly
subAssembly1->setVisible(false);  // Affects visualization only
```

## Technical Details

### Memory Management

The AssemblyTree uses:
- std::shared_ptr for component ownership
- std::weak_ptr for parent references (prevents cycles)
- std::unordered_map for O(1) component lookup
- std::vector for child collections

### Thread Safety

The current implementation is not thread-safe. When using in a multi-threaded environment:
- Protect tree modifications with appropriate synchronization
- Consider using a read-write lock for concurrent read access
- Avoid modifying the tree while iterating

### Future Enhancements

Planned features include:
- Serialization support
- Undo/redo operations
- Multi-threaded access
- Change notification system
- Component filtering and search
