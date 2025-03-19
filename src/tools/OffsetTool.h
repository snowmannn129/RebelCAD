#pragma once

#include <vector>
#include <memory>
#include <array>
#include <core/Error.h>
using rebel::core::Error;
using rebel::core::ErrorCode;
#include "sketching/Line.h"
#include "sketching/Arc.h"
#include "sketching/Circle.h"

namespace RebelCAD {
namespace Sketching {

/**
 * @class OffsetTool
 * @brief Tool for creating parallel copies of sketch geometry at a specified distance
 * 
 * The OffsetTool creates offset copies of existing geometry (lines, arcs, circles)
 * at a user-specified distance. It handles both internal and external offsets and
 * maintains geometric relationships between connected elements.
 */
class OffsetTool {
public:
    /**
     * @brief Configuration options for the OffsetTool
     */
    struct Config {
        float gridCellSize = 10.0f;              // Size of grid cells for spatial partitioning
        float tolerance = 1e-6f;                 // General tolerance for geometric comparisons
        size_t spatialPartitioningThreshold = 100; // Number of elements before using spatial partitioning
        
        /**
         * @brief Sets the grid cell size for spatial partitioning
         * @param size New grid cell size
         * @return Reference to this config for chaining
         */
        Config& setGridCellSize(float size) {
            if (size <= 0.0f) {
                throw rebel::core::Error(
                    ErrorCode::GeometryError,
                    "Grid cell size must be positive"
                );
            }
            gridCellSize = size;
            return *this;
        }
        
        /**
         * @brief Sets the general tolerance for geometric comparisons
         * @param tol New tolerance value
         * @return Reference to this config for chaining
         */
        Config& setTolerance(float tol) {
            if (tol <= 0.0f) {
                throw rebel::core::Error(
                    ErrorCode::GeometryError,
                    "Tolerance must be positive"
                );
            }
            tolerance = tol;
            return *this;
        }
        
        /**
         * @brief Sets the threshold for using spatial partitioning
         * @param threshold New threshold value
         * @return Reference to this config for chaining
         */
        Config& setSpatialPartitioningThreshold(size_t threshold) {
            spatialPartitioningThreshold = threshold;
            return *this;
        }
    };

    /**
     * @brief Gets the current configuration
     * @return Current configuration
     */
    static const Config& getConfig() { return sConfig; }

    /**
     * @brief Sets new configuration options
     * @param config New configuration
     */
    static void setConfig(const Config& config) { sConfig = config; }

    /**
     * @brief Creates an offset copy of a line at the specified distance
     * @param line The source line to offset
     * @param distance The offset distance (positive for one side, negative for other)
     * @return Newly created offset line
     * @throws rebel::core::Error if the operation fails
     */
    static std::shared_ptr<Line> offsetLine(const Line& line, float distance);

    /**
     * @brief Creates an offset copy of an arc at the specified distance
     * @param arc The source arc to offset
     * @param distance The offset distance (positive for external, negative for internal)
     * @return Newly created offset arc
     * @throws rebel::core::Error if the operation fails or would create invalid geometry
     */
    static std::shared_ptr<Arc> offsetArc(const Arc& arc, float distance);

    /**
     * @brief Creates an offset copy of a circle at the specified distance
     * @param circle The source circle to offset
     * @param distance The offset distance (positive for external, negative for internal)
     * @return Newly created offset circle
     * @throws rebel::core::Error if the operation fails or would create invalid geometry
     */
    static std::shared_ptr<Circle> offsetCircle(const Circle& circle, float distance);

    /**
     * @brief Creates offset copies of multiple connected elements while maintaining their relationships
     * @tparam T The geometry type (Line, Arc, or Circle)
     * @param elements Vector of geometry elements to offset
     * @param distance The offset distance (positive for external, negative for internal)
     * @return Vector of newly created offset elements
     * @throws Error if the operation fails or would create invalid geometry
     * @note This method handles:
     *       - Connected elements with proper intersection handling
     *       - Self-intersection detection and prevention
     *       - Parallel element relationships
     *       - Mixed geometry types through template specialization
     *       - Empty input validation
     *       Performance optimized for both small and large element sets
     */
    template<typename T>
    static std::vector<std::shared_ptr<T>> offsetElements(
        const std::vector<T>& elements,
        float distance
    );

