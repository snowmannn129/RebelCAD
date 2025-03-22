/**
 * @file AssetBrowserPanel.h
 * @brief Asset browser panel for RebelCAD.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>

// Forward declarations
namespace rebel {
namespace integration {
    class AssetManagementIntegration;
}
}

namespace RebelSUITE {
namespace AssetManagement {
    struct AssetMetadata;
}
}

namespace rebel {
namespace ui {

/**
 * @class AssetBrowserPanel
 * @brief Asset browser panel for RebelCAD.
 * 
 * This class provides a UI panel for browsing and managing assets in RebelCAD.
 */
class AssetBrowserPanel {
public:
    /**
     * @brief Callback type for asset selection.
     */
    using AssetSelectionCallback = std::function<void(const std::string&)>;

    /**
     * @brief Constructs a new AssetBrowserPanel instance.
     * @param assetManager The asset management integration.
     */
    explicit AssetBrowserPanel(std::shared_ptr<integration::AssetManagementIntegration> assetManager);

    /**
     * @brief Destroys the AssetBrowserPanel instance.
     */
    ~AssetBrowserPanel();

    // Delete copy and move constructors/assignments
    AssetBrowserPanel(const AssetBrowserPanel&) = delete;
    AssetBrowserPanel& operator=(const AssetBrowserPanel&) = delete;
    AssetBrowserPanel(AssetBrowserPanel&&) = delete;
    AssetBrowserPanel& operator=(AssetBrowserPanel&&) = delete;

    /**
     * @brief Initializes the asset browser panel.
     * @return true if successful, false otherwise.
     */
    bool initialize();

    /**
     * @brief Shuts down the asset browser panel.
     * @return true if successful, false otherwise.
     */
    bool shutdown();

    /**
     * @brief Renders the asset browser panel.
     */
    void render();

    /**
     * @brief Sets the current directory.
     * @param directory The directory path.
     */
    void setCurrentDirectory(const std::string& directory);

    /**
     * @brief Gets the current directory.
     * @return The current directory.
     */
    const std::string& getCurrentDirectory() const;

    /**
     * @brief Refreshes the asset browser.
     */
    void refresh();

    /**
     * @brief Adds an asset selection callback.
     * @param callback The callback function.
     * @return A unique identifier for the callback.
     */
    size_t addSelectionCallback(AssetSelectionCallback callback);

    /**
     * @brief Removes an asset selection callback.
     * @param callbackId The callback identifier.
     */
    void removeSelectionCallback(size_t callbackId);

private:
    /**
     * @brief Implementation class for the AssetBrowserPanel.
     */
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace ui
} // namespace rebel
