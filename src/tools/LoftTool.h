#pragma once

#include <vector>
#include <memory>
#include "core/Error.h"
#include "graphics/GraphicsSystem.h"
#include "sketching/Spline.h"

namespace RebelCAD {
namespace Modeling {

using Error = rebel::core::Error;

/**
 * @brief LoftTool creates a 3D solid or surface by transitioning between two or more profile sections
 * 
 * The LoftTool allows creation of complex 3D shapes by interpolating between multiple 2D profiles.
 * It supports various options like:
 * - Guide curves to control the loft path
 * - Different transition types (ruled, smooth)
 * - Start/end conditions for tangency control
 */
class LoftTool {
public:
    /**
     * @brief Defines how profiles should be connected during lofting
     */
    enum class TransitionType {
        Ruled,    ///< Linear interpolation between corresponding points
        Smooth    ///< Smooth interpolation using guide curves/rails
    };

    /**
     * @brief Defines tangency conditions at profile sections
     */
    enum class TangencyCondition {
        Natural,  ///< Natural end conditions
        Normal,   ///< Normal to profile
        Custom    ///< User-defined tangent direction
    };

    /**
     * @brief Configuration options for the loft operation
     */
    /**
     * @brief Custom tangent vector for controlling loft shape
     */
    struct TangentVector {
        float x;  ///< X component of tangent vector
        float y;  ///< Y component of tangent vector
        float z;  ///< Z component of tangent vector

        TangentVector(float x = 0.0f, float y = 0.0f, float z = 1.0f)
            : x(x), y(y), z(z) {}
    };

    /**
     * @brief Configuration options for the loft operation
     */
    struct LoftOptions {
        TransitionType transitionType = TransitionType::Smooth;
        TangencyCondition startCondition = TangencyCondition::Natural;
        TangencyCondition endCondition = TangencyCondition::Natural;
        bool closed = false;  ///< If true, connects last profile back to first
        
        // Custom tangent vectors for start and end conditions
        std::vector<TangentVector> startTangents;  ///< Custom tangent vectors for start profile
        std::vector<TangentVector> endTangents;    ///< Custom tangent vectors for end profile

        /**
         * @brief Sets custom tangent vectors for the start profile
         * @param tangents Vector of tangent vectors, one for each point in the profile
         */
        void SetStartTangents(const std::vector<TangentVector>& tangents) {
            startTangents = tangents;
            startCondition = TangencyCondition::Custom;
        }

        /**
         * @brief Sets custom tangent vectors for the end profile
         * @param tangents Vector of tangent vectors, one for each point in the profile
         */
        void SetEndTangents(const std::vector<TangentVector>& tangents) {
            endTangents = tangents;
            endCondition = TangencyCondition::Custom;
        }
    };

    LoftTool();
    ~LoftTool();

    /**
     * @brief Adds a profile section to be lofted
     * @param profile The 2D profile curve/contour
     * @return Error status
     */
    Error AddProfile(const std::shared_ptr<Sketching::Spline>& profile);

    /**
     * @brief Adds a guide curve to control the loft path
     * @param guide The 3D guide curve
     * @return Error status
     */
    Error AddGuideCurve(const std::shared_ptr<Sketching::Spline>& guide);

    /**
     * @brief Clears all profiles and guide curves
     */
    void Reset();

    /**
     * @brief Performs the loft operation with specified options
     * @param options Configuration for the loft operation
     * @return Error status
     */
    Error Execute(const LoftOptions& options = LoftOptions());

    /**
     * @brief Preview the loft result without committing it
     * @param graphics Graphics system for rendering the preview
     * @return Error status
     */
    Error Preview(std::shared_ptr<Graphics::GraphicsSystem> graphics);

    /**
     * @brief Cancel the current loft operation
     */
    void Cancel();

    /**
     * @brief Gets the current number of profile sections
     */
    size_t GetProfileCount() const;

    /**
     * @brief Gets the current number of guide curves
     */
    size_t GetGuideCurveCount() const;

private:
    class Implementation;
    std::unique_ptr<Implementation> impl;

    /**
     * @brief Validates profiles and guide curves before lofting
     * @return Error status
     */
    Error ValidateInput() const;

    /**
     * @brief Creates interpolation between profile sections
     * @param options Loft configuration options
     * @return Error status
     */
    Error CreateLoftGeometry(const LoftOptions& options);
};

} // namespace Modeling
} // namespace RebelCAD
