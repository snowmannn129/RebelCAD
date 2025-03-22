#pragma once

#include <memory>
#include <string>

namespace rebel::graphics {

/**
 * @enum GraphicsAPI
 * @brief Supported graphics APIs.
 * 
 * This enum defines the graphics APIs that can be used by the GraphicsSystem.
 */
enum class GraphicsAPI {
    OpenGL,     ///< OpenGL API
    Vulkan,     ///< Vulkan API
    DirectX11,  ///< DirectX 11 API
    DirectX12   ///< DirectX 12 API
};

/**
 * @struct WindowProperties
 * @brief Properties for the application window.
 * 
 * This struct defines the properties of the application window,
 * including title, dimensions, and display settings.
 */
struct WindowProperties {
    std::string title;  ///< Window title
    int width;          ///< Window width in pixels
    int height;         ///< Window height in pixels
    bool fullscreen;    ///< Fullscreen mode flag
    bool vsync;         ///< Vertical sync flag

    /**
     * @brief Constructs a new WindowProperties object.
     * 
     * @param title Window title
     * @param width Window width in pixels
     * @param height Window height in pixels
     * @param fullscreen Fullscreen mode flag
     * @param vsync Vertical sync flag
     */
    WindowProperties(
        const std::string& title = "RebelCAD",
        int width = 1280,
        int height = 720,
        bool fullscreen = false,
        bool vsync = true
    ) : title(title), width(width), height(height), fullscreen(fullscreen), vsync(vsync) {}
};

/**
 * @class GraphicsSystem
 * @brief Graphics system for RebelCAD.
 * 
 * The GraphicsSystem class manages the graphics subsystem of RebelCAD,
 * including window creation, rendering context setup, and frame rendering.
 * It supports multiple graphics APIs and follows the singleton pattern.
 */
class GraphicsSystem {
public:
    /**
     * @brief Gets the singleton instance of the GraphicsSystem.
     * @return Reference to the GraphicsSystem instance.
     */
    static GraphicsSystem& getInstance();

    /**
     * @brief Initializes the graphics system.
     * 
     * This method initializes the graphics system with the specified API
     * and window properties. It creates the application window and sets up
     * the rendering context.
     * 
     * @param api The graphics API to use.
     * @param props The window properties.
     * @throws Error If initialization fails.
     */
    void initialize(GraphicsAPI api, const WindowProperties& props);
    
    /**
     * @brief Shuts down the graphics system.
     * 
     * This method shuts down the graphics system, destroying the window
     * and rendering context, and releasing all graphics resources.
     */
    void shutdown();

    /**
     * @brief Begins a new frame.
     * 
     * This method begins a new frame, clearing the back buffer and
     * preparing for rendering.
     */
    void beginFrame();
    
    /**
     * @brief Ends the current frame.
     * 
     * This method ends the current frame, finalizing rendering operations.
     */
    void endFrame();
    
    /**
     * @brief Presents the rendered frame.
     * 
     * This method presents the rendered frame to the screen by swapping
     * the back and front buffers.
     */
    void present();

    /**
     * @brief Checks if the graphics system is initialized.
     * @return true if the graphics system is initialized, false otherwise.
     */
    bool isInitialized() const { return initialized; }
    
    /**
     * @brief Gets the current graphics API.
     * @return The current graphics API.
     */
    GraphicsAPI getAPI() const { return api; }

private:
    /**
     * @brief Constructs a new GraphicsSystem instance.
     * 
     * Private constructor to enforce the singleton pattern.
     */
    GraphicsSystem();
    
    /**
     * @brief Destroys the GraphicsSystem instance.
     */
    ~GraphicsSystem();

    // Delete copy and move constructors/assignments
    GraphicsSystem(const GraphicsSystem&) = delete;
    GraphicsSystem& operator=(const GraphicsSystem&) = delete;
    GraphicsSystem(GraphicsSystem&&) = delete;
    GraphicsSystem& operator=(GraphicsSystem&&) = delete;

    /**
     * @brief Implementation class for the GraphicsSystem.
     * 
     * This class implements the platform-specific and API-specific
     * details of the graphics system.
     */
    class GraphicsSystemImpl;
    std::unique_ptr<GraphicsSystemImpl> impl;

    bool initialized;  ///< Flag indicating if the graphics system is initialized
    GraphicsAPI api;   ///< The current graphics API
};

} // namespace rebel::graphics
