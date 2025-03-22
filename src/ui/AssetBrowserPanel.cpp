/**
 * @file AssetBrowserPanel.cpp
 * @brief Implementation of the asset browser panel for RebelCAD.
 */

#include "ui/AssetBrowserPanel.h"
#include "integration/AssetManagementIntegration.h"
#include "core/Log.h"

// Include ImGui for UI rendering
#include "imgui.h"

// Include the RebelSUITE Asset Management System
#include "asset_management/cpp/AssetManager.h"

#include <algorithm>
#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;

namespace rebel {
namespace ui {

class AssetBrowserPanel::Impl {
public:
    Impl(std::shared_ptr<integration::AssetManagementIntegration> assetManager)
        : assetManager(assetManager),
          currentDirectory(""),
          selectedAssetPath(""),
          nextCallbackId(1),
          initialized(false) {
    }

    ~Impl() {
        shutdown();
    }

    bool initialize() {
        try {
            REBEL_LOG_INFO("Initializing asset browser panel");

            if (!assetManager) {
                REBEL_LOG_ERROR("Asset manager is null");
                return false;
            }

            // Add change listener to asset manager
            changeListenerId = assetManager->addChangeListener(
                [this](const std::vector<RebelSUITE::AssetManagement::FileSystemChange>& changes) {
                    handleAssetChanges(changes);
                }
            );

            // Load assets in the current directory
            loadCurrentDirectory();

            initialized = true;
            REBEL_LOG_INFO("Asset browser panel initialized successfully");
            return true;
        }
        catch (const std::exception& e) {
            REBEL_LOG_ERROR("Failed to initialize asset browser panel: " + std::string(e.what()));
            return false;
        }
    }

    bool shutdown() {
        try {
            REBEL_LOG_INFO("Shutting down asset browser panel");

            if (assetManager) {
                assetManager->removeChangeListener(changeListenerId);
            }

            selectionCallbacks.clear();
            assets.clear();
            initialized = false;

            REBEL_LOG_INFO("Asset browser panel shut down successfully");
            return true;
        }
        catch (const std::exception& e) {
            REBEL_LOG_ERROR("Failed to shut down asset browser panel: " + std::string(e.what()));
            return false;
        }
    }

    void render() {
        if (!initialized || !assetManager) {
            return;
        }

        // Begin asset browser panel
        if (ImGui::Begin("Asset Browser")) {
            // Render toolbar
            renderToolbar();

            // Render directory tree
            ImGui::BeginChild("DirectoryTree", ImVec2(150, 0), true);
            renderDirectoryTree();
            ImGui::EndChild();

            ImGui::SameLine();

            // Render asset grid
            ImGui::BeginChild("AssetGrid", ImVec2(0, 0), true);
            renderAssetGrid();
            ImGui::EndChild();
        }
        ImGui::End();
    }

    void setCurrentDirectory(const std::string& directory) {
        currentDirectory = directory;
        loadCurrentDirectory();
    }

    const std::string& getCurrentDirectory() const {
        return currentDirectory;
    }

    void refresh() {
        loadCurrentDirectory();
    }

    size_t addSelectionCallback(AssetBrowserPanel::AssetSelectionCallback callback) {
        size_t id = nextCallbackId++;
        selectionCallbacks[id] = callback;
        return id;
    }

    void removeSelectionCallback(size_t callbackId) {
        selectionCallbacks.erase(callbackId);
    }

private:
    void loadCurrentDirectory() {
        if (!assetManager) {
            return;
        }

        // Load assets in the current directory
        assets = assetManager->browseDirectory(currentDirectory);

        // Sort assets by type and name
        std::sort(assets.begin(), assets.end(), [](const RebelSUITE::AssetManagement::AssetMetadata& a, const RebelSUITE::AssetManagement::AssetMetadata& b) {
            if (a.type != b.type) {
                return static_cast<int>(a.type) < static_cast<int>(b.type);
            }
            return a.name < b.name;
        });
    }

    void handleAssetChanges(const std::vector<RebelSUITE::AssetManagement::FileSystemChange>& changes) {
        // Reload the current directory if any changes affect it
        bool shouldReload = false;
        for (const auto& change : changes) {
            std::string changePath = change.path;
            std::string changeDir = fs::path(changePath).parent_path().string();
            
            // Replace backslashes with forward slashes
            std::replace(changeDir.begin(), changeDir.end(), '\\', '/');
            
            if (changeDir == currentDirectory) {
                shouldReload = true;
                break;
            }
        }

        if (shouldReload) {
            loadCurrentDirectory();
        }
    }

    void renderToolbar() {
        // Back button
        if (ImGui::Button("Back")) {
            if (!currentDirectory.empty()) {
                std::string parentDir = fs::path(currentDirectory).parent_path().string();
                // Replace backslashes with forward slashes
                std::replace(parentDir.begin(), parentDir.end(), '\\', '/');
                setCurrentDirectory(parentDir);
            }
        }

        ImGui::SameLine();

        // Current path
        ImGui::Text("%s", currentDirectory.empty() ? "/" : ("/" + currentDirectory).c_str());

        ImGui::SameLine();

        // Refresh button
        if (ImGui::Button("Refresh")) {
            refresh();
        }

        ImGui::SameLine();

        // Import button
        if (ImGui::Button("Import")) {
            // TODO: Implement import dialog
        }

        ImGui::Separator();
    }

