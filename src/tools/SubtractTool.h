#pragma once

#include <memory>
#include <vector>
#include "core/Error.h"
#include "graphics/GraphicsSystem.h"
#include "modeling/Geometry.h"

namespace RebelCAD {
namespace Modeling {

/**
 * @brief Tool for performing Boolean subtraction operations between solid bodies
 * 
 * The SubtractTool removes volumes of one or more solid bodies (tools) from a main body.
 * The resulting body represents the volume of the main body minus the intersecting
 * volumes of the tool bodies. Internal faces are handled appropriately to create
 * new surfaces where the subtraction occurs.
 */
class SubtractTool {
public:
    SubtractTool();
    ~SubtractTool();

    /**
     * @brief Sets the main body from which other bodies will be subtracted
     * @param body The target solid body
     * @throws Error if the body is null or invalid
     */
    void SetMainBody(const std::shared_ptr<SolidBody>& body);

    /**
     * @brief Sets the tool bodies to be subtracted from the main body
     * @param bodies Vector of solid bodies to subtract
     * @throws Error if any body is invalid or no bodies are provided
     */
    void SetToolBodies(const std::vector<std::shared_ptr<SolidBody>>& bodies);

    /**
     * @brief Performs the subtraction operation
     * @return Pointer to the resulting solid body after subtraction
     * @throws Error if the operation fails or bodies haven't been set
     */
    std::shared_ptr<SolidBody> Execute();

    /**
     * @brief Previews the result of the subtraction operation
     * @param graphics Graphics system for rendering the preview
     */
    void Preview(std::shared_ptr<Graphics::GraphicsSystem> graphics);

private:
    /**
     * @brief Validates that all bodies are suitable for subtraction
     * @throws Error if validation fails
     */
    void ValidateBodies() const;

    /**
     * @brief Performs intersection detection between main body and tool bodies
     * @return true if any intersections exist, false if all bodies are disjoint
     */
    bool CheckIntersections() const;

    /**
     * @brief Processes the topology to create the subtracted result
     * @return Modified topology representing the subtracted shape
     */
    Topology ProcessSubtraction() const;

    /**
     * @brief Handles creation of new faces where subtraction occurs
     * @param toolBody The tool body being subtracted
     * @return Collection of new boundary faces
     */
    std::vector<Face> CreateBoundaryFaces(const std::shared_ptr<SolidBody>& toolBody) const;

    std::shared_ptr<SolidBody> m_mainBody;
    std::vector<std::shared_ptr<SolidBody>> m_toolBodies;
    bool m_isValid;
};

} // namespace Modeling
} // namespace RebelCAD
