#include "simulation/Mesh.h"
#include "simulation/Material.h"
#include "core/Log.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <unordered_set>
#include <cmath>

namespace rebel::simulation {

Mesh::Mesh(const std::string& id)
    : id(id) {
}

std::shared_ptr<Mesh> Mesh::createFromGeometry(
    const std::string& geometryId,
    ElementType elementType,
    int refinementLevel) {
    
    if (refinementLevel < 1 || refinementLevel > 5) {
        REBEL_LOG_ERROR("Invalid refinement level: " + std::to_string(refinementLevel));
        return nullptr;
    }
    
    REBEL_LOG_INFO("Creating mesh from geometry: " + geometryId);
    REBEL_LOG_INFO("Element type: " + std::to_string(static_cast<int>(elementType)));
    REBEL_LOG_INFO("Refinement level: " + std::to_string(refinementLevel));
    
    try {
        // Create a new mesh
        auto mesh = std::shared_ptr<Mesh>(new Mesh(geometryId + "_mesh"));
        
        // TODO: Implement actual mesh generation from geometry
        // For now, create a simple cube mesh for testing
        
        // Create nodes
        const double size = 1.0;
        const int divisions = refinementLevel;
        const double step = size / divisions;
        
        int nodeId = 0;
        for (int i = 0; i <= divisions; ++i) {
            for (int j = 0; j <= divisions; ++j) {
                for (int k = 0; k <= divisions; ++k) {
                    double x = i * step;
                    double y = j * step;
                    double z = k * step;
                    
                    mesh->nodes.push_back(Node(nodeId, x, y, z));
                    mesh->nodeIdToIndex[nodeId] = mesh->nodes.size() - 1;
                    ++nodeId;
                }
            }
        }
        
        // Create elements based on element type
        int elementId = 0;
        
        if (elementType == ElementType::Tetra) {
            // Create tetrahedral elements
            for (int i = 0; i < divisions; ++i) {
                for (int j = 0; j < divisions; ++j) {
                    for (int k = 0; k < divisions; ++k) {
                        // Get the 8 nodes of the cube
                        int n0 = i * (divisions + 1) * (divisions + 1) + j * (divisions + 1) + k;
                        int n1 = i * (divisions + 1) * (divisions + 1) + j * (divisions + 1) + (k + 1);
                        int n2 = i * (divisions + 1) * (divisions + 1) + (j + 1) * (divisions + 1) + k;
                        int n3 = i * (divisions + 1) * (divisions + 1) + (j + 1) * (divisions + 1) + (k + 1);
                        int n4 = (i + 1) * (divisions + 1) * (divisions + 1) + j * (divisions + 1) + k;
                        int n5 = (i + 1) * (divisions + 1) * (divisions + 1) + j * (divisions + 1) + (k + 1);
                        int n6 = (i + 1) * (divisions + 1) * (divisions + 1) + (j + 1) * (divisions + 1) + k;
                        int n7 = (i + 1) * (divisions + 1) * (divisions + 1) + (j + 1) * (divisions + 1) + (k + 1);
                        
                        // Create 5 tetrahedra from the cube
                        mesh->elements.push_back(Element(elementId++, ElementType::Tetra, {n0, n1, n2, n4}, nullptr));
                        mesh->elements.push_back(Element(elementId++, ElementType::Tetra, {n1, n3, n2, n7}, nullptr));
                        mesh->elements.push_back(Element(elementId++, ElementType::Tetra, {n1, n5, n4, n7}, nullptr));
                        mesh->elements.push_back(Element(elementId++, ElementType::Tetra, {n2, n6, n4, n7}, nullptr));
                        mesh->elements.push_back(Element(elementId++, ElementType::Tetra, {n1, n4, n2, n7}, nullptr));
                    }
                }
            }
        } else if (elementType == ElementType::Hexa) {
            // Create hexahedral elements
            for (int i = 0; i < divisions; ++i) {
                for (int j = 0; j < divisions; ++j) {
                    for (int k = 0; k < divisions; ++k) {
                        // Get the 8 nodes of the cube
                        int n0 = i * (divisions + 1) * (divisions + 1) + j * (divisions + 1) + k;
                        int n1 = i * (divisions + 1) * (divisions + 1) + j * (divisions + 1) + (k + 1);
                        int n2 = i * (divisions + 1) * (divisions + 1) + (j + 1) * (divisions + 1) + k;
                        int n3 = i * (divisions + 1) * (divisions + 1) + (j + 1) * (divisions + 1) + (k + 1);
                        int n4 = (i + 1) * (divisions + 1) * (divisions + 1) + j * (divisions + 1) + k;
                        int n5 = (i + 1) * (divisions + 1) * (divisions + 1) + j * (divisions + 1) + (k + 1);
                        int n6 = (i + 1) * (divisions + 1) * (divisions + 1) + (j + 1) * (divisions + 1) + k;
                        int n7 = (i + 1) * (divisions + 1) * (divisions + 1) + (j + 1) * (divisions + 1) + (k + 1);
                        
                        // Create a hexahedral element
                        mesh->elements.push_back(Element(elementId++, ElementType::Hexa, {n0, n1, n3, n2, n4, n5, n7, n6}, nullptr));
                    }
                }
            }
        } else {
            REBEL_LOG_ERROR("Unsupported element type: " + std::to_string(static_cast<int>(elementType)));
            return nullptr;
        }
        
        // Update element ID to index map
        for (size_t i = 0; i < mesh->elements.size(); ++i) {
            mesh->elementIdToIndex[mesh->elements[i].id] = i;
        }
        
        // Create node groups for boundary conditions
        mesh->createNodeGroup("Left");
        mesh->createNodeGroup("Right");
        mesh->createNodeGroup("Bottom");
        mesh->createNodeGroup("Top");
        mesh->createNodeGroup("Front");
        mesh->createNodeGroup("Back");
        
        // Add nodes to groups
        for (const auto& node : mesh->nodes) {
            if (std::abs(node.x) < 1e-6) {
                mesh->addNodeToGroup(mesh->getNodeGroupByName("Left")->id, node.id);
            }
            if (std::abs(node.x - size) < 1e-6) {
                mesh->addNodeToGroup(mesh->getNodeGroupByName("Right")->id, node.id);
            }
            if (std::abs(node.y) < 1e-6) {
                mesh->addNodeToGroup(mesh->getNodeGroupByName("Bottom")->id, node.id);
            }
            if (std::abs(node.y - size) < 1e-6) {
                mesh->addNodeToGroup(mesh->getNodeGroupByName("Top")->id, node.id);
            }
            if (std::abs(node.z) < 1e-6) {
                mesh->addNodeToGroup(mesh->getNodeGroupByName("Front")->id, node.id);
            }
            if (std::abs(node.z - size) < 1e-6) {
                mesh->addNodeToGroup(mesh->getNodeGroupByName("Back")->id, node.id);
            }
        }
        
        REBEL_LOG_INFO("Mesh created successfully with " + 
                      std::to_string(mesh->nodes.size()) + " nodes and " +
                      std::to_string(mesh->elements.size()) + " elements");
        
        return mesh;
    } catch (const std::exception& e) {
        REBEL_LOG_ERROR("Exception while creating mesh: " + std::string(e.what()));
        return nullptr;
    }
}

std::shared_ptr<Mesh> Mesh::createFromSTL(
    const std::string& filePath,
    ElementType elementType,
    int refinementLevel) {
    
    REBEL_LOG_INFO("Creating mesh from STL file: " + filePath);
    REBEL_LOG_INFO("Element type: " + std::to_string(static_cast<int>(elementType)));
    REBEL_LOG_INFO("Refinement level: " + std::to_string(refinementLevel));
    
    // TODO: Implement STL file parsing and mesh generation
    REBEL_LOG_ERROR("STL mesh generation not implemented yet");
    return nullptr;
}

std::shared_ptr<Mesh> Mesh::createFromSTEP(
    const std::string& filePath,
    ElementType elementType,
    int refinementLevel) {
    
    REBEL_LOG_INFO("Creating mesh from STEP file: " + filePath);
    REBEL_LOG_INFO("Element type: " + std::to_string(static_cast<int>(elementType)));
    REBEL_LOG_INFO("Refinement level: " + std::to_string(refinementLevel));
    
    // TODO: Implement STEP file parsing and mesh generation
    REBEL_LOG_ERROR("STEP mesh generation not implemented yet");
    return nullptr;
}

const Node* Mesh::getNode(int nodeId) const {
    auto it = nodeIdToIndex.find(nodeId);
    if (it != nodeIdToIndex.end()) {
        return &nodes[it->second];
    }
    return nullptr;
}

const Element* Mesh::getElement(int elementId) const {
    auto it = elementIdToIndex.find(elementId);
    if (it != elementIdToIndex.end()) {
        return &elements[it->second];
    }
    return nullptr;
}

int Mesh::createElementGroup(const std::string& name, std::shared_ptr<Material> material) {
    if (elementGroupNameToIndex.find(name) != elementGroupNameToIndex.end()) {
        REBEL_LOG_WARNING("Element group with name '" + name + "' already exists");
        return -1;
    }
    
    int id = static_cast<int>(elementGroups.size());
    elementGroups.push_back(ElementGroup(id, name, material));
    elementGroupIdToIndex[id] = elementGroups.size() - 1;
    elementGroupNameToIndex[name] = elementGroups.size() - 1;
    
    REBEL_LOG_INFO("Created element group: " + name);
    return id;
}

int Mesh::createNodeGroup(const std::string& name) {
    if (nodeGroupNameToIndex.find(name) != nodeGroupNameToIndex.end()) {
        REBEL_LOG_WARNING("Node group with name '" + name + "' already exists");
        return -1;
    }
    
    int id = static_cast<int>(nodeGroups.size());
    nodeGroups.push_back(NodeGroup(id, name));
    nodeGroupIdToIndex[id] = nodeGroups.size() - 1;
    nodeGroupNameToIndex[name] = nodeGroups.size() - 1;
    
    REBEL_LOG_INFO("Created node group: " + name);
    return id;
}

bool Mesh::addElementToGroup(int groupId, int elementId) {
    auto groupIt = elementGroupIdToIndex.find(groupId);
    if (groupIt == elementGroupIdToIndex.end()) {
        REBEL_LOG_ERROR("Element group with ID " + std::to_string(groupId) + " not found");
        return false;
    }
    
    auto elementIt = elementIdToIndex.find(elementId);
    if (elementIt == elementIdToIndex.end()) {
        REBEL_LOG_ERROR("Element with ID " + std::to_string(elementId) + " not found");
        return false;
    }
    
    ElementGroup& group = elementGroups[groupIt->second];
    
    // Check if element is already in the group
    if (std::find(group.elementIds.begin(), group.elementIds.end(), elementId) != group.elementIds.end()) {
        REBEL_LOG_WARNING("Element with ID " + std::to_string(elementId) + " already in group " + group.name);
        return true;
    }
    
    group.elementIds.push_back(elementId);
    
    // Set the material for the element
    if (group.material) {
        elements[elementIt->second].material = group.material;
    }
    
    return true;
}

bool Mesh::addNodeToGroup(int groupId, int nodeId) {
    auto groupIt = nodeGroupIdToIndex.find(groupId);
    if (groupIt == nodeGroupIdToIndex.end()) {
        REBEL_LOG_ERROR("Node group with ID " + std::to_string(groupId) + " not found");
        return false;
    }
    
    auto nodeIt = nodeIdToIndex.find(nodeId);
    if (nodeIt == nodeIdToIndex.end()) {
        REBEL_LOG_ERROR("Node with ID " + std::to_string(nodeId) + " not found");
        return false;
    }
    
    NodeGroup& group = nodeGroups[groupIt->second];
    
    // Check if node is already in the group
    if (std::find(group.nodeIds.begin(), group.nodeIds.end(), nodeId) != group.nodeIds.end()) {
        REBEL_LOG_WARNING("Node with ID " + std::to_string(nodeId) + " already in group " + group.name);
        return true;
    }
    
    group.nodeIds.push_back(nodeId);
    return true;
}

const ElementGroup* Mesh::getElementGroup(int groupId) const {
    auto it = elementGroupIdToIndex.find(groupId);
    if (it != elementGroupIdToIndex.end()) {
        return &elementGroups[it->second];
    }
    return nullptr;
}

const NodeGroup* Mesh::getNodeGroup(int groupId) const {
    auto it = nodeGroupIdToIndex.find(groupId);
    if (it != nodeGroupIdToIndex.end()) {
        return &nodeGroups[it->second];
    }
    return nullptr;
}

const ElementGroup* Mesh::getElementGroupByName(const std::string& name) const {
    auto it = elementGroupNameToIndex.find(name);
    if (it != elementGroupNameToIndex.end()) {
        return &elementGroups[it->second];
    }
    return nullptr;
}

const NodeGroup* Mesh::getNodeGroupByName(const std::string& name) const {
    auto it = nodeGroupNameToIndex.find(name);
    if (it != nodeGroupNameToIndex.end()) {
        return &nodeGroups[it->second];
    }
    return nullptr;
}

bool Mesh::setElementMaterial(int elementId, std::shared_ptr<Material> material) {
    auto it = elementIdToIndex.find(elementId);
    if (it == elementIdToIndex.end()) {
        REBEL_LOG_ERROR("Element with ID " + std::to_string(elementId) + " not found");
        return false;
    }
    
    elements[it->second].material = material;
    return true;
}

bool Mesh::setElementGroupMaterial(int groupId, std::shared_ptr<Material> material) {
    auto it = elementGroupIdToIndex.find(groupId);
    if (it == elementGroupIdToIndex.end()) {
        REBEL_LOG_ERROR("Element group with ID " + std::to_string(groupId) + " not found");
        return false;
    }
    
    ElementGroup& group = elementGroups[it->second];
    group.material = material;
    
    // Update material for all elements in the group
    for (int elementId : group.elementIds) {
        auto elemIt = elementIdToIndex.find(elementId);
        if (elemIt != elementIdToIndex.end()) {
            elements[elemIt->second].material = material;
        }
    }
    
    return true;
}

bool Mesh::refine(int refinementLevel) {
    if (refinementLevel < 1 || refinementLevel > 5) {
        REBEL_LOG_ERROR("Invalid refinement level: " + std::to_string(refinementLevel));
        return false;
    }
    
    REBEL_LOG_INFO("Refining mesh to level " + std::to_string(refinementLevel));
    
    // TODO: Implement mesh refinement
    REBEL_LOG_ERROR("Mesh refinement not implemented yet");
    return false;
}

MeshQuality Mesh::evaluateQuality(const std::vector<MeshQualityMetric>& metrics) const {
    MeshQuality quality;
    
    if (elements.empty()) {
        REBEL_LOG_WARNING("Cannot evaluate quality of empty mesh");
        return quality;
    }
    
    REBEL_LOG_INFO("Evaluating mesh quality");
    
    // Initialize with extreme values
    quality.minAspectRatio = std::numeric_limits<double>::max();
    quality.maxAspectRatio = 0.0;
    quality.avgAspectRatio = 0.0;
    quality.minSkewness = std::numeric_limits<double>::max();
    quality.maxSkewness = 0.0;
    quality.avgSkewness = 0.0;
    quality.minJacobian = std::numeric_limits<double>::max();
    quality.minOrthogonalQuality = std::numeric_limits<double>::max();
    quality.maxWarpingFactor = 0.0;
    
    // TODO: Implement actual quality metrics calculation
    // For now, just set some dummy values
    quality.minAspectRatio = 1.0;
    quality.maxAspectRatio = 3.0;
    quality.avgAspectRatio = 1.5;
    quality.minSkewness = 0.1;
    quality.maxSkewness = 0.5;
    quality.avgSkewness = 0.3;
    quality.minJacobian = 0.8;
    quality.minOrthogonalQuality = 0.7;
    quality.maxWarpingFactor = 0.2;
    
    REBEL_LOG_INFO("Mesh quality evaluation complete");
    return quality;
}

bool Mesh::exportToFile(const std::string& filePath) const {
    REBEL_LOG_INFO("Exporting mesh to file: " + filePath);
    
    try {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            REBEL_LOG_ERROR("Failed to open file for writing: " + filePath);
            return false;
        }
        
        // Write mesh ID
        file << "# Mesh: " << id << std::endl;
        
        // Write nodes
        file << "# Nodes: " << nodes.size() << std::endl;
        file << "Nodes" << std::endl;
        for (const auto& node : nodes) {
            file << node.id << " " << node.x << " " << node.y << " " << node.z << std::endl;
        }
        
        // Write elements
        file << "# Elements: " << elements.size() << std::endl;
        file << "Elements" << std::endl;
        for (const auto& element : elements) {
            file << element.id << " " << static_cast<int>(element.type) << " " << element.nodeIds.size();
            for (int nodeId : element.nodeIds) {
                file << " " << nodeId;
            }
            file << std::endl;
        }
        
        // Write node groups
        file << "# Node Groups: " << nodeGroups.size() << std::endl;
        file << "NodeGroups" << std::endl;
        for (const auto& group : nodeGroups) {
            file << group.id << " " << group.name << " " << group.nodeIds.size();
            for (int nodeId : group.nodeIds) {
                file << " " << nodeId;
            }
            file << std::endl;
        }
        
        // Write element groups
        file << "# Element Groups: " << elementGroups.size() << std::endl;
        file << "ElementGroups" << std::endl;
        for (const auto& group : elementGroups) {
            file << group.id << " " << group.name << " " << (group.material ? group.material->getName() : "None") << " " << group.elementIds.size();
            for (int elementId : group.elementIds) {
                file << " " << elementId;
            }
            file << std::endl;
        }
        
        file.close();
        REBEL_LOG_INFO("Mesh exported successfully");
        return true;
    } catch (const std::exception& e) {
        REBEL_LOG_ERROR("Exception while exporting mesh: " + std::string(e.what()));
        return false;
    }
}

