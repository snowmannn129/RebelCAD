#pragma once

#include <vector>
#include <memory>
#include <array>
#include <glm/glm.hpp>
#include <string>
#include <stdexcept>

namespace RebelCAD {
namespace Simulation {

/**
 * @brief Parameters controlling mesh generation quality and behavior
 */
struct MeshParameters {
    float maxElementSize = 1.0f;      // Maximum size of mesh elements
    float minElementSize = 0.1f;      // Minimum size of mesh elements
    float aspectRatioLimit = 5.0f;    // Maximum allowed aspect ratio for elements
    float gradingRate = 1.2f;         // Maximum size ratio between adjacent elements
    bool preserveFeatures = true;     // Preserve sharp features and boundaries
    int optimizationPasses = 3;       // Number of optimization passes
};

/**
 * @brief Exception class for mesh generation errors
 */
class MeshGenerationError : public std::runtime_error {
public:
    explicit MeshGenerationError(const std::string& message) 
        : std::runtime_error(message) {}
};

/**
 * @brief Represents a 3D mesh element (tetrahedron)
 */
struct MeshElement {
    std::array<size_t, 4> vertices;   // Indices of vertices forming the tetrahedron
    float quality;                    // Quality metric (0-1, higher is better)
    
    MeshElement(size_t v1, size_t v2, size_t v3, size_t v4) : quality(0.0f) {
        vertices[0] = v1;
        vertices[1] = v2;
        vertices[2] = v3;
        vertices[3] = v4;
    }
};

/**
 * @brief Core mesh generation system for finite element analysis
 * 
 * Generates high-quality tetrahedral meshes from 3D solid models while
 * respecting geometric features and maintaining element quality constraints.
 */
class MeshGenerator {
public:
    MeshGenerator();
    ~MeshGenerator() = default;

    /**
     * @brief Sets mesh generation parameters
     * @param params Parameters controlling mesh generation
     * @throws MeshGenerationError if parameters are invalid
     */
    void setParameters(const MeshParameters& params);

    /**
     * @brief Generates a tetrahedral mesh from input geometry
     * @param vertices Input geometry vertices
     * @param indices Input geometry triangle indices
     * @return Number of elements generated
     * @throws MeshGenerationError if mesh generation fails
     */
    size_t generateMesh(const std::vector<glm::vec3>& vertices,
                       const std::vector<uint32_t>& indices);

    /**
     * @brief Optimizes the generated mesh for better quality
     * @return Quality improvement percentage
     */
    float optimizeMesh();

    /**
     * @brief Validates mesh quality and topology
     * @return true if mesh meets all quality criteria
     */
    bool validateMesh() const;

    /**
     * @brief Gets the generated mesh vertices
     * @return Vector of vertex positions
     */
    const std::vector<glm::vec3>& getVertices() const { return m_vertices; }

    /**
     * @brief Gets the generated mesh elements
     * @return Vector of tetrahedral elements
     */
    const std::vector<MeshElement>& getElements() const { return m_elements; }

    /**
     * @brief Gets mesh statistics
     * @return String containing mesh quality statistics
     */
    std::string getMeshStatistics() const;

private:
    // Core mesh generation methods
    void generateInitialMesh();
    void refineMesh();
    void optimizeElementQuality();
    void enforceQualityConstraints();
    
    // Quality metrics
    float calculateElementQuality(const MeshElement& element) const;
    float calculateAspectRatio(const MeshElement& element) const;
    
    // Mesh data
    std::vector<glm::vec3> m_vertices;
    std::vector<MeshElement> m_elements;
    MeshParameters m_parameters;
    
    // Internal state
    bool m_isInitialized;
    bool m_isOptimized;
};

} // namespace Simulation
} // namespace RebelCAD
