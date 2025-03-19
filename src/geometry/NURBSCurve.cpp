#include "modeling/NURBSCurve.h"
#include "modeling/NURBSCurveImpl.h"
#include "graphics/IPreviewRenderer.h"
#include "graphics/PreviewRenderer.h"
#include "graphics/GraphicsTypes.h"
#include <algorithm>
#include <glm/gtx/norm.hpp>

namespace RebelCAD {
namespace Modeling {

NURBSCurve::NURBSCurve() : impl(std::make_unique<Impl>()) {}
NURBSCurve::~NURBSCurve() = default;

NURBSCurve::ErrorCode NURBSCurve::SetControlPoints(
    const std::vector<glm::vec3>& points) {
    
    if (points.empty()) {
        return ErrorCode::InvalidMesh;
    }

    // Check for valid points
    for (const auto& point : points) {
        if (!std::isfinite(point.x) || 
            !std::isfinite(point.y) || 
            !std::isfinite(point.z)) {
            return ErrorCode::InvalidMesh;
        }
    }

    impl->control_points = points;
    impl->Clear();

    return ErrorCode::None;
}

NURBSCurve::ErrorCode NURBSCurve::SetWeights(
    const std::vector<float>& weights) {
    
    if (weights.empty()) {
        return ErrorCode::InvalidMesh;
    }

    // Check for valid weights
    for (float w : weights) {
        if (w <= 0.0f || !std::isfinite(w)) {
            return ErrorCode::InvalidMesh;
        }
    }

    impl->weights = weights;
    impl->Clear();

    return ErrorCode::None;
}

NURBSCurve::ErrorCode NURBSCurve::Configure(
    const NURBSCurveParams& params) {
    
    // Validate parameters
    if (params.degree < 1) {
        return ErrorCode::InvalidMesh;
    }
    if (params.tolerance <= 0.0f) {
        return ErrorCode::InvalidMesh;
    }

    impl->params = params;
    impl->is_configured = true;
    impl->Clear();

    // Initialize knot vector
    KnotVectorParams knot_params;
    knot_params.uniform = false;  // NURBS typically use non-uniform knots
    knot_params.tolerance = params.tolerance;
    knot_params.normalize = true;
    knot_params.min_multiplicity = 1;
    knot_params.max_multiplicity = params.degree + 1;  // For end point interpolation

    if (impl->knot_vector->Configure(knot_params) != KnotVector::ErrorCode::None) {
        return ErrorCode::CADError;
    }

    return ErrorCode::None;
}

glm::vec3 NURBSCurve::Evaluate(float t) const {
    if (!impl->ValidateConfiguration()) {
        return glm::vec3(0.0f);
    }

    if (!std::isfinite(t)) {
        return glm::vec3(0.0f);
    }

    return impl->Evaluate(t);
}

std::vector<glm::vec3> NURBSCurve::EvaluateDerivatives(
    float t, int order) const {
    
    if (!impl->ValidateConfiguration()) {
        return {};
    }

    if (!std::isfinite(t) || order < 1 || order > impl->params.degree) {
        return {};
    }

    return impl->EvaluateDerivatives(t, order);
}

NURBSCurve::ErrorCode NURBSCurve::GenerateGeometry(int segments) {
    if (!impl->ValidateConfiguration()) {
        return ErrorCode::CADError;
    }

    if (segments < 1) {
        return ErrorCode::InvalidMesh;
    }

    if (!impl->GenerateGeometry(segments)) {
        return ErrorCode::CADError;
    }

    return ErrorCode::None;
}

const std::vector<float>& NURBSCurve::GetResultVertices() const {
    return impl->result.vertices;
}

const std::vector<float>& NURBSCurve::GetResultTangents() const {
    return impl->result.tangents;
}

const std::vector<float>& NURBSCurve::GetResultNormals() const {
    return impl->result.normals;
}

const std::vector<unsigned int>& NURBSCurve::GetResultIndices() const {
    return impl->result.indices;
}

NURBSCurve::ErrorCode NURBSCurve::ElevateDegree(int target_degree) {
    if (!impl->ValidateConfiguration()) {
        return ErrorCode::CADError;
    }

    if (target_degree <= impl->params.degree) {
        return ErrorCode::InvalidMesh;
    }

    if (!impl->ElevateDegree(target_degree)) {
        return ErrorCode::CADError;
    }
    return ErrorCode::None;
}

NURBSCurve::ErrorCode NURBSCurve::ReduceDegree(int target_degree) {
    if (!impl->ValidateConfiguration()) {
        return ErrorCode::CADError;
    }

    if (target_degree >= impl->params.degree || target_degree < 1) {
        return ErrorCode::InvalidMesh;
    }

    if (!impl->ReduceDegree(target_degree)) {
        return ErrorCode::CADError;
    }
    return ErrorCode::None;
}

NURBSCurve::ErrorCode NURBSCurve::InsertKnot(float t, int multiplicity) {
    if (!impl->ValidateConfiguration()) {
        return ErrorCode::CADError;
    }

    if (!std::isfinite(t) || multiplicity < 1) {
        return ErrorCode::InvalidMesh;
    }

    if (!impl->InsertKnot(t, multiplicity)) {
        return ErrorCode::CADError;
    }
    return ErrorCode::None;
}

NURBSCurve::ErrorCode NURBSCurve::RemoveKnot(float t, int multiplicity) {
    if (!impl->ValidateConfiguration()) {
        return ErrorCode::CADError;
    }

    if (!std::isfinite(t) || multiplicity < 1) {
        return ErrorCode::InvalidMesh;
    }

    if (!impl->RemoveKnot(t, multiplicity)) {
        return ErrorCode::CADError;
    }
    return ErrorCode::None;
}

bool NURBSCurve::IsValid() const {
    return impl->ValidateConfiguration();
}

int NURBSCurve::GetDegree() const {
    return impl->params.degree;
}

NURBSCurve::ErrorCode NURBSCurve::Preview(std::shared_ptr<Graphics::GraphicsSystem> graphics) const {
    if (!impl->ValidateConfiguration()) {
        return ErrorCode::CADError;
    }

    if (!graphics) {
        return ErrorCode::InvalidMesh;
    }

    // Generate geometry for preview
    if (!impl->GenerateGeometry(32)) {  // Use 32 segments for preview
        return ErrorCode::CADError;
    }

    // Get preview renderer
    auto renderer = std::dynamic_pointer_cast<Graphics::IPreviewRenderer>(graphics);
    if (!renderer) {
        return ErrorCode::CADError;
    }

    // Helper function to create quad vertices
    auto createQuad = [](const glm::vec3& center, float width, float height) {
        std::vector<float> vertices;
        vertices.reserve(12);  // 4 vertices * 3 components
        
        vertices.push_back(center.x - width/2);
        vertices.push_back(center.y - height/2);
        vertices.push_back(center.z);
        
        vertices.push_back(center.x + width/2);
        vertices.push_back(center.y - height/2);
        vertices.push_back(center.z);
        
        vertices.push_back(center.x + width/2);
        vertices.push_back(center.y + height/2);
        vertices.push_back(center.z);
        
        vertices.push_back(center.x - width/2);
        vertices.push_back(center.y + height/2);
        vertices.push_back(center.z);
        
        return vertices;
    };

    // Helper function to create quad indices
    auto createQuadIndices = [](size_t baseVertex) {
        unsigned int base = static_cast<unsigned int>(baseVertex);
        return std::vector<unsigned int>{
            base, base + 1, base + 2,
            base, base + 2, base + 3
        };
    };

    // Helper function to create quad normals
    auto createQuadNormals = [](const glm::vec3& normal) {
        std::vector<float> normals;
        normals.reserve(12);  // 4 vertices * 3 components
        for (int i = 0; i < 4; ++i) {
            normals.push_back(normal.x);
            normals.push_back(normal.y);
            normals.push_back(normal.z);
        }
        return normals;
    };

    // Helper function to create line segment geometry
    auto createLineSegment = [](const glm::vec3& start, const glm::vec3& end, float width) {
        std::vector<float> vertices;
        vertices.reserve(12);  // 4 vertices * 3 components

        glm::vec3 dir = glm::normalize(end - start);
        glm::vec3 up(0.0f, 0.0f, 1.0f);
        glm::vec3 right = glm::normalize(glm::cross(dir, up)) * width;

        // First point
        vertices.push_back(start.x - right.x);
        vertices.push_back(start.y - right.y);
        vertices.push_back(start.z);
        
        vertices.push_back(start.x + right.x);
        vertices.push_back(start.y + right.y);
        vertices.push_back(start.z);
        
        // Second point
        vertices.push_back(end.x + right.x);
        vertices.push_back(end.y + right.y);
        vertices.push_back(end.z);
        
        vertices.push_back(end.x - right.x);
        vertices.push_back(end.y - right.y);
        vertices.push_back(end.z);

        return vertices;
    };

    // Helper function to create arrow head geometry
    auto createArrowHead = [](const glm::vec3& pos, const glm::vec3& dir, float size) {
        std::vector<float> vertices;
        vertices.reserve(18);  // 6 vertices * 3 components

        glm::vec3 up(0.0f, 0.0f, 1.0f);
        glm::vec3 right = glm::normalize(glm::cross(dir, up)) * size;
        glm::vec3 tip = pos + dir * size * 2.0f;

        // Create two triangles for double-sided arrow head
        // First triangle (front face)
        vertices.push_back(pos.x - right.x);
        vertices.push_back(pos.y - right.y);
        vertices.push_back(pos.z);
        
        vertices.push_back(pos.x + right.x);
        vertices.push_back(pos.y + right.y);
        vertices.push_back(pos.z);
        
        vertices.push_back(tip.x);
        vertices.push_back(tip.y);
        vertices.push_back(tip.z);

        // Second triangle (back face)
        vertices.push_back(pos.x + right.x);
        vertices.push_back(pos.y + right.y);
        vertices.push_back(pos.z);
        
        vertices.push_back(pos.x - right.x);
        vertices.push_back(pos.y - right.y);
        vertices.push_back(pos.z);
        
        vertices.push_back(tip.x);
        vertices.push_back(tip.y);
        vertices.push_back(tip.z);

        return vertices;
    };

    // Constants for visualization
    const float depth_offset = 0.001f;  // Base depth offset
    const glm::vec4 curve_color(0.0f, 0.7f, 1.0f, 1.0f);  // Blue
    const glm::vec4 control_color(1.0f, 0.3f, 0.0f, 0.5f);  // Semi-transparent orange
    const glm::vec4 point_color(1.0f, 0.3f, 0.0f, 1.0f);  // Solid orange
    const glm::vec4 knot_color(0.2f, 0.8f, 0.2f, 1.0f);  // Green
    const glm::vec4 param_color(0.8f, 0.8f, 0.2f, 1.0f);  // Yellow
    const glm::vec4 tangent_color(0.7f, 0.0f, 0.7f, 1.0f);  // Purple
    const glm::vec4 weight_color(0.0f, 0.5f, 0.5f, 1.0f);  // Teal

    // Draw curve (slightly offset forward for better visibility)
    std::vector<float> curve_vertices = impl->result.vertices;
    std::vector<float> curve_colors;
    curve_colors.reserve(curve_vertices.size() * 4 / 3);  // 4 components per vertex

    for (size_t i = 0; i < curve_vertices.size(); i += 3) {
        curve_vertices[i + 2] += depth_offset;  // Z offset
        curve_colors.push_back(curve_color.r);
        curve_colors.push_back(curve_color.g);
        curve_colors.push_back(curve_color.b);
        curve_colors.push_back(curve_color.a);
    }

    renderer->beginPreview();
    renderer->renderTriangleMeshWithAttributes(
        curve_vertices.data(),
        impl->result.tangents.data(),
        curve_colors.data(),  // Use colors as UVs
        impl->result.indices.data(),
        curve_vertices.size() / 3,
        impl->result.indices.size()
    );
    renderer->endPreview();

    // Draw control points and polygon
    const float point_size = 0.1f;
    const float line_width = 0.05f;
    std::vector<float> control_vertices;
    std::vector<float> control_normals;
    std::vector<float> control_colors;
    std::vector<unsigned int> control_indices;

    // Control polygon
    for (size_t i = 0; i < impl->control_points.size() - 1; ++i) {
        auto line_verts = createLineSegment(
            impl->control_points[i],
            impl->control_points[i + 1],
            line_width
        );
        auto line_norms = createQuadNormals({0.0f, 0.0f, 1.0f});
        auto line_indices = createQuadIndices(control_vertices.size() / 3);

        // Add colors for line segment
        for (int j = 0; j < 4; ++j) {  // 4 vertices per quad
            control_colors.push_back(control_color.r);
            control_colors.push_back(control_color.g);
            control_colors.push_back(control_color.b);
            control_colors.push_back(control_color.a);
        }

        control_vertices.insert(control_vertices.end(), line_verts.begin(), line_verts.end());
        control_normals.insert(control_normals.end(), line_norms.begin(), line_norms.end());
        control_indices.insert(control_indices.end(), line_indices.begin(), line_indices.end());
    }

    // Control points
    for (const auto& point : impl->control_points) {
        glm::vec3 offset_point = point;
        offset_point.z += depth_offset * 2.0f;  // Ensure points are visible
        
        auto quad_verts = createQuad(offset_point, point_size, point_size);
        auto quad_norms = createQuadNormals({0.0f, 0.0f, 1.0f});
        auto quad_indices = createQuadIndices(control_vertices.size() / 3);

        // Add colors for point quad
        for (int j = 0; j < 4; ++j) {  // 4 vertices per quad
            control_colors.push_back(point_color.r);
            control_colors.push_back(point_color.g);
            control_colors.push_back(point_color.b);
            control_colors.push_back(point_color.a);
        }

        control_vertices.insert(control_vertices.end(), quad_verts.begin(), quad_verts.end());
        control_normals.insert(control_normals.end(), quad_norms.begin(), quad_norms.end());
        control_indices.insert(control_indices.end(), quad_indices.begin(), quad_indices.end());
    }

    renderer->beginPreview();
    renderer->renderTriangleMeshWithAttributes(
        control_vertices.data(),
        control_normals.data(),
        control_colors.data(),  // Use colors as UVs
        control_indices.data(),
        control_vertices.size() / 3,
        control_indices.size()
    );
    renderer->endPreview();

    // Draw knots and parameter markers
    const float knot_size = 0.08f;
    const float param_size = 0.05f;
    const float marker_depth = depth_offset * 3.0f;  // Above curve and control points
    std::vector<float> marker_vertices;
    std::vector<float> marker_normals;
    std::vector<float> marker_colors;
    std::vector<unsigned int> marker_indices;

    // Draw knots
    for (float t : impl->knot_vector->GetKnots()) {
        glm::vec3 pos = impl->Evaluate(t);
        pos.z += marker_depth;  // Offset for visibility
        auto quad_verts = createQuad(pos, knot_size, knot_size);
        auto quad_norms = createQuadNormals({0.0f, 0.0f, 1.0f});
        auto quad_indices = createQuadIndices(marker_vertices.size() / 3);

        // Add colors for knot quad
        for (int j = 0; j < 4; ++j) {  // 4 vertices per quad
            marker_colors.push_back(knot_color.r);
            marker_colors.push_back(knot_color.g);
            marker_colors.push_back(knot_color.b);
            marker_colors.push_back(knot_color.a);
        }

        marker_vertices.insert(marker_vertices.end(), quad_verts.begin(), quad_verts.end());
        marker_normals.insert(marker_normals.end(), quad_norms.begin(), quad_norms.end());
        marker_indices.insert(marker_indices.end(), quad_indices.begin(), quad_indices.end());
    }

    // Draw parameter markers
    const int num_params = 10;
    for (int i = 0; i <= num_params; ++i) {
        float t = static_cast<float>(i) / num_params;
        glm::vec3 pos = impl->Evaluate(t);
        pos.z += marker_depth;  // Offset for visibility
        auto quad_verts = createQuad(pos, param_size, param_size);
        auto quad_norms = createQuadNormals({0.0f, 0.0f, 1.0f});
        auto quad_indices = createQuadIndices(marker_vertices.size() / 3);

        // Add colors for parameter quad
        for (int j = 0; j < 4; ++j) {  // 4 vertices per quad
            marker_colors.push_back(param_color.r);
            marker_colors.push_back(param_color.g);
            marker_colors.push_back(param_color.b);
            marker_colors.push_back(param_color.a);
        }

        marker_vertices.insert(marker_vertices.end(), quad_verts.begin(), quad_verts.end());
        marker_normals.insert(marker_normals.end(), quad_norms.begin(), quad_norms.end());
        marker_indices.insert(marker_indices.end(), quad_indices.begin(), quad_indices.end());
    }

    renderer->beginPreview();
    renderer->renderTriangleMeshWithAttributes(
        marker_vertices.data(),
        marker_normals.data(),
        marker_colors.data(),  // Use colors as UVs
        marker_indices.data(),
        marker_vertices.size() / 3,
        marker_indices.size()
    );
    renderer->endPreview();

    // Draw tangents with arrow heads
    const float tangent_length = 0.2f;
    const float tangent_width = 0.02f;
    const float arrow_size = 0.05f;
    const float tangent_depth = marker_depth + depth_offset;  // Above markers
    std::vector<float> tangent_vertices;
    std::vector<float> tangent_normals;
    std::vector<float> tangent_colors;
    std::vector<unsigned int> tangent_indices;

    for (int i = 0; i <= num_params; ++i) {
        float t = static_cast<float>(i) / num_params;
        glm::vec3 pos = impl->Evaluate(t);
        pos.z += tangent_depth;  // Offset for visibility
        
        auto derivatives = impl->EvaluateDerivatives(t, 1);
        if (!derivatives.empty()) {
            glm::vec3 tangent = glm::normalize(derivatives[0]);
            glm::vec3 end = pos + tangent * tangent_length;
            end.z += tangent_depth;  // Offset for visibility
            
            // Create line segment for tangent
            auto line_verts = createLineSegment(pos, end, tangent_width);
            auto line_norms = createQuadNormals({0.0f, 0.0f, 1.0f});
            auto line_indices = createQuadIndices(tangent_vertices.size() / 3);

            // Add colors for line segment
            for (int j = 0; j < 4; ++j) {  // 4 vertices per quad
                tangent_colors.push_back(tangent_color.r);
                tangent_colors.push_back(tangent_color.g);
                tangent_colors.push_back(tangent_color.b);
                tangent_colors.push_back(tangent_color.a);
            }

            tangent_vertices.insert(tangent_vertices.end(), line_verts.begin(), line_verts.end());
            tangent_normals.insert(tangent_normals.end(), line_norms.begin(), line_norms.end());
            tangent_indices.insert(tangent_indices.end(), line_indices.begin(), line_indices.end());

            // Create arrow head
            auto arrow_verts = createArrowHead(end - tangent * arrow_size, tangent, arrow_size);
            
            // Create indices for both triangles
            unsigned int base_idx = static_cast<unsigned int>(tangent_vertices.size() / 3);
            std::vector<unsigned int> arrow_indices = {
                base_idx, base_idx + 1, base_idx + 2,  // Front face
                base_idx + 3, base_idx + 4, base_idx + 5  // Back face
            };

            // Create normals for both triangles
            std::vector<float> arrow_norms;
            arrow_norms.reserve(18);  // 6 vertices * 3 components
            
            // Front face normal
            for (int j = 0; j < 3; ++j) {
                arrow_norms.push_back(0.0f);
                arrow_norms.push_back(0.0f);
                arrow_norms.push_back(1.0f);
            }
            
            // Back face normal
            for (int j = 0; j < 3; ++j) {
                arrow_norms.push_back(0.0f);
                arrow_norms.push_back(0.0f);
                arrow_norms.push_back(-1.0f);
            }

            // Add colors for both triangles
            for (int j = 0; j < 6; ++j) {  // 6 vertices total
                tangent_colors.push_back(tangent_color.r);
                tangent_colors.push_back(tangent_color.g);
                tangent_colors.push_back(tangent_color.b);
                tangent_colors.push_back(tangent_color.a);
            }

            tangent_vertices.insert(tangent_vertices.end(), arrow_verts.begin(), arrow_verts.end());
            tangent_normals.insert(tangent_normals.end(), arrow_norms.begin(), arrow_norms.end());
            tangent_indices.insert(tangent_indices.end(), arrow_indices.begin(), arrow_indices.end());
        }
    }

    renderer->beginPreview();
    renderer->renderTriangleMeshWithAttributes(
        tangent_vertices.data(),
        tangent_normals.data(),
        tangent_colors.data(),  // Use colors as UVs
        tangent_indices.data(),
        tangent_vertices.size() / 3,
        tangent_indices.size()
    );
    renderer->endPreview();

    // Draw weights for rational curves
    if (impl->params.rational && !impl->weights.empty()) {
        const float weight_size = 0.07f;
        const float weight_depth = tangent_depth + depth_offset;  // Above tangents
        std::vector<float> weight_vertices;
        std::vector<float> weight_normals;
        std::vector<float> weight_colors;
        std::vector<unsigned int> weight_indices;

        for (size_t i = 0; i < impl->weights.size(); ++i) {
            float scale = impl->weights[i];
            glm::vec3 pos = impl->control_points[i];
            pos.z += weight_depth;  // Offset for visibility
            
            auto quad_verts = createQuad(pos, weight_size * scale, weight_size * scale);
            auto quad_norms = createQuadNormals({0.0f, 0.0f, 1.0f});
            auto quad_indices = createQuadIndices(weight_vertices.size() / 3);

            // Add colors for weight quad
            for (int j = 0; j < 4; ++j) {  // 4 vertices per quad
                weight_colors.push_back(weight_color.r);
                weight_colors.push_back(weight_color.g);
                weight_colors.push_back(weight_color.b);
                weight_colors.push_back(weight_color.a);
            }

            weight_vertices.insert(weight_vertices.end(), quad_verts.begin(), quad_verts.end());
            weight_normals.insert(weight_normals.end(), quad_norms.begin(), quad_norms.end());
            weight_indices.insert(weight_indices.end(), quad_indices.begin(), quad_indices.end());
        }

        renderer->beginPreview();
        renderer->renderTriangleMeshWithAttributes(
            weight_vertices.data(),
            weight_normals.data(),
            weight_colors.data(),  // Use colors as UVs
            weight_indices.data(),
            weight_vertices.size() / 3,
            weight_indices.size()
        );
        renderer->endPreview();
    }

    return ErrorCode::None;
}

NURBSCurve::ErrorCode NURBSCurve::Split(float t, NURBSCurve& left, NURBSCurve& right) const {
    if (!impl->ValidateConfiguration()) {
        return ErrorCode::CADError;
    }

    if (!std::isfinite(t) || t <= 0.0f || t >= 1.0f) {
        return ErrorCode::InvalidMesh;
    }

    if (!impl->Split(t, left, right)) {
        return ErrorCode::CADError;
    }

    return ErrorCode::None;
}

NURBSCurve::ErrorCode NURBSCurve::Join(const NURBSCurve& other) {
    if (!impl->ValidateConfiguration()) {
        return ErrorCode::CADError;
    }

    if (!other.impl->ValidateConfiguration()) {
        return ErrorCode::CADError;
    }

    if (impl->params.degree != other.impl->params.degree ||
        impl->params.rational != other.impl->params.rational) {
        return ErrorCode::InvalidMesh;
    }

    if (!impl->Join(other)) {
        return ErrorCode::CADError;
    }

    return ErrorCode::None;
}

} // namespace Modeling
} // namespace RebelCAD
