#pragma once

#include <memory>
#include "core/Error.h"
#include "graphics/GraphicsSystem.h"
#include "sketching/Spline.h"

namespace RebelCAD {
namespace Modeling {

using Error = rebel::core::Error;

/**
 * @brief SweepTool creates a 3D solid by sweeping a 2D profile along a path
 * 
 * The SweepTool generates 3D geometry by moving a profile curve along a specified path.
 * It supports various options like:
 * - Profile orientation control
 * - Scaling along the sweep path
 * - Twist angle specification
 * - Guide rail for orientation control
 */
class SweepTool {
public:
    /**
     * @brief Defines how the profile orientation is maintained during sweep
     */
    enum class OrientationMode {
        Fixed,      ///< Maintains original profile orientation
        PathNormal, ///< Orients normal to path
        GuideRail   ///< Uses guide rail for orientation
    };

    /**
     * @brief Configuration options for the sweep operation
     */
    struct SweepOptions {
        OrientationMode orientationMode = OrientationMode::PathNormal;
        bool maintainSize = true;     ///< If true, maintains profile size along path
        float twistAngle = 0.0f;      ///< Rotation angle along sweep path (degrees)
        float scaleStart = 1.0f;      ///< Scale factor at start of sweep
        float scaleEnd = 1.0f;        ///< Scale factor at end of sweep
        bool alignToPath = true;      ///< Align profile perpendicular to path

        /**
         * @brief Sets scale factors for the sweep
         * @param start Scale factor at sweep start
         * @param end Scale factor at sweep end
         */
        void SetScaleFactors(float start, float end) {
            scaleStart = start;
            scaleEnd = end;
            maintainSize = false;
        }

        /**
         * @brief Sets twist angle for the sweep
         * @param angle Twist angle in degrees
         */
        void SetTwistAngle(float angle) {
            twistAngle = angle;
        }
    };

    SweepTool();
    ~SweepTool();

    /**
     * @brief Sets the profile to be swept
     * @param profile The 2D profile curve/contour
     * @return Error status
     */
    Error SetProfile(const std::shared_ptr<Sketching::Spline>& profile);

    /**
     * @brief Sets the path along which to sweep
     * @param path The 3D path curve
     * @return Error status
     */
    Error SetPath(const std::shared_ptr<Sketching::Spline>& path);

    /**
     * @brief Sets an optional guide rail for orientation control
     * @param rail The 3D guide rail curve
     * @return Error status
     */
    Error SetGuideRail(const std::shared_ptr<Sketching::Spline>& rail);

    /**
     * @brief Clears all profile and path data
     */
    void Reset();

    /**
     * @brief Performs the sweep operation with specified options
     * @param options Configuration for the sweep operation
     * @return Error status
     */
    Error Execute(const SweepOptions& options = SweepOptions());

    /**
     * @brief Preview the sweep result without committing it
     * @param graphics Graphics system for rendering the preview
     * @return Error status
     */
    Error Preview(std::shared_ptr<Graphics::GraphicsSystem> graphics);

    /**
     * @brief Cancel the current sweep operation
     */
    void Cancel();

    /**
     * @brief Checks if the tool has valid profile and path set
     * @return true if tool is ready to execute
     */
    bool IsReady() const;

private:
    class Implementation;
    std::unique_ptr<Implementation> impl;

    /**
     * @brief Validates profile and path before sweeping
     * @return Error status
     */
    Error ValidateInput() const;

    /**
     * @brief Creates sweep geometry along the path
     * @param options Sweep configuration options
     * @return Error status
     */
    Error CreateSweepGeometry(const SweepOptions& options);
};

} // namespace Modeling
} // namespace RebelCAD
