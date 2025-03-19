#pragma once

#include <memory>
#include <vector>
#include <stdexcept>
#include <glm/glm.hpp>
#include "modeling/Mesh.h"
#include "modeling/Profile.h"

namespace rebel_cad {
namespace modeling {

class Mesh;  // Forward declaration for the mesh data structure
class Profile;  // Forward declaration for sketch profile

/**
 * @brief Tool for creating 3D geometry by extruding 2D profiles
 * 
 * The ExtrudeTool takes a 2D sketch profile and creates a 3D solid by
 * extending the profile along a specified direction for a given distance.
 * It supports features like draft angles and validates profile integrity.
 */
class ExtrudeTool {
public:
    /**
     * @brief Represents a section in multi-section extrusion
     */
    struct Section {
        Profile profile;                         // Profile for this section
        float height{0.0f};                      // Height along extrusion path
        float scale{1.0f};                       // Scale factor for this section
        float twist{0.0f};                       // Twist angle in radians
    };

    /**
     * @brief Configuration parameters for the extrusion operation
     */
    struct ExtrudeParams {
        glm::vec3 direction{0.0f, 0.0f, 1.0f};  // Default extrude direction is Z+
        float distance{1.0f};                    // Extrusion distance
        float draftAngle{0.0f};                  // Draft angle in radians
        bool reverseSide{false};                 // Whether to extrude in opposite direction
        std::vector<Section> sections;           // Additional sections for multi-section extrusion
        bool interpolateProfiles{true};          // Whether to smoothly interpolate between sections
    };

    ExtrudeTool();
    ~ExtrudeTool();

    /**
     * @brief Validates if profiles are compatible for multi-section extrusion
     * 
     * @param sections Vector of sections to validate
     * @return true if sections are compatible
     * @return false if sections have incompatible profiles
     */
    bool validateSections(const std::vector<Section>& sections) const;

    /**
     * @brief Validates if a profile is suitable for extrusion
     * 
     * @param profile The sketch profile to validate
     * @return true if profile is valid for extrusion
     * @return false if profile has issues preventing extrusion
     */
    bool validateProfile(const Profile& profile) const;

    /**
     * @brief Performs the extrusion operation
     * 
     * @param profile The sketch profile to extrude
     * @param params Parameters controlling the extrusion
     * @return std::shared_ptr<Mesh> The resulting 3D mesh
     * @throws std::invalid_argument if profile is invalid
     */
    std::shared_ptr<Mesh> extrude(const Profile& profile, const ExtrudeParams& params);

    /**
     * @brief Generates a preview mesh for the extrusion
     * 
     * Creates a lightweight preview mesh for real-time feedback during
     * parameter adjustment.
     * 
     * @param profile The sketch profile to preview
     * @param params Current extrusion parameters
     * @return std::shared_ptr<Mesh> A simplified preview mesh
     */
    std::shared_ptr<Mesh> generatePreview(const Profile& profile, const ExtrudeParams& params);

    /**
     * @brief Interpolates between two profiles at a given ratio
     * 
     * @param profile1 First profile
     * @param profile2 Second profile
     * @param t Interpolation ratio (0-1)
     * @return Profile Interpolated profile
     */
    Profile interpolateProfiles(const Profile& profile1, const Profile& profile2, float t) const;

    /**
     * @brief Applies transformation (scale and twist) to a profile
     * 
     * @param profile The profile to transform
     * @param scale Scale factor to apply
     * @param twist Twist angle in radians
     * @return Profile The transformed profile
     */
    Profile transformProfile(const Profile& profile, float scale, float twist) const;

    /**
     * @brief Creates intermediate profiles for smooth multi-section extrusion
     * 
     * @param sections Vector of sections to interpolate between
     * @param subdivisions Number of intermediate profiles to generate
     * @return std::vector<Profile> Vector of interpolated profiles
     */
    std::vector<Profile> generateIntermediateProfiles(const std::vector<Section>& sections, int subdivisions) const;

private:
    /**
     * @brief Creates the base mesh from a profile
     */
    std::shared_ptr<Mesh> createBaseMesh(const Profile& profile);

    /**
     * @brief Applies draft angle to mesh faces
     */
    void applyDraftAngle(Mesh& mesh, float angle);

    /**
     * @brief Validates mesh topology after operations
     */
    bool validateMeshTopology(const Mesh& mesh) const;

    /**
     * @brief Generates UV coordinates for the mesh
     */
    void generateUVCoordinates(Mesh& mesh);

    // Cache for preview mesh to avoid regeneration when parameters haven't changed
    struct PreviewCache {
        std::shared_ptr<Mesh> mesh;
        Profile cachedProfile;
        ExtrudeParams cachedParams;
        bool valid{false};
    };

    PreviewCache previewCache_;
};

} // namespace modeling
} // namespace rebel_cad
