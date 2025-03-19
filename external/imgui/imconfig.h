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
