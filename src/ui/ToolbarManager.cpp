#include "ui/ToolbarManager.h"
#include "ui/MainWindow.h"
#include "imgui.h"
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace RebelCAD {
namespace UI {

// Toolbar implementation
void Toolbar::addItem(std::shared_ptr<ToolbarItem> item) {
    m_Items.push_back(item);
}

void Toolbar::removeItem(const std::string& itemId) {
    m_Items.erase(
        std::remove_if(m_Items.begin(), m_Items.end(),
            [&itemId](const auto& item) { return item->id == itemId; }),
        m_Items.end());
}

// ToolbarManager implementation
ToolbarManager::ToolbarManager(MainWindow* mainWindow)
    : m_MainWindow(mainWindow), m_ShowCustomizationDialog(false) {
    setupDefaultToolbars();
}

void ToolbarManager::createToolbar(const std::string& name) {
    if (m_Toolbars.find(name) == m_Toolbars.end()) {
        m_Toolbars[name] = std::make_shared<Toolbar>(name);
    }
}

void ToolbarManager::removeToolbar(const std::string& name) {
    m_Toolbars.erase(name);
}

void ToolbarManager::addItemToToolbar(const std::string& toolbarName, std::shared_ptr<ToolbarItem> item) {
    auto it = m_Toolbars.find(toolbarName);
    if (it != m_Toolbars.end()) {
        it->second->addItem(item);
    }
}

void ToolbarManager::removeItemFromToolbar(const std::string& toolbarName, const std::string& itemId) {
    auto it = m_Toolbars.find(toolbarName);
    if (it != m_Toolbars.end()) {
        it->second->removeItem(itemId);
    }
}

void ToolbarManager::render() {
    // Render each toolbar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Customize Toolbars...")) {
                m_ShowCustomizationDialog = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // Render toolbars
    float toolbarHeight = 32.0f;
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight()));
    ImGui::SetNextWindowSize(ImVec2(m_MainWindow->getWidth(), toolbarHeight));
    
    ImGuiWindowFlags flags = 
        ImGuiWindowFlags_NoTitleBar | 
        ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove | 
        ImGuiWindowFlags_NoScrollbar | 
        ImGuiWindowFlags_NoSavedSettings;

    if (ImGui::Begin("##Toolbars", nullptr, flags)) {
        for (const auto& [name, toolbar] : m_Toolbars) {
            if (!toolbar->isVisible()) continue;

            ImGui::PushID(name.c_str());
            
            // Add separator between toolbars
            if (ImGui::GetCursorPosX() > 0) {
                ImGui::SameLine();
                ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
                ImGui::SameLine();
            }

            // Render toolbar items
            for (const auto& item : toolbar->getItems()) {
                if (!item->visible) continue;

                if (ImGui::Button(item->name.c_str())) {
                    if (item->enabled && item->action) {
                        item->action();
                    }
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("%s", item->name.c_str());
                }
                ImGui::SameLine();
            }

            ImGui::PopID();
        }
    }
    ImGui::End();

    if (m_ShowCustomizationDialog) {
        renderCustomizationDialog();
    }
}

void ToolbarManager::renderCustomizationDialog() {
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Customize Toolbars", &m_ShowCustomizationDialog)) {
        // Toolbar list
        ImGui::BeginChild("Toolbars", ImVec2(150, 0), true);
        for (const auto& [name, toolbar] : m_Toolbars) {
            if (ImGui::Selectable(name.c_str())) {
                // Handle toolbar selection
            }
        }
        ImGui::EndChild();
        
        ImGui::SameLine();

        // Toolbar items
        ImGui::BeginChild("Items", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
        if (ImGui::BeginTable("##items", 3, ImGuiTableFlags_Borders)) {
            ImGui::TableSetupColumn("Visible");
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Action");
            ImGui::TableHeadersRow();

            for (const auto& [name, toolbar] : m_Toolbars) {
                for (const auto& item : toolbar->getItems()) {
                    ImGui::TableNextRow();
                    
                    ImGui::TableNextColumn();
                    bool visible = item->visible;
                    if (ImGui::Checkbox(("##visible" + item->id).c_str(), &visible)) {
                        item->visible = visible;
                    }

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", item->name.c_str());

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", item->id.c_str());
                }
            }
            ImGui::EndTable();
        }
        ImGui::EndChild();

        // Bottom buttons
        if (ImGui::Button("Save Layout")) {
            saveLayout("toolbar_layout.json");
        }
        ImGui::SameLine();
        if (ImGui::Button("Load Layout")) {
            loadLayout("toolbar_layout.json");
        }
    }
    ImGui::End();
}

