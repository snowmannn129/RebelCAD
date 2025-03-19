#include "sketching/FilletTool.h"
#include <cmath>

namespace RebelCAD {
namespace Sketching {

std::shared_ptr<Arc> FilletTool::createLineLine(
    std::shared_ptr<Line> line1,
    std::shared_ptr<Line> line2,
    double radius) {
    
    validateRadius(radius);

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

    // Calculate angle between lines
    double angle = std::acos(dx1 * dx2 + dy1 * dy2);
    if (angle < 1e-6 || std::abs(angle - M_PI) < 1e-6) {
        throw Core::Error(Core::ErrorCode::GeometryError, "Lines are parallel");
    }

    // Calculate tangent points
    double tangentDist = radius / std::tan(angle / 2.0);

    // Calculate fillet center point
    double nx1 = -dy1;  // Normal vector to line1
    double ny1 = dx1;
    double nx2 = -dy2;  // Normal vector to line2
    double ny2 = dx2;

    // Choose correct side for fillet
    if (nx1 * dx2 + ny1 * dy2 < 0) {
        nx1 = -nx1;
        ny1 = -ny1;
    }
    if (nx2 * dx1 + ny2 * dy1 < 0) {
        nx2 = -nx2;
        ny2 = -ny2;
    }

    // Calculate center point
    double centerX = intersection.first + radius * (nx1 + nx2) / std::sqrt(2.0 * (1.0 + std::cos(angle)));
    double centerY = intersection.second + radius * (ny1 + ny2) / std::sqrt(2.0 * (1.0 + std::cos(angle)));

    // Calculate start and end angles
    double startAngle = std::atan2(-ny1, -nx1);
    double endAngle = std::atan2(-ny2, -nx2);

    // Ensure angles are in correct order
    if (startAngle > endAngle) {
        std::swap(startAngle, endAngle);
    }

    // Create fillet arc
    return std::make_shared<Arc>(
        static_cast<float>(centerX),
        static_cast<float>(centerY),
        static_cast<float>(radius),
        static_cast<float>(startAngle),
        static_cast<float>(endAngle)
    );
}

std::shared_ptr<Arc> FilletTool::createLineArc(
    std::shared_ptr<Line> line,
    std::shared_ptr<Arc> arc,
    double radius) {
    
    validateRadius(radius);

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
    if (dist > arcRadius + radius || dist < std::abs(arcRadius - radius)) {
        throw Core::Error(Core::ErrorCode::GeometryError, "Line and arc cannot be filleted with given radius");
    }

    // Calculate fillet center point
    double nx = -dy;  // Normal vector to line
    double ny = dx;

    // Choose correct side for fillet
    if ((arcCenter[0] - lineStart[0]) * nx + (arcCenter[1] - lineStart[1]) * ny < 0) {
        nx = -nx;
        ny = -ny;
    }

    double centerX = arcCenter[0] + (arcRadius + radius) * nx;
    double centerY = arcCenter[1] + (arcRadius + radius) * ny;

    // Calculate tangent points and angles
    double startAngle = std::atan2(-ny, -nx);
    double endAngle = std::atan2(centerY - arcCenter[1], centerX - arcCenter[0]);

    // Ensure angles are in correct order
    if (startAngle > endAngle) {
        std::swap(startAngle, endAngle);
    }

    // Create fillet arc
    return std::make_shared<Arc>(
        static_cast<float>(centerX),
        static_cast<float>(centerY),
        static_cast<float>(radius),
        static_cast<float>(startAngle),
        static_cast<float>(endAngle)
    );
}

std::shared_ptr<Arc> FilletTool::createArcArc(
    std::shared_ptr<Arc> arc1,
    std::shared_ptr<Arc> arc2,
    double radius) {
    
    validateRadius(radius);

    // Get arc centers and radii
    auto center1 = arc1->getCenter();
    auto center2 = arc2->getCenter();
    double r1 = arc1->getRadius();
    double r2 = arc2->getRadius();

    // Calculate distance between centers
    double dx = center2[0] - center1[0];
    double dy = center2[1] - center1[1];
    double centerDist = std::sqrt(dx * dx + dy * dy);

    // Check if arcs can be filleted
    if (centerDist > r1 + r2 + 2 * radius ||
        centerDist < std::abs(r1 - r2)) {
        throw Core::Error(Core::ErrorCode::GeometryError, "Arcs cannot be filleted with given radius");
    }

    // Calculate fillet center point
    double angle = std::atan2(dy, dx);
    double offset = std::asin((r2 - r1) / centerDist);
    double filletAngle = angle + offset;

    double centerX = center1[0] + (r1 + radius) * std::cos(filletAngle);
    double centerY = center1[1] + (r1 + radius) * std::sin(filletAngle);

    // Calculate tangent points and angles
    double startAngle = filletAngle + M_PI;
    double endAngle = std::atan2(center2[1] - centerY, center2[0] - centerX);

    // Ensure angles are in correct order
    if (startAngle > endAngle) {
        std::swap(startAngle, endAngle);
    }

    // Create fillet arc
    return std::make_shared<Arc>(
        static_cast<float>(centerX),
        static_cast<float>(centerY),
        static_cast<float>(radius),
        static_cast<float>(startAngle),
        static_cast<float>(endAngle)
    );
}

void FilletTool::validateRadius(double radius) {
    if (radius <= 0.0) {
        throw Core::Error(Core::ErrorCode::GeometryError, "Fillet radius must be positive");
    }
}

std::pair<double, double> FilletTool::calculateLineLineIntersection(
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
