#include "graphics/GraphicsSystem.h"
#include "core/Log.h"
#include "core/Error.h"
#include <iostream>

namespace rebel::graphics {

class GraphicsSystem::GraphicsSystemImpl {
public:
    GraphicsSystemImpl() {}

    void initialize(GraphicsAPI api, const WindowProperties& props) {
        REBEL_LOG_INFO("Initializing graphics system with " + getAPIName(api));
        REBEL_LOG_INFO("Window properties: " + props.title + ", " + 
                      std::to_string(props.width) + "x" + std::to_string(props.height) + 
                      (props.fullscreen ? ", fullscreen" : ", windowed") + 
                      (props.vsync ? ", vsync on" : ", vsync off"));

        // Placeholder for actual graphics initialization
        // In a real implementation, this would initialize GLFW, Vulkan/OpenGL, etc.
    }

    void shutdown() {
        REBEL_LOG_INFO("Shutting down graphics system");
        // Placeholder for actual graphics shutdown
    }

    void beginFrame() {
        // Placeholder for frame begin operations
    }

    void endFrame() {
        // Placeholder for frame end operations
    }

    void present() {
        // Placeholder for frame presentation
    }

private:
    std::string getAPIName(GraphicsAPI api) {
        switch (api) {
            case GraphicsAPI::OpenGL: return "OpenGL";
            case GraphicsAPI::Vulkan: return "Vulkan";
            case GraphicsAPI::DirectX11: return "DirectX 11";
            case GraphicsAPI::DirectX12: return "DirectX 12";
            default: return "Unknown API";
        }
    }
};

// Singleton instance
GraphicsSystem& GraphicsSystem::getInstance() {
    static GraphicsSystem instance;
    return instance;
}

GraphicsSystem::GraphicsSystem()
    : impl(std::make_unique<GraphicsSystemImpl>()),
      initialized(false),
      api(GraphicsAPI::Vulkan) {
}

GraphicsSystem::~GraphicsSystem() {
    if (initialized) {
        shutdown();
    }
}

void GraphicsSystem::initialize(GraphicsAPI api, const WindowProperties& props) {
    if (initialized) {
        REBEL_LOG_WARNING("Graphics system already initialized");
        return;
    }

    try {
        impl->initialize(api, props);
        this->api = api;
        initialized = true;
    }
    catch (const std::exception& e) {
        REBEL_LOG_CRITICAL("Failed to initialize graphics system: " + std::string(e.what()));
        throw rebel::core::Error(rebel::core::ErrorCode::GraphicsError, 
                               "Graphics system initialization failed: " + std::string(e.what()));
    }
}

void GraphicsSystem::shutdown() {
    if (!initialized) {
        return;
    }

    try {
        impl->shutdown();
        initialized = false;
    }
    catch (const std::exception& e) {
        REBEL_LOG_CRITICAL("Error during graphics system shutdown: " + std::string(e.what()));
        // We don't rethrow here to allow cleanup to continue
    }
}

void GraphicsSystem::beginFrame() {
    if (!initialized) {
        REBEL_THROW_ERROR(GraphicsError, "Graphics system not initialized");
    }
    impl->beginFrame();
}

void GraphicsSystem::endFrame() {
    if (!initialized) {
        REBEL_THROW_ERROR(GraphicsError, "Graphics system not initialized");
    }
    impl->endFrame();
}

void GraphicsSystem::present() {
    if (!initialized) {
        REBEL_THROW_ERROR(GraphicsError, "Graphics system not initialized");
    }
    impl->present();
}

} // namespace rebel::graphics
