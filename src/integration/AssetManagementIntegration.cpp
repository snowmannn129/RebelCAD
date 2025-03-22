/**
 * @file AssetManagementIntegration.cpp
 * @brief Implementation of the RebelSUITE Unified Asset Management System integration for RebelCAD.
 */

#include "integration/AssetManagementIntegration.h"
#include "core/Log.h"

// Include the RebelSUITE Asset Management System
#include "asset_management/cpp/AssetManager.h"

#include <filesystem>
#include <fstream>
#include <algorithm>

namespace fs = std::filesystem;

namespace rebel {
namespace integration {

class AssetManagementIntegration::Impl {
public:
    Impl(const std::string& assetRoot)
        : assetRoot(assetRoot.empty() ? getDefaultAssetRoot() : assetRoot),
          currentDirectory(""),
          assetManager(nullptr),
          nextListenerId(1) {
    }

    ~Impl() {
        shutdown();
    }

    bool initialize() {
        try {
            REBEL_LOG_INFO("Initializing asset management integration");

            // Create asset root directory if it doesn't exist
            fs::create_directories(assetRoot);

            // Initialize asset manager
            RebelSUITE::AssetManagement::AssetManagerConfig config;
            config.rootDirectory = assetRoot;
            config.thumbnailSize = { 128, 128 };
            config.cacheSize = 1000;
            config.autoRefresh = true;

            assetManager = std::make_unique<RebelSUITE::AssetManagement::AssetManager>(config);

            // Add change listener
            changeListenerId = assetManager->AddChangeListener(
                [this](const std::vector<RebelSUITE::AssetManagement::FileSystemChange>& changes) {
                    handleAssetChanges(changes);
                }
            );

            // Load assets
            assetManager->LoadDirectory("");

            REBEL_LOG_INFO("Asset management integration initialized successfully");
            return true;
        }
        catch (const std::exception& e) {
            REBEL_LOG_ERROR("Failed to initialize asset management integration: " + std::string(e.what()));
            return false;
        }
    }

    bool shutdown() {
        try {
            REBEL_LOG_INFO("Shutting down asset management integration");

            if (assetManager) {
                assetManager->RemoveChangeListener(changeListenerId);
                assetManager.reset();
            }

            changeCallbacks.clear();
            selectionCallbacks.clear();

            REBEL_LOG_INFO("Asset management integration shut down successfully");
            return true;
        }
        catch (const std::exception& e) {
            REBEL_LOG_ERROR("Failed to shut down asset management integration: " + std::string(e.what()));
            return false;
        }
    }

    bool isAvailable() const {
        return assetManager != nullptr;
    }

    size_t addChangeListener(AssetManagementIntegration::AssetChangeCallback callback) {
        size_t id = nextListenerId++;
        changeCallbacks[id] = callback;
        return id;
    }

    void removeChangeListener(size_t listenerId) {
        changeCallbacks.erase(listenerId);
    }

    size_t addSelectionListener(AssetManagementIntegration::AssetSelectionCallback callback) {
        size_t id = nextListenerId++;
        selectionCallbacks[id] = callback;
        return id;
    }

    void removeSelectionListener(size_t listenerId) {
        selectionCallbacks.erase(listenerId);
    }

    std::vector<RebelSUITE::AssetManagement::AssetMetadata> browseDirectory(const std::string& directory) {
        try {
            REBEL_LOG_INFO("Browsing directory: " + directory);

            if (!isAvailable()) {
                REBEL_LOG_WARNING("Asset management system is not available");
                return {};
            }

            // Load directory
            auto assets = assetManager->LoadDirectory(directory);

            // Update current directory
            currentDirectory = directory;

            return assets;
        }
        catch (const std::exception& e) {
            REBEL_LOG_ERROR("Failed to browse directory " + directory + ": " + std::string(e.what()));
            return {};
        }
    }

    std::vector<RebelSUITE::AssetManagement::AssetMetadata> searchAssets(
        const RebelSUITE::AssetManagement::AssetFilter& filter,
        size_t page,
        size_t pageSize
    ) {
        try {
            REBEL_LOG_INFO("Searching assets");

            if (!isAvailable()) {
                REBEL_LOG_WARNING("Asset management system is not available");
                return {};
            }

            // Search assets
            auto result = assetManager->SearchAssets(filter, page, pageSize);

            return result.assets;
        }
        catch (const std::exception& e) {
            REBEL_LOG_ERROR("Failed to search assets: " + std::string(e.what()));
            return {};
        }
    }

