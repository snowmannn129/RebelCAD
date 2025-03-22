#include "Component.h"

namespace RebelCAD {
namespace Assembly {

Component::Component(const std::string& name)
    : m_name(name)
    , m_position(Eigen::Vector3d::Zero())
    , m_orientation(Eigen::Quaterniond::Identity())
{
}

std::string Component::getName() const
{
    return m_name;
}

void Component::setName(const std::string& name)
{
    m_name = name;
}

Eigen::Vector3d Component::getPosition() const
{
    return m_position;
}

void Component::setPosition(const Eigen::Vector3d& position)
{
    m_position = position;
}

Eigen::Quaterniond Component::getOrientation() const
{
    return m_orientation;
}

void Component::setOrientation(const Eigen::Quaterniond& orientation)
{
    m_orientation = orientation;
}

Eigen::Matrix4d Component::getTransformationMatrix() const
{
    Eigen::Matrix4d matrix = Eigen::Matrix4d::Identity();
    
    // Set rotation part (3x3 top-left submatrix)
    matrix.block<3, 3>(0, 0) = m_orientation.toRotationMatrix();
    
    // Set translation part (3x1 top-right submatrix)
    matrix.block<3, 1>(0, 3) = m_position;
    
    return matrix;
}

void Component::setTransformationMatrix(const Eigen::Matrix4d& matrix)
{
    // Extract position (translation part)
    m_position = matrix.block<3, 1>(0, 3);
    
    // Extract orientation (rotation part)
    Eigen::Matrix3d rotationMatrix = matrix.block<3, 3>(0, 0);
    m_orientation = Eigen::Quaterniond(rotationMatrix);
    m_orientation.normalize(); // Ensure it's a valid rotation
}

void Component::addChild(std::shared_ptr<Component> child)
{
    if (child) {
        // Remove from previous parent if any
        if (auto parent = child->getParent()) {
            parent->removeChild(child);
        }
        
        // Add to this component's children
        m_children.push_back(child);
        
        // Set this as the child's parent
        child->setParent(shared_from_this());
    }
}

bool Component::removeChild(std::shared_ptr<Component> child)
{
    if (!child) {
        return false;
    }
    
    auto it = std::find(m_children.begin(), m_children.end(), child);
    if (it != m_children.end()) {
        // Remove parent reference
        (*it)->setParent(nullptr);
        
        // Remove from children
        m_children.erase(it);
        return true;
    }
    
    return false;
}

std::vector<std::shared_ptr<Component>> Component::getChildren() const
{
    return m_children;
}

std::shared_ptr<Component> Component::getParent() const
{
    return m_parent.lock();
}

void Component::setParent(std::shared_ptr<Component> parent)
{
    m_parent = parent;
}

bool Component::isRoot() const
{
    return m_parent.expired();
}

bool Component::isLeaf() const
{
    return m_children.empty();
}

} // namespace Assembly
} // namespace RebelCAD
