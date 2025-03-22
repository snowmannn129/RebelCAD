#include "core/Application.h"
#include "graphics/GraphicsSystem.h"
#include "integration/AssetManagementIntegration.h"
#include <chrono>
#include <thread>
#include <cmath>
#include <memory>

#define _USE_MATH_DEFINES

using namespace rebel::graphics;

namespace rebel::core {

class Application::ApplicationImpl {
public:
    ApplicationImpl() : assetManager(std::make_shared<integration::AssetManagementIntegration>()) {}

    void initialize() {
        REBEL_LOG_INFO("Initializing graphics subsystem...");
        GraphicsSystem::getInstance().initialize(
            GraphicsAPI::Vulkan,
            WindowProperties("RebelCAD", 1280, 720)
        );

        REBEL_LOG_INFO("Initializing UI subsystem...");
        // UI initialization will be implemented here

        REBEL_LOG_INFO("Initializing asset management system...");
        if (!assetManager->initialize()) {
            REBEL_LOG_ERROR("Failed to initialize asset management system");
        } else {
            REBEL_LOG_INFO("Asset management system initialized successfully");
            
            // Create asset browser panel
            assetBrowser = std::make_unique<ui::AssetBrowserPanel>(assetManager);
            if (!assetBrowser->initialize()) {
                REBEL_LOG_ERROR("Failed to initialize asset browser panel");
            } else {
                REBEL_LOG_INFO("Asset browser panel initialized successfully");
            }
        }

        REBEL_LOG_INFO("Loading plugins...");
        // Plugin loading will be implemented here
    }

    void shutdown() {
        REBEL_LOG_INFO("Shutting down application...");
        
        // Shutdown asset browser panel
        if (assetBrowser) {
            REBEL_LOG_INFO("Shutting down asset browser panel...");
            assetBrowser->shutdown();
            assetBrowser.reset();
        }
        
        // Shutdown asset management system
        if (assetManager) {
            REBEL_LOG_INFO("Shutting down asset management system...");
            assetManager->shutdown();
        }
        
        GraphicsSystem::getInstance().shutdown();
    }
    
    void renderUI() {
        // Render asset browser panel
        if (assetBrowser) {
            assetBrowser->render();
        }
    }
    
    std::shared_ptr<integration::AssetManagementIntegration> getAssetManager() const {
        return assetManager;
    }

private:
    std::shared_ptr<integration::AssetManagementIntegration> assetManager;
    std::unique_ptr<ui::AssetBrowserPanel> assetBrowser;
};

// Singleton instance
Application& Application::getInstance() {
    static Application instance;
    return instance;
}

Application::Application(const std::string& name)
    : impl(std::make_unique<ApplicationImpl>()),
      appName(name),
      running(false) {
    REBEL_LOG_INFO("Creating RebelCAD application instance");
}

Application::~Application() {
    if (running) {
        shutdown();
    }
}

void Application::initialize() {
    try {
        REBEL_LOG_INFO("Initializing " + appName);

        initializeLogging();
        impl->initialize();

        running = true;
        REBEL_LOG_INFO("Initialization complete");
    }
    catch (const Error& e) {
        REBEL_LOG_CRITICAL("Failed to initialize application: " + std::string(e.what()));
        throw;
    }
}

void Application::run() {
    if (!running) {
        REBEL_THROW_ERROR(SystemError, "Application not initialized");
    }

    REBEL_LOG_INFO("Starting main loop");

    try {
        while (running) {
            processEvents();
            update();
            render();

            // Temporary: Add a small sleep to prevent CPU overuse
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
        }
    }
    catch (const Error& e) {
        REBEL_LOG_CRITICAL("Fatal error in main loop: " + std::string(e.what()));
        throw;
    }
}

void Application::shutdown() {
    if (!running) {
        return;
    }

    try {
        impl->shutdown();
        running = false;
        REBEL_LOG_INFO("Application shutdown complete");
    }
    catch (const Error& e) {
        REBEL_LOG_CRITICAL("Error during shutdown: " + std::string(e.what()));
        throw;
    }
}

void Application::initializeLogging() {
    // Logging system is self-initializing via singleton
    REBEL_LOG_INFO("Logging system initialized");
}

void Application::initializeGraphics() {
    REBEL_LOG_INFO("Graphics system initialization placeholder");
}

void Application::initializeUI() {
    REBEL_LOG_INFO("UI system initialization placeholder");
}

void Application::initializePlugins() {
    REBEL_LOG_INFO("Plugin system initialization placeholder");
}

void Application::processEvents() {
    // Event processing handled by graphics system in beginFrame
}

void Application::update() {
    // Update logic will be implemented here
}

void Application::render() {
    auto& graphics = GraphicsSystem::getInstance();

    graphics.beginFrame();

    // Render scene
    // ...

    // Render UI
    impl->renderUI();

    graphics.endFrame();
    graphics.present();
}

std::shared_ptr<integration::AssetManagementIntegration> Application::getAssetManager() const {
    return impl->getAssetManager();
}

} // namespace rebel::core