    std::shared_ptr<RebelSUITE::AssetManagement::AssetMetadata> importAsset(
        const std::string& filePath,
        RebelSUITE::AssetManagement::AssetType assetType
    ) {
        try {
            REBEL_LOG_INFO("Importing asset: " + filePath);

            if (!isAvailable()) {
                REBEL_LOG_WARNING("Asset management system is not available");
                return nullptr;
            }

            // Check if file exists
            if (!fs::exists(filePath)) {
                REBEL_LOG_WARNING("File not found: " + filePath);
                return nullptr;
            }

            // Get file name
            std::string fileName = fs::path(filePath).filename().string();

            // Determine asset path
            std::string assetPath = currentDirectory.empty() ? fileName : (currentDirectory + "/" + fileName);

            // Read file data
            std::ifstream file(filePath, std::ios::binary);
            if (!file) {
                REBEL_LOG_WARNING("Failed to open file: " + filePath);
                return nullptr;
            }

            // Get file size
            file.seekg(0, std::ios::end);
            size_t fileSize = file.tellg();
            file.seekg(0, std::ios::beg);

            // Read file content
            std::vector<uint8_t> data(fileSize);
            file.read(reinterpret_cast<char*>(data.data()), fileSize);

            // Create asset
            auto metadata = assetManager->CreateAsset(assetPath, data);

            if (!metadata) {
                REBEL_LOG_WARNING("Failed to create asset: " + assetPath);
                return nullptr;
            }

            return std::make_shared<RebelSUITE::AssetManagement::AssetMetadata>(*metadata);
        }
        catch (const std::exception& e) {
            REBEL_LOG_ERROR("Failed to import asset " + filePath + ": " + std::string(e.what()));
            return nullptr;
        }
    }

    bool exportAsset(const std::string& assetPath, const std::string& exportPath) {
        try {
            REBEL_LOG_INFO("Exporting asset " + assetPath + " to " + exportPath);

            if (!isAvailable()) {
                REBEL_LOG_WARNING("Asset management system is not available");
                return false;
            }

            // Get asset metadata
            auto metadata = assetManager->GetAssetMetadata(assetPath);

            if (!metadata) {
                REBEL_LOG_WARNING("Asset not found: " + assetPath);
                return false;
            }

            // Create export directory if it doesn't exist
            fs::create_directories(fs::path(exportPath).parent_path());

            // Copy the asset
            fs::path sourcePath = fs::path(assetRoot) / assetPath;
            fs::copy_file(sourcePath, exportPath, fs::copy_options::overwrite_existing);

            return true;
        }
        catch (const std::exception& e) {
            REBEL_LOG_ERROR("Failed to export asset " + assetPath + " to " + exportPath + ": " + std::string(e.what()));
            return false;
        }
    }

    std::shared_ptr<RebelSUITE::AssetManagement::AssetMetadata> getAssetMetadata(const std::string& assetPath) {
        try {
            REBEL_LOG_INFO("Getting metadata for asset: " + assetPath);

            if (!isAvailable()) {
                REBEL_LOG_WARNING("Asset management system is not available");
                return nullptr;
            }

            // Get asset metadata
            auto metadata = assetManager->GetAssetMetadata(assetPath);

            if (!metadata) {
                REBEL_LOG_WARNING("Asset not found: " + assetPath);
                return nullptr;
            }

            return std::make_shared<RebelSUITE::AssetManagement::AssetMetadata>(*metadata);
        }
        catch (const std::exception& e) {
            REBEL_LOG_ERROR("Failed to get metadata for asset " + assetPath + ": " + std::string(e.what()));
            return nullptr;
        }
    }

    std::string getAssetThumbnail(const std::string& assetPath) {
        try {
            REBEL_LOG_INFO("Getting thumbnail for asset: " + assetPath);

            if (!isAvailable()) {
                REBEL_LOG_WARNING("Asset management system is not available");
                return "";
            }

            // Get asset thumbnail
            auto thumbnail = assetManager->GetThumbnail(assetPath);

            if (!thumbnail) {
                REBEL_LOG_WARNING("Thumbnail not available for asset: " + assetPath);
                return "";
            }

            return *thumbnail;
        }
        catch (const std::exception& e) {
            REBEL_LOG_ERROR("Failed to get thumbnail for asset " + assetPath + ": " + std::string(e.what()));
            return "";
        }
    }

