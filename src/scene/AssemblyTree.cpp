#include "assembly/AssemblyTree.h"
#include <algorithm>
#include <stdexcept>
#include <cmath>
#include <glm/gtx/transform.hpp>

namespace RebelCAD {
namespace Assembly {

AssemblyNode::AssemblyNode(std::string id, std::string name)
    : id_(std::move(id))
    , name_(std::move(name)) {}

bool AssemblyNode::addChild(Ptr child) {
    // Check if child already exists
    auto it = std::find_if(children_.begin(), children_.end(),
        [&](const Ptr& existing) { return existing->getId() == child->getId(); });
    
    if (it != children_.end()) {
        return false;
    }

    children_.push_back(child);
    child->parent_ = shared_from_this();
    return true;
}

bool AssemblyNode::removeChild(const std::string& childId) {
    auto it = std::find_if(children_.begin(), children_.end(),
        [&](const Ptr& child) { return child->getId() == childId; });
    
    if (it == children_.end()) {
        return false;
    }

    (*it)->parent_.reset();
    children_.erase(it);
    return true;
}

AssemblyTree::AssemblyTree() = default;

AssemblyNode::Ptr AssemblyTree::addComponent(
    const std::string& id,
    const std::string& name,
    const std::optional<std::string>& parentId) {
    
    // Check if ID already exists
    if (nodeMap_.find(id) != nodeMap_.end()) {
        return nullptr;
    }

    // Create new node
    auto node = std::make_shared<AssemblyNode>(id, name);
    nodeMap_[id] = node;

    // Handle parent relationship
    if (parentId) {
        auto parent = findComponent(*parentId);
        if (!parent || !updateNodeParent(node, parent)) {
            nodeMap_.erase(id);
            return nullptr;
        }
    } else {
        rootNodes_.push_back(node);
    }

    return node;
}

bool AssemblyTree::removeComponent(const std::string& id) {
    auto it = nodeMap_.find(id);
    if (it == nodeMap_.end()) {
        return false;
    }

    auto node = it->second;

    // Remove from parent if exists
    if (auto parent = node->getParent().lock()) {
        parent->removeChild(id);
    } else {
        // Remove from root nodes
        auto rootIt = std::find(rootNodes_.begin(), rootNodes_.end(), node);
        if (rootIt != rootNodes_.end()) {
            rootNodes_.erase(rootIt);
        }
    }

    // Move children to root level
    for (const auto& child : node->getChildren()) {
        updateNodeParent(child, std::nullopt);
    }

    // Remove from node map
    nodeMap_.erase(it);
    return true;
}

bool AssemblyTree::moveComponent(
    const std::string& id,
    const std::optional<std::string>& newParentId) {
    
    auto node = findComponent(id);
    if (!node) {
        return false;
    }

    // Get new parent node if specified
    std::optional<AssemblyNode::Ptr> newParent;
    if (newParentId) {
        newParent = findComponent(*newParentId);
        if (!newParent) {
            return false;
        }

        // Check for circular reference
        auto parent = *newParent;
        while (parent) {
            if (parent->getId() == id) {
                return false;
            }
            parent = parent->getParent().lock();
        }
    }

    return updateNodeParent(node, newParent);
}

AssemblyNode::Ptr AssemblyTree::findComponent(const std::string& id) {
    auto it = nodeMap_.find(id);
    return (it != nodeMap_.end()) ? it->second : nullptr;
}

void AssemblyTree::clear() {
    nodeMap_.clear();
    rootNodes_.clear();
    explodedViewEnabled_ = false;
    explosionFactor_ = 0.0f;
}

void AssemblyTree::setExplodedViewEnabled(bool enabled) {
    explodedViewEnabled_ = enabled;
    if (!enabled) {
        explosionFactor_ = 0.0f;
    }
}

void AssemblyTree::setExplosionFactor(float factor) {
    explosionFactor_ = std::clamp(factor, 0.0f, 1.0f);
}

bool AssemblyTree::setComponentExplodeOffset(const std::string& id, const glm::vec3& offset) {
    auto node = findComponent(id);
    if (!node) {
        return false;
    }
    node->explodeOffset_ = offset;
    return true;
}

std::optional<glm::vec3> AssemblyTree::getComponentExplodeOffset(const std::string& id) const {
    auto it = nodeMap_.find(id);
    if (it == nodeMap_.end()) {
        return std::nullopt;
    }
    return it->second->explodeOffset_;
}

std::optional<glm::vec3> AssemblyTree::getExplodedPosition(const std::string& id) const {
    if (!explodedViewEnabled_ || explosionFactor_ <= 0.0f) {
        return glm::vec3(0.0f);
    }

    auto node = nodeMap_.find(id);
    if (node == nodeMap_.end()) {
        return std::nullopt;
    }

    // Calculate cumulative explosion offset from parent chain
    glm::vec3 totalOffset(0.0f);
    auto current = node->second;
    
    while (current) {
        // Apply this node's explosion offset
        totalOffset += current->explodeOffset_ * explosionFactor_ * current->explodeFactor_;
        
        // Move up to parent
        current = current->getParent().lock();
    }

    return totalOffset;
}

void AssemblyTree::resetExplodedView() {
    // Reset global state
    explodedViewEnabled_ = false;
    explosionFactor_ = 0.0f;

    // Reset all component offsets
    for (auto& [id, node] : nodeMap_) {
        node->explodeOffset_ = glm::vec3(0.0f);
        node->explodeFactor_ = 1.0f;
    }
}

bool AssemblyTree::updateNodeParent(
    AssemblyNode::Ptr node,
    const std::optional<AssemblyNode::Ptr>& newParent) {
    
    // Remove from current parent
    if (auto currentParent = node->getParent().lock()) {
        currentParent->removeChild(node->getId());
    } else {
        auto it = std::find(rootNodes_.begin(), rootNodes_.end(), node);
        if (it != rootNodes_.end()) {
            rootNodes_.erase(it);
        }
    }

    // Update parent relationship
    if (newParent) {
        if (!(*newParent)->addChild(node)) {
            rootNodes_.push_back(node);
            return false;
        }
    } else {
        rootNodes_.push_back(node);
    }

    return true;
}

} // namespace Assembly
} // namespace RebelCAD
