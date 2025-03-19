#pragma once

#include <memory>
#include <vector>
#include "core/Error.h"
#include "sketching/Line.h"
#include "sketching/Arc.h"

namespace RebelCAD {
namespace Sketching {

/**
 * @class FilletTool
 * @brief Tool for creating fillets between two sketch entities
 * 
 * The FilletTool creates a smooth transition (arc) between two intersecting
 * lines or between a line and an arc. It automatically trims or extends the
 * input entities to meet the fillet arc tangentially.
 */
class FilletTool {
public:
    /**
     * @brief Default constructor
     */
    FilletTool() = default;

    /**
     * @brief Creates a fillet between two lines
     * @param line1 First line to fillet
     * @param line2 Second line to fillet
     * @param radius Desired fillet radius
     * @return Created fillet arc
     * @throws Error if lines are parallel or radius is invalid
     */
    std::shared_ptr<Arc> createLineLine(
        std::shared_ptr<Line> line1,
        std::shared_ptr<Line> line2,
        double radius);

    /**
     * @brief Creates a fillet between a line and an arc
     * @param line Line to fillet
     * @param arc Arc to fillet
     * @param radius Desired fillet radius
     * @return Created fillet arc
     * @throws Error if entities don't intersect or radius is invalid
     */
    std::shared_ptr<Arc> createLineArc(
        std::shared_ptr<Line> line,
        std::shared_ptr<Arc> arc,
        double radius);

    /**
     * @brief Creates a fillet between two arcs
     * @param arc1 First arc to fillet
     * @param arc2 Second arc to fillet
     * @param radius Desired fillet radius
     * @return Created fillet arc
     * @throws Error if arcs don't intersect or radius is invalid
     */
    std::shared_ptr<Arc> createArcArc(
        std::shared_ptr<Arc> arc1,
        std::shared_ptr<Arc> arc2,
        double radius);

private:
    /**
     * @brief Validates the fillet radius
     * @param radius Radius to validate
     * @throws Error if radius is invalid (zero or negative)
     */
    void validateRadius(double radius);

    /**
     * @brief Calculates intersection point between two lines
     * @param line1 First line
     * @param line2 Second line
     * @return Intersection point coordinates
     * @throws Error if lines are parallel
     */
    std::pair<double, double> calculateLineLineIntersection(
        const std::shared_ptr<Line>& line1,
        const std::shared_ptr<Line>& line2);
};

} // namespace Sketching
} // namespace RebelCAD
