#pragma once

#include <memory>
#include <vector>
#include "core/Error.h"
#include "graphics/GraphicsSystem.h"
#include "modeling/Geometry.h"

namespace RebelCAD {
namespace Modeling {

/**
 * @brief Tool for performing Boolean union operations between solid bodies
 * 
 * The UnionTool combines two or more solid bodies into a single body by merging
 * their volumes. Any internal faces where the bodies intersect are removed, and
 * the resulting outer shell forms the new unified solid.
 */
class UnionTool {
public:
    UnionTool();
    ~UnionTool();

    /**
     * @brief Sets the target bodies to be united
     * @param bodies Vector of solid bodies to combine
     * @throws Error if fewer than 2 bodies are provided
     */
    void SetBodies(const std::vector<std::shared_ptr<SolidBody>>& bodies);

    /**
     * @brief Performs the union operation
     * @return Pointer to the resulting unified solid body
     * @throws Error if the operation fails or bodies haven't been set
     */
    std::shared_ptr<SolidBody> Execute();

    /**
     * @brief Previews the result of the union operation
     * @param graphics Graphics system for rendering the preview
     */
    void Preview(std::shared_ptr<Graphics::GraphicsSystem> graphics);

private:
    /**
     * @brief Validates that the bodies are suitable for union
     * @throws Error if validation fails
     */
    void ValidateBodies() const;

    /**
     * @brief Performs intersection detection between bodies
     * @return true if bodies intersect, false if disjoint
     */
    bool CheckIntersection() const;

    /**
     * @brief Merges the topology of intersecting bodies
     * @return Combined topology representation
     */
    Topology MergeTopology() const;

    std::vector<std::shared_ptr<SolidBody>> m_bodies;
    bool m_isValid;
};

} // namespace Modeling
} // namespace RebelCAD
