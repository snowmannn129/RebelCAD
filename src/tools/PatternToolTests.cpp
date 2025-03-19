#include <gtest/gtest.h>
#include "modeling/PatternTool.hpp"
#include "modeling/Mesh.hpp"
#include <glm/gtc/constants.hpp>
#include <chrono>

using namespace RebelCAD::Modeling;
using namespace std::chrono;

class PatternToolTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a simple cube mesh for testing
        m_testMesh = createCubeMesh();
    }

    void TearDown() override {
        m_testMesh.reset();
    }

    std::shared_ptr<Mesh> createCubeMesh() {
        auto mesh = std::make_shared<Mesh>();
        
        // Vertices
        Mesh::Vertex vertices[8] = {
            {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
            {{ 1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
            {{-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
            {{-1.0f, -1.0f,  1.0f}, {0.0f, 0.0f,  1.0f}, {0.0f, 0.0f}},
            {{ 1.0f, -1.0f,  1.0f}, {0.0f, 0.0f,  1.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f,  1.0f}, {0.0f, 0.0f,  1.0f}, {1.0f, 1.0f}},
            {{-1.0f,  1.0f,  1.0f}, {0.0f, 0.0f,  1.0f}, {0.0f, 1.0f}}
        };

        for (const auto& vertex : vertices) {
            mesh->addVertex(vertex);
        }

        // Faces (12 triangles for 6 sides)
        Mesh::Face faces[12] = {
            // Front
            {{0, 1, 2}, {0.0f, 0.0f, -1.0f}},
            {{0, 2, 3}, {0.0f, 0.0f, -1.0f}},
            // Back
            {{4, 6, 5}, {0.0f, 0.0f, 1.0f}},
            {{4, 7, 6}, {0.0f, 0.0f, 1.0f}},
            // Left
            {{0, 3, 7}, {-1.0f, 0.0f, 0.0f}},
            {{0, 7, 4}, {-1.0f, 0.0f, 0.0f}},
            // Right
            {{1, 5, 6}, {1.0f, 0.0f, 0.0f}},
            {{1, 6, 2}, {1.0f, 0.0f, 0.0f}},
            // Top
            {{3, 2, 6}, {0.0f, 1.0f, 0.0f}},
            {{3, 6, 7}, {0.0f, 1.0f, 0.0f}},
            // Bottom
            {{0, 4, 5}, {0.0f, -1.0f, 0.0f}},
            {{0, 5, 1}, {0.0f, -1.0f, 0.0f}}
        };

        for (const auto& face : faces) {
            mesh->addFace(face);
        }

        mesh->recalculateNormals();
        return mesh;
    }

    std::shared_ptr<Mesh> m_testMesh;
};

// Test initialization
TEST_F(PatternToolTests, Initialization) {
    PatternTool tool;
    EXPECT_NO_THROW(tool.initialize(PatternType::Linear));
    EXPECT_NO_THROW(tool.initialize(PatternType::Circular));
    EXPECT_NO_THROW(tool.initialize(PatternType::Mirror));
}

// Test parameter validation
TEST_F(PatternToolTests, ParameterValidation) {
    PatternTool tool;
    tool.initialize(PatternType::Linear);

    // Test invalid parameters
    auto invalidParams = std::make_shared<LinearPatternParameters>();
    invalidParams->direction = glm::vec3(0.0f);
    invalidParams->spacing = 0.0f;
    invalidParams->count = 1;
    tool.setParameters(invalidParams);
    EXPECT_FALSE(tool.validateParameters());

    // Test valid parameters
    auto validParams = std::make_shared<LinearPatternParameters>();
    validParams->direction = glm::vec3(1.0f, 0.0f, 0.0f);
    validParams->spacing = 2.0f;
    validParams->count = 3;
    tool.setParameters(validParams);
    EXPECT_TRUE(tool.validateParameters());
}

// Test linear pattern generation
TEST_F(PatternToolTests, LinearPattern) {
    PatternTool tool;
    tool.initialize(PatternType::Linear);
    tool.setTargetGeometry(m_testMesh);

    auto params = std::make_shared<LinearPatternParameters>();
    params->direction = glm::vec3(1.0f, 0.0f, 0.0f);
    params->spacing = 3.0f;
    params->count = 3;
    params->includeOriginal = true;
    tool.setParameters(params);

    EXPECT_TRUE(tool.generatePreview());
    auto preview = tool.getPreview();
    EXPECT_TRUE(preview != nullptr);
    EXPECT_EQ(preview->getVertexCount(), m_testMesh->getVertexCount() * 3);
    EXPECT_EQ(preview->getFaceCount(), m_testMesh->getFaceCount() * 3);
}

// Test circular pattern generation
TEST_F(PatternToolTests, CircularPattern) {
    PatternTool tool;
    tool.initialize(PatternType::Circular);
    tool.setTargetGeometry(m_testMesh);

    auto params = std::make_shared<CircularPatternParameters>();
    params->axis = glm::vec3(0.0f, 1.0f, 0.0f);
    params->center = glm::vec3(0.0f);
    params->angle = glm::pi<float>();
    params->count = 3;
    params->includeOriginal = true;
    tool.setParameters(params);

    EXPECT_TRUE(tool.generatePreview());
    auto preview = tool.getPreview();
    EXPECT_TRUE(preview != nullptr);
    EXPECT_EQ(preview->getVertexCount(), m_testMesh->getVertexCount() * 3);
    EXPECT_EQ(preview->getFaceCount(), m_testMesh->getFaceCount() * 3);
}

// Test mirror pattern generation
TEST_F(PatternToolTests, MirrorPattern) {
    PatternTool tool;
    tool.initialize(PatternType::Mirror);
    tool.setTargetGeometry(m_testMesh);

    auto params = std::make_shared<MirrorPatternParameters>();
    params->normal = glm::vec3(1.0f, 0.0f, 0.0f);
    params->point = glm::vec3(0.0f);
    params->includeOriginal = true;
    tool.setParameters(params);

    EXPECT_TRUE(tool.generatePreview());
    auto preview = tool.getPreview();
    EXPECT_TRUE(preview != nullptr);
    EXPECT_EQ(preview->getVertexCount(), m_testMesh->getVertexCount() * 2);
    EXPECT_EQ(preview->getFaceCount(), m_testMesh->getFaceCount() * 2);
}

// Test preview system
TEST_F(PatternToolTests, PreviewSystem) {
    PatternTool tool;
    tool.initialize(PatternType::Linear);
    tool.setTargetGeometry(m_testMesh);

    // Initial preview should be null
    EXPECT_TRUE(tool.getPreview() == nullptr);

    // Set valid parameters and generate preview
    auto params = std::make_shared<LinearPatternParameters>();
    params->direction = glm::vec3(1.0f, 0.0f, 0.0f);
    params->spacing = 2.0f;
    params->count = 3;
    tool.setParameters(params);

    EXPECT_TRUE(tool.generatePreview());
    EXPECT_TRUE(tool.getPreview() != nullptr);

    // Clear preview
    tool.cancel();
    EXPECT_TRUE(tool.getPreview() == nullptr);
}

// Test error handling
TEST_F(PatternToolTests, ErrorHandling) {
    PatternTool tool;
    tool.initialize(PatternType::Linear);

    // Test with null geometry
    EXPECT_FALSE(tool.generatePreview());

    // Test with null parameters
    tool.setTargetGeometry(m_testMesh);
    EXPECT_FALSE(tool.generatePreview());

    // Test with invalid parameter type
    auto circularParams = std::make_shared<CircularPatternParameters>();
    EXPECT_NO_THROW(tool.setParameters(circularParams));
    EXPECT_FALSE(tool.generatePreview());
}

// Test edge cases for linear pattern
TEST_F(PatternToolTests, LinearPatternEdgeCases) {
    PatternTool tool;
    tool.initialize(PatternType::Linear);
    tool.setTargetGeometry(m_testMesh);

    // Test zero spacing
    auto zeroSpacing = std::make_shared<LinearPatternParameters>();
    zeroSpacing->direction = glm::vec3(1.0f, 0.0f, 0.0f);
    zeroSpacing->spacing = 0.0f;
    zeroSpacing->count = 3;
    tool.setParameters(zeroSpacing);
    EXPECT_FALSE(tool.validateParameters());

    // Test negative spacing
    auto negativeSpacing = std::make_shared<LinearPatternParameters>();
    negativeSpacing->direction = glm::vec3(1.0f, 0.0f, 0.0f);
    negativeSpacing->spacing = -2.0f;
    negativeSpacing->count = 3;
    tool.setParameters(negativeSpacing);
    EXPECT_FALSE(tool.validateParameters());

    // Test zero direction vector
    auto zeroDirection = std::make_shared<LinearPatternParameters>();
    zeroDirection->direction = glm::vec3(0.0f);
    zeroDirection->spacing = 2.0f;
    zeroDirection->count = 3;
    tool.setParameters(zeroDirection);
    EXPECT_FALSE(tool.validateParameters());

    // Test minimum count
    auto minCount = std::make_shared<LinearPatternParameters>();
    minCount->direction = glm::vec3(1.0f, 0.0f, 0.0f);
    minCount->spacing = 2.0f;
    minCount->count = 1;
    tool.setParameters(minCount);
    EXPECT_FALSE(tool.validateParameters());
}

// Test edge cases for circular pattern
TEST_F(PatternToolTests, CircularPatternEdgeCases) {
    PatternTool tool;
    tool.initialize(PatternType::Circular);
    tool.setTargetGeometry(m_testMesh);

    // Test zero angle
    auto zeroAngle = std::make_shared<CircularPatternParameters>();
    zeroAngle->axis = glm::vec3(0.0f, 1.0f, 0.0f);
    zeroAngle->center = glm::vec3(0.0f);
    zeroAngle->angle = 0.0f;
    zeroAngle->count = 3;
    tool.setParameters(zeroAngle);
    EXPECT_FALSE(tool.validateParameters());

    // Test negative angle
    auto negativeAngle = std::make_shared<CircularPatternParameters>();
    negativeAngle->axis = glm::vec3(0.0f, 1.0f, 0.0f);
    negativeAngle->center = glm::vec3(0.0f);
    negativeAngle->angle = -glm::pi<float>();
    negativeAngle->count = 3;
    tool.setParameters(negativeAngle);
    EXPECT_FALSE(tool.validateParameters());

    // Test zero axis vector
    auto zeroAxis = std::make_shared<CircularPatternParameters>();
    zeroAxis->axis = glm::vec3(0.0f);
    zeroAxis->center = glm::vec3(0.0f);
    zeroAxis->angle = glm::pi<float>();
    zeroAxis->count = 3;
    tool.setParameters(zeroAxis);
    EXPECT_FALSE(tool.validateParameters());
}

// Test edge cases for mirror pattern
TEST_F(PatternToolTests, MirrorPatternEdgeCases) {
    PatternTool tool;
    tool.initialize(PatternType::Mirror);
    tool.setTargetGeometry(m_testMesh);

    // Test zero normal vector
    auto zeroNormal = std::make_shared<MirrorPatternParameters>();
    zeroNormal->normal = glm::vec3(0.0f);
    zeroNormal->point = glm::vec3(0.0f);
    tool.setParameters(zeroNormal);
    EXPECT_FALSE(tool.validateParameters());

    // Test non-normalized normal vector
    auto nonNormalizedNormal = std::make_shared<MirrorPatternParameters>();
    nonNormalizedNormal->normal = glm::vec3(2.0f, 2.0f, 2.0f);
    nonNormalizedNormal->point = glm::vec3(0.0f);
    tool.setParameters(nonNormalizedNormal);
    EXPECT_FALSE(tool.validateParameters());
}

// Test topology verification
TEST_F(PatternToolTests, TopologyVerification) {
    PatternTool tool;
    tool.initialize(PatternType::Linear);
    tool.setTargetGeometry(m_testMesh);

    auto params = std::make_shared<LinearPatternParameters>();
    params->direction = glm::vec3(1.0f, 0.0f, 0.0f);
    params->spacing = 2.0f;
    params->count = 3;
    tool.setParameters(params);

    EXPECT_TRUE(tool.generatePreview());
    auto preview = tool.getPreview();
    EXPECT_TRUE(preview != nullptr);

    // Verify mesh integrity
    EXPECT_TRUE(preview->hasValidTopology());
    EXPECT_TRUE(preview->isValid());

    // Verify normal consistency
    const auto& vertices = preview->getVertices();
    for (const auto& vertex : vertices) {
        EXPECT_NEAR(glm::length(vertex.normal), 1.0f, 1e-6f);
    }

    // Verify UV coordinate preservation
    for (size_t i = 0; i < m_testMesh->getVertexCount(); ++i) {
        const auto& origUV = m_testMesh->getVertices()[i].texCoord;
        for (size_t j = 0; j < params->count; ++j) {
            const auto& patternUV = preview->getVertices()[i + j * m_testMesh->getVertexCount()].texCoord;
            EXPECT_FLOAT_EQ(origUV.x, patternUV.x);
            EXPECT_FLOAT_EQ(origUV.y, patternUV.y);
        }
    }
}

// Test memory management and resource cleanup
TEST_F(PatternToolTests, MemoryManagement) {
    PatternTool* tool = new PatternTool();
    tool->initialize(PatternType::Linear);
    tool->setTargetGeometry(m_testMesh);

    auto params = std::make_shared<LinearPatternParameters>();
    params->direction = glm::vec3(1.0f, 0.0f, 0.0f);
    params->spacing = 2.0f;
    params->count = 3;
    tool->setParameters(params);

    // Generate multiple previews to test cache behavior
    for (int i = 0; i < 5; ++i) {
        params->spacing = static_cast<float>(i + 1);
        tool->setParameters(params);
        EXPECT_TRUE(tool->generatePreview());
        auto preview = tool->getPreview();
        EXPECT_TRUE(preview != nullptr);
    }

    // Test preview cleanup
    tool->cancel();
    EXPECT_TRUE(tool->getPreview() == nullptr);

    // Clean up should not cause any memory leaks or crashes
    delete tool;
}

// Test performance with large patterns
TEST_F(PatternToolTests, LargePatternPerformance) {
    PatternTool tool;
    tool.initialize(PatternType::Linear);
    tool.setTargetGeometry(m_testMesh);

    auto params = std::make_shared<LinearPatternParameters>();
    params->direction = glm::vec3(1.0f, 0.0f, 0.0f);
    params->spacing = 2.0f;
    params->count = 100; // Large number of instances
    tool.setParameters(params);

    // Measure time for preview generation
    auto start = high_resolution_clock::now();
    EXPECT_TRUE(tool.generatePreview());
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);

    // Verify preview was generated
    auto preview = tool.getPreview();
    EXPECT_TRUE(preview != nullptr);
    EXPECT_EQ(preview->getVertexCount(), m_testMesh->getVertexCount() * 100);
    EXPECT_EQ(preview->getFaceCount(), m_testMesh->getFaceCount() * 100);

    // Performance threshold (adjust based on system capabilities)
    EXPECT_LT(duration.count(), 1000); // Should complete within 1 second
}