    void renderDirectoryTree() {
        // TODO: Implement directory tree rendering
        ImGui::Text("Directory Tree");
    }

    void renderAssetGrid() {
        // Calculate grid size based on available width
        float panelWidth = ImGui::GetContentRegionAvail().x;
        float thumbnailSize = 100.0f;
        int columns = std::max(1, static_cast<int>(panelWidth / (thumbnailSize + 10.0f)));
        
        // Begin grid
        ImGui::Columns(columns, "AssetGrid", false);

        // Render assets
        for (const auto& asset : assets) {
            // Create a unique ID for the asset
            ImGui::PushID(asset.path.c_str());

            // Render thumbnail
            bool selected = selectedAssetPath == asset.path;
            if (selected) {
                ImGui::PushStyleColor(ImGui::Col_Button, ImGui::GetStyle().Colors[ImGui::Col_ButtonActive]);
            }

            // Placeholder thumbnail (colored rectangle based on asset type)
            ImVec4 color = getColorForAssetType(asset.type);
            ImGui::PushStyleColor(ImGui::Col_Button, color);
            if (ImGui::Button("##Thumbnail", ImVec2(thumbnailSize, thumbnailSize))) {
                selectedAssetPath = asset.path;
                notifyAssetSelected(asset.path);
            }
            ImGui::PopStyleColor();

            if (selected) {
                ImGui::PopStyleColor();
            }

            // Render asset name
            ImGui::TextWrapped("%s", asset.name.c_str());

            ImGui::NextColumn();
            ImGui::PopID();
        }

        ImGui::Columns(1);
    }

    ImVec4 getColorForAssetType(RebelSUITE::AssetManagement::AssetType type) {
        // Define colors for different asset types
        switch (type) {
            case RebelSUITE::AssetManagement::AssetType::Model:
            case RebelSUITE::AssetManagement::AssetType::CADModel:
            case RebelSUITE::AssetManagement::AssetType::CADAssembly:
                return ImVec4(0.2f, 0.6f, 0.8f, 1.0f); // Blue
            case RebelSUITE::AssetManagement::AssetType::Texture:
                return ImVec4(0.8f, 0.2f, 0.2f, 1.0f); // Red
            case RebelSUITE::AssetManagement::AssetType::Material:
                return ImVec4(0.8f, 0.6f, 0.2f, 1.0f); // Orange
            case RebelSUITE::AssetManagement::AssetType::Script:
                return ImVec4(0.2f, 0.8f, 0.2f, 1.0f); // Green
            case RebelSUITE::AssetManagement::AssetType::Audio:
                return ImVec4(0.6f, 0.2f, 0.8f, 1.0f); // Purple
            case RebelSUITE::AssetManagement::AssetType::Document:
                return ImVec4(0.8f, 0.8f, 0.2f, 1.0f); // Yellow
            default:
                return ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // Gray
        }
    }

    void notifyAssetSelected(const std::string& assetPath) {
        if (assetManager) {
            assetManager->notifyAssetSelected(assetPath);
        }

        for (const auto& [id, callback] : selectionCallbacks) {
            try {
                callback(assetPath);
            }
            catch (const std::exception& e) {
                REBEL_LOG_ERROR("Error in selection callback: " + std::string(e.what()));
            }
        }
    }

    std::shared_ptr<integration::AssetManagementIntegration> assetManager;
    std::string currentDirectory;
    std::string selectedAssetPath;
    std::vector<RebelSUITE::AssetManagement::AssetMetadata> assets;
    size_t changeListenerId;
    size_t nextCallbackId;
    std::unordered_map<size_t, AssetBrowserPanel::AssetSelectionCallback> selectionCallbacks;
    bool initialized;
};

// AssetBrowserPanel implementation

AssetBrowserPanel::AssetBrowserPanel(std::shared_ptr<integration::AssetManagementIntegration> assetManager)
    : pImpl(std::make_unique<Impl>(assetManager)) {
}

AssetBrowserPanel::~AssetBrowserPanel() = default;

bool AssetBrowserPanel::initialize() {
    return pImpl->initialize();
}

bool AssetBrowserPanel::shutdown() {
    return pImpl->shutdown();
}

void AssetBrowserPanel::render() {
    pImpl->render();
}

void AssetBrowserPanel::setCurrentDirectory(const std::string& directory) {
    pImpl->setCurrentDirectory(directory);
}

const std::string& AssetBrowserPanel::getCurrentDirectory() const {
    return pImpl->getCurrentDirectory();
}

void AssetBrowserPanel::refresh() {
    pImpl->refresh();
}

size_t AssetBrowserPanel::addSelectionCallback(AssetSelectionCallback callback) {
    return pImpl->addSelectionCallback(std::move(callback));
}

void AssetBrowserPanel::removeSelectionCallback(size_t callbackId) {
    pImpl->removeSelectionCallback(callbackId);
}

} // namespace ui
} // namespace rebel
