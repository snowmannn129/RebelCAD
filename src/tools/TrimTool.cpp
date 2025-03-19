#include "sketching/TrimTool.h"
#include <cmath>
#include <limits>

namespace RebelCAD {
namespace Sketching {

std::vector<TrimTool::IntersectionPoint> TrimTool::findLineIntersections(
    const Line& line1, const Line& line2) const {
    
    std::vector<IntersectionPoint> points;

    // Get line coordinates
    auto start1 = line1.getStartPoint();
    auto end1 = line1.getEndPoint();
    auto start2 = line2.getStartPoint();
    auto end2 = line2.getEndPoint();

    // Calculate line vectors
    float dx1 = end1[0] - start1[0];
    float dy1 = end1[1] - start1[1];
    float dx2 = end2[0] - start2[0];
    float dy2 = end2[1] - start2[1];

    // Calculate determinant
    float det = dx1 * dy2 - dy1 * dx2;

    // Check if lines are parallel
    if (std::abs(det) < EPSILON) {
        return points;
    }

    // Calculate intersection parameters
    float t1 = ((start2[0] - start1[0]) * dy2 - (start2[1] - start1[1]) * dx2) / det;
    float t2 = ((start2[0] - start1[0]) * dy1 - (start2[1] - start1[1]) * dx1) / det;

    // Check if intersection is within line segments
    if (t1 >= 0.0f && t1 <= 1.0f && t2 >= 0.0f && t2 <= 1.0f) {
        float x = start1[0] + t1 * dx1;
        float y = start1[1] + t1 * dy1;

        points.push_back({
            x, y,
            0, 1, // Element indices (assuming first and second line)
            t1, t2
        });
    }

    return points;
}

template<typename CurveType>
std::vector<TrimTool::IntersectionPoint> TrimTool::findLineCurveIntersections(
    const Line& line, const CurveType& curve) const {
    
    std::vector<IntersectionPoint> points;

    // Get line coordinates
    auto start = line.getStartPoint();
    auto end = line.getEndPoint();

    // Sample curve at multiple points to find approximate intersections
    const size_t numSamples = 100;
    for (size_t i = 0; i < numSamples; ++i) {
        float t = static_cast<float>(i) / (numSamples - 1);
        auto point = curve.evaluatePoint(t);

        // Project point onto line
        float dx = end[0] - start[0];
        float dy = end[1] - start[1];
        float len = std::sqrt(dx * dx + dy * dy);
        
        if (len < EPSILON) continue;

        // Calculate projection parameter
        float proj = ((point.first - start[0]) * dx + 
                     (point.second - start[1]) * dy) / (len * len);

        // Check if projection is on line segment
        if (proj >= 0.0f && proj <= 1.0f) {
            // Calculate distance to line
            float px = start[0] + proj * dx;
            float py = start[1] + proj * dy;
            float dist = std::hypot(point.first - px, point.second - py);

            // If point is close enough to line, it's an intersection
            if (dist < EPSILON) {
                points.push_back({
                    static_cast<float>(point.first),
                    static_cast<float>(point.second),
                    0, 1, // Element indices
                    proj, t
                });
            }
        }
    }

    return points;
}

Line TrimTool::trimLine(const Line& line, float x, float y, bool keepStart) const {
    validateTrimPoint(x, y);

    if (!isPointOnLine(line, x, y)) {
        throw Core::Error(Core::ErrorCode::GeometryError, "Trim point not on line");
    }

    auto start = line.getStartPoint();
    auto end = line.getEndPoint();

    if (keepStart) {
        return Line(start[0], start[1], x, y);
    } else {
        return Line(x, y, end[0], end[1]);
    }
}

template<typename CurveType>
CurveType TrimTool::trimCurve(
    const CurveType& curve, float x, float y, bool keepStart) const {
    
    validateTrimPoint(x, y);

    // Find parameter value at trim point
    auto closest = curve.findClosestPoint(x, y);
    if (std::abs(closest.first.first - x) > EPSILON ||
        std::abs(closest.first.second - y) > EPSILON) {
        throw Core::Error(Core::ErrorCode::GeometryError, "Trim point not on curve");
    }

    // Split curve at parameter value
    auto parts = curve.splitAtParameter(closest.second);
    return keepStart ? parts.first : parts.second;
}

Polygon TrimTool::trimPolygonEdge(
    const Polygon& polygon,
    size_t edgeIndex,
    float x, float y,
    bool keepStart) const {
    
    validateTrimPoint(x, y);

    auto vertices = polygon.getVertices();
    if (edgeIndex >= vertices.size()) {
        throw Core::Error(Core::ErrorCode::GeometryError, "Invalid edge index");
    }

    // Create line segment for the edge
    size_t nextIndex = (edgeIndex + 1) % vertices.size();
    Line edge(vertices[edgeIndex][0], vertices[edgeIndex][1],
             vertices[nextIndex][0], vertices[nextIndex][1]);

    if (!isPointOnLine(edge, x, y)) {
        throw Core::Error(Core::ErrorCode::GeometryError, "Trim point not on edge");
    }

    // Update vertex position
    if (keepStart) {
        vertices[nextIndex] = {x, y};
    } else {
        vertices[edgeIndex] = {x, y};
    }

    return Polygon(vertices);
}

bool TrimTool::isPointOnLine(const Line& line, float x, float y) const {
    auto start = line.getStartPoint();
    auto end = line.getEndPoint();

    // Calculate distances
    float lineLength = std::hypot(end[0] - start[0], end[1] - start[1]);
    float d1 = std::hypot(x - start[0], y - start[1]);
    float d2 = std::hypot(end[0] - x, end[1] - y);

    // Point is on line if sum of distances equals line length
    return std::abs(d1 + d2 - lineLength) < EPSILON;
}

float TrimTool::getLineParameter(const Line& line, float x, float y) const {
    auto start = line.getStartPoint();
    auto end = line.getEndPoint();

    float dx = end[0] - start[0];
    float dy = end[1] - start[1];
    float len = std::sqrt(dx * dx + dy * dy);

    if (len < EPSILON) return 0.0f;

    return ((x - start[0]) * dx + (y - start[1]) * dy) / (len * len);
}

void TrimTool::validateTrimPoint(float x, float y) const {
    if (!std::isfinite(x) || !std::isfinite(y)) {
        throw Core::Error(Core::ErrorCode::GeometryError, "Invalid trim point");
    }
}

// Explicit template instantiations
template std::vector<TrimTool::IntersectionPoint>
TrimTool::findLineCurveIntersections<BezierCurve>(const Line&, const BezierCurve&) const;

template std::vector<TrimTool::IntersectionPoint>
TrimTool::findLineCurveIntersections<Spline>(const Line&, const Spline&) const;

template BezierCurve TrimTool::trimCurve<BezierCurve>(
    const BezierCurve&, float, float, bool) const;

template Spline TrimTool::trimCurve<Spline>(
    const Spline&, float, float, bool) const;

} // namespace Sketching
} // namespace RebelCAD
