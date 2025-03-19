#pragma once

#include "Geometry.h"
#include <memory>
#include <vector>

namespace RebelCAD {
namespace Modeling {

/**
 * @brief Tool for creating offset surfaces from existing surfaces
 * 
 * The OffsetSurfaceTool creates a new surface that is offset from an input surface
 * by a specified distance. The offset can be positive (outward) or negative (inward).
 * The tool handles:
 * - Maintaining surface continuity
 * - Adjusting for self-intersections
 * - Preserving sharp features
 * - Handling multiple connected surfaces
 */
class OffsetSurfaceTool {
public:
    OffsetSurfaceTool();
    ~OffsetSurfaceTool();

    /**
     * @brief Sets the input surface to offset
     * @param surface Pointer to the input surface
     * @return true if surface is valid for offsetting
     */
    bool setInputSurface(std::shared_ptr<Surface> surface);

    /**
     * @brief Sets the offset distance
     * @param distance Positive for outward offset, negative for inward
     */
    void setOffsetDistance(double distance);

    /**
     * @brief Sets the tolerance for surface approximation
     * @param tolerance Maximum deviation allowed from true offset
     */
    void setTolerance(double tolerance);

    /**
     * @brief Enables or disables sharp feature preservation
     * @param preserve If true, sharp features will be maintained
     */
    void setPreserveSharpFeatures(bool preserve);

    /**
     * @brief Performs the offset operation
     * @return Pointer to the newly created offset surface
     * @throws std::runtime_error if operation fails
     */
    std::shared_ptr<Surface> execute();

    /**
     * @brief Validates if the offset operation is possible
     * @return true if operation can be performed
     */
    bool validate() const;

private:
    std::shared_ptr<Surface> m_inputSurface;
    double m_offsetDistance;
    double m_tolerance;
    bool m_preserveSharpFeatures;

    /**
     * @brief Checks for self-intersections in the offset surface
     * @return true if self-intersections are found
     */
    bool checkSelfIntersections() const;

    /**
     * @brief Adjusts the offset surface to handle self-intersections
     * @param surface Surface to adjust
     */
    void handleSelfIntersections(std::shared_ptr<Surface>& surface);

    /**
     * @brief Creates the initial offset surface
     * @return Raw offset surface before adjustments
     */
    std::shared_ptr<Surface> createInitialOffset();

    /**
     * @brief Preserves sharp features during offset
     * @param surface Surface to process
     */
    void preserveSharpFeatures(std::shared_ptr<Surface>& surface);
};

} // namespace Modeling
} // namespace RebelCAD
