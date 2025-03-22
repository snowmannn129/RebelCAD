/**
 * @file AssetManagementIntegrationTests.cpp
 * @brief Tests for the RebelSUITE Unified Asset Management System integration for RebelCAD.
 */

#include "integration/AssetManagementIntegration.h"
#include "core/Log.h"

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <fstream>
#include <cassert>

namespace fs = std::filesystem;

// Test helper functions
namespace {

// Create a test file
bool createTestFile(const std::string& path, const std::string& content) {
    try {
        // Create directory if it doesn't exist
        fs::create_directories(fs::path(path).parent_path());

        // Write file
        std::ofstream file(path);
        if (!file) {
            std::cerr << "Failed to create test file: " << path << std::endl;
            return false;
        }

        file << content;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error creating test file: " << e.what() << std::endl;
        return false;
    }
}

// Delete a test file
bool deleteTestFile(const std::string& path) {
    try {
        return fs::remove(path);
    }
    catch (const std::exception& e) {
        std::cerr << "Error deleting test file: " << e.what() << std::endl;
        return false;
    }
}

// Create a test directory
bool createTestDirectory(const std::string& path) {
    try {
        return fs::create_directories(path);
    }
    catch (const std::exception& e) {
        std::cerr << "Error creating test directory: " << e.what() << std::endl;
        return false;
    }
}

// Delete a test directory
bool deleteTestDirectory(const std::string& path) {
    try {
        return fs::remove_all(path) > 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error deleting test directory: " << e.what() << std::endl;
        return false;
    }
}

} // namespace

