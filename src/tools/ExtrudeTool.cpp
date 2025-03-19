#include "modeling/ExtrudeTool.h"
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>
#include <map>
#include <limits>

namespace rebel_cad {
namespace modeling {

ExtrudeTool::ExtrudeTool() = default;
ExtrudeTool::~ExtrudeTool() = default;

bool ExtrudeTool::validateSections(const std::vector<Section>& sections) const {
    if (sections.empty()) {
        return true;  // No sections is valid (falls back to simple extrusion)
    }

    // Check that heights are strictly increasing
    for (size_t i = 1; i < sections.size(); ++i) {
        if (sections[i].height <= sections[i-1].height) {
            return false;
        }
    }

    // Validate each profile and check compatibility
    const auto& first_profile = sections[0].profile;
    if (!validateProfile(first_profile)) {
        return false;
    }

    // All profiles must have the same number of points and holes
    const size_t boundary_points = first_profile.getBoundary().points.size();
    const size_t hole_count = first_profile.getHoles().size();

    for (size_t i = 1; i < sections.size(); ++i) {
        const auto& profile = sections[i].profile;
        if (!validateProfile(profile)) {
            return false;
        }

        if (profile.getBoundary().points.size() != boundary_points) {
            return false;
        }

        if (profile.getHoles().size() != hole_count) {
            return false;
        }

        // For each hole, verify point count matches
        for (size_t h = 0; h < hole_count; ++h) {
            if (profile.getHoles()[h].points.size() != 
                first_profile.getHoles()[h].points.size()) {
                return false;
            }
        }
    }

    return true;
}

bool ExtrudeTool::validateProfile(const Profile& profile) const {
    // Basic validation
    if (!profile.validate()) {
        return false;
    }

    // Check minimum point count
    const auto& boundary = profile.getBoundary();
    if (boundary.points.size() < 3) {
        return false;
    }

    // Validate holes
    for (const auto& hole : profile.getHoles()) {
        if (hole.points.size() < 3) {
            return false;
        }
    }

    return true;
}

std::shared_ptr<Mesh> ExtrudeTool::createBaseMesh(const Profile& profile) {
    auto mesh = std::make_shared<Mesh>();

    // Triangulate the profile
    std::vector<glm::vec2> triangles = profile.triangulate();

    // Create vertices for the base face
    for (const auto& point : triangles) {
        mesh->addVertex(glm::vec3(point.x, point.y, 0.0f));
    }

    // Create triangular faces
    for (size_t i = 0; i < triangles.size(); i += 3) {
        mesh->addFace({i, i + 1, i + 2});
    }

    return mesh;
}

void ExtrudeTool::applyDraftAngle(Mesh& mesh, float angle) {
    if (angle == 0.0f) return;

    const float tan_angle = std::tan(angle);
    const size_t vertex_count = mesh.getVertexCount();
    const size_t face_count = mesh.getFaceCount();

    // First identify side faces vs top/bottom faces
    std::vector<bool> is_side_face(face_count, false);
    for (size_t i = 0; i < face_count; ++i) {
        const auto& face = mesh.getFace(i);
        // Side faces have significant Z component in their normal
        is_side_face[i] = std::abs(face.normal.z) < 0.707f; // cos(45°)
    }

    // Create map of vertices to their connected faces
    std::vector<std::vector<size_t>> vertex_faces(vertex_count);
    for (size_t i = 0; i < face_count; ++i) {
        const auto& face = mesh.getFace(i);
        for (size_t idx : face.indices) {
            vertex_faces[idx].push_back(i);
        }
    }

    // Store original positions before modification
    std::vector<glm::vec3> original_positions;
    original_positions.reserve(vertex_count);
    for (size_t i = 0; i < vertex_count; ++i) {
        original_positions.push_back(mesh.getVertex(i).position);
    }

    // Apply draft angle to vertices on side faces
    for (size_t i = 0; i < vertex_count; ++i) {
        const auto& vertex = mesh.getVertex(i);
        const float height = vertex.position.z;
        
        // Skip vertices at z=0 (base)
        if (height <= 0.0f) continue;

        // Check if vertex belongs to any side face
        bool on_side_face = false;
        glm::vec3 avg_normal(0.0f);
        size_t side_face_count = 0;

        for (size_t face_idx : vertex_faces[i]) {
            if (is_side_face[face_idx]) {
                on_side_face = true;
                avg_normal += mesh.getFace(face_idx).normal;
                side_face_count++;
            }
        }

        if (!on_side_face) continue;

        // Average normal of connected side faces
        avg_normal = glm::normalize(avg_normal / static_cast<float>(side_face_count));

        // Project normal onto XY plane for draft direction
        glm::vec3 draft_dir = glm::normalize(glm::vec3(avg_normal.x, avg_normal.y, 0.0f));

        // Calculate draft offset
        glm::vec3 offset = draft_dir * (height * tan_angle);
        
        // Apply offset to vertex position
        glm::vec3 new_pos = original_positions[i] + offset;
        mesh.transform(glm::translate(glm::mat4(1.0f), new_pos - vertex.position));
    }

    // Update mesh topology and recompute normals
    mesh.updateEdgeTopology();
    mesh.computeNormals();
}

void ExtrudeTool::generateUVCoordinates(Mesh& mesh) {
    // Get mesh bounds for UV normalization
    glm::vec3 min_bounds(std::numeric_limits<float>::max());
    glm::vec3 max_bounds(std::numeric_limits<float>::lowest());
    
    for (size_t i = 0; i < mesh.getVertexCount(); ++i) {
        const auto& pos = mesh.getVertex(i).position;
        min_bounds = glm::min(min_bounds, pos);
        max_bounds = glm::max(max_bounds, pos);
    }
    
    glm::vec3 size = max_bounds - min_bounds;
    float height = size.z;
    
    // Group vertices by their z-coordinate (height layers)
    std::map<float, std::vector<size_t>> height_layers;
    for (size_t i = 0; i < mesh.getVertexCount(); ++i) {
        const auto& pos = mesh.getVertex(i).position;
        height_layers[pos.z].push_back(i);
    }
    
    // Process each layer
    size_t layer_count = height_layers.size();
    size_t current_layer = 0;
    
    for (const auto& layer : height_layers) {
        float v = height > 0 ? layer.first / height : 0;
        
        // Process vertices in this layer
        for (size_t vertex_idx : layer.second) {
            const auto& pos = mesh.getVertex(vertex_idx).position;
            
            // Calculate u coordinate based on angle around z-axis
            float angle = std::atan2(pos.y - min_bounds.y, pos.x - min_bounds.x);
            float u = (angle + glm::pi<float>()) / (2.0f * glm::pi<float>());
            
            // Set UV coordinates
            mesh.setVertexUV(vertex_idx, glm::vec2(u, v));
        }
        
        current_layer++;
    }
    
    // Special handling for side faces to ensure texture continuity
    for (size_t i = 0; i < mesh.getFaceCount(); ++i) {
        const auto& face = mesh.getFace(i);
        
        // Check if this is a side face (normal has small z component)
        if (std::abs(face.normal.z) < 0.707f) { // cos(45°)
            // Calculate average u coordinate for the face
            float avg_u = 0.0f;
            for (size_t idx : face.indices) {
                avg_u += mesh.getVertex(idx).uv.x;
            }
            avg_u /= face.indices.size();
            
            // If face wraps around 0/1 boundary, adjust UVs
            bool wraps = false;
            for (size_t idx : face.indices) {
                float u = mesh.getVertex(idx).uv.x;
                if (std::abs(u - avg_u) > 0.5f) {
                    wraps = true;
                    break;
                }
            }
            
            if (wraps) {
                for (size_t idx : face.indices) {
                    auto uv = mesh.getVertex(idx).uv;
                    if (uv.x < 0.5f) {
                        uv.x += 1.0f;
                    }
                    mesh.setVertexUV(idx, uv);
                }
            }
        }
    }
}

bool ExtrudeTool::validateMeshTopology(const Mesh& mesh) const {
    // Check for basic topology issues
    if (mesh.getVertexCount() < 4 || mesh.getFaceCount() < 4) {
        return false;
    }

    // Validate edge connectivity
    const auto& edges = mesh.getEdges();
    for (const auto& edge : edges) {
        if (edge.adjacentFaces.empty() || edge.adjacentFaces.size() > 2) {
            return false;
        }
    }

    return true;
}

Profile ExtrudeTool::interpolateProfiles(const Profile& profile1, const Profile& profile2, float t) const {
    Profile result;
    
    // Interpolate boundary points
    const auto& boundary1 = profile1.getBoundary();
    const auto& boundary2 = profile2.getBoundary();
    
    std::vector<glm::vec2> boundary_points;
    for (size_t i = 0; i < boundary1.points.size(); ++i) {
        boundary_points.push_back(glm::mix(boundary1.points[i], boundary2.points[i], t));
    }
    result.setBoundary(boundary_points);

    // Interpolate holes
    const auto& holes1 = profile1.getHoles();
    const auto& holes2 = profile2.getHoles();
    
    for (size_t h = 0; h < holes1.size(); ++h) {
        std::vector<glm::vec2> hole_points;
        for (size_t i = 0; i < holes1[h].points.size(); ++i) {
            hole_points.push_back(glm::mix(holes1[h].points[i], holes2[h].points[i], t));
        }
        result.addHole(hole_points);
    }

    return result;
}

Profile ExtrudeTool::transformProfile(const Profile& profile, float scale, float twist) const {
    Profile result;
    
    // Create transformation matrix
    glm::mat2 rotation(
        std::cos(twist), -std::sin(twist),
        std::sin(twist), std::cos(twist)
    );
    
    // Transform boundary points
    const auto& boundary = profile.getBoundary();
    std::vector<glm::vec2> boundary_points;
    for (const auto& point : boundary.points) {
        glm::vec2 transformed = rotation * (point * scale);
        boundary_points.push_back(transformed);
    }
    result.setBoundary(boundary_points);
    
    // Transform holes
    for (const auto& hole : profile.getHoles()) {
        std::vector<glm::vec2> hole_points;
        for (const auto& point : hole.points) {
            glm::vec2 transformed = rotation * (point * scale);
            hole_points.push_back(transformed);
        }
        result.addHole(hole_points);
    }
    
    return result;
}

std::vector<Profile> ExtrudeTool::generateIntermediateProfiles(
    const std::vector<Section>& sections, int subdivisions) const {
    if (sections.empty()) {
        return {};
    }

    std::vector<Profile> profiles;
    profiles.reserve((sections.size() - 1) * subdivisions + 1);

    // Add first profile
    profiles.push_back(sections[0].profile);

    // Generate intermediate profiles between each pair of sections
    for (size_t i = 0; i < sections.size() - 1; ++i) {
        const auto& section1 = sections[i];
        const auto& section2 = sections[i + 1];
        
        float height_diff = section2.height - section1.height;
        float scale_diff = section2.scale - section1.scale;
        float twist_diff = section2.twist - section1.twist;

        for (int j = 1; j <= subdivisions; ++j) {
            float t = static_cast<float>(j) / subdivisions;
            
            // Interpolate profile
            Profile interpolated = interpolateProfiles(section1.profile, section2.profile, t);
            
            // Apply transformation
            float scale = section1.scale + scale_diff * t;
            float twist = section1.twist + twist_diff * t;
            Profile transformed = transformProfile(interpolated, scale, twist);
            
            profiles.push_back(transformed);
        }
    }

    return profiles;
}

std::shared_ptr<Mesh> ExtrudeTool::extrude(const Profile& profile, const ExtrudeParams& params) {
    // Validate input
    if (!validateProfile(profile)) {
        throw std::invalid_argument("Invalid profile for extrusion");
    }
    if (!validateSections(params.sections)) {
        throw std::invalid_argument("Invalid sections for multi-section extrusion");
    }

    // Handle simple extrusion if no additional sections
    if (params.sections.empty()) {
        // Create base mesh from profile
        auto mesh = createBaseMesh(profile);

        // Create top face by duplicating and translating vertices
        const size_t base_vertex_count = mesh->getVertexCount();
        const glm::vec3 extrusion_vec = params.direction * params.distance * (params.reverseSide ? -1.0f : 1.0f);

        // Duplicate vertices for top face
        for (size_t i = 0; i < base_vertex_count; ++i) {
            const auto& base_vertex = mesh->getVertex(i);
            mesh->addVertex(base_vertex.position + extrusion_vec);
        }

        // Create side faces
        const auto& boundary = profile.getBoundary();
        for (size_t i = 0; i < boundary.points.size(); ++i) {
            size_t next = (i + 1) % boundary.points.size();
            size_t top_i = i + base_vertex_count;
            size_t top_next = next + base_vertex_count;

            mesh->addFace({i, next, top_next});
            mesh->addFace({i, top_next, top_i});
        }

        // Handle holes
        for (const auto& hole : profile.getHoles()) {
            for (size_t i = 0; i < hole.points.size(); ++i) {
                size_t next = (i + 1) % hole.points.size();
                size_t top_i = i + base_vertex_count;
                size_t top_next = next + base_vertex_count;

                mesh->addFace({i, top_next, next});
                mesh->addFace({i, top_i, top_next});
            }
        }

        // Apply draft angle if specified
        if (params.draftAngle != 0.0f) {
            applyDraftAngle(*mesh, params.draftAngle);
        }

        generateUVCoordinates(*mesh);
        mesh->computeNormals();
        return mesh;
    }

    // Multi-section extrusion
    const int subdivisions = params.interpolateProfiles ? 4 : 1;
    std::vector<Profile> intermediate_profiles = generateIntermediateProfiles(params.sections, subdivisions);

    // Create base mesh from first profile
    auto mesh = createBaseMesh(profile);
    size_t last_vertex_offset = mesh->getVertexCount();

    // Create intermediate layers
    const float total_height = params.distance;
    const float layer_height = total_height / (intermediate_profiles.size() - 1);

    // Store initial vertex count for final face creation
    const size_t base_vertex_count = last_vertex_offset;

    for (size_t i = 1; i < intermediate_profiles.size(); ++i) {
        const auto& current_profile = intermediate_profiles[i];
        const float height = i * layer_height;

        // Add vertices for current layer
        size_t current_vertex_offset = mesh->getVertexCount();
        const auto& boundary = current_profile.getBoundary();
        
        for (const auto& point : boundary.points) {
            mesh->addVertex(glm::vec3(point.x, point.y, height));
        }

        // Create faces between layers
        for (size_t j = 0; j < boundary.points.size(); ++j) {
            size_t next = (j + 1) % boundary.points.size();
            size_t bottom_current = last_vertex_offset + j;
            size_t bottom_next = last_vertex_offset + next;
            size_t top_current = current_vertex_offset + j;
            size_t top_next = current_vertex_offset + next;

            mesh->addFace({bottom_current, bottom_next, top_next});
            mesh->addFace({bottom_current, top_next, top_current});
        }

        last_vertex_offset = current_vertex_offset;
    }

    // Create side faces by connecting base and top vertices
    const auto& boundary = profile.getBoundary();
    for (size_t i = 0; i < boundary.points.size(); ++i) {
        size_t next = (i + 1) % boundary.points.size();
        size_t top_i = i + base_vertex_count;
        size_t top_next = next + base_vertex_count;

        // Add quad face (as two triangles)
        mesh->addFace({i, next, top_next});
        mesh->addFace({i, top_next, top_i});
    }

    // Handle holes
    for (const auto& hole : profile.getHoles()) {
        for (size_t i = 0; i < hole.points.size(); ++i) {
            size_t next = (i + 1) % hole.points.size();
            size_t top_i = i + base_vertex_count;
            size_t top_next = next + base_vertex_count;

            mesh->addFace({i, top_next, next});
            mesh->addFace({i, top_i, top_next});
        }
    }

    // Apply draft angle if specified
    if (params.draftAngle != 0.0f) {
        applyDraftAngle(*mesh, params.draftAngle);
    }

    // Generate UV coordinates
    generateUVCoordinates(*mesh);

    // Compute normals
    mesh->computeNormals();

    // Validate final mesh
    if (!validateMeshTopology(*mesh)) {
        throw std::runtime_error("Generated mesh has invalid topology");
    }

    return mesh;
}

std::shared_ptr<Mesh> ExtrudeTool::generatePreview(const Profile& profile, const ExtrudeParams& params) {
    // Check if we can use cached preview
    if (previewCache_.valid &&
        previewCache_.cachedProfile.getBoundary().points == profile.getBoundary().points &&
        previewCache_.cachedParams.direction == params.direction &&
        previewCache_.cachedParams.distance == params.distance &&
        previewCache_.cachedParams.draftAngle == params.draftAngle &&
        previewCache_.cachedParams.reverseSide == params.reverseSide &&
        previewCache_.cachedParams.sections.size() == params.sections.size() &&
        previewCache_.cachedParams.interpolateProfiles == params.interpolateProfiles) {
        
        // Check each section's parameters
        bool sections_match = true;
        for (size_t i = 0; i < params.sections.size(); ++i) {
            const auto& cached_section = previewCache_.cachedParams.sections[i];
            const auto& new_section = params.sections[i];
            
            if (cached_section.height != new_section.height ||
                cached_section.scale != new_section.scale ||
                cached_section.twist != new_section.twist ||
                cached_section.profile.getBoundary().points != new_section.profile.getBoundary().points) {
                sections_match = false;
                break;
            }
        }
        
        if (sections_match) {
            return previewCache_.mesh;
        }
    }

    // Generate new preview mesh
    try {
        auto preview_mesh = extrude(profile, params);
        
        // Update cache
        previewCache_.mesh = preview_mesh;
        previewCache_.cachedProfile = profile;
        previewCache_.cachedParams = params;
        previewCache_.valid = true;

        return preview_mesh;
    }
    catch (const std::exception&) {
        previewCache_.valid = false;
        return nullptr;
    }
}

} // namespace modeling
} // namespace rebel_cad
