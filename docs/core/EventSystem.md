# Event System Documentation

The RebelCAD Event System provides a robust, type-safe, and thread-safe mechanism for inter-module communication. It implements a publish-subscribe pattern with support for event priorities, asynchronous event handling, and category-based filtering.

## Features

- Type-safe event handling using templates
- Priority-based event processing with multiple priority levels
- Thread-safe event publishing and subscription
- Asynchronous event processing with futures
- Category-based event filtering
- Support for critical events that must be handled by all subscribers
- Event timestamps and naming for debugging
- Exception handling and structured logging
- Memory-efficient event handling

## Priority Levels

The system defines several priority levels through the `EventPriority` enum class:

```cpp
enum class EventPriority {
    Lowest = 0,   // Background tasks, non-critical updates
    Low = 1,      // Low priority tasks
    Normal = 2,   // Default priority for most events
    High = 3,     // Important events that should be processed quickly
    Critical = 4, // Must-handle events that are processed by all subscribers
    Immediate = 5 // Bypass queue and process immediately
};
```

## Usage Examples

### 1. Creating a Custom Event

```cpp
class ModelChangedEvent : public Event {
public:
    ModelChangedEvent(const std::string& modelId) 
        : Event("ModelChanged"), modelId(modelId) {
        // Add categories for filtering
        addCategory("model");
        addCategory("change");
    }
    
    std::string modelId;
};
```

### 2. Subscribing to Events

```cpp
auto& eventBus = EventBus::getInstance();

// Normal priority subscription
auto subscription = eventBus.subscribe<ModelChangedEvent>(
    [](Event& e) {
        auto& modelEvent = static_cast<ModelChangedEvent&>(e);
        // Handle the event
        std::cout << "Model " << modelEvent.modelId << " changed\n";
    }
);

// High priority subscription with category filter
auto filter = eventBus.createCategoryFilter({"model"});
auto highPrioritySubscription = eventBus.subscribe<ModelChangedEvent>(
    [](Event& e) {
        auto& modelEvent = static_cast<ModelChangedEvent&>(e);
        // Handle the event with high priority
    },
    EventPriority::High,
    filter
);
```

### 3. Publishing Events

```cpp
// Create and publish an event
auto event = std::make_shared<ModelChangedEvent>("model123");
auto future = eventBus.publish(event);

// Wait for event processing to complete
future.wait();

// Or handle completion asynchronously
future.then([](auto) {
    // Event has been processed
});
```

### 4. Category Filtering

```cpp
// Create a filter for specific categories
auto filter = eventBus.createCategoryFilter({"model", "change"});

// Subscribe with the filter
auto subscription = eventBus.subscribe<ModelChangedEvent>(
    [](Event& e) {
        // Only called for events with matching categories
    },
    EventPriority::Normal,
    filter
);
```

### 5. Immediate Priority Events

```cpp
auto event = std::make_shared<ModelChangedEvent>("model123");
event->setPriority(EventPriority::Immediate);
eventBus.publish(event).wait(); // Processed immediately, bypassing queue
```

## Thread Safety

The event system is thread-safe and handles events asynchronously:

- Event publishing is non-blocking by default
- Multiple threads can publish events simultaneously
- Event handlers are executed in a dedicated thread
- Subscription management is thread-safe
- Priority queue ensures ordered processing
- Mutex protection for all shared resources

## Best Practices

1. **Event Design**
   - Keep events focused and specific
   - Use categories for logical grouping
   - Include all necessary data in the event object
   - Use meaningful event names for debugging

2. **Event Handling**
   - Keep handlers lightweight
   - Use appropriate priority levels
   - Handle exceptions appropriately
   - Consider using category filters for efficient event routing

3. **Memory Management**
   - Use shared_ptr for event instances
   - Store subscription IDs if you need to unsubscribe later
   - Clear subscriptions when they're no longer needed
   - Be careful with lambda captures to avoid dangling references

4. **Performance Considerations**
   - Use category filters to reduce unnecessary event processing
   - Choose appropriate priority levels
   - Consider batching related events
   - Use Immediate priority sparingly

## Logging Integration

The event system integrates with the RebelCAD logging system to provide detailed insights:

- Event publications with priority and timestamp
- Handler execution and timing
- Exception handling and error reporting
- System state changes
- Category filtering results

## Error Handling

The system includes comprehensive error handling:

- Exception catching and logging in event handlers
- Type safety checks at compile-time
- Thread safety protection via mutex locks
- Invalid event handling prevention
- Proper cleanup on system shutdown

## Future Considerations

The event system is designed to be extensible for future features such as:
- Event persistence and replay
- Network distribution of events
- Performance metrics and monitoring
- Advanced filtering mechanisms
- Event batching and coalescing

## Contributing

When extending the event system:
1. Maintain thread safety
2. Add comprehensive unit tests
3. Update this documentation
4. Consider backward compatibility
5. Follow memory management best practices
