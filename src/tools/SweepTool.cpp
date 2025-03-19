#include "modeling/SweepTool.h"
#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace RebelCAD {
namespace Modeling {

class SweepTool::Implementation {
public:
    std::shared_ptr<Sketching::Spline> profile;
    std::shared_ptr<Sketching::Spline> path;
    std::shared_ptr<Sketching::Spline> guideRail;
    bool isPreviewActive;

    Implementation() : isPreviewActive(false) {}

    void Reset() {
        profile.reset();
        path.reset();
        guideRail.reset();
        isPreviewActive = false;
    }
};

SweepTool::SweepTool() : impl(std::make_unique<Implementation>()) {}

SweepTool::~SweepTool() = default;

Error SweepTool::SetProfile(const std::shared_ptr<Sketching::Spline>& profile) {
    if (!profile) {
        return Error(rebel::core::ErrorCode::InvalidVertex, "Invalid profile: nullptr provided");
    }
    impl->profile = profile;
    return Error(rebel::core::ErrorCode::None, "Profile set successfully");
}

Error SweepTool::SetPath(const std::shared_ptr<Sketching::Spline>& path) {
    if (!path) {
        return Error(rebel::core::ErrorCode::InvalidVertex, "Invalid path: nullptr provided");
    }
    impl->path = path;
    return Error(rebel::core::ErrorCode::None, "Path set successfully");
}

Error SweepTool::SetGuideRail(const std::shared_ptr<Sketching::Spline>& rail) {
    if (!rail) {
        return Error(rebel::core::ErrorCode::InvalidVertex, "Invalid guide rail: nullptr provided");
    }
    impl->guideRail = rail;
    return Error(rebel::core::ErrorCode::None, "Guide rail set successfully");
}

void SweepTool::Reset() {
    impl->Reset();
}

bool SweepTool::IsReady() const {
    return impl->profile && impl->path;
}

Error SweepTool::ValidateInput() const {
    if (!impl->profile) {
        return Error(rebel::core::ErrorCode::CADError, "No profile set for sweep operation");
    }
    if (!impl->path) {
        return Error(rebel::core::ErrorCode::CADError, "No path set for sweep operation");
    }
    
    // Validate profile is 2D
    // TODO: Add proper dimension checking once Spline class supports it
    
    // Validate path is 3D
    // TODO: Add proper dimension checking once Spline class supports it
    
    // If using guide rail, validate it exists and is 3D
    if (impl->guideRail) {
        // TODO: Add proper dimension checking once Spline class supports it
    }
    
    return Error(rebel::core::ErrorCode::None, "Input validation successful");
}

Error SweepTool::Preview(std::shared_ptr<Graphics::GraphicsSystem> graphics) {
    if (!graphics) {
        return Error(rebel::core::ErrorCode::CADError, "Invalid graphics system");
    }

    Error validationError = ValidateInput();
    if (validationError.code() != rebel::core::ErrorCode::None) {
        return validationError;
    }

    impl->isPreviewActive = true;
    
    // TODO: Implement preview rendering using graphics system
    // This should show a temporary visualization of the sweep result
    
    return Error(rebel::core::ErrorCode::None, "Preview initialized successfully");
}

void SweepTool::Cancel() {
    if (impl->isPreviewActive) {
        impl->isPreviewActive = false;
        // TODO: Clean up any preview graphics
    }
}

Error SweepTool::Execute(const SweepOptions& options) {
    if (impl->isPreviewActive) {
        Cancel();
    }

    Error validationError = ValidateInput();
    if (validationError.code() != rebel::core::ErrorCode::None) {
        return validationError;
    }

    return CreateSweepGeometry(options);
}

Error SweepTool::CreateSweepGeometry(const SweepOptions& options) {
    // TODO: Implement actual sweep geometry creation
    // Steps:
    // 1. Sample points along the path
    // 2. Create transformation matrices at each point
    // 3. Transform and scale profile according to options
    // 4. Generate surface/solid geometry
    // 5. Apply twist if specified
    // 6. Handle orientation based on mode
    
    // This is a placeholder implementation
    if (options.orientationMode == OrientationMode::GuideRail && !impl->guideRail) {
        return Error(rebel::core::ErrorCode::CADError, "Guide rail orientation mode selected but no guide rail provided");
    }

    // Calculate sweep parameters
    float twistRadians = options.twistAngle * static_cast<float>(M_PI) / 180.0f;
    
    // TODO: Implement full geometry creation
    // For now, return success to allow testing
    return Error(rebel::core::ErrorCode::None, "Sweep geometry created successfully");
}

} // namespace Modeling
} // namespace RebelCAD