    std::shared_ptr<RebelSUITE::AssetManagement::AssetMetadata> createAsset(
        const std::string& assetPath,
        const std::vector<uint8_t>& data,
        const std::unordered_map<std::string, std::string>& metadata
    ) {
        try {
            REBEL_LOG_INFO("Creating asset: " + assetPath);

            if (!isAvailable()) {
                REBEL_LOG_WARNING("Asset management system is not available");
                return nullptr;
            }

            // Create asset
            auto assetMetadata = assetManager->CreateAsset(assetPath, data);

            if (!assetMetadata) {
                REBEL_LOG_WARNING("Failed to create asset: " + assetPath);
                return nullptr;
            }

            return std::make_shared<RebelSUITE::AssetManagement::AssetMetadata>(*assetMetadata);
        }
        catch (const std::exception& e) {
            REBEL_LOG_ERROR("Failed to create asset " + assetPath + ": " + std::string(e.what()));
            return nullptr;
        }
    }

    std::shared_ptr<RebelSUITE::AssetManagement::AssetMetadata> updateAsset(
        const std::string& assetPath,
        const std::vector<uint8_t>& data,
        const std::unordered_map<std::string, std::string>& metadata
    ) {
        try {
            REBEL_LOG_INFO("Updating asset: " + assetPath);

            if (!isAvailable()) {
                REBEL_LOG_WARNING("Asset management system is not available");
                return nullptr;
            }

            // Update asset
            auto assetMetadata = assetManager->UpdateAsset(assetPath, data.empty() ? std::nullopt : std::optional<std::vector<uint8_t>>(data));

            if (!assetMetadata) {
                REBEL_LOG_WARNING("Failed to update asset: " + assetPath);
                return nullptr;
            }

            return std::make_shared<RebelSUITE::AssetManagement::AssetMetadata>(*assetMetadata);
        }
        catch (const std::exception& e) {
            REBEL_LOG_ERROR("Failed to update asset " + assetPath + ": " + std::string(e.what()));
            return nullptr;
        }
    }

    bool deleteAsset(const std::string& assetPath) {
        try {
            REBEL_LOG_INFO("Deleting asset: " + assetPath);

            if (!isAvailable()) {
                REBEL_LOG_WARNING("Asset management system is not available");
                return false;
            }

            // Delete asset
            bool success = assetManager->DeleteAsset(assetPath);

            if (!success) {
                REBEL_LOG_WARNING("Failed to delete asset: " + assetPath);
                return false;
            }

            return true;
        }
        catch (const std::exception& e) {
            REBEL_LOG_ERROR("Failed to delete asset " + assetPath + ": " + std::string(e.what()));
            return false;
        }
    }

    const std::string& getCurrentDirectory() const {
        return currentDirectory;
    }

    void setCurrentDirectory(const std::string& directory) {
        currentDirectory = directory;
    }

    std::string getRootDirectory() const {
        return assetRoot;
    }

    void setRootDirectory(const std::string& rootDirectory) {
        if (assetManager) {
            assetManager->SetRootDirectory(rootDirectory);
        }
        assetRoot = rootDirectory;
    }

    void notifyAssetSelected(const std::string& assetPath) {
        for (const auto& [id, callback] : selectionCallbacks) {
            try {
                callback(assetPath);
            }
            catch (const std::exception& e) {
                REBEL_LOG_ERROR("Error in selection listener: " + std::string(e.what()));
            }
        }
    }

private:
    std::string getDefaultAssetRoot() {
        // Default to a directory in the user's home directory
        std::string homeDir = std::getenv("USERPROFILE") ? std::getenv("USERPROFILE") : std::getenv("HOME");
        return fs::path(homeDir) / "RebelSUITE" / "Assets";
    }

    void handleAssetChanges(const std::vector<RebelSUITE::AssetManagement::FileSystemChange>& changes) {
        // Notify all change listeners
        for (const auto& [id, callback] : changeCallbacks) {
            try {
                callback(changes);
            }
            catch (const std::exception& e) {
                REBEL_LOG_ERROR("Error in change listener: " + std::string(e.what()));
            }
        }
    }

