$baseUrl = "https://raw.githubusercontent.com/ocornut/imgui/docking"
$files = @(
    "imgui.h",
    "imgui.cpp",
    "imgui_internal.h",
    "imgui_demo.cpp",
    "imgui_draw.cpp",
    "imgui_widgets.cpp",
    "imgui_tables.cpp",
    "imstb_rectpack.h",
    "imstb_textedit.h",
    "imstb_truetype.h",
    "backends/imgui_impl_glfw.h",
    "backends/imgui_impl_glfw.cpp",
    "backends/imgui_impl_opengl3.h",
    "backends/imgui_impl_opengl3.cpp",
    "backends/imgui_impl_opengl3_loader.h"
)

# Create directories
New-Item -ItemType Directory -Force -Path "external/imgui/backends"

# Download each file
foreach ($file in $files) {
    $url = "$baseUrl/$file"
    $outputPath = "external/imgui/$file"
    
    Write-Host "Downloading $file..."
    Invoke-WebRequest -Uri $url -OutFile $outputPath
}

Write-Host "Done downloading ImGui files!"

# Copy imconfig.h with docking enabled
$imconfigContent = @"
#pragma once

// Enable docking
#define IMGUI_ENABLE_DOCKING
#define IMGUI_HAS_VIEWPORT

// Include imgui_user.h at the end of imgui.h
//#define IMGUI_INCLUDE_IMGUI_USER_H

// Use 32-bit vertex indices (default is 16-bit) to allow large meshes
//#define ImDrawIdx unsigned int

// Override ImDrawCallback signature
//struct ImDrawList;
//struct ImDrawCmd;
//typedef void (*MyImDrawCallback)(const ImDrawList* draw_list, const ImDrawCmd* cmd, void* my_renderer_user_data);
//#define ImDrawCallback MyImDrawCallback

// Debug Tools
//#define IM_DEBUG_BREAK  IM_ASSERT(0)
//#define IMGUI_DEBUG_PARANOID
"@

Set-Content -Path "external/imgui/imconfig.h" -Value $imconfigContent
Write-Host "Created imconfig.h with docking enabled"
