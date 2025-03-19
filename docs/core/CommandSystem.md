# Command System Documentation

## Overview

The Command System implements the Command pattern to provide undo/redo functionality throughout RebelCAD. This system allows all operations to be recorded, undone, and redone, providing a robust history management system for the application.

## Core Components

### Command Interface

The `Command` class serves as the base interface for all commands in RebelCAD:

```cpp
class Command {
    virtual bool execute() = 0;
    virtual bool undo() = 0;
    virtual bool redo() = 0;
    virtual std::string getDescription() const = 0;
    virtual bool canMergeWith(const Command* other) const;
    virtual bool mergeWith(const Command* other);
};
```

### UndoRedoManager

The `UndoRedoManager` class manages the command history and handles execution, undo, and redo operations:

```cpp
class UndoRedoManager {
    bool executeCommand(CommandPtr command);
    bool undo();
    bool redo();
    void beginCommandGroup(const std::string& description);
    void endCommandGroup();
    void setCommandMerging(bool enable);
};
```

## Features

### Command Execution
- Commands represent atomic operations that can be executed, undone, and redone
- Each command maintains its own state for undo/redo operations
- Commands provide descriptions for UI feedback

### Command Groups
- Multiple commands can be grouped into a single undoable operation
- Groups are treated as atomic units in the undo/redo stack
- Useful for complex operations that involve multiple steps

### Command Merging
- Continuous operations (like dragging) can be merged into a single command
- Reduces memory usage and provides a cleaner undo history
- Can be enabled/disabled as needed

### Event Notifications
- State changes are broadcast through the EventSystem
- UI components can subscribe to updates
- Provides real-time feedback about undo/redo availability

## Usage Examples

### Creating a Simple Command

```cpp
class MoveEntityCommand : public Command {
public:
    MoveEntityCommand(Entity* entity, const Vector3& newPosition)
        : entity_(entity)
        , oldPosition_(entity->getPosition())
        , newPosition_(newPosition) {}

    bool execute() override {
        entity_->setPosition(newPosition_);
        return true;
    }

    bool undo() override {
        entity_->setPosition(oldPosition_);
        return true;
    }

    bool redo() override {
        return execute();
    }

    std::string getDescription() const override {
        return "Move entity";
    }

private:
    Entity* entity_;
    Vector3 oldPosition_;
    Vector3 newPosition_;
};
```

### Using Command Groups

```cpp
// Start a group of related operations
undoRedoManager.beginCommandGroup("Create and position entity");

// Execute multiple commands as part of the group
undoRedoManager.executeCommand(std::make_shared<CreateEntityCommand>());
undoRedoManager.executeCommand(std::make_shared<MoveEntityCommand>());
undoRedoManager.executeCommand(std::make_shared<RotateEntityCommand>());

// End the group - all commands will be undone/redone together
undoRedoManager.endCommandGroup();
```

### Implementing Command Merging

```cpp
class DragEntityCommand : public Command {
public:
    bool canMergeWith(const Command* other) const override {
        auto dragCmd = dynamic_cast<const DragEntityCommand*>(other);
        return dragCmd && dragCmd->entity_ == this->entity_;
    }

    bool mergeWith(const Command* other) override {
        auto dragCmd = dynamic_cast<const DragEntityCommand*>(other);
        newPosition_ = dragCmd->newPosition_;
        return true;
    }
};
```

## Best Practices

1. **Command Design**
   - Keep commands focused on single operations
   - Store only necessary state for undo/redo
   - Provide clear, descriptive command names

2. **Error Handling**
   - Commands should validate operations before execution
   - Return false if execution/undo/redo fails
   - Maintain consistent state on failure

3. **Performance**
   - Use command merging for continuous operations
   - Clean up resources when commands are destroyed
   - Consider memory usage in undo/redo stacks

4. **Integration**
   - Subscribe to UndoRedoStateChanged events for UI updates
   - Use command groups for complex operations
   - Consider thread safety when executing commands

## Event System Integration

The Command System integrates with RebelCAD's event system through the `UndoRedoStateChangedEvent`:

```cpp
// Subscribe to undo/redo state changes
EventBus::getInstance().subscribe<UndoRedoStateChangedEvent>(
    [](Event& e) {
        auto& event = static_cast<UndoRedoStateChangedEvent&>(e);
        // Update UI based on new state
        updateUndoRedoButtons(event.canUndo(), event.canRedo());
        updateStatusBar(event.getUndoDescription(), event.getRedoDescription());
    }
);
```

## Future Enhancements

- Transaction system for more complex operations
- Command serialization for save/load functionality
- Enhanced command merging strategies
- Command preview functionality
- Command search and filtering
