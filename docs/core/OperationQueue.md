# Operation Queue System

The Operation Queue system provides a robust framework for managing and executing operations in RebelCAD. It extends the Command pattern with additional features like prioritization, progress tracking, cancellation support, and dependency management.

## Key Features

- Priority-based operation scheduling
- Asynchronous operation support
- Operation dependency management
- Progress tracking and status updates
- Cancellation support
- Multi-threaded execution

## Components

### Operation Class

The `Operation` class extends the base `Command` class with additional capabilities:

```cpp
class Operation : public Command {
public:
    // Priority levels for execution ordering
    enum class OperationPriority {
        Low,        // Background operations
        Normal,     // Standard user operations
        High,       // Time-sensitive operations
        Critical    // Must execute immediately
    };

    // Operation states
    enum class OperationState {
        Pending,    // Operation is queued
        Running,    // Operation is executing
        Completed,  // Operation completed successfully
        Failed,     // Operation failed to complete
        Cancelled   // Operation was cancelled
    };
};
```

### OperationQueue Class

The `OperationQueue` class manages the execution of operations:

```cpp
class OperationQueue {
public:
    explicit OperationQueue(size_t numWorkers = 2);
    
    bool enqueue(OperationPtr operation, 
                const std::vector<OperationPtr>& dependencies = {});
    bool cancel(OperationPtr operation);
    void cancelAll();
    bool waitForCompletion(uint32_t timeout_ms = 0);
};
```

## Usage Examples

### Basic Operation Execution

```cpp
// Create and configure the operation queue
auto queue = std::make_shared<OperationQueue>(2); // 2 worker threads

// Create and enqueue an operation
auto operation = std::make_shared<MyOperation>();
queue->enqueue(operation);

// Wait for completion
queue->waitForCompletion();
```

### Operation with Dependencies

```cpp
// Create operations
auto op1 = std::make_shared<MyOperation>();
auto op2 = std::make_shared<MyOperation>();
auto op3 = std::make_shared<MyOperation>();

// Set up dependencies (op3 depends on op2, which depends on op1)
queue->enqueue(op3, {op2});
queue->enqueue(op2, {op1});
queue->enqueue(op1);
```

### Progress Tracking

```cpp
auto operation = std::make_shared<MyOperation>();

// Set progress callback
operation->setProgressCallback([](float progress) {
    std::cout << "Progress: " << (progress * 100) << "%" << std::endl;
});

// Set completion callback
operation->setCompletionCallback([](bool success) {
    std::cout << "Operation " << (success ? "succeeded" : "failed") << std::endl;
});

queue->enqueue(operation);
```

### Cancellation

```cpp
auto operation = std::make_shared<MyCancellableOperation>();
queue->enqueue(operation);

// Later...
if (queue->cancel(operation)) {
    std::cout << "Operation cancelled successfully" << std::endl;
}
```

## Best Practices

1. **Operation Design**
   - Make operations self-contained and focused on a single task
   - Implement proper cleanup in case of cancellation
   - Use progress updates for long-running operations

2. **Resource Management**
   - Use smart pointers for operation management
   - Clean up resources in operation destructors
   - Handle operation failures gracefully

3. **Threading Considerations**
   - Make operations thread-safe
   - Avoid shared state between operations
   - Use appropriate synchronization mechanisms

4. **Error Handling**
   - Check operation return values
   - Handle exceptions within operations
   - Provide meaningful error messages

## Integration with Other Systems

The Operation Queue system integrates with several other RebelCAD systems:

- **Command System**: Operations extend the Command pattern
- **Event System**: Operations can emit events for status updates
- **Memory Management**: Uses smart pointers for safe memory handling
- **Resource Management**: Coordinates with the resource manager for asset handling

## Performance Considerations

- Use appropriate priority levels for operations
- Consider operation dependencies carefully to avoid bottlenecks
- Monitor operation execution times for optimization opportunities
- Use async operations for potentially long-running tasks

## Future Enhancements

Planned improvements to the Operation Queue system:

- Operation batching for improved performance
- Enhanced progress reporting
- Operation pause/resume support
- Operation scheduling (delayed execution)
- Operation result caching
