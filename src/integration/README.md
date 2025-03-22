# RebelCAD Integration with RebelSUITE Unified Asset Management System

This directory contains the integration of RebelCAD with the RebelSUITE Unified Asset Management System. The integration enables asset browsing, searching, and management across all RebelSUITE components.

## Overview

The RebelSUITE Unified Asset Management System provides a centralized way to manage assets across all RebelSUITE components. It supports:

- Cross-component asset management
- Unified asset metadata format (RSA - RebelSUITE Asset)
- Asset versioning and dependency tracking
- Advanced search and filtering
- Thumbnail generation and caching
- Asset conversion between components
- Asset database for tracking all assets

## Integration Components

The integration consists of the following components:

1. **AssetManagementIntegration**: Core integration class that provides access to the RebelSUITE Asset Management System.
2. **AssetBrowserPanel**: UI panel for browsing and managing assets in RebelCAD.

## Usage

### Initializing the Asset Management Integration

```cpp
#include "integration/AssetManagementIntegration.h"

// Create asset management integration
auto assetManager = std::make_shared<rebel::integration::AssetManagementIntegration>();

// Initialize
if (!assetManager->initialize()) {
    // Handle initialization failure
}
```

### Browsing Assets

```cpp
// Browse assets in a directory
auto assets = assetManager->browseDirectory("models");

// Process assets
for (const auto& asset : assets) {
    std::cout << "Asset: " << asset.name << " (" << asset.path << ")" << std::endl;
}
```

### Searching Assets

```cpp
// Create search filter
RebelSUITE::AssetManagement::AssetFilter filter;
filter.types = { RebelSUITE::AssetManagement::AssetType::Model };
filter.tags = { "character" };
filter.searchQuery = "player";

// Search assets
auto assets = assetManager->searchAssets(filter);

// Process search results
for (const auto& asset : assets) {
    std::cout << "Found: " << asset.name << " (" << asset.path << ")" << std::endl;
}
```

### Importing Assets

```cpp
// Import an asset
auto asset = assetManager->importAsset("C:/path/to/model.fbx");

if (asset) {
    std::cout << "Imported: " << asset->name << " (" << asset->path << ")" << std::endl;
}
```

### Exporting Assets

```cpp
// Export an asset
bool success = assetManager->exportAsset("models/character.fbx", "C:/path/to/export/character.fbx");

if (success) {
    std::cout << "Export successful" << std::endl;
}
```

### Creating Assets

```cpp
// Create asset data
std::vector<uint8_t> data = { /* asset data */ };

// Create asset
auto asset = assetManager->createAsset("models/new_model.fbx", data);

if (asset) {
    std::cout << "Created: " << asset->name << " (" << asset->path << ")" << std::endl;
}
```

### Updating Assets

```cpp
// Update asset data
std::vector<uint8_t> newData = { /* new asset data */ };

// Update asset
auto asset = assetManager->updateAsset("models/model.fbx", newData);

if (asset) {
    std::cout << "Updated: " << asset->name << " (" << asset->path << ")" << std::endl;
}
```

### Deleting Assets

```cpp
// Delete an asset
bool success = assetManager->deleteAsset("models/model.fbx");

if (success) {
    std::cout << "Deletion successful" << std::endl;
}
```

### Using the Asset Browser Panel

```cpp
#include "ui/AssetBrowserPanel.h"

// Create asset browser panel
auto assetBrowser = std::make_unique<rebel::ui::AssetBrowserPanel>(assetManager);

// Initialize
if (!assetBrowser->initialize()) {
    // Handle initialization failure
}

// Add selection callback
assetBrowser->addSelectionCallback([](const std::string& assetPath) {
    std::cout << "Selected: " << assetPath << std::endl;
});

// In your render loop
void renderUI() {
    // Render asset browser panel
    assetBrowser->render();
}
```

## Asset Metadata

The RebelSUITE Asset Management System uses a unified metadata format for all assets. The metadata includes:

- Basic information (name, type, size, etc.)
- Tags for categorization
- Dependencies on other assets
- Version history
- Component-specific metadata

## Integration with Other RebelSUITE Components

The asset management integration enables seamless asset sharing between RebelCAD and other RebelSUITE components:

- **RebelENGINE**: Import CAD models into the game engine
- **RebelFLOW**: Automate CAD operations through workflows
- **RebelCODE**: Provide scripting capabilities for CAD operations
- **RebelDESK**: Provide a unified IDE for CAD development
- **RebelSCRIBE**: Generate documentation for CAD models and assemblies

## Testing

The integration includes comprehensive tests to ensure proper functionality:

```cpp
// Run tests
#include "integration/AssetManagementIntegrationTests.cpp"

int main() {
    return rebel::integration::tests::runAllTests() ? 0 : 1;
}
```

## Future Improvements

- Implement asset dependency tracking
- Add support for asset versioning
- Enhance the asset browser UI with thumbnails and previews
- Implement asset conversion between different formats
- Add support for component-specific metadata
