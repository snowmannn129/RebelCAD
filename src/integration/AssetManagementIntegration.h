/**
 * @file AssetManagementIntegration.h
 * @brief Integration with the RebelSUITE Unified Asset Management System for RebelCAD.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

// Forward declarations for RebelSUITE Asset Management
namespace RebelSUITE {
namespace AssetManagement {
    class AssetManager;
    struct AssetMetadata;
    struct AssetFilter;
    struct FileSystemChange;
    enum class AssetType;
}
}

namespace rebel {
namespace integration {

/**
 * @class AssetManagementIntegration
 * @brief Integration with the RebelSUITE Unified Asset Management System.
 * 
 * This class provides integration with the RebelSUITE Unified Asset Management System,
 * enabling asset browsing, searching, and management across all RebelSUITE components.
 */
class AssetManagementIntegration {
public:
    /**
     * @brief Callback type for asset change notifications.
     */
    using AssetChangeCallback = std::function<void(const std::vector<RebelSUITE::AssetManagement::FileSystemChange>&)>;

    /**
     * @brief Callback type for asset selection notifications.
     */
    using AssetSelectionCallback = std::function<void(const std::string&)>;

    /**
     * @brief Constructs a new AssetManagementIntegration instance.
     * @param assetRoot The root directory for assets.
     */
    explicit AssetManagementIntegration(const std::string& assetRoot = "");

    /**
     * @brief Destroys the AssetManagementIntegration instance.
     */
    ~AssetManagementIntegration();

    // Delete copy and move constructors/assignments
    AssetManagementIntegration(const AssetManagementIntegration&) = delete;
    AssetManagementIntegration& operator=(const AssetManagementIntegration&) = delete;
    AssetManagementIntegration(AssetManagementIntegration&&) = delete;
    AssetManagementIntegration& operator=(AssetManagementIntegration&&) = delete;

    /**
     * @brief Initializes the asset management integration.
     * @return true if successful, false otherwise.
     */
    bool initialize();

    /**
     * @brief Shuts down the asset management integration.
     * @return true if successful, false otherwise.
     */
    bool shutdown();

    /**
     * @brief Checks if the asset management system is available.
     * @return true if available, false otherwise.
     */
    bool isAvailable() const;

    /**
     * @brief Adds a change listener for asset changes.
     * @param callback The callback function to add.
     * @return A unique identifier for the listener.
     */
    size_t addChangeListener(AssetChangeCallback callback);

    /**
     * @brief Removes a change listener.
     * @param listenerId The identifier of the listener to remove.
     */
    void removeChangeListener(size_t listenerId);

    /**
     * @brief Adds a selection listener for asset selection.
     * @param callback The callback function to add.
     * @return A unique identifier for the listener.
     */
    size_t addSelectionListener(AssetSelectionCallback callback);

    /**
     * @brief Removes a selection listener.
     * @param listenerId The identifier of the listener to remove.
     */
    void removeSelectionListener(size_t listenerId);

    /**
     * @brief Browses a directory in the asset management system.
     * @param directory The directory path.
     * @return The assets in the directory, or empty vector if browsing failed.
     */
    std::vector<RebelSUITE::AssetManagement::AssetMetadata> browseDirectory(const std::string& directory);

    /**
     * @brief Searches for assets in the asset management system.
     * @param filter The filter criteria.
     * @param page The page number (1-based).
     * @param pageSize The page size.
     * @return The search results.
     */
    std::vector<RebelSUITE::AssetManagement::AssetMetadata> searchAssets(
        const RebelSUITE::AssetManagement::AssetFilter& filter,
        size_t page = 1,
        size_t pageSize = 50
    );

    /**
     * @brief Imports an asset into the asset management system.
     * @param filePath The file path.
     * @param assetType The asset type (optional).
     * @return The imported asset metadata, or nullptr if import failed.
     */
    std::shared_ptr<RebelSUITE::AssetManagement::AssetMetadata> importAsset(
        const std::string& filePath,
        RebelSUITE::AssetManagement::AssetType assetType = static_cast<RebelSUITE::AssetManagement::AssetType>(0)
    );

    /**
     * @brief Exports an asset from the asset management system.
     * @param assetPath The asset path.
     * @param exportPath The export path.
     * @return true if successful, false otherwise.
     */
    bool exportAsset(const std::string& assetPath, const std::string& exportPath);

    /**
     * @brief Gets metadata for an asset in the asset management system.
     * @param assetPath The asset path.
     * @return The asset metadata, or nullptr if not found.
     */
    std::shared_ptr<RebelSUITE::AssetManagement::AssetMetadata> getAssetMetadata(const std::string& assetPath);

    /**
     * @brief Gets a thumbnail for an asset in the asset management system.
     * @param assetPath The asset path.
     * @return The thumbnail data URL, or empty string if not available.
     */
    std::string getAssetThumbnail(const std::string& assetPath);

    /**
     * @brief Creates a new asset in the asset management system.
     * @param assetPath The asset path.
     * @param data The asset data.
     * @param metadata Additional metadata (optional).
     * @return The created asset metadata, or nullptr if creation failed.
     */
    std::shared_ptr<RebelSUITE::AssetManagement::AssetMetadata> createAsset(
        const std::string& assetPath,
        const std::vector<uint8_t>& data,
        const std::unordered_map<std::string, std::string>& metadata = {}
    );

    /**
     * @brief Updates an existing asset in the asset management system.
     * @param assetPath The asset path.
     * @param data The new asset data (optional).
     * @param metadata The new metadata (optional).
     * @return The updated asset metadata, or nullptr if update failed.
     */
    std::shared_ptr<RebelSUITE::AssetManagement::AssetMetadata> updateAsset(
        const std::string& assetPath,
        const std::vector<uint8_t>& data = {},
        const std::unordered_map<std::string, std::string>& metadata = {}
    );

    /**
     * @brief Deletes an asset from the asset management system.
     * @param assetPath The asset path.
     * @return true if successful, false otherwise.
     */
    bool deleteAsset(const std::string& assetPath);

    /**
     * @brief Gets the current directory.
     * @return The current directory.
     */
    const std::string& getCurrentDirectory() const;

    /**
     * @brief Sets the current directory.
     * @param directory The new current directory.
     */
    void setCurrentDirectory(const std::string& directory);

    /**
     * @brief Gets the root directory.
     * @return The root directory.
     */
    std::string getRootDirectory() const;

    /**
     * @brief Sets the root directory.
     * @param rootDirectory The new root directory.
     */
    void setRootDirectory(const std::string& rootDirectory);

    /**
     * @brief Notifies selection listeners of asset selection.
     * @param assetPath The selected asset path.
     */
    void notifyAssetSelected(const std::string& assetPath);

private:
    /**
     * @brief Implementation class for the AssetManagementIntegration.
     */
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace integration
} // namespace rebel
