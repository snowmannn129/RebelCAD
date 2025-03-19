#include <gtest/gtest.h>
#include "simulation/mesh/MeshGenerator.h"
#include <glm/glm.hpp>

using namespace RebelCAD::Simulation;

class MeshGeneratorTest : public ::testing::Test {
protected:
    MeshGenerator generator;
    
    // Test data for a simple tetrahedron
    std::vector<glm::vec3> tetrahedronVertices = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    };
    
    std::vector<uint32_t> tetrahedronIndices = {
        0, 1, 2,  // front face
        0, 2, 3,  // right face
        0, 3, 1,  // bottom face
        1, 3, 2   // back face
    };
};

// Test parameter validation
TEST_F(MeshGeneratorTest, ValidateParameters) {
    MeshParameters params;
    
    // Test valid parameters
    EXPECT_NO_THROW(generator.setParameters(params));
    
    // Test invalid max element size
    params.maxElementSize = -1.0f;
    EXPECT_THROW(generator.setParameters(params), MeshGenerationError);
    
    // Test invalid min element size
    params.maxElementSize = 1.0f;
    params.minElementSize = 2.0f;
    EXPECT_THROW(generator.setParameters(params), MeshGenerationError);
    
    // Test invalid aspect ratio
    params.minElementSize = 0.1f;
    params.aspectRatioLimit = 0.5f;
    EXPECT_THROW(generator.setParameters(params), MeshGenerationError);
    
    // Test invalid grading rate
    params.aspectRatioLimit = 5.0f;
    params.gradingRate = 0.8f;
    EXPECT_THROW(generator.setParameters(params), MeshGenerationError);
}

// Test mesh generation with valid input
TEST_F(MeshGeneratorTest, GenerateValidMesh) {
    size_t elementCount = generator.generateMesh(tetrahedronVertices, tetrahedronIndices);
    EXPECT_GT(elementCount, 0);
    EXPECT_TRUE(generator.validateMesh());
}

// Test mesh generation with invalid input
TEST_F(MeshGeneratorTest, GenerateInvalidMesh) {
    // Empty vertices
    EXPECT_THROW(generator.generateMesh({}, tetrahedronIndices), MeshGenerationError);
    
    // Empty indices
    EXPECT_THROW(generator.generateMesh(tetrahedronVertices, {}), MeshGenerationError);
    
    // Invalid index count (not multiple of 3)
    std::vector<uint32_t> invalidIndices = {0, 1};
    EXPECT_THROW(generator.generateMesh(tetrahedronVertices, invalidIndices), MeshGenerationError);
    
    // Too few vertices
    std::vector<glm::vec3> fewVertices = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    };
    EXPECT_THROW(generator.generateMesh(fewVertices, tetrahedronIndices), MeshGenerationError);
}

// Test mesh optimization
TEST_F(MeshGeneratorTest, OptimizeMesh) {
    // Generate initial mesh
    generator.generateMesh(tetrahedronVertices, tetrahedronIndices);
    
    // Test optimization without initialization
    EXPECT_NO_THROW(generator.optimizeMesh());
    
    // Verify optimization improved quality
    float improvement = generator.optimizeMesh();
    EXPECT_GE(improvement, 0.0f);
    
    // Second optimization should return 0 (already optimized)
    float secondImprovement = generator.optimizeMesh();
    EXPECT_FLOAT_EQ(secondImprovement, 0.0f);
}

// Test mesh statistics
TEST_F(MeshGeneratorTest, MeshStatistics) {
    // Test statistics before initialization
    std::string emptyStats = generator.getMeshStatistics();
    EXPECT_EQ(emptyStats, "Mesh not initialized");
    
    // Generate mesh and test statistics
    generator.generateMesh(tetrahedronVertices, tetrahedronIndices);
    std::string stats = generator.getMeshStatistics();
    
    // Verify statistics contain expected information
    EXPECT_NE(stats.find("Vertices:"), std::string::npos);
    EXPECT_NE(stats.find("Elements:"), std::string::npos);
    EXPECT_NE(stats.find("Quality Metrics:"), std::string::npos);
    EXPECT_NE(stats.find("Aspect Ratio:"), std::string::npos);
}

// Test mesh validation
TEST_F(MeshGeneratorTest, ValidateMeshState) {
    // Test validation before initialization
    EXPECT_FALSE(generator.validateMesh());
    
    // Generate valid mesh
    generator.generateMesh(tetrahedronVertices, tetrahedronIndices);
    EXPECT_TRUE(generator.validateMesh());
    
    // Verify mesh data accessors
    EXPECT_FALSE(generator.getVertices().empty());
    EXPECT_FALSE(generator.getElements().empty());
    EXPECT_EQ(generator.getVertices().size(), tetrahedronVertices.size());
}

// Test quality metrics
TEST_F(MeshGeneratorTest, QualityMetrics) {
    generator.generateMesh(tetrahedronVertices, tetrahedronIndices);
    
    // Verify element quality values
    for (const auto& element : generator.getElements()) {
        EXPECT_GE(element.quality, 0.0f);
        EXPECT_LE(element.quality, 1.0f);
    }
    
    // Optimize and verify quality improvement
    float improvement = generator.optimizeMesh();
    EXPECT_GE(improvement, 0.0f);
}