// Test functions
namespace rebel {
namespace integration {
namespace tests {

// Test initialization and shutdown
bool testInitializationAndShutdown() {
    std::cout << "Testing initialization and shutdown..." << std::endl;

    // Create a temporary test directory
    std::string testDir = fs::temp_directory_path().string() + "/RebelCAD_AssetManagementTest";
    if (!createTestDirectory(testDir)) {
        std::cerr << "Failed to create test directory" << std::endl;
        return false;
    }

    // Create asset management integration
    AssetManagementIntegration assetManager(testDir);

    // Initialize
    bool initResult = assetManager.initialize();
    if (!initResult) {
        std::cerr << "Failed to initialize asset management integration" << std::endl;
        deleteTestDirectory(testDir);
        return false;
    }

    // Check if available
    if (!assetManager.isAvailable()) {
        std::cerr << "Asset management integration is not available after initialization" << std::endl;
        assetManager.shutdown();
        deleteTestDirectory(testDir);
        return false;
    }

    // Shutdown
    bool shutdownResult = assetManager.shutdown();
    if (!shutdownResult) {
        std::cerr << "Failed to shut down asset management integration" << std::endl;
        deleteTestDirectory(testDir);
        return false;
    }

    // Check if not available
    if (assetManager.isAvailable()) {
        std::cerr << "Asset management integration is still available after shutdown" << std::endl;
        deleteTestDirectory(testDir);
        return false;
    }

    // Clean up
    deleteTestDirectory(testDir);

    std::cout << "Initialization and shutdown test passed" << std::endl;
    return true;
}

// Test asset creation and retrieval
bool testAssetCreationAndRetrieval() {
    std::cout << "Testing asset creation and retrieval..." << std::endl;

    // Create a temporary test directory
    std::string testDir = fs::temp_directory_path().string() + "/RebelCAD_AssetManagementTest";
    if (!createTestDirectory(testDir)) {
        std::cerr << "Failed to create test directory" << std::endl;
        return false;
    }

    // Create asset management integration
    AssetManagementIntegration assetManager(testDir);
    assetManager.initialize();

    // Create test asset data
    std::vector<uint8_t> assetData = {'T', 'e', 's', 't', ' ', 'A', 's', 's', 'e', 't'};
    std::string assetPath = "test_asset.txt";

    // Create asset
    auto asset = assetManager.createAsset(assetPath, assetData);
    if (!asset) {
        std::cerr << "Failed to create asset" << std::endl;
        assetManager.shutdown();
        deleteTestDirectory(testDir);
        return false;
    }

    // Check asset path
    if (asset->path != assetPath) {
        std::cerr << "Asset path mismatch: " << asset->path << " != " << assetPath << std::endl;
        assetManager.shutdown();
        deleteTestDirectory(testDir);
        return false;
    }

    // Get asset metadata
    auto retrievedAsset = assetManager.getAssetMetadata(assetPath);
    if (!retrievedAsset) {
        std::cerr << "Failed to retrieve asset metadata" << std::endl;
        assetManager.shutdown();
        deleteTestDirectory(testDir);
        return false;
    }

    // Check asset path
    if (retrievedAsset->path != assetPath) {
        std::cerr << "Retrieved asset path mismatch: " << retrievedAsset->path << " != " << assetPath << std::endl;
        assetManager.shutdown();
        deleteTestDirectory(testDir);
        return false;
    }

    // Delete asset
    bool deleteResult = assetManager.deleteAsset(assetPath);
    if (!deleteResult) {
        std::cerr << "Failed to delete asset" << std::endl;
        assetManager.shutdown();
        deleteTestDirectory(testDir);
        return false;
    }

    // Check if asset is deleted
    auto deletedAsset = assetManager.getAssetMetadata(assetPath);
    if (deletedAsset) {
        std::cerr << "Asset still exists after deletion" << std::endl;
        assetManager.shutdown();
        deleteTestDirectory(testDir);
        return false;
    }

    // Clean up
    assetManager.shutdown();
    deleteTestDirectory(testDir);

    std::cout << "Asset creation and retrieval test passed" << std::endl;
    return true;
}

// Test directory browsing
bool testDirectoryBrowsing() {
    std::cout << "Testing directory browsing..." << std::endl;

    // Create a temporary test directory
    std::string testDir = fs::temp_directory_path().string() + "/RebelCAD_AssetManagementTest";
    if (!createTestDirectory(testDir)) {
        std::cerr << "Failed to create test directory" << std::endl;
        return false;
    }

    // Create asset management integration
    AssetManagementIntegration assetManager(testDir);
    assetManager.initialize();

    // Create test subdirectory
    std::string subDir = "test_subdir";
    if (!createTestDirectory(testDir + "/" + subDir)) {
        std::cerr << "Failed to create test subdirectory" << std::endl;
        assetManager.shutdown();
        deleteTestDirectory(testDir);
        return false;
    }

    // Create test assets
    std::vector<uint8_t> assetData = {'T', 'e', 's', 't', ' ', 'A', 's', 's', 'e', 't'};
    std::string assetPath1 = "test_asset1.txt";
    std::string assetPath2 = subDir + "/test_asset2.txt";

    // Create assets
    auto asset1 = assetManager.createAsset(assetPath1, assetData);
    auto asset2 = assetManager.createAsset(assetPath2, assetData);

    if (!asset1 || !asset2) {
        std::cerr << "Failed to create test assets" << std::endl;
        assetManager.shutdown();
        deleteTestDirectory(testDir);
        return false;
    }

    // Browse root directory
    auto rootAssets = assetManager.browseDirectory("");
    if (rootAssets.empty()) {
        std::cerr << "Root directory is empty" << std::endl;
        assetManager.shutdown();
        deleteTestDirectory(testDir);
        return false;
    }

    // Check if root directory contains the first asset
    bool foundAsset1 = false;
    for (const auto& asset : rootAssets) {
        if (asset.path == assetPath1) {
            foundAsset1 = true;
            break;
        }
    }

    if (!foundAsset1) {
        std::cerr << "Asset 1 not found in root directory" << std::endl;
        assetManager.shutdown();
        deleteTestDirectory(testDir);
        return false;
    }

    // Browse subdirectory
    auto subDirAssets = assetManager.browseDirectory(subDir);
    if (subDirAssets.empty()) {
        std::cerr << "Subdirectory is empty" << std::endl;
        assetManager.shutdown();
        deleteTestDirectory(testDir);
        return false;
    }

    // Check if subdirectory contains the second asset
    bool foundAsset2 = false;
    for (const auto& asset : subDirAssets) {
        if (asset.path == assetPath2) {
            foundAsset2 = true;
            break;
        }
    }

    if (!foundAsset2) {
        std::cerr << "Asset 2 not found in subdirectory" << std::endl;
        assetManager.shutdown();
        deleteTestDirectory(testDir);
        return false;
    }

    // Clean up
    assetManager.deleteAsset(assetPath1);
    assetManager.deleteAsset(assetPath2);
    assetManager.shutdown();
    deleteTestDirectory(testDir);

    std::cout << "Directory browsing test passed" << std::endl;
    return true;
}

// Test asset import and export
bool testAssetImportExport() {
    std::cout << "Testing asset import and export..." << std::endl;

    // Create a temporary test directory
    std::string testDir = fs::temp_directory_path().string() + "/RebelCAD_AssetManagementTest";
    if (!createTestDirectory(testDir)) {
        std::cerr << "Failed to create test directory" << std::endl;
        return false;
    }

    // Create a temporary file to import
    std::string importFilePath = testDir + "/import_file.txt";
    std::string importContent = "This is a test file for import";
    if (!createTestFile(importFilePath, importContent)) {
        std::cerr << "Failed to create import file" << std::endl;
        deleteTestDirectory(testDir);
        return false;
    }

    // Create asset management integration
    AssetManagementIntegration assetManager(testDir);
    assetManager.initialize();

    // Import asset
    auto importedAsset = assetManager.importAsset(importFilePath);
    if (!importedAsset) {
        std::cerr << "Failed to import asset" << std::endl;
        assetManager.shutdown();
        deleteTestDirectory(testDir);
        return false;
    }

    // Check imported asset
    std::string assetPath = fs::path(importFilePath).filename().string();
    if (importedAsset->path != assetPath) {
        std::cerr << "Imported asset path mismatch: " << importedAsset->path << " != " << assetPath << std::endl;
        assetManager.shutdown();
        deleteTestDirectory(testDir);
        return false;
    }

    // Export asset
    std::string exportFilePath = testDir + "/export_file.txt";
    bool exportResult = assetManager.exportAsset(assetPath, exportFilePath);
    if (!exportResult) {
        std::cerr << "Failed to export asset" << std::endl;
        assetManager.shutdown();
        deleteTestDirectory(testDir);
        return false;
    }

    // Check if export file exists
    if (!fs::exists(exportFilePath)) {
        std::cerr << "Export file does not exist" << std::endl;
        assetManager.shutdown();
        deleteTestDirectory(testDir);
        return false;
    }

    // Clean up
    assetManager.deleteAsset(assetPath);
    assetManager.shutdown();
    deleteTestFile(importFilePath);
    deleteTestFile(exportFilePath);
    deleteTestDirectory(testDir);

    std::cout << "Asset import and export test passed" << std::endl;
    return true;
}

// Run all tests
bool runAllTests() {
    bool success = true;

    success &= testInitializationAndShutdown();
    success &= testAssetCreationAndRetrieval();
    success &= testDirectoryBrowsing();
    success &= testAssetImportExport();

    return success;
}

} // namespace tests
} // namespace integration
} // namespace rebel

// Main function
int main() {
    std::cout << "Running AssetManagementIntegration tests..." << std::endl;

    bool success = rebel::integration::tests::runAllTests();

    if (success) {
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } else {
        std::cerr << "Some tests failed!" << std::endl;
        return 1;
    }
}
