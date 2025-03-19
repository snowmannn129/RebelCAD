#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace RebelCAD {
namespace Modeling {

// Forward declarations
class RevolveTool;

// Implementation details
struct RevolveSection {
    float angle;             ///< Angle at this section in degrees
    float scale;             ///< Scale factor for the profile at this section
    float twist;             ///< Twist angle in radians at this section
    std::vector<float> profile_vertices; ///< Profile vertices at this section
};

struct RevolveParams {
    float angle;             ///< Total angle to revolve in degrees (up to 360)
    bool isSubtractive;      ///< True for cut/subtract, false for add
    glm::vec3 axis_start;    ///< Start point of rotation axis
    glm::vec3 axis_end;      ///< End point of rotation axis
    bool full_revolution;    ///< Optimization flag for 360-degree revolutions
    int segments;            ///< Number of segments for revolution (min 4)
    std::vector<RevolveSection> sections; ///< Optional intermediate sections
    bool interpolateSections;///< Whether to interpolate between sections
};

class RevolveToolImpl {
public:
    std::vector<float> profile_vertices;
    RevolveParams params;
    bool is_configured;
    bool has_profile;
    
    // Cached mesh data
    std::vector<float> mesh_vertices;
    std::vector<float> mesh_normals;
    std::vector<float> mesh_uvs;
    std::vector<unsigned int> mesh_indices;
    
    // Preview cache
    struct PreviewCache {
        bool valid;
        std::vector<float> cached_profile;
        RevolveParams cached_params;
        std::vector<float> mesh_vertices;
        std::vector<float> mesh_normals;
        std::vector<float> mesh_uvs;
        std::vector<unsigned int> mesh_indices;
    } preview_cache;
    
    RevolveToolImpl();
    void Clear();
    
    // Helper methods
    bool ValidateSection(const RevolveSection& section) const;
    std::vector<float> InterpolateProfiles(
        const std::vector<float>& profile1,
        const std::vector<float>& profile2,
        float t) const;
    std::vector<float> TransformProfile(
        const std::vector<float>& profile,
        float scale,
        float twist) const;
};

} // namespace Modeling
} // namespace RebelCAD