void ToolbarManager::setupDefaultToolbars() {
    // Create default toolbars
    createToolbar("Standard");
    createToolbar("Drawing");
    createToolbar("Modeling");
    
    // Add default items to Standard toolbar
    addItemToToolbar("Standard", std::make_shared<ToolbarItem>(
        "new", "New", "icons/new.png",
        []() { /* New file action */ }
    ));
    
    addItemToToolbar("Standard", std::make_shared<ToolbarItem>(
        "open", "Open", "icons/open.png",
        []() { /* Open file action */ }
    ));
    
    addItemToToolbar("Standard", std::make_shared<ToolbarItem>(
        "save", "Save", "icons/save.png",
        []() { /* Save file action */ }
    ));

    // Add default items to Drawing toolbar
    addItemToToolbar("Drawing", std::make_shared<ToolbarItem>(
        "line", "Line", "icons/line.png",
        []() { /* Line tool action */ }
    ));
    
    addItemToToolbar("Drawing", std::make_shared<ToolbarItem>(
        "circle", "Circle", "icons/circle.png",
        []() { /* Circle tool action */ }
    ));
    
    addItemToToolbar("Drawing", std::make_shared<ToolbarItem>(
        "rectangle", "Rectangle", "icons/rectangle.png",
        []() { /* Rectangle tool action */ }
    ));

    // Add default items to Modeling toolbar
    addItemToToolbar("Modeling", std::make_shared<ToolbarItem>(
        "extrude", "Extrude", "icons/extrude.png",
        []() { /* Extrude action */ }
    ));
    
    addItemToToolbar("Modeling", std::make_shared<ToolbarItem>(
        "revolve", "Revolve", "icons/revolve.png",
        []() { /* Revolve action */ }
    ));
    
    addItemToToolbar("Modeling", std::make_shared<ToolbarItem>(
        "sweep", "Sweep", "icons/sweep.png",
        []() { /* Sweep action */ }
    ));
}

void ToolbarManager::saveLayout(const std::string& filename) {
    json j;
    
    // Serialize toolbars
    for (const auto& [name, toolbar] : m_Toolbars) {
        json toolbarJson;
        toolbarJson["name"] = name;
        toolbarJson["visible"] = toolbar->isVisible();
        
        json itemsJson = json::array();
        for (const auto& item : toolbar->getItems()) {
            json itemJson;
            itemJson["id"] = item->id;
            itemJson["visible"] = item->visible;
            itemJson["enabled"] = item->enabled;
            itemsJson.push_back(itemJson);
        }
        toolbarJson["items"] = itemsJson;
        
        j["toolbars"].push_back(toolbarJson);
    }
    
    // Write to file
    std::ofstream file(filename);
    file << j.dump(4);
}

void ToolbarManager::loadLayout(const std::string& filename) {
    try {
        std::ifstream file(filename);
        json j;
        file >> j;
        
        // Clear existing toolbars
        m_Toolbars.clear();
        
        // Load toolbars
        for (const auto& toolbarJson : j["toolbars"]) {
            std::string name = toolbarJson["name"];
            createToolbar(name);
            
            auto toolbar = m_Toolbars[name];
            toolbar->setVisible(toolbarJson["visible"]);
            
            // Load items
            for (const auto& itemJson : toolbarJson["items"]) {
                std::string id = itemJson["id"];
                // Find corresponding default item and update its properties
                auto item = std::make_shared<ToolbarItem>(
                    id, id, "icons/" + id + ".png",
                    []() { /* Default action */ }
                );
                item->visible = itemJson["visible"];
                item->enabled = itemJson["enabled"];
                toolbar->addItem(item);
            }
        }
    } catch (const std::exception& e) {
        // Handle loading errors
    }
}

} // namespace UI
} // namespace RebelCAD
