#include <gtest/gtest.h>
#include "modeling/UnionTool.h"
#include "modeling/Geometry.h"
#include "core/Error.h"

namespace RebelCAD {
namespace Modeling {
namespace Tests {

class UnionToolTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test bodies
        auto cube1_topology = CreateCubeTopology({0, 0, 0}, 1.0);
        auto cube2_topology = CreateCubeTopology({0.5, 0.5, 0.5}, 1.0);
        
        m_cube1 = SolidBody::FromTopology(cube1_topology);
        m_cube2 = SolidBody::FromTopology(cube2_topology);
        m_tool = std::make_unique<UnionTool>();
    }

    // Helper to create a cube topology for testing
    static Topology CreateCubeTopology(const Point3D& center, double size) {
        Topology topology;
        // Add vertices
        double half = size / 2.0;
        topology.vertices = {
            {center.x - half, center.y - half, center.z - half}, // 0
            {center.x + half, center.y - half, center.z - half}, // 1
            {center.x + half, center.y + half, center.z - half}, // 2
            {center.x - half, center.y + half, center.z - half}, // 3
            {center.x - half, center.y - half, center.z + half}, // 4
            {center.x + half, center.y - half, center.z + half}, // 5
            {center.x + half, center.y + half, center.z + half}, // 6
            {center.x - half, center.y + half, center.z + half}  // 7
        };

        // Add edges
        topology.edges = {
            {0, 1}, {1, 2}, {2, 3}, {3, 0},  // Bottom face
            {4, 5}, {5, 6}, {6, 7}, {7, 4},  // Top face
            {0, 4}, {1, 5}, {2, 6}, {3, 7}   // Vertical edges
        };

        // Add faces (each face defined by its edges)
        topology.faces = {
            {{0, 1, 2, 3}, {true, true, true, true}},      // Bottom
            {{4, 5, 6, 7}, {true, true, true, true}},      // Top
            {{0, 9, 4, 8}, {true, true, false, false}},    // Front
            {{1, 10, 5, 9}, {true, true, false, false}},   // Right
            {{2, 11, 6, 10}, {true, true, false, false}},  // Back
            {{3, 8, 7, 11}, {true, true, false, false}}    // Left
        };

        return topology;
    }

    std::unique_ptr<UnionTool> m_tool;
    std::shared_ptr<SolidBody> m_cube1;
    std::shared_ptr<SolidBody> m_cube2;
};

TEST_F(UnionToolTests, ThrowsOnEmptyBodies) {
    std::vector<std::shared_ptr<SolidBody>> empty_bodies;
    EXPECT_THROW(m_tool->SetBodies(empty_bodies), rebel::core::Error);
}

TEST_F(UnionToolTests, ThrowsOnSingleBody) {
    std::vector<std::shared_ptr<SolidBody>> single_body = {m_cube1};
    EXPECT_THROW(m_tool->SetBodies(single_body), rebel::core::Error);
}

TEST_F(UnionToolTests, AcceptsTwoBodies) {
    std::vector<std::shared_ptr<SolidBody>> bodies = {m_cube1, m_cube2};
    EXPECT_NO_THROW(m_tool->SetBodies(bodies));
}

TEST_F(UnionToolTests, ThrowsOnNullBody) {
    std::vector<std::shared_ptr<SolidBody>> bodies = {m_cube1, nullptr};
    EXPECT_THROW(m_tool->SetBodies(bodies), rebel::core::Error);
}

TEST_F(UnionToolTests, ExecuteFailsWithoutBodies) {
    EXPECT_THROW(m_tool->Execute(), rebel::core::Error);
}

TEST_F(UnionToolTests, UnionOfIntersectingBodies) {
    std::vector<std::shared_ptr<SolidBody>> bodies = {m_cube1, m_cube2};
    m_tool->SetBodies(bodies);
    
    auto result = m_tool->Execute();
    EXPECT_NE(result, nullptr);
    
    // The resulting body should have valid topology
    EXPECT_TRUE(result->GetTopology().IsValid());
}

TEST_F(UnionToolTests, UnionOfNonIntersectingBodies) {
    // Create two cubes far apart
    auto cube2_topology = CreateCubeTopology({3.0, 3.0, 3.0}, 1.0);
    auto far_cube = SolidBody::FromTopology(cube2_topology);
    
    std::vector<std::shared_ptr<SolidBody>> bodies = {m_cube1, far_cube};
    m_tool->SetBodies(bodies);
    
    auto result = m_tool->Execute();
    EXPECT_NE(result, nullptr);
    
    // Should create a compound body
    EXPECT_TRUE(result->GetTopology().IsValid());
}

} // namespace Tests
} // namespace Modeling
} // namespace RebelCAD
