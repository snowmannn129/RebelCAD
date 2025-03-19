#include "sketching/OffsetTool.h"
#include <cmath>
#include <unordered_map>
#include <algorithm>

namespace RebelCAD {
using Core::Error;
using Core::ErrorCode;
namespace Sketching {

// Initialize static config with defaults
OffsetTool::Config OffsetTool::sConfig;

std::shared_ptr<Line> OffsetTool::offsetLine(const Line& line, float distance) {
    validateOffset(distance);

    auto start = line.getStartPoint();
    auto end = line.getEndPoint();

    // Calculate line vector and normalize
    float dx = end[0] - start[0];
    float dy = end[1] - start[1];
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < sConfig.tolerance) {
        throw Error(ErrorCode::GeometryError, "Line length too small");
    }

    dx /= len;
    dy /= len;

    // Calculate perpendicular vector
    float nx = -dy;
    float ny = dx;

    // Create offset line
    return std::make_shared<Line>(
        start[0] + nx * distance,
        start[1] + ny * distance,
        end[0] + nx * distance,
        end[1] + ny * distance
    );
}

std::shared_ptr<Arc> OffsetTool::offsetArc(const Arc& arc, float distance) {
    validateOffset(distance, arc.getRadius());

    auto center = arc.getCenter();
    float newRadius = arc.getRadius() + distance;

    if (newRadius < sConfig.tolerance) {
        throw Error(ErrorCode::GeometryError, "Offset would create invalid arc");
    }

    // Keep same angles but adjust radius
    return std::make_shared<Arc>(
        center[0],
        center[1],
        newRadius,
        arc.getStartAngle(),
        arc.getEndAngle()
    );
}

std::shared_ptr<Circle> OffsetTool::offsetCircle(const Circle& circle, float distance) {
    validateOffset(distance, circle.getRadius());

    auto center = circle.getCenter();
    float newRadius = circle.getRadius() + distance;

    if (newRadius < sConfig.tolerance) {
        throw Error(ErrorCode::GeometryError, "Offset would create invalid circle");
    }

    return std::make_shared<Circle>(
        center[0],
        center[1],
        newRadius
    );
}

template<typename T>
std::vector<std::shared_ptr<T>> OffsetTool::offsetElements(
    const std::vector<T>& elements,
    float distance) {
    
    if (elements.empty()) {
        return {};
    }

    validateOffset(distance);

    // First pass: offset each element individually
    auto offsetElements = offsetElementsFirstPass(elements, distance);

    // Second pass: adjust connections between elements
    adjustElementConnections(offsetElements);

    // Check for self-intersections
    if (detectSelfIntersections(offsetElements)) {
        throw Error(ErrorCode::GeometryError, "Offset would create self-intersections");
    }

    return offsetElements;
}

template<typename T>
std::vector<std::shared_ptr<T>> OffsetTool::offsetElementsWithVariableDistance(
    const std::vector<T>& elements,
    float startDistance,
    float endDistance) {
    
    if (elements.empty()) {
        return {};
    }

    validateOffset(startDistance);
    validateOffset(endDistance);

    std::vector<std::shared_ptr<T>> result;
    result.reserve(elements.size());

    // Calculate total path length for interpolation
    float totalLength = 0.0f;
    std::vector<float> lengths;
    lengths.reserve(elements.size());

    for (const auto& element : elements) {
        float length = element.getLength();
        totalLength += length;
        lengths.push_back(length);
    }

    // Offset each element with interpolated distance
    float currentLength = 0.0f;
    for (size_t i = 0; i < elements.size(); ++i) {
        float position = currentLength / totalLength;
        float distance = interpolateOffset(startDistance, endDistance, position);

        if (std::is_same<T, Line>::value) {
            result.push_back(offsetLine(elements[i], distance));
        }
        else if (std::is_same<T, Arc>::value) {
            result.push_back(offsetArc(elements[i], distance));
        }
        else if (std::is_same<T, Circle>::value) {
            result.push_back(offsetCircle(elements[i], distance));
        }

        currentLength += lengths[i];
    }

    // Adjust connections between elements
    adjustElementConnections(result);

    // Check for self-intersections
    if (detectSelfIntersections(result)) {
        throw Error(ErrorCode::GeometryError, "Variable offset would create self-intersections");
    }

    return result;
}

float OffsetTool::interpolateOffset(float startDistance, float endDistance, float position) {
    if (position < 0.0f || position > 1.0f) {
        throw Error(ErrorCode::GeometryError, "Position must be between 0 and 1");
    }
    return startDistance + (endDistance - startDistance) * position;
}

bool OffsetTool::validateOffset(float distance, float minRadius, float tolerance) {
    if (!std::isfinite(distance)) {
        throw Error(ErrorCode::GeometryError, "Invalid offset distance");
    }

    if (minRadius > 0.0f && std::abs(distance) >= minRadius) {
        throw Error(ErrorCode::GeometryError, "Offset distance too large for geometry");
    }

    return true;
}

std::array<float, 2> OffsetTool::calculateIntersection(
    const std::array<float, 2>& p1,
    const std::array<float, 2>& v1,
    const std::array<float, 2>& p2,
    const std::array<float, 2>& v2) {
    
    float det = v1[0] * v2[1] - v1[1] * v2[0];
    if (std::abs(det) < sConfig.tolerance) {
        throw Error(ErrorCode::GeometryError, "Lines are parallel");
    }

    float t = ((p2[0] - p1[0]) * v2[1] - (p2[1] - p1[1]) * v2[0]) / det;
    return {
        p1[0] + v1[0] * t,
        p1[1] + v1[1] * t
    };
}

template<typename T>
std::vector<std::shared_ptr<T>> OffsetTool::offsetElementsFirstPass(
    const std::vector<T>& elements,
    float distance) {
    
    std::vector<std::shared_ptr<T>> result;
    result.reserve(elements.size());

    for (const auto& element : elements) {
        if (std::is_same<T, Line>::value) {
            result.push_back(offsetLine(static_cast<const Line&>(element), distance));
        }
        else if (std::is_same<T, Arc>::value) {
            result.push_back(offsetArc(static_cast<const Arc&>(element), distance));
        }
        else if (std::is_same<T, Circle>::value) {
            result.push_back(offsetCircle(static_cast<const Circle&>(element), distance));
        }
    }

    return result;
}

template<typename T>
void OffsetTool::adjustElementConnections(std::vector<std::shared_ptr<T>>& elements) {
    if (elements.size() < 2) return;

    for (size_t i = 0; i < elements.size() - 1; ++i) {
        auto& current = elements[i];
        auto& next = elements[i + 1];

        // Get direction vectors at connection point
        auto v1 = getDirectionVector(current, true);  // At end
        auto v2 = getDirectionVector(next, false);    // At start

        try {
            // Calculate intersection point
            auto p1 = current->getEndPoint();
            auto p2 = next->getStartPoint();
            auto intersection = calculateIntersection(
                {p1[0], p1[1]}, v1,
                {p2[0], p2[1]}, v2
            );

            // Update connection point
            updateConnectionPoint(current, next, intersection);
        }
        catch (const Error& e) {
            // If lines are parallel, keep original endpoints
            continue;
        }
    }
}

template<typename T>
bool OffsetTool::detectSelfIntersectionsWithPartitioning(
    const std::vector<std::shared_ptr<T>>& elements) {
    
    // Create spatial grid
    struct GridCell {
        std::vector<size_t> elementIndices;
    };
    std::unordered_map<size_t, std::unordered_map<size_t, GridCell>> grid;

    // Calculate bounds
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxX = -std::numeric_limits<float>::max();
    float maxY = -std::numeric_limits<float>::max();

    for (const auto& element : elements) {
        auto box = element->getBoundingBox();
        minX = std::min(minX, box.min[0]);
        minY = std::min(minY, box.min[1]);
        maxX = std::max(maxX, box.max[0]);
        maxY = std::max(maxY, box.max[1]);
    }

    // Insert elements into grid
    for (size_t i = 0; i < elements.size(); ++i) {
        auto box = elements[i]->getBoundingBox();
        size_t startCellX = static_cast<size_t>((box.min[0] - minX) / sConfig.gridCellSize);
        size_t startCellY = static_cast<size_t>((box.min[1] - minY) / sConfig.gridCellSize);
        size_t endCellX = static_cast<size_t>((box.max[0] - minX) / sConfig.gridCellSize);
        size_t endCellY = static_cast<size_t>((box.max[1] - minY) / sConfig.gridCellSize);

        for (size_t x = startCellX; x <= endCellX; ++x) {
            for (size_t y = startCellY; y <= endCellY; ++y) {
                grid[x][y].elementIndices.push_back(i);
            }
        }
    }

    // Check for intersections within each cell
    for (const auto& row : grid) {
        for (const auto& cell : row.second) {
            const auto& indices = cell.second.elementIndices;
            for (size_t i = 0; i < indices.size(); ++i) {
                for (size_t j = i + 1; j < indices.size(); ++j) {
                    // Skip adjacent elements
                    if (std::abs(static_cast<int>(indices[i] - indices[j])) <= 1) continue;

                    if (elements[indices[i]]->intersects(*elements[indices[j]])) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

template<typename T>
std::array<float, 2> OffsetTool::getDirectionVector(
    const std::shared_ptr<T>& element,
    bool atEnd) {
    
    if (std::is_same<T, Line>::value) {
        auto line = std::static_pointer_cast<Line>(element);
        auto start = line->getStartPoint();
        auto end = line->getEndPoint();
        float dx = end[0] - start[0];
        float dy = end[1] - start[1];
        float len = std::sqrt(dx * dx + dy * dy);
        return {dx / len, dy / len};
    }
    else if (std::is_same<T, Arc>::value) {
        auto arc = std::static_pointer_cast<Arc>(element);
        float angle = atEnd ? arc->getEndAngle() : arc->getStartAngle();
        return {std::cos(angle), std::sin(angle)};
    }
    else {
        throw Error(ErrorCode::GeometryError, "Unsupported element type for direction vector");
    }
}

template<typename T>
void OffsetTool::updateConnectionPoint(
    const std::shared_ptr<T>& current,
    const std::shared_ptr<T>& next,
    const std::array<float, 2>& intersectPoint) {
    
    if (std::is_same<T, Line>::value) {
        auto currentLine = std::static_pointer_cast<Line>(current);
        auto nextLine = std::static_pointer_cast<Line>(next);
        
        auto start1 = currentLine->getStartPoint();
        auto end2 = nextLine->getEndPoint();
        
        currentLine->setEndPoint(intersectPoint[0], intersectPoint[1]);
        nextLine->setStartPoint(intersectPoint[0], intersectPoint[1]);
    }
    else if (std::is_same<T, Arc>::value) {
        auto currentArc = std::static_pointer_cast<Arc>(current);
        auto nextArc = std::static_pointer_cast<Arc>(next);
        
        // Calculate new angles based on intersection point
        float angle1 = std::atan2(
            intersectPoint[1] - currentArc->getCenter()[1],
            intersectPoint[0] - currentArc->getCenter()[0]
        );
        float angle2 = std::atan2(
            intersectPoint[1] - nextArc->getCenter()[1],
            intersectPoint[0] - nextArc->getCenter()[0]
        );
        
        currentArc->setEndAngle(angle1);
        nextArc->setStartAngle(angle2);
    }
}

// Explicit template instantiations
template std::vector<std::shared_ptr<Line>> OffsetTool::offsetElements<Line>(
    const std::vector<Line>&, float);
template std::vector<std::shared_ptr<Arc>> OffsetTool::offsetElements<Arc>(
    const std::vector<Arc>&, float);
template std::vector<std::shared_ptr<Circle>> OffsetTool::offsetElements<Circle>(
    const std::vector<Circle>&, float);

template std::vector<std::shared_ptr<Line>> OffsetTool::offsetElementsWithVariableDistance<Line>(
    const std::vector<Line>&, float, float);
template std::vector<std::shared_ptr<Arc>> OffsetTool::offsetElementsWithVariableDistance<Arc>(
    const std::vector<Arc>&, float, float);
template std::vector<std::shared_ptr<Circle>> OffsetTool::offsetElementsWithVariableDistance<Circle>(
    const std::vector<Circle>&, float, float);

} // namespace Sketching
} // namespace RebelCAD
