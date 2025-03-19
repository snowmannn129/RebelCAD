#include "modeling/NURBSCurve.h"
#include "modeling/NURBSCurveImpl.h"
#include "graphics/PreviewRenderer.h"
#include "core/Error.h"
#include "graphics/GraphicsPreviewFwd.h"

namespace RebelCAD {
namespace Modeling {

NURBSCurve::ErrorCode NURBSCurve::Preview(std::shared_ptr<RebelCAD::Graphics::GraphicsSystem> graphics) {
    if (!impl->ValidateConfiguration()) {
        return ErrorCode::CADError;
    }

    // Check if we can use cached preview
    if (impl->preview_cache.valid &&
        impl->preview_cache.cached_points == impl->control_points &&
        impl->preview_cache.cached_weights == impl->weights &&
        impl->preview_cache.cached_params.degree == impl->params.degree &&
        impl->preview_cache.cached_params.rational == impl->params.rational &&
        impl->preview_cache.cached_params.tolerance == impl->params.tolerance) {
        
        // Use cached geometry
        RebelCAD::Graphics::PreviewRenderer::RenderCurveWithAttributes(
            graphics,
            impl->preview_cache.geometry.vertices.data(),
            impl->preview_cache.geometry.tangents.data(),
            impl->preview_cache.geometry.indices.data(),
            impl->preview_cache.geometry.vertices.size() / 3,
            impl->preview_cache.geometry.indices.size()
        );

        // Render control points and polygon
        RebelCAD::Graphics::PreviewRenderer::RenderControlPoints(
            graphics,
            reinterpret_cast<const float*>(impl->control_points.data()),
            impl->control_points.size()
        );

        return ErrorCode::None;
    }

    // Generate preview geometry
    if (!impl->GenerateGeometry(32)) {  // Use fixed segment count for preview
        return ErrorCode::CADError;
    }

    // Cache the preview data
    impl->preview_cache.valid = true;
    impl->preview_cache.cached_points = impl->control_points;
    impl->preview_cache.cached_weights = impl->weights;
    impl->preview_cache.cached_params = impl->params;
    impl->preview_cache.geometry = impl->result;

    // Render curve
    RebelCAD::Graphics::PreviewRenderer::RenderCurveWithAttributes(
        graphics,
        impl->result.vertices.data(),
        impl->result.tangents.data(),
        impl->result.indices.data(),
        impl->result.vertices.size() / 3,
        impl->result.indices.size()
    );

    // Render control points and polygon
    RebelCAD::Graphics::PreviewRenderer::RenderControlPoints(
        graphics,
        reinterpret_cast<const float*>(impl->control_points.data()),
        impl->control_points.size()
    );

    return ErrorCode::None;
}

} // namespace Modeling
} // namespace RebelCAD
