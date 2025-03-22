# RebelCAD UI Components

This directory contains the UI components for RebelCAD. These components provide the user interface for interacting with the application.

## Overview

The RebelCAD UI is built using ImGui, a lightweight immediate-mode GUI library. The UI components are designed to be modular and reusable, allowing for easy customization and extension.

## Components

### AssetBrowserPanel

The AssetBrowserPanel provides a UI for browsing and managing assets in RebelCAD. It integrates with the RebelSUITE Unified Asset Management System to provide a seamless experience for working with assets across all RebelSUITE components.

#### Features

- Browse assets in a directory structure
- Search for assets using various criteria
- Import and export assets
- Create, update, and delete assets
- View asset metadata and thumbnails

#### Usage

```cpp
#include "ui/AssetBrowserPanel.h"
#include "integration/AssetManagementIntegration.h"

// Create asset management integration
auto assetManager = std::make_shared<rebel::integration::AssetManagementIntegration>();
assetManager->initialize();

// Create asset browser panel
auto assetBrowser = std::make_unique<rebel::ui::AssetBrowserPanel>(assetManager);
assetBrowser->initialize();

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

## UI Layout

The RebelCAD UI is organized into several panels:

1. **Main Menu Bar**: Provides access to application-level functionality
2. **Toolbar**: Provides quick access to commonly used tools
3. **Viewport**: Displays the 3D scene
4. **Properties Panel**: Displays properties of the selected object
5. **Asset Browser Panel**: Displays assets available in the project
6. **Console Panel**: Displays log messages and allows command input

## Theming

The RebelCAD UI supports theming through ImGui's style system. The default theme is a dark theme with blue accents, but this can be customized to match the user's preferences.

## Future Improvements

- Add support for docking and window management
- Implement a more comprehensive theming system
- Add support for high-DPI displays
- Implement accessibility features
- Add support for custom UI extensions through plugins
