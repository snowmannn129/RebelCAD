#pragma once

#include <memory>
#include <vector>
#include "core/Error.h"
#include "modeling/Geometry.h"

namespace RebelCAD {
namespace Modeling {

using Error = rebel::core::Error;

/**
 * @brief Tool for creating draft angles on faces for manufacturing purposes
 * 
 * The DraftTool allows users to add draft angles to faces to ensure parts can be
 * easily removed from molds during manufacturing processes like injection molding.
 * It works by tilting selected faces at a specified angle relative to a pull direction.
 */
class DraftTool {
public:
    DraftTool();
    ~DraftTool();

    /**
     * @brief Parameters for the draft operation
     */
    struct DraftParams {
        double angle;           ///< Draft angle in degrees
        float pull_direction[3];///< Pull direction vector
        double neutral_plane;   ///< Height of neutral plane (optional)
        bool is_internal;       ///< True for internal faces, false for external
    };

    /**
     * @brief Apply draft angle to selected faces
     * 
     * @param faces Vector of faces to modify
     * @param params Draft parameters
     * @return Error::None on success, appropriate error code otherwise
     */
    Error ApplyDraft(
        const std::vector<std::shared_ptr<SolidBody>>& faces,
        const DraftParams& params
    );

    /**
     * @brief Check if faces can have draft applied
     * 
     * @param faces Vector of faces to check
     * @return true if draft can be applied to all faces
     */
    bool CanApplyDraft(const std::vector<std::shared_ptr<SolidBody>>& faces) const;

    /**
     * @brief Get the minimum recommended draft angle for the material type
     * 
     * @param material_type String identifier for material (e.g. "plastic", "aluminum")
     * @return double Minimum recommended draft angle in degrees
     */
    double GetMinimumDraftAngle(const std::string& material_type) const;

private:
    /**
     * @brief Validate draft angle is within acceptable range
     * 
     * @param angle Draft angle to validate
     * @throws std::invalid_argument If angle is outside valid range
     */
    void ValidateAngle(double angle) const;

    /**
     * @brief Calculate draft direction for a face based on pull direction
     * 
     * @param face Face to calculate draft direction for
     * @param pull_direction Pull direction array
     * @return Array of 3 floats representing draft direction
     */
    std::array<float, 3> CalculateDraftDirection(
        const std::shared_ptr<SolidBody>& face,
        const float pull_direction[3]
    ) const;

private:
    class Implementation;
    std::unique_ptr<Implementation> impl;
};

} // namespace Modeling
} // namespace RebelCAD
