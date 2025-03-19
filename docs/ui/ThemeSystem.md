# Theme System

The RebelCAD Theme System provides a flexible and extensible way to customize the application's appearance. It supports multiple themes with persistent preferences and seamless switching between different visual styles.

## Features

- Light and Dark theme support
- Persistent theme preferences
- Runtime theme switching
- Extensible theme creation system
- Complete ImGui style customization

## Usage

### Initializing the Theme System

The theme system is automatically initialized with the application. It loads the last used theme preference or defaults to the Light theme if no preference is found.

```cpp
// The theme system is a singleton
auto& themeManager = ThemeManager::getInstance();

// Initialize with default themes (called during app startup)
themeManager.initialize();
```

### Switching Themes

```cpp
// Get available themes
auto themes = themeManager.getAvailableThemes();

// Switch to dark theme
themeManager.applyTheme("Dark");

// Switch to light theme
themeManager.applyTheme("Light");
```

### Saving and Loading Preferences

The theme system automatically saves user preferences to disk and loads them on startup.

```cpp
// Save current theme preference
themeManager.saveThemePreference();

// Load saved preference
themeManager.loadThemePreference();
```

## Theme Structure

Each theme in RebelCAD defines colors for various UI elements:

- Text colors (normal, disabled, selected)
- Background colors (windows, popups, frames)
- Border and shadow colors
- Interactive element colors (buttons, sliders, checkmarks)
- Header and title colors
- Table colors
- Navigation and modal colors

## Default Themes

### Light Theme

The light theme provides a clean, professional look with:
- White backgrounds
- Dark text
- Blue accents for interactive elements
- Subtle shadows and borders
- High contrast for readability

### Dark Theme

The dark theme offers a modern, eye-friendly alternative with:
- Dark backgrounds
- Light text
- Blue accents for interactive elements
- Reduced eye strain for long sessions
- Maintained readability with careful contrast

## Extending the Theme System

New themes can be added by implementing the Theme struct and registering with ThemeManager:

```cpp
Theme customTheme;
customTheme.name = "Custom";
// Set colors...

themeManager.registerTheme("Custom", customTheme);
```

## Implementation Details

The theme system is implemented as a singleton to ensure consistent theming across the application. It uses:

- ImGui's style system for immediate application of theme changes
- JSON for theme preference persistence
- RAII for resource management
- Modern C++ practices for type safety and efficiency

## Best Practices

1. Always use the theme system for color management rather than hard-coding colors
2. Test themes with different UI components to ensure consistency
3. Consider accessibility when creating custom themes
4. Use the preference system to maintain user choices
5. Verify theme changes in both light and dark environments

## Integration with ImGui

The theme system seamlessly integrates with ImGui's styling system, automatically applying colors to all ImGui widgets and windows. This ensures consistent theming across the entire application interface.
