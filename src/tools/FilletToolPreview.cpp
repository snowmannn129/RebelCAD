#include "modeling/FilletTool.h"
#include "modeling/FilletToolImpl.h"
#include "graphics/PreviewRenderer.h"
#include "core/Error.h"
#include "graphics/GraphicsPreviewFwd.h"

namespace RebelCAD {
namespace Modeling {

FilletTool::ErrorCode FilletTool::Preview(std::shared_ptr<RebelCAD::Graphics::GraphicsSystem> graphics) {
    if (!impl->has_mesh || !impl->is_configured) {
        return ErrorCode::CADError;
    }

    // Check if we can use cached preview
    if (impl->preview_cache.valid &&
        impl->preview_cache.cached_vertices == impl->mesh_vertices &&
        impl->preview_cache.cached_indices == impl->mesh_indices &&
        impl->preview_cache.cached_edges == impl->edges_to_fillet &&
        impl->preview_cache.cached_faces == impl->faces_to_fillet &&
        impl->preview_cache.cached_params.radius == impl->params.radius &&
        impl->preview_cache.cached_params.segments == impl->params.segments &&
        impl->preview_cache.cached_params.variable_radius == impl->params.variable_radius) {
        
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
    ErrorCode err = Execute();
    if (err != ErrorCode::None) {
        return err;
    }

    // Cache the preview data
    impl->preview_cache.valid = true;
    impl->preview_cache.cached_vertices = impl->mesh_vertices;
    impl->preview_cache.cached_indices = impl->mesh_indices;
    impl->preview_cache.cached_edges = impl->edges_to_fillet;
    impl->preview_cache.cached_faces = impl->faces_to_fillet;
    impl->preview_cache.cached_params = impl->params;
    impl->preview_cache.mesh_vertices = impl->result_vertices;
    impl->preview_cache.mesh_normals = impl->result_normals;
    impl->preview_cache.mesh_uvs = impl->result_uvs;
    impl->preview_cache.mesh_indices = impl->result_indices;

    // Render the mesh with all attributes
    RebelCAD::Graphics::PreviewRenderer::RenderMeshWithAttributes(
        graphics,
        impl->result_vertices.data(),
        impl->result_normals.data(),
        impl->result_uvs.data(),
        impl->result_indices.data(),
        impl->result_vertices.size() / 3,
        impl->result_indices.size()
    );

    return ErrorCode::None;
}

} // namespace Modeling
} // namespace RebelCAD
