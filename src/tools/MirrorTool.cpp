#include "sketching/MirrorTool.h"
#include <cmath>

namespace RebelCAD {
namespace Sketching {

void MirrorTool::setMirrorLine(double x1, double y1, double x2, double y2) {
    // Check if points are coincident
    if (std::abs(x2 - x1) < 1e-6 && std::abs(y2 - y1) < 1e-6) {
        throw Core::Error(Core::ErrorCode::GeometryError, "Mirror line points are coincident");
    }

    mirrorLine_ = std::make_shared<Line>(
        static_cast<float>(x1),
        static_cast<float>(y1),
        static_cast<float>(x2),
        static_cast<float>(y2)
    );
    isSet_ = true;
}

void MirrorTool::setMirrorLine(std::shared_ptr<Line> line) {
    if (!line) {
        throw Core::Error(Core::ErrorCode::GeometryError, "Mirror line is null");
    }

    mirrorLine_ = line;
    isSet_ = true;
}

std::shared_ptr<Line> MirrorTool::mirrorLine(const std::shared_ptr<Line>& line) {
    validateMirrorLine();
    if (!line) {
        throw Core::Error(Core::ErrorCode::GeometryError, "Input line is null");
    }

    auto start = line->getStartPoint();
    auto end = line->getEndPoint();

    // Mirror both endpoints
    auto mirroredStart = mirrorPoint(start[0], start[1]);
    auto mirroredEnd = mirrorPoint(end[0], end[1]);

    return std::make_shared<Line>(
        static_cast<float>(mirroredStart.first),
        static_cast<float>(mirroredStart.second),
        static_cast<float>(mirroredEnd.first),
        static_cast<float>(mirroredEnd.second)
    );
}

std::shared_ptr<Arc> MirrorTool::mirrorArc(const std::shared_ptr<Arc>& arc) {
    validateMirrorLine();
    if (!arc) {
        throw Core::Error(Core::ErrorCode::GeometryError, "Input arc is null");
    }

    // Mirror center point
    auto center = arc->getCenter();
    auto mirroredCenter = mirrorPoint(center[0], center[1]);

    // Start and end angles need to be reversed
    float startAngle = arc->getStartAngle();
    float endAngle = arc->getEndAngle();

    // Adjust angles for mirroring
    float newStartAngle = static_cast<float>(M_PI) - endAngle;
    float newEndAngle = static_cast<float>(M_PI) - startAngle;

    // Ensure angles are in correct range
    while (newStartAngle < 0) newStartAngle += 2.0f * static_cast<float>(M_PI);
    while (newEndAngle < 0) newEndAngle += 2.0f * static_cast<float>(M_PI);

    return std::make_shared<Arc>(
        static_cast<float>(mirroredCenter.first),
        static_cast<float>(mirroredCenter.second),
        arc->getRadius(),
        newStartAngle,
        newEndAngle
    );
}

std::shared_ptr<Circle> MirrorTool::mirrorCircle(const std::shared_ptr<Circle>& circle) {
    validateMirrorLine();
    if (!circle) {
        throw Core::Error(Core::ErrorCode::GeometryError, "Input circle is null");
    }

    // Mirror center point
    auto center = circle->getCenter();
    auto mirroredCenter = mirrorPoint(center[0], center[1]);

    return std::make_shared<Circle>(
        static_cast<float>(mirroredCenter.first),
        static_cast<float>(mirroredCenter.second),
        circle->getRadius()
    );
}

std::shared_ptr<BezierCurve> MirrorTool::mirrorBezierCurve(
    const std::shared_ptr<BezierCurve>& curve) {
    
    validateMirrorLine();
    if (!curve) {
        throw Core::Error(Core::ErrorCode::GeometryError, "Input curve is null");
    }

    // Mirror all control points
    auto controlPoints = curve->getControlPoints();
    std::vector<std::pair<double, double>> mirroredPoints;
    mirroredPoints.reserve(controlPoints.size());

    for (const auto& point : controlPoints) {
        auto mirroredPoint = mirrorPoint(point.first, point.second);
        mirroredPoints.push_back(mirroredPoint);
    }

    // Reverse point order to maintain curve direction
    std::reverse(mirroredPoints.begin(), mirroredPoints.end());

    return std::make_shared<BezierCurve>(mirroredPoints);
}

std::shared_ptr<Spline> MirrorTool::mirrorSpline(const std::shared_ptr<Spline>& spline) {
    validateMirrorLine();
    if (!spline) {
        throw Core::Error(Core::ErrorCode::GeometryError, "Input spline is null");
    }

    // Mirror all control points
    auto controlPoints = spline->getControlPoints();
    std::vector<std::pair<double, double>> mirroredPoints;
    mirroredPoints.reserve(controlPoints.size());

    for (const auto& point : controlPoints) {
        auto mirroredPoint = mirrorPoint(point.first, point.second);
        mirroredPoints.push_back(mirroredPoint);
    }

    // Reverse point order to maintain curve direction
    std::reverse(mirroredPoints.begin(), mirroredPoints.end());

    return std::make_shared<Spline>(mirroredPoints);
}

std::pair<double, double> MirrorTool::mirrorPoint(double x, double y) const {
    validateMirrorLine();

    auto start = mirrorLine_->getStartPoint();
    auto end = mirrorLine_->getEndPoint();

    // Calculate mirror line vector
    double dx = end[0] - start[0];
    double dy = end[1] - start[1];
    double len = std::sqrt(dx * dx + dy * dy);
    dx /= len;
    dy /= len;

    // Calculate perpendicular distance
    double dist = calculateDistance(x, y);

    // Calculate projection point
    double t = ((x - start[0]) * dx + (y - start[1]) * dy);
    double projX = start[0] + t * dx;
    double projY = start[1] + t * dy;

    // Calculate mirrored point
    return {
        projX + (projX - x),
        projY + (projY - y)
    };
}

void MirrorTool::validateMirrorLine() const {
    if (!isSet_ || !mirrorLine_) {
        throw Core::Error(Core::ErrorCode::GeometryError, "Mirror line not set");
    }
}

double MirrorTool::calculateDistance(double x, double y) const {
    auto start = mirrorLine_->getStartPoint();
    auto end = mirrorLine_->getEndPoint();

    double dx = end[0] - start[0];
    double dy = end[1] - start[1];
    double len = std::sqrt(dx * dx + dy * dy);

    return std::abs((y - start[1]) * dx - (x - start[0]) * dy) / len;
}

} // namespace Sketching
} // namespace RebelCAD
