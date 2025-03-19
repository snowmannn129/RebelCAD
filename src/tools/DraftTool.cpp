#include "modeling/DraftTool.h"
#include "core/Log.h"
#include <cmath>
#include <array>
#include <unordered_map>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace RebelCAD {
namespace Modeling {

class DraftTool::Implementation {
public:
    Implementation() {}
    ~Implementation() {}

    Error ValidateParams(const DraftParams& params) {
        // Check angle is within valid range (-89 to 89 degrees)
        if (std::abs(params.angle) >= 89.0) {
            return Error(rebel::core::ErrorCode::CADError, "Draft angle must be between -89 and 89 degrees");
        }

        // Validate pull direction is not zero vector
        float length = std::sqrt(
            params.pull_direction[0] * params.pull_direction[0] +
            params.pull_direction[1] * params.pull_direction[1] +
            params.pull_direction[2] * params.pull_direction[2]
        );
        if (length < 1e-6) {
            return Error(rebel::core::ErrorCode::CADError, "Pull direction cannot be zero vector");
        }

        return Error(rebel::core::ErrorCode::None, "Parameters validated successfully");
    }

    std::array<float, 3> NormalizePullDirection(const float pull_direction[3]) {
        float length = std::sqrt(
            pull_direction[0] * pull_direction[0] +
            pull_direction[1] * pull_direction[1] +
            pull_direction[2] * pull_direction[2]
        );

        return {
            pull_direction[0] / length,
            pull_direction[1] / length,
            pull_direction[2] / length
        };
    }

    bool IsFaceDraftable(const std::shared_ptr<SolidBody>& face, const float pull_direction[3]) const {
        const auto& topology = face->GetTopology();
        
        // Check if face normal is not parallel to pull direction
        auto normal = CalculateFaceNormal(topology);
        float dot_product = 
            normal[0] * pull_direction[0] +
            normal[1] * pull_direction[1] +
            normal[2] * pull_direction[2];
        
        // Face is draftable if not perpendicular to pull direction
        return std::abs(dot_product) > 1e-6;
    }

    std::array<float, 3> CalculateFaceNormal(const Topology& topology) const {
        // Simple normal calculation for planar face
        // In a real implementation, this would handle non-planar faces
        if (topology.faces.empty() || topology.vertices.size() < 3) {
            return {0.0f, 1.0f, 0.0f}; // Default to up vector if invalid
        }

        const auto& face = topology.faces[0];
        const auto& v0 = topology.vertices[0];
        const auto& v1 = topology.vertices[1];
        const auto& v2 = topology.vertices[2];

        // Calculate cross product of two edges
        float ux = v1.x - v0.x;
        float uy = v1.y - v0.y;
        float uz = v1.z - v0.z;
        
        float vx = v2.x - v0.x;
        float vy = v2.y - v0.y;
        float vz = v2.z - v0.z;

        float nx = uy * vz - uz * vy;
        float ny = uz * vx - ux * vz;
        float nz = ux * vy - uy * vx;

        float length = std::sqrt(nx * nx + ny * ny + nz * nz);
        return {nx / length, ny / length, nz / length};
    }
};

// Main class implementation
DraftTool::DraftTool() : impl(std::make_unique<Implementation>()) {}
DraftTool::~DraftTool() = default;

Error DraftTool::ApplyDraft(
    const std::vector<std::shared_ptr<SolidBody>>& faces,
    const DraftParams& params
) {
    if (faces.empty()) {
        return Error(rebel::core::ErrorCode::InvalidVertex, "No faces provided for draft operation");
    }

    // Validate parameters
    Error validation_error = impl->ValidateParams(params);
    if (validation_error.code() != rebel::core::ErrorCode::None) {
        return validation_error;
    }

    // Check if all faces can be drafted
    if (!CanApplyDraft(faces)) {
        return Error(rebel::core::ErrorCode::CADError, "One or more faces cannot have draft applied");
    }

    // Normalize pull direction
    auto normalized_dir = impl->NormalizePullDirection(params.pull_direction);

    // Create modified bodies
    std::vector<std::shared_ptr<SolidBody>> modified_bodies;
    modified_bodies.reserve(faces.size());

    // Apply draft to each face
    for (const auto& face : faces) {
        // Get the topology
        Topology topology = face->GetTopology();
        
        // Calculate draft transformation matrix based on angle and pull direction
        double angle_rad = params.angle * M_PI / 180.0;
        double tan_angle = std::tan(angle_rad);

        // Create modified vertices
        std::vector<Point3D> new_vertices;
        new_vertices.reserve(topology.vertices.size());

        for (const auto& vertex : topology.vertices) {
            Point3D new_vertex = vertex;
            
            // Calculate height relative to neutral plane
            double height = vertex.y - params.neutral_plane;
            
            // Apply draft transformation
            if (params.is_internal) {
                new_vertex.x -= height * tan_angle * normalized_dir[0];
                new_vertex.z -= height * tan_angle * normalized_dir[2];
            } else {
                new_vertex.x += height * tan_angle * normalized_dir[0];
                new_vertex.z += height * tan_angle * normalized_dir[2];
            }
            
            new_vertices.push_back(new_vertex);
        }

        // Create new topology with modified vertices
        Topology new_topology = topology;
        new_topology.vertices = std::move(new_vertices);

        // Create new solid body with modified topology
        auto modified_body = SolidBody::FromTopology(new_topology);
        if (!modified_body) {
            return Error(rebel::core::ErrorCode::CADError, "Failed to create modified body");
        }
        modified_bodies.push_back(modified_body);
    }

    // Create compound body from all modified faces
    auto result = SolidBody::CreateCompound(modified_bodies);
    if (!result) {
        return Error(rebel::core::ErrorCode::CADError, "Failed to create compound body");
    }

    return Error(rebel::core::ErrorCode::None, "Draft operation completed successfully");
}

bool DraftTool::CanApplyDraft(const std::vector<std::shared_ptr<SolidBody>>& faces) const {
    if (faces.empty()) return false;

    for (const auto& face : faces) {
    const float up_dir[3] = {0, 1, 0};
    const float down_dir[3] = {0, -1, 0};
    if (!impl->IsFaceDraftable(face, face->GetTopology().vertices[0].y > 0 ? up_dir : down_dir)) {
            return false;
        }
    }
    return true;
}

double DraftTool::GetMinimumDraftAngle(const std::string& material_type) const {
    static const std::unordered_map<std::string, double> min_angles = {
        {"plastic", 0.5},    // 0.5° minimum for plastic
        {"aluminum", 1.0},   // 1° minimum for aluminum
        {"steel", 1.5},      // 1.5° minimum for steel
        {"zinc", 0.75}       // 0.75° minimum for zinc
    };

    auto it = min_angles.find(material_type);
    return it != min_angles.end() ? it->second : 1.0; // Default to 1° if material not found
}

void DraftTool::ValidateAngle(double angle) const {
    if (std::abs(angle) >= 89.0) {
        throw std::invalid_argument("Draft angle must be between -89 and 89 degrees");
    }
}

std::array<float, 3> DraftTool::CalculateDraftDirection(
    const std::shared_ptr<SolidBody>& face,
    const float pull_direction[3]
) const {
    return impl->NormalizePullDirection(pull_direction);
}

} // namespace Modeling
} // namespace RebelCAD
