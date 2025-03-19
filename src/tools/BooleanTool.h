#pragma once

#include <memory>
#include <vector>
#include "core/Error.h"
#include "graphics/GraphicsSystemFwd.h"

namespace RebelCAD {
namespace Modeling {

/**
 * @brief Enum defining the type of boolean operation
 */
enum class BooleanOperationType {
    Union,      ///< Combine two bodies, keeping all material
    Subtract,   ///< Remove second body from first body
    Intersect   ///< Keep only material common to both bodies
};

/**
 * @brief Parameters for configuring a boolean operation
 */
struct BooleanParams {
    BooleanOperationType operation_type = BooleanOperationType::Union;
    bool maintain_features = true;  ///< Try to maintain feature edges/faces
    float intersection_tolerance = 1e-6f;  ///< Tolerance for intersection detection
    bool optimize_result = true;    ///< Optimize resulting mesh
    int max_refinement_steps = 3;   ///< Maximum mesh refinement iterations
};

/**
 * @class BooleanTool
 * @brief Tool for performing boolean operations between solid bodies
 * 
 * The BooleanTool performs boolean operations (union, subtract, intersect)
 * between two solid bodies. It handles mesh intersection detection, topology
 * management, and proper handling of normals and UV coordinates.
 */
class BooleanTool {
public:
    using ErrorCode = RebelCAD::Core::ErrorCode;

    BooleanTool();
    ~BooleanTool();

    /**
     * @brief Set the first body for the boolean operation
     * @param vertices Vertex data for the first body
     * @param indices Index data for the first body
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode SetFirstBody(
        const std::vector<float>& vertices,
        const std::vector<unsigned int>& indices);

    /**
     * @brief Set the second body for the boolean operation
     * @param vertices Vertex data for the second body
     * @param indices Index data for the second body
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode SetSecondBody(
        const std::vector<float>& vertices,
        const std::vector<unsigned int>& indices);

    /**
     * @brief Configure the boolean operation parameters
     * @param params BooleanParams struct containing operation settings
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode Configure(const BooleanParams& params);

    /**
     * @brief Execute the boolean operation
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode Execute();

    /**
     * @brief Preview the boolean operation result without committing it
     * @param graphics Graphics system for rendering the preview
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode Preview(std::shared_ptr<RebelCAD::Graphics::GraphicsSystem> graphics);

    /**
     * @brief Cancel the current boolean operation
     */
    void Cancel();

    /**
     * @brief Get the resulting mesh vertices after the operation
     * @return Vector of vertex data (x,y,z coordinates)
     */
    const std::vector<float>& GetResultVertices() const;

    /**
     * @brief Get the resulting mesh indices after the operation
     * @return Vector of triangle indices
     */
    const std::vector<unsigned int>& GetResultIndices() const;

protected:
    class Impl;
    std::unique_ptr<Impl> impl;

    /**
     * @brief Validate input meshes are suitable for boolean operations
     * @return true if meshes are valid, false otherwise
     */
    bool ValidateMeshes() const;

    /**
     * @brief Generate intersection geometry between the two bodies
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode GenerateIntersectionGeometry();

    /**
     * @brief Perform the actual boolean operation
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode PerformOperation();

    /**
     * @brief Clean and optimize the resulting mesh
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode OptimizeResult();
};

} // namespace Modeling
} // namespace RebelCAD