    std::string assetRoot;
    std::string currentDirectory;
    std::unique_ptr<RebelSUITE::AssetManagement::AssetManager> assetManager;
    size_t changeListenerId;
    size_t nextListenerId;
    std::unordered_map<size_t, AssetManagementIntegration::AssetChangeCallback> changeCallbacks;
    std::unordered_map<size_t, AssetManagementIntegration::AssetSelectionCallback> selectionCallbacks;
};

// AssetManagementIntegration implementation

AssetManagementIntegration::AssetManagementIntegration(const std::string& assetRoot)
    : pImpl(std::make_unique<Impl>(assetRoot)) {
}

AssetManagementIntegration::~AssetManagementIntegration() = default;

bool AssetManagementIntegration::initialize() {
    return pImpl->initialize();
}

bool AssetManagementIntegration::shutdown() {
    return pImpl->shutdown();
}

bool AssetManagementIntegration::isAvailable() const {
    return pImpl->isAvailable();
}

size_t AssetManagementIntegration::addChangeListener(AssetChangeCallback callback) {
    return pImpl->addChangeListener(std::move(callback));
}

void AssetManagementIntegration::removeChangeListener(size_t listenerId) {
    pImpl->removeChangeListener(listenerId);
}

size_t AssetManagementIntegration::addSelectionListener(AssetSelectionCallback callback) {
    return pImpl->addSelectionListener(std::move(callback));
}

void AssetManagementIntegration::removeSelectionListener(size_t listenerId) {
    pImpl->removeSelectionListener(listenerId);
}

std::vector<RebelSUITE::AssetManagement::AssetMetadata> AssetManagementIntegration::browseDirectory(const std::string& directory) {
    return pImpl->browseDirectory(directory);
}

std::vector<RebelSUITE::AssetManagement::AssetMetadata> AssetManagementIntegration::searchAssets(
    const RebelSUITE::AssetManagement::AssetFilter& filter,
    size_t page,
    size_t pageSize
) {
    return pImpl->searchAssets(filter, page, pageSize);
}

std::shared_ptr<RebelSUITE::AssetManagement::AssetMetadata> AssetManagementIntegration::importAsset(
    const std::string& filePath,
    RebelSUITE::AssetManagement::AssetType assetType
) {
    return pImpl->importAsset(filePath, assetType);
}

bool AssetManagementIntegration::exportAsset(const std::string& assetPath, const std::string& exportPath) {
    return pImpl->exportAsset(assetPath, exportPath);
}

std::shared_ptr<RebelSUITE::AssetManagement::AssetMetadata> AssetManagementIntegration::getAssetMetadata(const std::string& assetPath) {
    return pImpl->getAssetMetadata(assetPath);
}

std::string AssetManagementIntegration::getAssetThumbnail(const std::string& assetPath) {
    return pImpl->getAssetThumbnail(assetPath);
}

std::shared_ptr<RebelSUITE::AssetManagement::AssetMetadata> AssetManagementIntegration::createAsset(
    const std::string& assetPath,
    const std::vector<uint8_t>& data,
    const std::unordered_map<std::string, std::string>& metadata
) {
    return pImpl->createAsset(assetPath, data, metadata);
}

std::shared_ptr<RebelSUITE::AssetManagement::AssetMetadata> AssetManagementIntegration::updateAsset(
    const std::string& assetPath,
    const std::vector<uint8_t>& data,
    const std::unordered_map<std::string, std::string>& metadata
) {
    return pImpl->updateAsset(assetPath, data, metadata);
}

bool AssetManagementIntegration::deleteAsset(const std::string& assetPath) {
    return pImpl->deleteAsset(assetPath);
}

const std::string& AssetManagementIntegration::getCurrentDirectory() const {
    return pImpl->getCurrentDirectory();
}

void AssetManagementIntegration::setCurrentDirectory(const std::string& directory) {
    pImpl->setCurrentDirectory(directory);
}

std::string AssetManagementIntegration::getRootDirectory() const {
    return pImpl->getRootDirectory();
}

void AssetManagementIntegration::setRootDirectory(const std::string& rootDirectory) {
    pImpl->setRootDirectory(rootDirectory);
}

void AssetManagementIntegration::notifyAssetSelected(const std::string& assetPath) {
    pImpl->notifyAssetSelected(assetPath);
}

} // namespace integration
} // namespace rebel
