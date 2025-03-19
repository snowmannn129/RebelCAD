#include "sketching/ChamferTool.h"
#include <cmath>

namespace RebelCAD {
namespace Sketching {

std::shared_ptr<Line> ChamferTool::createLineLine(
    std::shared_ptr<Line> line1,
    std::shared_ptr<Line> line2,
    double distance1,
    double distance2) {
    
    validateDistances(distance1, distance2);

    // Calculate intersection point
    auto intersection = calculateLineLineIntersection(line1, line2);

    // Get line vectors
    auto start1 = line1->getStartPoint();
    auto end1 = line1->getEndPoint();
    auto start2 = line2->getStartPoint();
    auto end2 = line2->getEndPoint();

    // Calculate direction vectors
    double dx1 = end1[0] - start1[0];
    double dy1 = end1[1] - start1[1];
    double len1 = std::sqrt(dx1 * dx1 + dy1 * dy1);
    dx1 /= len1;
    dy1 /= len1;

    double dx2 = end2[0] - start2[0];
    double dy2 = end2[1] - start2[1];
    double len2 = std::sqrt(dx2 * dx2 + dy2 * dy2);
    dx2 /= len2;
    dy2 /= len2;

    // Calculate chamfer points
    double x1 = intersection.first + dx1 * distance1;
    double y1 = intersection.second + dy1 * distance1;
    double x2 = intersection.first + dx2 * distance2;
    double y2 = intersection.second + dy2 * distance2;

    // Create chamfer line
    return std::make_shared<Line>(
        static_cast<float>(x1),
        static_cast<float>(y1),
        static_cast<float>(x2),
        static_cast<float>(y2)
    );
}

std::shared_ptr<Line> ChamferTool::createLineArc(
    std::shared_ptr<Line> line,
    std::shared_ptr<Arc> arc,
    double lineDistance,
    double arcDistance) {
    
    validateDistances(lineDistance, arcDistance);

    // Get line direction
    auto lineStart = line->getStartPoint();
    auto lineEnd = line->getEndPoint();
    double dx = lineEnd[0] - lineStart[0];
    double dy = lineEnd[1] - lineStart[1];
    double lineLen = std::sqrt(dx * dx + dy * dy);
    dx /= lineLen;
    dy /= lineLen;

    // Get arc center and radius
    auto arcCenter = arc->getCenter();
    double arcRadius = arc->getRadius();

    // Calculate perpendicular distance from arc center to line
    double dist = std::abs((arcCenter[1] - lineStart[1]) * dx -
                          (arcCenter[0] - lineStart[0]) * dy);

    // Check if line and arc intersect
    if (dist > arcRadius) {
        throw Core::Error(Core::ErrorCode::GeometryError, "Line and arc do not intersect");
    }

    // Calculate intersection point
    double t = ((arcCenter[0] - lineStart[0]) * dx +
                (arcCenter[1] - lineStart[1]) * dy);
    double ix = lineStart[0] + t * dx;
    double iy = lineStart[1] + t * dy;

    // Calculate point on line
    double x1 = ix + dx * lineDistance;
    double y1 = iy + dy * lineDistance;

    // Calculate angle to intersection point
    double angle = std::atan2(iy - arcCenter[1], ix - arcCenter[0]);
    
    // Calculate point on arc
    double x2 = arcCenter[0] + arcRadius * std::cos(angle + arcDistance / arcRadius);
    double y2 = arcCenter[1] + arcRadius * std::sin(angle + arcDistance / arcRadius);

    // Create chamfer line
    return std::make_shared<Line>(
        static_cast<float>(x1),
        static_cast<float>(y1),
        static_cast<float>(x2),
        static_cast<float>(y2)
    );
}

std::shared_ptr<Line> ChamferTool::createArcArc(
    std::shared_ptr<Arc> arc1,
    std::shared_ptr<Arc> arc2,
    double distance1,
    double distance2) {
    
    validateDistances(distance1, distance2);

    // Get arc centers and radii
    auto center1 = arc1->getCenter();
    auto center2 = arc2->getCenter();
    double r1 = arc1->getRadius();
    double r2 = arc2->getRadius();

    // Calculate distance between centers
    double dx = center2[0] - center1[0];
    double dy = center2[1] - center1[1];
    double centerDist = std::sqrt(dx * dx + dy * dy);

    // Check if arcs intersect
    if (centerDist > r1 + r2 || centerDist < std::abs(r1 - r2)) {
        throw Core::Error(Core::ErrorCode::GeometryError, "Arcs do not intersect");
    }

    // Calculate intersection angle
    double angle = std::atan2(dy, dx);
    double a = (r1 * r1 - r2 * r2 + centerDist * centerDist) / (2 * centerDist);
    double h = std::sqrt(r1 * r1 - a * a);
    double intersectAngle = angle + std::atan2(h, a);

    // Calculate points on arcs
    double x1 = center1[0] + r1 * std::cos(intersectAngle + distance1 / r1);
    double y1 = center1[1] + r1 * std::sin(intersectAngle + distance1 / r1);
    double x2 = center2[0] + r2 * std::cos(intersectAngle + distance2 / r2);
    double y2 = center2[1] + r2 * std::sin(intersectAngle + distance2 / r2);

    // Create chamfer line
    return std::make_shared<Line>(
        static_cast<float>(x1),
        static_cast<float>(y1),
        static_cast<float>(x2),
        static_cast<float>(y2)
    );
}

void ChamferTool::validateDistances(double distance1, double distance2) {
    if (distance1 <= 0.0 || distance2 <= 0.0) {
        throw Core::Error(Core::ErrorCode::GeometryError, "Chamfer distances must be positive");
    }
}

std::pair<double, double> ChamferTool::calculateLineLineIntersection(
    const std::shared_ptr<Line>& line1,
    const std::shared_ptr<Line>& line2) {
    
    auto start1 = line1->getStartPoint();
    auto end1 = line1->getEndPoint();
    auto start2 = line2->getStartPoint();
    auto end2 = line2->getEndPoint();

    double dx1 = end1[0] - start1[0];
    double dy1 = end1[1] - start1[1];
    double dx2 = end2[0] - start2[0];
    double dy2 = end2[1] - start2[1];

    double det = dx1 * dy2 - dy1 * dx2;
    if (std::abs(det) < 1e-6) {
        throw Core::Error(Core::ErrorCode::GeometryError, "Lines are parallel");
    }

    double t = ((start2[0] - start1[0]) * dy2 - (start2[1] - start1[1]) * dx2) / det;
    return {
        start1[0] + t * dx1,
        start1[1] + t * dy1
    };
}

} // namespace Sketching
} // namespace RebelCAD
