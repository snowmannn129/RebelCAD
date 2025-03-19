#include "sketching/ExtendTool.h"
#include <cmath>
#include <limits>

namespace RebelCAD {
namespace Sketching {

void ExtendTool::validateExtensionPoint(float x, float y) const {
    if (!std::isfinite(x) || !std::isfinite(y)) {
        throw Core::Error(Core::ErrorCode::GeometryError, "Invalid extension point");
    }
}

float ExtendTool::calculateExtensionDistance(
    float sourceX, float sourceY,
    float targetX, float targetY) const {
    
    float dx = targetX - sourceX;
    float dy = targetY - sourceY;
    return std::sqrt(dx * dx + dy * dy);
}

std::pair<float, float> ExtendTool::projectPointOnLine(
    float lineStartX, float lineStartY,
    float lineEndX, float lineEndY,
    float pointX, float pointY) const {
    
    // Calculate line direction vector
    float dx = lineEndX - lineStartX;
    float dy = lineEndY - lineStartY;
    float len = std::sqrt(dx * dx + dy * dy);
    
    if (len < EPSILON) {
        return {lineStartX, lineStartY};
    }

    // Normalize direction vector
    dx /= len;
    dy /= len;

    // Calculate vector from line start to point
    float vx = pointX - lineStartX;
    float vy = pointY - lineStartY;

    // Calculate projection distance
    float proj = vx * dx + vy * dy;

    // Calculate projected point
    return {
        lineStartX + proj * dx,
        lineStartY + proj * dy
    };
}

Line ExtendTool::extendLine(const Line& line, float x, float y, bool fromStart) const {
    validateExtensionPoint(x, y);

    auto start = line.getStartPoint();
    auto end = line.getEndPoint();

    if (fromStart) {
        return Line(x, y, end[0], end[1]);
    } else {
        return Line(start[0], start[1], x, y);
    }
}

template<typename CurveType>
CurveType ExtendTool::extendCurve(
    const CurveType& curve, float x, float y, bool fromStart) const {
    
    validateExtensionPoint(x, y);

    // Create a copy of the curve
    CurveType extended = curve;

    // Extend the appropriate end
    if (fromStart) {
        extended.setStartPoint(x, y);
    } else {
        extended.setEndPoint(x, y);
    }

    return extended;
}

Polygon ExtendTool::extendPolygonEdge(
    const Polygon& polygon,
    size_t edgeIndex,
    float x, float y,
    bool fromStart) const {
    
    validateExtensionPoint(x, y);

    // Get polygon vertices
    auto vertices = polygon.getVertices();
    if (edgeIndex >= vertices.size()) {
        throw Core::Error(Core::ErrorCode::GeometryError, "Invalid edge index");
    }

    // Calculate vertex indices
    size_t startIdx = edgeIndex;
    size_t endIdx = (edgeIndex + 1) % vertices.size();

    // Update the appropriate vertex
    if (fromStart) {
        vertices[startIdx] = {x, y};
    } else {
        vertices[endIdx] = {x, y};
    }

    return Polygon(vertices);
}

template<typename ElementType>
std::vector<ExtendTool::ExtensionPoint> ExtendTool::findExtensionPoints(
    const Line& line,
    const std::vector<std::shared_ptr<ElementType>>& elements) const {
    
    std::vector<ExtensionPoint> points;
    auto start = line.getStartPoint();
    auto end = line.getEndPoint();

    // Calculate line direction vector
    float dx = end[0] - start[0];
    float dy = end[1] - start[1];
    float len = std::sqrt(dx * dx + dy * dy);
    
    if (len < EPSILON) {
        return points;
    }

    // Normalize direction vector
    dx /= len;
    dy /= len;

    // Check each element for intersection
    for (size_t i = 0; i < elements.size(); ++i) {
        const auto& element = elements[i];
        if (!element) continue;

        // Project element's endpoints onto the line
        auto elementStart = element->getStartPoint();
        auto elementEnd = element->getEndPoint();

        auto [projStartX, projStartY] = projectPointOnLine(
            start[0], start[1], end[0], end[1],
            elementStart[0], elementStart[1]
        );

        auto [projEndX, projEndY] = projectPointOnLine(
            start[0], start[1], end[0], end[1],
            elementEnd[0], elementEnd[1]
        );

        // Check if projections are valid extension points
        float startDist = calculateExtensionDistance(start[0], start[1], projStartX, projStartY);
        float endDist = calculateExtensionDistance(end[0], end[1], projEndX, projEndY);

        // Add valid extension points
        if (startDist > EPSILON) {
            points.push_back({
                projStartX, projStartY,
                i, startDist, true
            });
        }
        if (endDist > EPSILON) {
            points.push_back({
                projEndX, projEndY,
                i, endDist, false
            });
        }
    }

    return points;
}

template<typename ElementType>
std::unique_ptr<ExtendTool::ExtensionPoint> ExtendTool::findNearestExtensionPoint(
    float x, float y,
    const std::vector<std::shared_ptr<ElementType>>& elements) const {
    
    float minDist = std::numeric_limits<float>::infinity();
    std::unique_ptr<ExtensionPoint> nearest;

    for (size_t i = 0; i < elements.size(); ++i) {
        const auto& element = elements[i];
        if (!element) continue;

        // Get element endpoints
        auto start = element->getStartPoint();
        auto end = element->getEndPoint();

        // Calculate distances to endpoints
        float startDist = calculateExtensionDistance(x, y, start[0], start[1]);
        float endDist = calculateExtensionDistance(x, y, end[0], end[1]);

        // Update nearest point if closer
        if (startDist < minDist) {
            minDist = startDist;
            nearest = std::make_unique<ExtensionPoint>(ExtensionPoint{
                start[0], start[1], i, startDist, true
            });
        }
        if (endDist < minDist) {
            minDist = endDist;
            nearest = std::make_unique<ExtensionPoint>(ExtensionPoint{
                end[0], end[1], i, endDist, false
            });
        }
    }

    return nearest;
}

// Explicit template instantiations
template std::vector<ExtendTool::ExtensionPoint> ExtendTool::findExtensionPoints<Line>(
    const Line&, const std::vector<std::shared_ptr<Line>>&) const;
template std::vector<ExtendTool::ExtensionPoint> ExtendTool::findExtensionPoints<BezierCurve>(
    const Line&, const std::vector<std::shared_ptr<BezierCurve>>&) const;
template std::vector<ExtendTool::ExtensionPoint> ExtendTool::findExtensionPoints<Spline>(
    const Line&, const std::vector<std::shared_ptr<Spline>>&) const;

template BezierCurve ExtendTool::extendCurve<BezierCurve>(
    const BezierCurve&, float, float, bool) const;
template Spline ExtendTool::extendCurve<Spline>(
    const Spline&, float, float, bool) const;

template std::unique_ptr<ExtendTool::ExtensionPoint> ExtendTool::findNearestExtensionPoint<Line>(
    float, float, const std::vector<std::shared_ptr<Line>>&) const;
template std::unique_ptr<ExtendTool::ExtensionPoint> ExtendTool::findNearestExtensionPoint<BezierCurve>(
    float, float, const std::vector<std::shared_ptr<BezierCurve>>&) const;
template std::unique_ptr<ExtendTool::ExtensionPoint> ExtendTool::findNearestExtensionPoint<Spline>(
    float, float, const std::vector<std::shared_ptr<Spline>>&) const;

} // namespace Sketching
} // namespace RebelCAD
