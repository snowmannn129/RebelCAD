#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include "simulation/FEASystem.h"

namespace rebel::simulation {

// Forward declarations
class Material;

/**
 * @struct Node
 * @brief Represents a node in the finite element mesh.
 */
struct Node {
    int id;                 ///< Node ID
    double x, y, z;         ///< Node coordinates
    std::vector<int> dofs;  ///< Degrees of freedom indices

    /**
     * @brief Constructs a new Node object.
     * 
     * @param id Node ID
     * @param x X coordinate
     * @param y Y coordinate
     * @param z Z coordinate
     */
    Node(int id, double x, double y, double z)
        : id(id), x(x), y(y), z(z) {}
};

/**
 * @struct Element
 * @brief Represents an element in the finite element mesh.
 */
struct Element {
    int id;                         ///< Element ID
    ElementType type;               ///< Element type
    std::vector<int> nodeIds;       ///< Node IDs
    std::shared_ptr<Material> material; ///< Material
    
    /**
     * @brief Constructs a new Element object.
     * 
     * @param id Element ID
     * @param type Element type
     * @param nodeIds Node IDs
     * @param material Material
     */
    Element(int id, ElementType type, const std::vector<int>& nodeIds, std::shared_ptr<Material> material)
        : id(id), type(type), nodeIds(nodeIds), material(material) {}
};

/**
 * @struct ElementGroup
 * @brief Represents a group of elements with the same properties.
 */
struct ElementGroup {
    int id;                         ///< Group ID
    std::string name;               ///< Group name
    std::vector<int> elementIds;    ///< Element IDs
    std::shared_ptr<Material> material; ///< Material
    
    /**
     * @brief Constructs a new ElementGroup object.
     * 
     * @param id Group ID
     * @param name Group name
     * @param material Material
     */
    ElementGroup(int id, const std::string& name, std::shared_ptr<Material> material)
        : id(id), name(name), material(material) {}
};

/**
 * @struct NodeGroup
 * @brief Represents a group of nodes with the same properties.
 */
struct NodeGroup {
    int id;                      ///< Group ID
    std::string name;            ///< Group name
    std::vector<int> nodeIds;    ///< Node IDs
    
    /**
     * @brief Constructs a new NodeGroup object.
     * 
     * @param id Group ID
     * @param name Group name
     */
    NodeGroup(int id, const std::string& name)
        : id(id), name(name) {}
};

/**
 * @enum MeshQualityMetric
 * @brief Metrics for evaluating mesh quality.
 */
enum class MeshQualityMetric {
    AspectRatio,        ///< Aspect ratio of elements
    Skewness,           ///< Skewness of elements
    Jacobian,           ///< Jacobian determinant
    OrthogonalQuality,  ///< Orthogonal quality
    WarpingFactor       ///< Warping factor
};

/**
 * @struct MeshQuality
 * @brief Represents the quality of a mesh.
 */
struct MeshQuality {
    double minAspectRatio;       ///< Minimum aspect ratio
    double maxAspectRatio;       ///< Maximum aspect ratio
    double avgAspectRatio;       ///< Average aspect ratio
    double minSkewness;          ///< Minimum skewness
    double maxSkewness;          ///< Maximum skewness
    double avgSkewness;          ///< Average skewness
    double minJacobian;          ///< Minimum Jacobian determinant
    double minOrthogonalQuality; ///< Minimum orthogonal quality
    double maxWarpingFactor;     ///< Maximum warping factor
    
    /**
     * @brief Constructs a new MeshQuality object with default values.
     */
    MeshQuality()
        : minAspectRatio(0.0), maxAspectRatio(0.0), avgAspectRatio(0.0),
          minSkewness(0.0), maxSkewness(0.0), avgSkewness(0.0),
          minJacobian(0.0), minOrthogonalQuality(0.0), maxWarpingFactor(0.0) {}
};

/**
 * @class Mesh
 * @brief Class representing a finite element mesh.
 * 
 * The Mesh class defines the geometry of the finite element model,
 * including nodes, elements, and their connectivity.
 */
class Mesh {
public:
    /**
     * @brief Creates a new mesh from a geometry model.
     * 
     * @param geometryId The ID of the geometry to mesh.
     * @param elementType The type of elements to use.
     * @param refinementLevel The mesh refinement level (1-5).
     * @return Shared pointer to the created mesh, or nullptr if failed.
     */
    static std::shared_ptr<Mesh> createFromGeometry(
        const std::string& geometryId,
        ElementType elementType = ElementType::Tetra,
        int refinementLevel = 1);

    /**
     * @brief Creates a new mesh from an STL file.
     * 
     * @param filePath The path to the STL file.
     * @param elementType The type of elements to use.
     * @param refinementLevel The mesh refinement level (1-5).
     * @return Shared pointer to the created mesh, or nullptr if failed.
     */
    static std::shared_ptr<Mesh> createFromSTL(
        const std::string& filePath,
        ElementType elementType = ElementType::Tetra,
        int refinementLevel = 1);

    /**
     * @brief Creates a new mesh from a STEP file.
     * 
     * @param filePath The path to the STEP file.
     * @param elementType The type of elements to use.
     * @param refinementLevel The mesh refinement level (1-5).
     * @return Shared pointer to the created mesh, or nullptr if failed.
     */
    static std::shared_ptr<Mesh> createFromSTEP(
        const std::string& filePath,
        ElementType elementType = ElementType::Tetra,
        int refinementLevel = 1);

    /**
     * @brief Gets the ID of the mesh.
     * @return The ID of the mesh.
     */
    const std::string& getId() const { return id; }

    /**
     * @brief Gets the number of nodes in the mesh.
     * @return The number of nodes.
     */
    size_t getNodeCount() const { return nodes.size(); }

