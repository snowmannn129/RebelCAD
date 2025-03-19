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
 * @class ExtendTool
 * @brief Provides functionality for extending sketch elements to their intersection points
 * 
 * The ExtendTool class enables extending sketch elements (lines, curves, polygon edges)
 * to meet other elements. It supports:
 * - Line-to-line extension
 * - Line-to-curve extension
 * - Curve-to-curve extension
 * - Polygon edge extension
 * 
 * The tool maintains the mathematical validity of the extended elements and
 * updates any associated constraints appropriately.
 */
class ExtendTool {
public:
    /**
     * @brief Structure representing a potential extension point
     */
    struct ExtensionPoint {
        float x;                    // X coordinate of extension point
        float y;                    // Y coordinate of extension point
        size_t targetElementIndex;  // Index of element to extend to
        float extensionDistance;    // Distance needed to extend
        bool isStart;              // True if extending from start point
    };

    /**
     * @brief Default constructor
     */
    ExtendTool() = default;

    /**
     * @brief Finds potential extension points for a line to other elements
     * @param line Line to extend
     * @param elements Vector of elements that could be extended to
     * @return Vector of possible extension points
     */
    template<typename ElementType>
    std::vector<ExtensionPoint> findExtensionPoints(
        const Line& line,
        const std::vector<std::shared_ptr<ElementType>>& elements) const;

    /**
     * @brief Extends a line to meet another element
     * @param line Line to extend
     * @param x X coordinate to extend to
     * @param y Y coordinate to extend to
     * @param fromStart If true, extends from start point instead of end point
     * @return Extended line
     * @throws Error if extension point is invalid
     */
    Line extendLine(const Line& line, float x, float y, bool fromStart) const;

    /**
     * @brief Extends a curve to meet another element
     * @param curve Curve to extend
     * @param x X coordinate to extend to
     * @param y Y coordinate to extend to
     * @param fromStart If true, extends from start point instead of end point
     * @return Extended curve
     * @throws Error if extension point is invalid
     */
    template<typename CurveType>
    CurveType extendCurve(const CurveType& curve, float x, float y, bool fromStart) const;

    /**
     * @brief Extends a polygon edge to meet another element
     * @param polygon Polygon to modify
     * @param edgeIndex Index of edge to extend
     * @param x X coordinate to extend to
     * @param y Y coordinate to extend to
     * @param fromStart If true, extends from start vertex instead of end vertex
     * @return Modified polygon with extended edge
     * @throws Error if extension point is invalid
     */
    Polygon extendPolygonEdge(const Polygon& polygon,
                             size_t edgeIndex,
                             float x, float y,
                             bool fromStart) const;

    /**
     * @brief Finds the nearest valid extension point for an element
     * @param x Source point X coordinate
     * @param y Source point Y coordinate
     * @param elements Vector of potential target elements
     * @return Nearest valid extension point or nullptr if none found
     */
    template<typename ElementType>
    std::unique_ptr<ExtensionPoint> findNearestExtensionPoint(
        float x, float y,
        const std::vector<std::shared_ptr<ElementType>>& elements) const;

private:
    // Tolerance for floating point comparisons
    static constexpr float EPSILON = 1e-6f;

    /**
     * @brief Validates if a point is a valid extension target
     * @param x Point x coordinate
     * @param y Point y coordinate
     * @throws Error if point is not valid for extension
     */
    void validateExtensionPoint(float x, float y) const;

    /**
     * @brief Calculates the extension distance to a target point
     * @param sourceX Source point X coordinate
     * @param sourceY Source point Y coordinate
     * @param targetX Target point X coordinate
     * @param targetY Target point Y coordinate
     * @return Distance to extend
     */
    float calculateExtensionDistance(
        float sourceX, float sourceY,
        float targetX, float targetY) const;

    /**
     * @brief Projects a point onto an infinite line
     * @param lineStart Start point of line
     * @param lineEnd End point of line
     * @param pointX X coordinate to project
     * @param pointY Y coordinate to project
     * @return Projected point coordinates
     */
    std::pair<float, float> projectPointOnLine(
        float lineStartX, float lineStartY,
        float lineEndX, float lineEndY,
        float pointX, float pointY) const;
};

} // namespace Sketching
} // namespace RebelCAD
