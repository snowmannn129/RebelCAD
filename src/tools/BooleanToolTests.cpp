#include <gtest/gtest.h>
#include "modeling/BooleanTool.h"
#include "core/Error.h"
#include <memory>
#include <vector>
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace RebelCAD::Modeling;
using namespace RebelCAD::Core;

class BooleanToolTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a cube mesh
        cubeVertices = {
            // Front face
            0.0f, 0.0f, 0.0f,  // 0
            1.0f, 0.0f, 0.0f,  // 1
            1.0f, 1.0f, 0.0f,  // 2
            0.0f, 1.0f, 0.0f,  // 3
            // Back face
            0.0f, 0.0f, 1.0f,  // 4
            1.0f, 0.0f, 1.0f,  // 5
            1.0f, 1.0f, 1.0f,  // 6
            0.0f, 1.0f, 1.0f   // 7
        };

        cubeIndices = {
            // Front face
            0, 1, 2,
            0, 2, 3,
            // Back face
            4, 5, 6,
            4, 6, 7,
            // Top face
            3, 2, 6,
            3, 6, 7,
            // Bottom face
            0, 1, 5,
            0, 5, 4,
            // Right face
            1, 2, 6,
            1, 6, 5,
            // Left face
            0, 3, 7,
            0, 7, 4
        };

        // Create a sphere mesh
        CreateSphereMesh(0.5f, 16, sphereVertices, sphereIndices);

        // Create boolean tool
        tool = std::make_unique<BooleanTool>();
    }

    void CreateSphereMesh(float radius, int segments, 
                         std::vector<float>& vertices,
                         std::vector<unsigned int>& indices) {
        vertices.clear();
        indices.clear();

        // Generate vertices
        for (int i = 0; i <= segments; ++i) {
            float phi = static_cast<float>(M_PI) * static_cast<float>(i) / static_cast<float>(segments);
            for (int j = 0; j <= segments; ++j) {
                float theta = 2.0f * static_cast<float>(M_PI) * static_cast<float>(j) / static_cast<float>(segments);
                float x = radius * std::sin(phi) * std::cos(theta);
                float y = radius * std::sin(phi) * std::sin(theta);
                float z = radius * std::cos(phi);
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
            }
        }

        // Generate indices
        for (int i = 0; i < segments; ++i) {
            for (int j = 0; j < segments; ++j) {
                unsigned int v1 = i * (segments + 1) + j;
                unsigned int v2 = v1 + 1;
                unsigned int v3 = (i + 1) * (segments + 1) + j;
                unsigned int v4 = v3 + 1;

                // First triangle
                indices.push_back(v1);
                indices.push_back(v2);
                indices.push_back(v3);

                // Second triangle
                indices.push_back(v2);
                indices.push_back(v4);
                indices.push_back(v3);
            }
        }
    }

    std::vector<float> cubeVertices;
    std::vector<unsigned int> cubeIndices;
    std::vector<float> sphereVertices;
    std::vector<unsigned int> sphereIndices;
    std::unique_ptr<BooleanTool> tool;
};

TEST_F(BooleanToolTest, SetFirstBody) {
    auto result = tool->SetFirstBody(cubeVertices, cubeIndices);
    EXPECT_EQ(result, ErrorCode::None);
}

TEST_F(BooleanToolTest, SetSecondBody) {
    auto result = tool->SetSecondBody(sphereVertices, sphereIndices);
    EXPECT_EQ(result, ErrorCode::None);
}

TEST_F(BooleanToolTest, InvalidInput) {
    // Empty vertices
    auto result = tool->SetFirstBody({}, cubeIndices);
    EXPECT_EQ(result, ErrorCode::InvalidMesh);

    // Empty indices
    result = tool->SetFirstBody(cubeVertices, {});
    EXPECT_EQ(result, ErrorCode::InvalidMesh);

    // Invalid vertex count
    std::vector<float> invalidVertices = {0.0f, 0.0f}; // Not multiple of 3
    result = tool->SetFirstBody(invalidVertices, cubeIndices);
    EXPECT_EQ(result, ErrorCode::InvalidMesh);

    // Invalid index
    std::vector<unsigned int> invalidIndices = cubeIndices;
    invalidIndices.push_back(999); // Index beyond vertex count
    result = tool->SetFirstBody(cubeVertices, invalidIndices);
    EXPECT_EQ(result, ErrorCode::InvalidMesh);
}

