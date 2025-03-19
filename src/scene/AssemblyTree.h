#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <glm/glm.hpp>

namespace RebelCAD {
namespace Assembly {

/**
 * @brief Represents a node in the assembly hierarchy
 * 
 * AssemblyNode stores component data and maintains parent-child relationships
 * in the assembly tree structure. Each node can have one parent and multiple
 * children, forming a hierarchical representation of the assembly.
 */
class AssemblyNode : public std::enable_shared_from_this<AssemblyNode> {
public:
    using Ptr = std::shared_ptr<AssemblyNode>;
    using WeakPtr = std::weak_ptr<AssemblyNode>;

    /**
     * @brief Create a new assembly node
     * @param id Unique identifier for the component
     * @param name Display name for the component
     */
    AssemblyNode(std::string id, std::string name);

    /**
     * @brief Add a child node to this node
     * @param child Node to add as a child
     * @return true if successful, false if child already exists
     */
    bool addChild(Ptr child);

    /**
     * @brief Remove a child node from this node
     * @param childId ID of child node to remove
     * @return true if successful, false if child not found
     */
    bool removeChild(const std::string& childId);

    /**
     * @brief Get the parent node
     * @return Weak pointer to parent node, or empty if root
     */
    WeakPtr getParent() const { return parent_; }

    /**
     * @brief Get all child nodes
     * @return Vector of child node pointers
     */
    const std::vector<Ptr>& getChildren() const { return children_; }

    /**
     * @brief Get node's unique ID
     * @return Node ID string
     */
    const std::string& getId() const { return id_; }

    /**
     * @brief Get node's display name
     * @return Node name string
     */
    const std::string& getName() const { return name_; }

    /**
     * @brief Set node's display name
     * @param name New name for the node
     */
    void setName(const std::string& name) { name_ = name; }

    /**
     * @brief Check if node is visible
     * @return true if node is visible
     */
    bool isVisible() const { return isVisible_; }

    /**
     * @brief Set node visibility
     * @param visible New visibility state
     */
    void setVisible(bool visible) { isVisible_ = visible; }

private:
    std::string id_;              ///< Unique identifier
    std::string name_;            ///< Display name
    bool isVisible_ = true;       ///< Visibility state
    WeakPtr parent_;              ///< Parent node
    std::vector<Ptr> children_;   ///< Child nodes
    glm::vec3 explodeOffset_{0}; ///< Offset vector for exploded view
    float explodeFactor_ = 1.0f; ///< Multiplier for explosion effect

    friend class AssemblyTree;
};

/**
 * @brief Manages the hierarchical structure of an assembly
 * 
 * AssemblyTree maintains the overall assembly hierarchy, providing methods
 * to manipulate the structure and traverse the tree. It ensures proper
 * parent-child relationships and handles operations like adding/removing
 * components and restructuring the hierarchy.
 */
class AssemblyTree {
public:
    /**
     * @brief Create a new assembly tree
     */
    AssemblyTree();

    /**
     * @brief Add a new component to the assembly
     * @param id Unique identifier for the component
     * @param name Display name for the component
     * @param parentId Optional parent node ID
     * @return Pointer to created node, or nullptr if failed
     */
    AssemblyNode::Ptr addComponent(
        const std::string& id,
        const std::string& name,
        const std::optional<std::string>& parentId = std::nullopt);

    /**
     * @brief Remove a component from the assembly
     * @param id ID of component to remove
     * @return true if successful, false if component not found
     */
    bool removeComponent(const std::string& id);

    /**
     * @brief Move a component to a new parent
     * @param id ID of component to move
     * @param newParentId ID of new parent, or empty for root
     * @return true if successful, false if invalid operation
     */
    bool moveComponent(
        const std::string& id,
        const std::optional<std::string>& newParentId);

    /**
     * @brief Find a component by ID
     * @param id ID of component to find
     * @return Pointer to found node, or nullptr if not found
     */
    AssemblyNode::Ptr findComponent(const std::string& id);

    /**
     * @brief Get all root level components
     * @return Vector of root component pointers
     */
    const std::vector<AssemblyNode::Ptr>& getRootComponents() const {
        return rootNodes_;
    }

    /**
     * @brief Clear the entire assembly tree
     */
    void clear();

    /**
     * @brief Enable or disable exploded view mode
     * @param enabled True to enable exploded view
     */
    void setExplodedViewEnabled(bool enabled);

    /**
     * @brief Check if exploded view is enabled
     * @return True if exploded view is active
     */
    bool isExplodedViewEnabled() const { return explodedViewEnabled_; }

    /**
     * @brief Set the global explosion factor
     * @param factor Multiplier for explosion effect (0.0 - 1.0)
     */
    void setExplosionFactor(float factor);

    /**
     * @brief Get the current explosion factor
     * @return Current explosion factor
     */
    float getExplosionFactor() const { return explosionFactor_; }

    /**
     * @brief Set explosion offset for a component
     * @param id Component ID
     * @param offset Direction and magnitude of explosion
     * @return True if component found and offset set
     */
    bool setComponentExplodeOffset(const std::string& id, const glm::vec3& offset);

    /**
     * @brief Get explosion offset for a component
     * @param id Component ID
     * @return Optional containing offset vector, empty if not found
     */
    std::optional<glm::vec3> getComponentExplodeOffset(const std::string& id) const;

    /**
     * @brief Calculate final world position for a component in exploded view
     * @param id Component ID
     * @return Optional containing world position, empty if not found
     */
    std::optional<glm::vec3> getExplodedPosition(const std::string& id) const;

    /**
     * @brief Reset all explosion offsets to zero
     */
    void resetExplodedView();

    /**
     * @brief Check if the assembly tree is empty
     * @return true if tree contains no components
     */
    bool isEmpty() const { return nodeMap_.empty(); }

    /**
     * @brief Get total number of components in tree
     * @return Component count
     */
    size_t size() const { return nodeMap_.size(); }

private:
    std::vector<AssemblyNode::Ptr> rootNodes_;  ///< Top-level components
    std::unordered_map<std::string, AssemblyNode::Ptr> nodeMap_;  ///< All components
    bool explodedViewEnabled_ = false;  ///< Exploded view state
    float explosionFactor_ = 0.0f;      ///< Global explosion factor

    /**
     * @brief Internal helper to update parent-child relationships
     * @param node Node to update
     * @param newParent New parent node
     * @return true if successful
     */
    bool updateNodeParent(
        AssemblyNode::Ptr node,
        const std::optional<AssemblyNode::Ptr>& newParent);
};

} // namespace Assembly
} // namespace RebelCAD