    /**
     * @brief Creates offset copies of elements with variable distances along the path
     * @tparam T The geometry type (Line, Arc, or Circle)
     * @param elements Vector of geometry elements to offset
     * @param startDistance Starting offset distance
     * @param endDistance Ending offset distance
     * @return Vector of newly created offset elements
     * @throws Error if the operation fails or would create invalid geometry
     * @note This method:
     *       - Linearly interpolates the offset distance from start to end
     *       - Maintains smooth transitions between elements
     *       - Handles self-intersection detection
     *       - Supports both increasing and decreasing offset patterns
     */
    template<typename T>
    static std::vector<std::shared_ptr<T>> offsetElementsWithVariableDistance(
        const std::vector<T>& elements,
        float startDistance,
        float endDistance
    );

    /**
     * @brief Calculates the interpolated offset distance at a specific position
     * @param startDistance Starting offset distance
     * @param endDistance Ending offset distance
     * @param position Current position (0.0 to 1.0)
     * @return Interpolated offset distance
     */
    static float interpolateOffset(float startDistance, float endDistance, float position);

private:
    /**
     * @brief Performs the first pass of offsetting elements individually
     * @tparam T The geometry type
     * @param elements Input elements to offset
     * @param distance Offset distance
     * @return Vector of offset elements
     */
    template<typename T>
    static std::vector<std::shared_ptr<T>> offsetElementsFirstPass(
        const std::vector<T>& elements,
        float distance
    );

    /**
     * @brief Adjusts connections between adjacent offset elements
     * @tparam T The geometry type
     * @param elements Vector of offset elements to adjust
     */
    template<typename T>
    static void adjustElementConnections(std::vector<std::shared_ptr<T>>& elements);

    /**
     * @brief Detects if a set of offset elements would create self-intersections
     * @tparam T The geometry type
     * @param elements Vector of offset elements to check
     * @return true if self-intersections are detected, false otherwise
     */
    template<typename T>
    static bool detectSelfIntersections(const std::vector<std::shared_ptr<T>>& elements) {
        if (elements.size() < 2) return false;

        // Use spatial partitioning for large sets to improve performance
        if (elements.size() > 100) {
            return detectSelfIntersectionsWithPartitioning(elements);
        }

        for (size_t i = 0; i < elements.size(); ++i) {
            for (size_t j = i + 2; j < elements.size(); ++j) {
                // Skip adjacent elements as they're handled by connection logic
                if (j == i + 1) continue;

                // Get bounding boxes and check for potential intersection
                auto box1 = elements[i]->getBoundingBox();
                auto box2 = elements[j]->getBoundingBox();
                
                if (box1.intersects(box2)) {
                    // Detailed intersection check based on geometry type
                    if (elements[i]->intersects(*elements[j])) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    /**
     * @brief Optimized self-intersection detection for large element sets using spatial partitioning
     * @tparam T The geometry type
     * @param elements Vector of elements to check
     * @return true if self-intersections are detected, false otherwise
     */
    template<typename T>
    static bool detectSelfIntersectionsWithPartitioning(const std::vector<std::shared_ptr<T>>& elements);

    /**
     * @brief Gets the direction vector for a geometry element
     * @tparam T The geometry type
     * @param element The geometry element
     * @param atEnd Whether to get direction at end point (true) or start point (false)
     * @return Direction vector as {dx, dy}
     */
    template<typename T>
    static std::array<float, 2> getDirectionVector(const std::shared_ptr<T>& element, bool atEnd);

    /**
     * @brief Updates connection point between two elements
     * @tparam T The geometry type
     * @param current First element
     * @param next Second element
     * @param intersectPoint Intersection point
     */
    template<typename T>
    static void updateConnectionPoint(
        const std::shared_ptr<T>& current,
        const std::shared_ptr<T>& next,
        const std::array<float, 2>& intersectPoint
    );

private:
    static Config sConfig; // Static configuration instance

    /**
     * @brief Validates that the offset distance won't create invalid geometry
     * @param distance The proposed offset distance
     * @param minRadius Minimum radius of any curved geometry being offset
     * @param tolerance Optional tolerance for near-zero comparisons
     * @return true if offset is valid, false otherwise
     * @throws rebel::core::Error with detailed message if validation fails
     */
    static bool validateOffset(float distance, float minRadius = 0.0f, float tolerance = 1e-6f);

    /**
     * @brief Calculates the intersection point of two lines defined by point and direction
     * @param p1 Start point of first line
     * @param v1 Direction vector of first line
     * @param p2 Start point of second line
     * @param v2 Direction vector of second line
     * @return Intersection point
     * @throws rebel::core::Error if lines are parallel or calculation fails
     */
    static std::array<float, 2> calculateIntersection(
        const std::array<float, 2>& p1,
        const std::array<float, 2>& v1,
        const std::array<float, 2>& p2,
        const std::array<float, 2>& v2
    );
};

} // namespace Sketching
} // namespace RebelCAD
