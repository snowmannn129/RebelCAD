#include "modeling/IntersectTool.h"
#include "modeling/Geometry.h"
#include <gtest/gtest.h>
#include <memory>

using namespace RebelCAD::Modeling;

class IntersectToolTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test bodies that we know intersect
        Topology topo1;
        topo1.vertices = {
            {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0},
            {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}
        };
        topo1.edges = {
            {0, 1}, {1, 2}, {2, 3}, {3, 0},
            {4, 5}, {5, 6}, {6, 7}, {7, 4},
            {0, 4}, {1, 5}, {2, 6}, {3, 7}
        };
        topo1.faces.resize(6); // Cube faces
        body1 = SolidBody::FromTopology(topo1);

        // Create second body that partially overlaps first
        Topology topo2;
        topo2.vertices = {
            {0.5, 0.5, 0.5}, {1.5, 0.5, 0.5}, {1.5, 1.5, 0.5}, {0.5, 1.5, 0.5},
            {0.5, 0.5, 1.5}, {1.5, 0.5, 1.5}, {1.5, 1.5, 1.5}, {0.5, 1.5, 1.5}
        };
        topo2.edges = {
            {0, 1}, {1, 2}, {2, 3}, {3, 0},
            {4, 5}, {5, 6}, {6, 7}, {7, 4},
            {0, 4}, {1, 5}, {2, 6}, {3, 7}
        };
        topo2.faces.resize(6); // Cube faces
        body2 = SolidBody::FromTopology(topo2);

        // Create third body for multiple intersection tests
        Topology topo3;
        topo3.vertices = {
            {0.25, 0.25, 0.25}, {1.25, 0.25, 0.25}, {1.25, 1.25, 0.25}, {0.25, 1.25, 0.25},
            {0.25, 0.25, 1.25}, {1.25, 0.25, 1.25}, {1.25, 1.25, 1.25}, {0.25, 1.25, 1.25}
        };
        topo3.edges = {
            {0, 1}, {1, 2}, {2, 3}, {3, 0},
            {4, 5}, {5, 6}, {6, 7}, {7, 4},
            {0, 4}, {1, 5}, {2, 6}, {3, 7}
        };
        topo3.faces.resize(6); // Cube faces
        body3 = SolidBody::FromTopology(topo3);
    }

    std::shared_ptr<SolidBody> body1;
    std::shared_ptr<SolidBody> body2;
    std::shared_ptr<SolidBody> body3;
    IntersectTool tool;
};

TEST_F(IntersectToolTests, ThrowsOnEmptyInput) {
    std::vector<std::shared_ptr<SolidBody>> empty;
    EXPECT_THROW(tool.SetBodies(empty), std::invalid_argument);
}

TEST_F(IntersectToolTests, ThrowsOnSingleBody) {
    std::vector<std::shared_ptr<SolidBody>> single = {body1};
    EXPECT_THROW(tool.SetBodies(single), std::invalid_argument);
}

TEST_F(IntersectToolTests, ValidatesIntersectingBodies) {
    std::vector<std::shared_ptr<SolidBody>> bodies = {body1, body2};
    tool.SetBodies(bodies);
    EXPECT_TRUE(tool.Validate());
}

TEST_F(IntersectToolTests, IntersectsTwoBodies) {
    std::vector<std::shared_ptr<SolidBody>> bodies = {body1, body2};
    tool.SetBodies(bodies);
    auto result = tool.Execute();
    EXPECT_TRUE(result != nullptr);
    EXPECT_TRUE(result->GetTopology().IsValid());
}

TEST_F(IntersectToolTests, IntersectsThreeBodies) {
    std::vector<std::shared_ptr<SolidBody>> bodies = {body1, body2, body3};
    tool.SetBodies(bodies);
    auto result = tool.Execute();
    EXPECT_TRUE(result != nullptr);
    EXPECT_TRUE(result->GetTopology().IsValid());
}

TEST_F(IntersectToolTests, ThrowsOnNonIntersectingBodies) {
    // Create a body that doesn't intersect with others
    Topology nonintersectingTopo;
    nonintersectingTopo.vertices = {
        {10, 10, 10}, {11, 10, 10}, {11, 11, 10}, {10, 11, 10},
        {10, 10, 11}, {11, 10, 11}, {11, 11, 11}, {10, 11, 11}
    };
    nonintersectingTopo.edges = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };
    nonintersectingTopo.faces.resize(6);
    auto nonintersectingBody = SolidBody::FromTopology(nonintersectingTopo);

    std::vector<std::shared_ptr<SolidBody>> bodies = {body1, nonintersectingBody};
    tool.SetBodies(bodies);
    EXPECT_FALSE(tool.Validate());
    EXPECT_THROW(tool.Execute(), std::runtime_error);
}

TEST_F(IntersectToolTests, ThrowsOnNullBody) {
    std::vector<std::shared_ptr<SolidBody>> bodies = {body1, nullptr};
    EXPECT_THROW(tool.SetBodies(bodies), std::invalid_argument);
}
