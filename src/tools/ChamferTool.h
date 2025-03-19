#pragma once

#include <memory>
#include <vector>
#include "core/Error.h"
#include "sketching/Line.h"
#include "sketching/Arc.h"

namespace RebelCAD {
namespace Sketching {

/**
 * @class ChamferTool
 * @brief Tool for creating chamfers between two sketch entities
 * 
 * The ChamferTool creates an angled transition (line) between two intersecting
 * lines or between a line and an arc. It automatically trims or extends the
 * input entities to meet the chamfer line at the specified distances.
 */
class ChamferTool {
public:
    /**
     * @brief Default constructor
     */
    ChamferTool() = default;

    /**
     * @brief Creates a chamfer between two lines
     * @param line1 First line to chamfer
     * @param line2 Second line to chamfer
     * @param distance1 Distance from intersection along first line
     * @param distance2 Distance from intersection along second line
     * @return Created chamfer line
     * @throws Error if lines are parallel or distances are invalid
     */
    std::shared_ptr<Line> createLineLine(
        std::shared_ptr<Line> line1,
        std::shared_ptr<Line> line2,
        double distance1,
        double distance2);

    /**
     * @brief Creates a chamfer between a line and an arc
     * @param line Line to chamfer
     * @param arc Arc to chamfer
     * @param lineDistance Distance from intersection along line
     * @param arcDistance Distance from intersection along arc
     * @return Created chamfer line
     * @throws Error if entities don't intersect or distances are invalid
     */
    std::shared_ptr<Line> createLineArc(
        std::shared_ptr<Line> line,
        std::shared_ptr<Arc> arc,
        double lineDistance,
        double arcDistance);

    /**
     * @brief Creates a chamfer between two arcs
     * @param arc1 First arc to chamfer
     * @param arc2 Second arc to chamfer
     * @param distance1 Distance from intersection along first arc
     * @param distance2 Distance from intersection along second arc
     * @return Created chamfer line
     * @throws Error if arcs don't intersect or distances are invalid
     */
    std::shared_ptr<Line> createArcArc(
        std::shared_ptr<Arc> arc1,
        std::shared_ptr<Arc> arc2,
        double distance1,
        double distance2);

private:
    /**
     * @brief Validates the chamfer distances
     * @param distance1 First distance to validate
     * @param distance2 Second distance to validate
     * @throws Error if distances are invalid (zero or negative)
     */
    void validateDistances(double distance1, double distance2);

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
