#pragma once

#include <memory>
#include <vector>
#include "core/Error.h"
#include "sketching/Line.h"
#include "sketching/Arc.h"
#include "sketching/Circle.h"
#include "sketching/BezierCurve.h"
#include "sketching/Spline.h"

namespace RebelCAD {
namespace Sketching {

/**
 * @class MirrorTool
 * @brief Tool for creating mirrored copies of sketch entities
 * 
 * The MirrorTool creates symmetrical copies of sketch entities about a specified
 * mirror line. It supports mirroring of basic entities (lines, arcs, circles) and
 * complex curves (bezier curves, splines). The mirror line can be defined by two
 * points or by an existing line entity.
 */
class MirrorTool {
public:
    /**
     * @brief Default constructor
     */
    MirrorTool() = default;

    /**
     * @brief Creates a mirror line from two points
     * @param x1 X-coordinate of first point
     * @param y1 Y-coordinate of first point
     * @param x2 X-coordinate of second point
     * @param y2 Y-coordinate of second point
     * @throws Error if points are coincident
     */
    void setMirrorLine(double x1, double y1, double x2, double y2);

    /**
     * @brief Sets an existing line as the mirror line
     * @param line Line to use as mirror line
     * @throws Error if line is null
     */
    void setMirrorLine(std::shared_ptr<Line> line);

    /**
     * @brief Mirrors a line about the mirror line
     * @param line Line to mirror
     * @return Mirrored line
     * @throws Error if mirror line not set or input is null
     */
    std::shared_ptr<Line> mirrorLine(const std::shared_ptr<Line>& line);

    /**
     * @brief Mirrors an arc about the mirror line
     * @param arc Arc to mirror
     * @return Mirrored arc
     * @throws Error if mirror line not set or input is null
     */
    std::shared_ptr<Arc> mirrorArc(const std::shared_ptr<Arc>& arc);

    /**
     * @brief Mirrors a circle about the mirror line
     * @param circle Circle to mirror
     * @return Mirrored circle
     * @throws Error if mirror line not set or input is null
     */
    std::shared_ptr<Circle> mirrorCircle(const std::shared_ptr<Circle>& circle);

    /**
     * @brief Mirrors a bezier curve about the mirror line
     * @param curve Bezier curve to mirror
     * @return Mirrored bezier curve
     * @throws Error if mirror line not set or input is null
     */
    std::shared_ptr<BezierCurve> mirrorBezierCurve(const std::shared_ptr<BezierCurve>& curve);

    /**
     * @brief Mirrors a spline about the mirror line
     * @param spline Spline to mirror
     * @return Mirrored spline
     * @throws Error if mirror line not set or input is null
     */
    std::shared_ptr<Spline> mirrorSpline(const std::shared_ptr<Spline>& spline);

    /**
     * @brief Mirrors a point about the mirror line
     * @param x X-coordinate of point to mirror
     * @param y Y-coordinate of point to mirror
     * @return Pair of x,y coordinates of mirrored point
     * @throws Error if mirror line not set
     */
    std::pair<double, double> mirrorPoint(double x, double y) const;

private:
    /**
     * @brief Validates that the mirror line has been set
     * @throws Error if mirror line not set
     */
    void validateMirrorLine() const;

    /**
     * @brief Calculates the perpendicular distance from a point to the mirror line
     * @param x X-coordinate of point
     * @param y Y-coordinate of point
     * @return Perpendicular distance
     */
    double calculateDistance(double x, double y) const;

    std::shared_ptr<Line> mirrorLine_; ///< The line about which entities are mirrored
    bool isSet_ = false; ///< Flag indicating if mirror line has been set
};

} // namespace Sketching
} // namespace RebelCAD
