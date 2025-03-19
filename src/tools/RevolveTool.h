#pragma once

#include <memory>
#include <vector>
#include "core/Error.h"
#include "graphics/GraphicsSystemFwd.h"
#include "modeling/RevolveToolImpl.h"

namespace RebelCAD {
namespace Modeling {

/**
 * @class RevolveTool
 * @brief Tool for creating 3D solids by revolving 2D sketches around an axis
 * 
 * The RevolveTool takes a 2D sketch profile and creates a 3D solid by rotating
 * the profile around a specified axis through a given angle. It supports both
 * additive (creating new material) and subtractive (removing material) revolve
 * operations, as well as partial and full 360-degree revolutions.
 */
class RevolveTool {
public:
    using ErrorCode = RebelCAD::Core::ErrorCode;

    RevolveTool();
    ~RevolveTool();

    /**
     * @brief Set the sketch profile to be revolved
     * @param profile_vertices Vector of vertices defining the sketch profile
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode SetProfile(const std::vector<float>& profile_vertices);

    /**
     * @brief Configure the revolve parameters
     * @param params RevolveParams struct containing revolve settings
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode Configure(const RevolveParams& params);

    /**
     * @brief Execute the revolve operation
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode Execute();

    /**
     * @brief Preview the revolve result without committing it
     * @param graphics Graphics system for rendering the preview
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode Preview(std::shared_ptr<RebelCAD::Graphics::GraphicsSystem> graphics);

    /**
     * @brief Cancel the current revolve operation
     */
    void Cancel();

protected:
    std::unique_ptr<RevolveToolImpl> impl;

    /**
     * @brief Validate the sketch profile is suitable for revolution
     * @return true if profile is valid, false otherwise
     * 
     * Checks:
     * 1. Profile doesn't intersect the rotation axis
     * 2. Profile is on one side of the axis
     * 3. Profile won't self-intersect during revolution
     */
    bool ValidateProfile() const;

    /**
     * @brief Generate the mesh for the revolved solid
     * @return ErrorCode::None on success, appropriate error code otherwise
     * 
     * For full revolutions (360°), uses optimized mesh generation
     * with shared vertices at the seam.
     */
    ErrorCode GenerateMesh();

    /**
     * @brief Generate mesh for multi-section revolve
     * @return ErrorCode::None on success, appropriate error code otherwise
     * 
     * Handles revolve with multiple sections, including scaling and twisting.
     */
    ErrorCode GenerateMultiSectionMesh();

    /**
     * @brief Generate geometry for a single profile
     * @param profile The profile vertices to generate geometry for
     * @param angle The angle at which to generate the profile
     * @param u The U texture coordinate for this profile
     * @param vertex_offset Running count of vertices, updated by this function
     */
    void GenerateProfileGeometry(
        const std::vector<float>& profile,
        float angle,
        float u,
        size_t& vertex_offset);

    /**
     * @brief Calculate the optimal number of segments based on revolution angle
     * @return Number of segments to use for smooth revolution
     */
    int CalculateOptimalSegments() const;

private:
};

} // namespace Modeling
} // namespace RebelCAD
