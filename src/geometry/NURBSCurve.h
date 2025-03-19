#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "core/Error.h"
#include "graphics/GraphicsSystem.h"

namespace RebelCAD {
namespace Modeling {

/**
 * @brief Parameters for configuring a NURBS curve
 */
struct NURBSCurveParams {
    int degree = 3;                      ///< Curve degree (default cubic)
    bool rational = false;               ///< Whether curve is rational (weights used)
    float tolerance = 1e-6f;             ///< Tolerance for curve operations
    bool optimize_result = true;         ///< Optimize resulting geometry
    bool maintain_tangency = true;       ///< Maintain tangency at endpoints
};

/**
 * @class NURBSCurve
 * @brief Class for creating and manipulating NURBS curves
 * 
 * The NURBSCurve class provides functionality for creating and
 * manipulating Non-Uniform Rational B-Spline curves. It supports
 * both rational and non-rational curves, degree elevation/reduction,
 * and control point manipulation.
 */
class NURBSCurve {
public:
    using ErrorCode = RebelCAD::Core::ErrorCode;

    NURBSCurve();
    ~NURBSCurve();

    /**
     * @brief Set control points for the curve
     * @param points Vector of control points
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode SetControlPoints(const std::vector<glm::vec3>& points);

    /**
     * @brief Set weights for rational curves
     * @param weights Vector of weights (one per control point)
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode SetWeights(const std::vector<float>& weights);

    /**
     * @brief Configure curve parameters
     * @param params NURBSCurveParams struct containing settings
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode Configure(const NURBSCurveParams& params);

    /**
     * @brief Evaluate curve at parameter value
     * @param t Parameter value in range [0,1]
     * @return Point on curve at parameter value
     */
    glm::vec3 Evaluate(float t) const;

    /**
     * @brief Evaluate curve derivative at parameter value
     * @param t Parameter value in range [0,1]
     * @param order Derivative order (1 = first derivative, etc.)
     * @return Vector of derivative values
     */
    std::vector<glm::vec3> EvaluateDerivatives(float t, int order) const;

    /**
     * @brief Generate discretized curve geometry
     * @param segments Number of segments for discretization
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode GenerateGeometry(int segments);

    /**
     * @brief Get curve vertices after generation
     * @return Vector of vertex data (x,y,z coordinates)
     */
    const std::vector<float>& GetResultVertices() const;

    /**
     * @brief Get curve tangents after generation
     * @return Vector of tangent data (x,y,z vectors)
     */
    const std::vector<float>& GetResultTangents() const;

    /**
     * @brief Get curve normals after generation
     * @return Vector of normal data (x,y,z vectors)
     */
    const std::vector<float>& GetResultNormals() const;

    /**
     * @brief Get curve indices after generation
     * @return Vector of line segment indices
     */
    const std::vector<unsigned int>& GetResultIndices() const;

    /**
     * @brief Elevate curve degree
     * @param target_degree Target degree for elevation
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode ElevateDegree(int target_degree);

    /**
     * @brief Reduce curve degree
     * @param target_degree Target degree for reduction
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode ReduceDegree(int target_degree);

    /**
     * @brief Insert knot into curve
     * @param t Parameter value for new knot
     * @param multiplicity Multiplicity of new knot
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode InsertKnot(float t, int multiplicity = 1);

    /**
     * @brief Remove knot from curve
     * @param t Parameter value of knot to remove
     * @param multiplicity Number of times to remove knot
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode RemoveKnot(float t, int multiplicity = 1);

    /**
     * @brief Check if curve is valid
     * @return true if valid, false otherwise
     */
    bool IsValid() const;

    /**
     * @brief Get curve degree
     * @return Current curve degree
     */
    int GetDegree() const;

    /**
     * @brief Generate preview visualization
     * @param graphics Graphics system for rendering
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode Preview(std::shared_ptr<Graphics::GraphicsSystem> graphics) const;

    /**
     * @brief Split curve at parameter value
     * @param t Parameter value to split at
     * @param left Left portion of split curve
     * @param right Right portion of split curve
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode Split(float t, NURBSCurve& left, NURBSCurve& right) const;

    /**
     * @brief Join this curve with another curve at endpoints
     * @param other Curve to join with
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode Join(const NURBSCurve& other);

protected:
    class Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace Modeling
} // namespace RebelCAD
