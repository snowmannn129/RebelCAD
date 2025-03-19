#include "modeling/RevolveTool.h"
#include "modeling/RevolveToolImpl.h"
#include "graphics/PreviewRenderer.h"
#include "core/Error.h"
#include "graphics/GraphicsPreviewFwd.h"

namespace RebelCAD {
namespace Modeling {

RevolveTool::ErrorCode RevolveTool::Preview(std::shared_ptr<RebelCAD::Graphics::GraphicsSystem> graphics) {
    if (!impl->has_profile || !impl->is_configured) {
        return ErrorCode::CADError;
    }

    // Check if we can use cached preview
    if (impl->preview_cache.valid &&
        impl->preview_cache.cached_profile == impl->profile_vertices &&
        impl->preview_cache.cached_params.angle == impl->params.angle &&
        impl->preview_cache.cached_params.axis_start == impl->params.axis_start &&
        impl->preview_cache.cached_params.axis_end == impl->params.axis_end &&
        impl->preview_cache.cached_params.segments == impl->params.segments &&
        impl->preview_cache.cached_params.sections == impl->params.sections) {
        
        // Use cached mesh data
        RebelCAD::Graphics::PreviewRenderer::RenderMeshWithAttributes(
            graphics,
            impl->preview_cache.mesh_vertices.data(),
            impl->preview_cache.mesh_normals.data(),
            impl->preview_cache.mesh_uvs.data(),
            impl->preview_cache.mesh_indices.data(),
            impl->preview_cache.mesh_vertices.size() / 3,
            impl->preview_cache.mesh_indices.size()
        );

        return ErrorCode::None;
    }

    // Generate preview mesh
    ErrorCode err = GenerateMesh();
    if (err != ErrorCode::None) {
        return err;
    }

    // Cache the preview data
    impl->preview_cache.valid = true;
    impl->preview_cache.cached_profile = impl->profile_vertices;
    impl->preview_cache.cached_params = impl->params;
    impl->preview_cache.mesh_vertices = impl->mesh_vertices;
    impl->preview_cache.mesh_normals = impl->mesh_normals;
    impl->preview_cache.mesh_uvs = impl->mesh_uvs;
    impl->preview_cache.mesh_indices = impl->mesh_indices;

    // Render the mesh with all attributes
    RebelCAD::Graphics::PreviewRenderer::RenderMeshWithAttributes(
        graphics,
        impl->mesh_vertices.data(),
        impl->mesh_normals.data(),
        impl->mesh_uvs.data(),
        impl->mesh_indices.data(),
        impl->mesh_vertices.size() / 3,
        impl->mesh_indices.size()
    );

    return ErrorCode::None;
}

} // namespace Modeling
} // namespace RebelCAD