std::shared_ptr<Mesh> Mesh::importFromFile(const std::string& filePath) {
    REBEL_LOG_INFO("Importing mesh from file: " + filePath);
    
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            REBEL_LOG_ERROR("Failed to open file for reading: " + filePath);
            return nullptr;
        }
        
        std::string line;
        std::string meshId;
        
        // Read mesh ID
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') {
                continue;
            }
            
            if (line.substr(0, 6) == "# Mesh:") {
                std::istringstream iss(line.substr(7));
                iss >> meshId;
                break;
            }
        }
        
        if (meshId.empty()) {
            meshId = "imported_mesh";
        }
        
        auto mesh = std::shared_ptr<Mesh>(new Mesh(meshId));
        
        // Read nodes
        bool readingNodes = false;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') {
                continue;
            }
            
            if (line == "Nodes") {
                readingNodes = true;
                continue;
            }
            
            if (readingNodes) {
                if (line == "Elements") {
                    break;
                }
                
                std::istringstream iss(line);
                int id;
                double x, y, z;
                iss >> id >> x >> y >> z;
                
                mesh->nodes.push_back(Node(id, x, y, z));
                mesh->nodeIdToIndex[id] = mesh->nodes.size() - 1;
            }
        }
        
        // Read elements
        bool readingElements = true;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') {
                continue;
            }
            
            if (line == "NodeGroups") {
                readingElements = false;
                break;
            }
            
            if (readingElements) {
                std::istringstream iss(line);
                int id, typeInt, numNodes;
                iss >> id >> typeInt >> numNodes;
                
                ElementType type = static_cast<ElementType>(typeInt);
                std::vector<int> nodeIds(numNodes);
                for (int i = 0; i < numNodes; ++i) {
                    iss >> nodeIds[i];
                }
                
                mesh->elements.push_back(Element(id, type, nodeIds, nullptr));
                mesh->elementIdToIndex[id] = mesh->elements.size() - 1;
            }
        }
        
        // Read node groups
        bool readingNodeGroups = true;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') {
                continue;
            }
            
            if (line == "ElementGroups") {
                readingNodeGroups = false;
                break;
            }
            
            if (readingNodeGroups) {
                std::istringstream iss(line);
                int id, numNodes;
                std::string name;
                iss >> id >> name >> numNodes;
                
                NodeGroup group(id, name);
                for (int i = 0; i < numNodes; ++i) {
                    int nodeId;
                    iss >> nodeId;
                    group.nodeIds.push_back(nodeId);
                }
                
                mesh->nodeGroups.push_back(group);
                mesh->nodeGroupIdToIndex[id] = mesh->nodeGroups.size() - 1;
                mesh->nodeGroupNameToIndex[name] = mesh->nodeGroups.size() - 1;
            }
        }
        
        // Read element groups
        bool readingElementGroups = true;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') {
                continue;
            }
            
            if (readingElementGroups) {
                std::istringstream iss(line);
                int id, numElements;
                std::string name, materialName;
                iss >> id >> name >> materialName >> numElements;
                
                std::shared_ptr<Material> material = nullptr;
                // TODO: Load material from library
                
                ElementGroup group(id, name, material);
                for (int i = 0; i < numElements; ++i) {
                    int elementId;
                    iss >> elementId;
                    group.elementIds.push_back(elementId);
                }
                
                mesh->elementGroups.push_back(group);
                mesh->elementGroupIdToIndex[id] = mesh->elementGroups.size() - 1;
                mesh->elementGroupNameToIndex[name] = mesh->elementGroups.size() - 1;
            }
        }
        
        file.close();
        REBEL_LOG_INFO("Mesh imported successfully with " + 
                      std::to_string(mesh->nodes.size()) + " nodes and " +
                      std::to_string(mesh->elements.size()) + " elements");
        
        return mesh;
    } catch (const std::exception& e) {
        REBEL_LOG_ERROR("Exception while importing mesh: " + std::string(e.what()));
        return nullptr;
    }
}

} // namespace rebel::simulation
