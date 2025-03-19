#pragma once

#include "modeling/Geometry.h"
#include <memory>
#include <vector>

namespace RebelCAD {
namespace Modeling {

/**
 * @brief Tool for performing boolean intersection operations between solid bodies
 * 
 * The IntersectTool creates a new solid body that represents the volume shared
 * between two or more input bodies. The resulting body contains only the space
 * that exists in ALL input bodies.
 */
class IntersectTool {
public:
    IntersectTool();
    ~IntersectTool();

    /**
     * @brief Sets the input bodies to be intersected
     * @param bodies Vector of solid bodies to intersect
     * @throws std::invalid_argument if fewer than 2 bodies are provided
     */
    void SetBodies(const std::vector<std::shared_ptr<SolidBody>>& bodies);

    /**
     * @brief Performs the intersection operation
     * @return Shared pointer to the resulting intersected geometry
     * @throws std::runtime_error if the operation fails or no bodies were set
     */
    std::shared_ptr<SolidBody> Execute();

    /**
     * @brief Validates that the input bodies are suitable for intersection
     * @return true if the bodies can be intersected, false otherwise
     */
    bool Validate() const;

private:
    std::vector<std::shared_ptr<SolidBody>> m_Bodies;
    
    /**
     * @brief Performs intersection between two bodies
     * @param body1 First input body
     * @param body2 Second input body
     * @return Result of intersecting the two bodies
     */
    std::shared_ptr<SolidBody> IntersectPair(
        const std::shared_ptr<SolidBody>& body1,
        const std::shared_ptr<SolidBody>& body2);
};

} // namespace Modeling
} // namespace RebelCAD