    /**
     * @brief Gets the number of elements in the mesh.
     * @return The number of elements.
     */
    size_t getElementCount() const { return elements.size(); }

    /**
     * @brief Gets a node by ID.
     * 
     * @param nodeId The ID of the node to get.
     * @return Pointer to the node, or nullptr if not found.
     */
    const Node* getNode(int nodeId) const;

    /**
     * @brief Gets an element by ID.
     * 
     * @param elementId The ID of the element to get.
     * @return Pointer to the element, or nullptr if not found.
     */
    const Element* getElement(int elementId) const;

    /**
     * @brief Gets all nodes in the mesh.
     * @return Vector of nodes.
     */
    const std::vector<Node>& getNodes() const { return nodes; }

    /**
     * @brief Gets all elements in the mesh.
     * @return Vector of elements.
     */
    const std::vector<Element>& getElements() const { return elements; }

    /**
     * @brief Creates a new element group.
     * 
     * @param name The name of the group.
     * @param material The material for the group.
     * @return The ID of the created group, or -1 if failed.
     */
    int createElementGroup(const std::string& name, std::shared_ptr<Material> material);

    /**
     * @brief Creates a new node group.
     * 
     * @param name The name of the group.
     * @return The ID of the created group, or -1 if failed.
     */
    int createNodeGroup(const std::string& name);

    /**
     * @brief Adds an element to a group.
     * 
     * @param groupId The ID of the group.
     * @param elementId The ID of the element to add.
     * @return True if addition was successful, false otherwise.
     */
    bool addElementToGroup(int groupId, int elementId);

    /**
     * @brief Adds a node to a group.
     * 
     * @param groupId The ID of the group.
     * @param nodeId The ID of the node to add.
     * @return True if addition was successful, false otherwise.
     */
    bool addNodeToGroup(int groupId, int nodeId);

    /**
     * @brief Gets an element group by ID.
     * 
     * @param groupId The ID of the group to get.
     * @return Pointer to the group, or nullptr if not found.
     */
    const ElementGroup* getElementGroup(int groupId) const;

    /**
     * @brief Gets a node group by ID.
     * 
     * @param groupId The ID of the group to get.
     * @return Pointer to the group, or nullptr if not found.
     */
    const NodeGroup* getNodeGroup(int groupId) const;

    /**
     * @brief Gets an element group by name.
     * 
     * @param name The name of the group to get.
     * @return Pointer to the group, or nullptr if not found.
     */
    const ElementGroup* getElementGroupByName(const std::string& name) const;

    /**
     * @brief Gets a node group by name.
     * 
     * @param name The name of the group to get.
     * @return Pointer to the group, or nullptr if not found.
     */
    const NodeGroup* getNodeGroupByName(const std::string& name) const;

    /**
     * @brief Gets all element groups in the mesh.
     * @return Vector of element groups.
     */
    const std::vector<ElementGroup>& getElementGroups() const { return elementGroups; }

    /**
     * @brief Gets all node groups in the mesh.
     * @return Vector of node groups.
     */
    const std::vector<NodeGroup>& getNodeGroups() const { return nodeGroups; }

    /**
     * @brief Sets the material for an element.
     * 
     * @param elementId The ID of the element.
     * @param material The material to set.
     * @return True if setting was successful, false otherwise.
     */
    bool setElementMaterial(int elementId, std::shared_ptr<Material> material);

    /**
     * @brief Sets the material for an element group.
     * 
     * @param groupId The ID of the group.
     * @param material The material to set.
     * @return True if setting was successful, false otherwise.
     */
    bool setElementGroupMaterial(int groupId, std::shared_ptr<Material> material);

    /**
     * @brief Refines the mesh.
     * 
     * @param refinementLevel The refinement level (1-5).
     * @return True if refinement was successful, false otherwise.
     */
    bool refine(int refinementLevel);

    /**
     * @brief Evaluates the quality of the mesh.
     * 
     * @param metrics The metrics to evaluate.
     * @return The mesh quality.
     */
    MeshQuality evaluateQuality(const std::vector<MeshQualityMetric>& metrics = {
        MeshQualityMetric::AspectRatio,
        MeshQualityMetric::Skewness,
        MeshQualityMetric::Jacobian
    }) const;

    /**
     * @brief Exports the mesh to a file.
     * 
     * @param filePath The path to export to.
     * @return True if export was successful, false otherwise.
     */
    bool exportToFile(const std::string& filePath) const;

    /**
     * @brief Imports a mesh from a file.
     * 
     * @param filePath The path to import from.
     * @return Shared pointer to the imported mesh, or nullptr if failed.
     */
    static std::shared_ptr<Mesh> importFromFile(const std::string& filePath);

private:
    /**
     * @brief Constructs a new Mesh object.
     * 
     * @param id The ID of the mesh.
     */
    Mesh(const std::string& id);

    std::string id;                                  ///< Mesh ID
    std::vector<Node> nodes;                         ///< Nodes
    std::vector<Element> elements;                   ///< Elements
    std::vector<ElementGroup> elementGroups;         ///< Element groups
    std::vector<NodeGroup> nodeGroups;               ///< Node groups
    std::unordered_map<int, size_t> nodeIdToIndex;   ///< Map from node ID to index
    std::unordered_map<int, size_t> elementIdToIndex; ///< Map from element ID to index
    std::unordered_map<int, size_t> elementGroupIdToIndex; ///< Map from element group ID to index
    std::unordered_map<int, size_t> nodeGroupIdToIndex; ///< Map from node group ID to index
    std::unordered_map<std::string, size_t> elementGroupNameToIndex; ///< Map from element group name to index
    std::unordered_map<std::string, size_t> nodeGroupNameToIndex; ///< Map from node group name to index
};

} // namespace rebel::simulation
