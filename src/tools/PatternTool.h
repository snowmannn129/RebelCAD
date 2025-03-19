#pragma once

#include "modeling/SolidBody.hpp"
#include "core/Error.h"
#include "graphics/GraphicsSystemFwd.h"
#include <memory>
#include <vector>
#include <glm/glm.hpp>

namespace RebelCAD {
namespace Modeling {

/**
 * @brief Enumeration of pattern types
 */
enum class PatternType {
    Linear,     ///< Linear pattern along a direction
    Circular,   ///< Circular pattern around an axis
    Mirror      ///< Mirror pattern across a plane
};

/**
 * @brief Parameters for linear pattern
 */
struct LinearPatternParams {
    glm::dvec3 direction;      ///< Direction vector for pattern
    double spacing;            ///< Distance between instances
    size_t count;             ///< Number of instances
    bool includeOriginal;     ///< Whether to include original body
};

/**
 * @brief Parameters for circular pattern
 */
struct CircularPatternParams {
    glm::dvec3 axis;          ///< Axis of rotation
    glm::dvec3 center;        ///< Center point of rotation
    double angle;             ///< Total angle to distribute instances over
    size_t count;             ///< Number of instances
    bool includeOriginal;     ///< Whether to include original body
};

/**
 * @brief Parameters for mirror pattern
 */
struct MirrorPatternParams {
    glm::dvec3 normal;        ///< Normal vector of mirror plane
    glm::dvec3 point;         ///< Point on mirror plane
    bool includeOriginal;     ///< Whether to include original body
};

/**
 * @brief Class for creating patterns of 3D bodies
 */
class PatternTool {
public:
    /**
     * @brief Create a linear pattern of a body
     * @param body Body to pattern
     * @param params Linear pattern parameters
     * @return Vector of pattern instances
     */
    static std::vector<std::shared_ptr<SolidBody>> CreateLinearPattern(
        std::shared_ptr<SolidBody> body,
        const LinearPatternParams& params);

    /**
     * @brief Create a circular pattern of a body
     * @param body Body to pattern
     * @param params Circular pattern parameters
     * @return Vector of pattern instances
     */
    static std::vector<std::shared_ptr<SolidBody>> CreateCircularPattern(
        std::shared_ptr<SolidBody> body,
        const CircularPatternParams& params);

    /**
     * @brief Create a mirror pattern of a body
     * @param body Body to pattern
     * @param params Mirror pattern parameters
     * @return Vector of pattern instances
     */
    static std::vector<std::shared_ptr<SolidBody>> CreateMirrorPattern(
        std::shared_ptr<SolidBody> body,
        const MirrorPatternParams& params);

    /**
     * @brief Preview a pattern operation
     * @param body Body to pattern
     * @param type Type of pattern
     * @param params Pattern parameters (as void* to support different types)
     * @param graphics Graphics system for rendering
     * @return Error code indicating success or failure
     */
    static Core::ErrorCode PreviewPattern(
        std::shared_ptr<SolidBody> body,
        PatternType type,
        const void* params,
        std::shared_ptr<Graphics::GraphicsSystem> graphics);

private:
    /**
     * @brief Transform a body by a matrix
     * @param body Body to transform
     * @param transform Transformation matrix
     * @return Transformed body
     */
    static std::shared_ptr<SolidBody> TransformBody(
        std::shared_ptr<SolidBody> body,
        const glm::dmat4& transform);

    /**
     * @brief Validate pattern parameters
     * @param type Pattern type
     * @param params Pattern parameters
     * @return True if parameters are valid
     */
    static bool ValidateParams(PatternType type, const void* params);

    /**
     * @brief Generate preview geometry for pattern
     * @param body Input body
     * @param type Pattern type
     * @param params Pattern parameters
     * @return Vector of preview vertices
     */
    static std::vector<float> GeneratePreviewGeometry(
        std::shared_ptr<SolidBody> body,
        PatternType type,
        const void* params);
};

} // namespace Modeling
} // namespace RebelCAD
