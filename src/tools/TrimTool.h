#pragma once

#include <memory>
#include <vector>
#include "sketching/Line.h"
#include "sketching/Polygon.h"
#include "sketching/BezierCurve.h"
#include "sketching/Spline.h"
#include "core/Error.h"

namespace RebelCAD {
namespace Sketching {

/**
 * @class TrimTool
 * @brief Provides functionality for trimming sketch elements at intersection points
 * 
 * The TrimTool class enables trimming of sketch elements (lines, curves, polygon edges)
 * at their intersection points. It supports:
 * - Line-to-line trimming
 * - Line-to-curve trimming
 * - Curve-to-curve trimming
 * - Polygon edge trimming
 * 
 * The tool maintains the mathematical validity of the trimmed elements and
 * updates any associated constraints appropriately.
 */
class TrimTool {
public:
    /**
     * @brief Structure representing an intersection point between sketch elements
     */
    struct IntersectionPoint {
        float x;                    // X coordinate of intersection
        float y;                    // Y coordinate of intersection
        size_t elementIndex1;       // Index of first intersecting element
        size_t elementIndex2;       // Index of second intersecting element
        float parameter1;           // Parameter value on first element (0-1)
        float parameter2;           // Parameter value on second element (0-1)
    };

    /**
     * @brief Default constructor
     */
    TrimTool() = default;

    /**
     * @brief Finds all intersection points between two lines
     * @param line1 First line
     * @param line2 Second line
     * @return Vector of intersection points
     */
    std::vector<IntersectionPoint> findLineIntersections(
        const Line& line1, const Line& line2) const;

    /**
     * @brief Finds intersection points between a line and a curve
     * @param line The line
     * @param curve The curve (BezierCurve or Spline)
     * @return Vector of intersection points
     */
    template<typename CurveType>
    std::vector<IntersectionPoint> findLineCurveIntersections(
        const Line& line, const CurveType& curve) const;

    /**
     * @brief Trims a line at the specified point
     * @param line Line to trim
     * @param x X coordinate of trim point
     * @param y Y coordinate of trim point
     * @param keepStart If true, keeps the portion from start to trim point
     * @return New trimmed line
     * @throws Error if trim point is not on the line
     */
    Line trimLine(const Line& line, float x, float y, bool keepStart) const;

    /**
     * @brief Trims a curve at the specified point
     * @param curve Curve to trim
     * @param x X coordinate of trim point
     * @param y Y coordinate of trim point
     * @param keepStart If true, keeps the portion from start to trim point
     * @return New trimmed curve
     * @throws Error if trim point is not on the curve
     */
    template<typename CurveType>
    CurveType trimCurve(const CurveType& curve, float x, float y, bool keepStart) const;

    /**
     * @brief Trims a polygon edge at the specified point
     * @param polygon Polygon to modify
     * @param edgeIndex Index of edge to trim
     * @param x X coordinate of trim point
     * @param y Y coordinate of trim point
     * @param keepStart If true, keeps the portion from start to trim point
     * @return Modified polygon with trimmed edge
     * @throws Error if trim point is not on the specified edge
     */
    Polygon trimPolygonEdge(const Polygon& polygon, 
                           size_t edgeIndex,
                           float x, float y, 
                           bool keepStart) const;

private:
    // Tolerance for floating point comparisons
    static constexpr float EPSILON = 1e-6f;

    /**
     * @brief Checks if a point lies on a line segment
     * @param line The line segment
     * @param x Point x coordinate
     * @param y Point y coordinate
     * @return true if point lies on line segment
     */
    bool isPointOnLine(const Line& line, float x, float y) const;

    /**
     * @brief Calculates parameter value for point on line (0-1)
     * @param line The line
     * @param x Point x coordinate
     * @param y Point y coordinate
     * @return Parameter value between 0 and 1
     */
    float getLineParameter(const Line& line, float x, float y) const;

    /**
     * @brief Validates trim point is on the specified element
     * @param x Point x coordinate
     * @param y Point y coordinate
     * @throws Error if point is not valid for trimming
     */
    void validateTrimPoint(float x, float y) const;
};

} // namespace Sketching
} // namespace RebelCAD