TEST_F(BooleanToolTest, UnionOperation) {
    // Set up bodies
    ASSERT_EQ(tool->SetFirstBody(cubeVertices, cubeIndices), ErrorCode::None);
    ASSERT_EQ(tool->SetSecondBody(sphereVertices, sphereIndices), ErrorCode::None);

    // Configure and execute
    BooleanParams params;
    params.operation_type = BooleanOperationType::Union;
    ASSERT_EQ(tool->Configure(params), ErrorCode::None);
    ASSERT_EQ(tool->Execute(), ErrorCode::None);

    // Check result
    const auto& resultVertices = tool->GetResultVertices();
    const auto& resultIndices = tool->GetResultIndices();
    EXPECT_FALSE(resultVertices.empty());
    EXPECT_FALSE(resultIndices.empty());
}

TEST_F(BooleanToolTest, SubtractOperation) {
    // Set up bodies
    ASSERT_EQ(tool->SetFirstBody(cubeVertices, cubeIndices), ErrorCode::None);
    ASSERT_EQ(tool->SetSecondBody(sphereVertices, sphereIndices), ErrorCode::None);

    // Configure and execute
    BooleanParams params;
    params.operation_type = BooleanOperationType::Subtract;
    ASSERT_EQ(tool->Configure(params), ErrorCode::None);
    ASSERT_EQ(tool->Execute(), ErrorCode::None);

    // Check result
    const auto& resultVertices = tool->GetResultVertices();
    const auto& resultIndices = tool->GetResultIndices();
    EXPECT_FALSE(resultVertices.empty());
    EXPECT_FALSE(resultIndices.empty());
}

TEST_F(BooleanToolTest, IntersectOperation) {
    // Set up bodies
    ASSERT_EQ(tool->SetFirstBody(cubeVertices, cubeIndices), ErrorCode::None);
    ASSERT_EQ(tool->SetSecondBody(sphereVertices, sphereIndices), ErrorCode::None);

    // Configure and execute
    BooleanParams params;
    params.operation_type = BooleanOperationType::Intersect;
    ASSERT_EQ(tool->Configure(params), ErrorCode::None);
    ASSERT_EQ(tool->Execute(), ErrorCode::None);

    // Check result
    const auto& resultVertices = tool->GetResultVertices();
    const auto& resultIndices = tool->GetResultIndices();
    EXPECT_FALSE(resultVertices.empty());
    EXPECT_FALSE(resultIndices.empty());
}

TEST_F(BooleanToolTest, InvalidOperation) {
    // Set up bodies
    ASSERT_EQ(tool->SetFirstBody(cubeVertices, cubeIndices), ErrorCode::None);
    ASSERT_EQ(tool->SetSecondBody(sphereVertices, sphereIndices), ErrorCode::None);

    // Execute without setting operation type
    BooleanParams params;
    ASSERT_EQ(tool->Configure(params), ErrorCode::None);
    EXPECT_EQ(tool->Execute(), ErrorCode::BooleanOperationFailed);
}

TEST_F(BooleanToolTest, Cancel) {
    // Set up bodies and start operation
    ASSERT_EQ(tool->SetFirstBody(cubeVertices, cubeIndices), ErrorCode::None);
    ASSERT_EQ(tool->SetSecondBody(sphereVertices, sphereIndices), ErrorCode::None);

    // Cancel operation
    tool->Cancel();

    // Check that results are cleared
    EXPECT_TRUE(tool->GetResultVertices().empty());
    EXPECT_TRUE(tool->GetResultIndices().empty());
}
