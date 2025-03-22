#pragma once

#include <memory>
#include <string>
#include "core/Log.h"
#include "core/Error.h"

// Forward declarations
namespace rebel {
namespace integration {
    class AssetManagementIntegration;
}
}

namespace rebel::core {

/**
 * @class Application
 * @brief Main application class for RebelCAD.
 * 
 * The Application class is the entry point for the RebelCAD application.
 * It manages the application lifecycle, including initialization, main loop,
 * and shutdown. It also provides access to various subsystems like graphics,
 * UI, and plugins.
 * 
 * This class follows the singleton pattern, ensuring only one instance exists.
 */
class Application {
public:
    /**
     * @brief Constructs a new Application instance.
     * @param appName The name of the application.
     */
    Application(const std::string& appName = "RebelCAD");
    
    /**
     * @brief Destroys the Application instance.
     * 
     * If the application is still running, it will be shut down.
     */
    ~Application();

    // Delete copy and move constructors/assignments
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    /**
     * @brief Initializes the application and all subsystems.
     * 
     * This method initializes the logging system, graphics system, UI system,
     * and plugin system. It must be called before run().
     * 
     * @throws Error If initialization fails.
     */
    void initialize();
    
    /**
     * @brief Runs the application main loop.
     * 
     * This method enters the main application loop, processing events,
     * updating the application state, and rendering the scene.
     * 
     * @throws Error If an error occurs during execution.
     */
    void run();
    
    /**
     * @brief Shuts down the application and all subsystems.
     * 
     * This method shuts down all subsystems and releases resources.
     * It is called automatically by the destructor if the application
     * is still running.
     * 
     * @throws Error If shutdown fails.
     */
    void shutdown();

    /**
     * @brief Checks if the application is running.
     * @return true if the application is running, false otherwise.
     */
    bool isRunning() const { return running; }
    
    /**
     * @brief Gets the application name.
     * @return The application name.
     */
    const std::string& getAppName() const { return appName; }

    /**
     * @brief Gets the singleton instance of the Application.
     * @return Reference to the Application instance.
     */
    static Application& getInstance();
    
    /**
     * @brief Gets the asset management integration.
     * @return Shared pointer to the asset management integration.
     */
    std::shared_ptr<integration::AssetManagementIntegration> getAssetManager() const;

private:
    /**
     * @brief Implementation class for the Application.
     * 
     * This class implements the platform-specific details of the application.
     */
    class ApplicationImpl;
    std::unique_ptr<ApplicationImpl> impl;

    std::string appName;  ///< The application name
    bool running;         ///< Flag indicating if the application is running

    /**
     * @brief Initializes the logging subsystem.
     */
    void initializeLogging();
    
    /**
     * @brief Initializes the graphics subsystem.
     */
    void initializeGraphics();
    
    /**
     * @brief Initializes the UI subsystem.
     */
    void initializeUI();
    
    /**
     * @brief Initializes the plugin subsystem.
     */
    void initializePlugins();

    /**
     * @brief Processes input events.
     * 
     * This method is called once per frame to process input events
     * from the window system.
     */
    void processEvents();
    
    /**
     * @brief Updates the application state.
     * 
     * This method is called once per frame to update the application
     * state, including physics, animations, and other time-dependent
     * operations.
     */
    void update();
    
    /**
     * @brief Renders the scene.
     * 
     * This method is called once per frame to render the scene.
     */
    void render();
};

} // namespace rebel::core
