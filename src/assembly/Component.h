#pragma once

#include <memory>
#include <string>
#include <vector>
#include "../../external/eigen/Eigen/Geometry"

namespace RebelCAD {
namespace Assembly {

/**
 * @brief Represents a component in an assembly
 * 
 * The Component class represents a single component in an assembly.
 * It can be a part or a sub-assembly, and it has a position and orientation in 3D space.
 */
class Component : public std::enable_shared_from_this<Component> {
public:
    /**
     * @brief Constructs a component with a name
     * @param name The name of the component
     */
    explicit Component(const std::string& name);

    /**
     * @brief Virtual destructor for proper cleanup
     */
    virtual ~Component() = default;

    /**
     * @brief Gets the name of the component
     * @return The component name
     */
    std::string getName() const;

    /**
     * @brief Sets the name of the component
     * @param name The new component name
     */
    void setName(const std::string& name);

    /**
     * @brief Gets the position of the component in world space
     * @return The position as a 3D vector
     */
    Eigen::Vector3d getPosition() const;

    /**
     * @brief Sets the position of the component in world space
     * @param position The new position as a 3D vector
     */
    void setPosition(const Eigen::Vector3d& position);

    /**
     * @brief Gets the orientation of the component in world space
     * @return The orientation as a quaternion
     */
    Eigen::Quaterniond getOrientation() const;

    /**
     * @brief Sets the orientation of the component in world space
     * @param orientation The new orientation as a quaternion
     */
    void setOrientation(const Eigen::Quaterniond& orientation);

    /**
     * @brief Gets the transformation matrix of the component in world space
     * @return The 4x4 transformation matrix
     */
    Eigen::Matrix4d getTransformationMatrix() const;

    /**
     * @brief Sets the transformation matrix of the component in world space
     * @param matrix The new 4x4 transformation matrix
     */
    void setTransformationMatrix(const Eigen::Matrix4d& matrix);

    /**
     * @brief Adds a child component to this component
     * @param child The child component to add
     */
    void addChild(std::shared_ptr<Component> child);

    /**
     * @brief Removes a child component from this component
     * @param child The child component to remove
     * @return true if the child was found and removed, false otherwise
     */
    bool removeChild(std::shared_ptr<Component> child);

    /**
     * @brief Gets all child components
     * @return Vector of child components
     */
    std::vector<std::shared_ptr<Component>> getChildren() const;

    /**
     * @brief Gets the parent component
     * @return Shared pointer to the parent component, or nullptr if this is a root component
     */
    std::shared_ptr<Component> getParent() const;

    /**
     * @brief Sets the parent component
     * @param parent The new parent component
     */
    void setParent(std::shared_ptr<Component> parent);

    /**
     * @brief Checks if this component is a root component (has no parent)
     * @return true if this is a root component, false otherwise
     */
    bool isRoot() const;

    /**
     * @brief Checks if this component is a leaf component (has no children)
     * @return true if this is a leaf component, false otherwise
     */
    bool isLeaf() const;

private:
    std::string m_name;
    Eigen::Vector3d m_position;
    Eigen::Quaterniond m_orientation;
    std::vector<std::shared_ptr<Component>> m_children;
    std::weak_ptr<Component> m_parent;
};

} // namespace Assembly
} // namespace RebelCAD
