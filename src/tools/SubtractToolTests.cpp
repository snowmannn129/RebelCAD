#include "modeling/SubtractTool.h"
#include "modeling/Geometry.h"
#include "core/Error.h"
#include <gtest/gtest.h>
#include <memory>

using namespace RebelCAD::Modeling;
using namespace rebel::core;

class SubtractToolTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a simple cube as the main body
        Topology cubeTopology;
        cubeTopology.vertices = {
            {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0},
            {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}
        };
        // Add edges and faces for cube
        // In a real implementation, this would be more detailed
        mainBody = SolidBody::FromTopology(cubeTopology);
        
        // Create a smaller cube as the tool body
        Topology toolTopology;
        toolTopology.vertices = {
            {0.25, 0.25, 0.25}, {0.75, 0.25, 0.25},
            {0.75, 0.75, 0.25}, {0.25, 0.75, 0.25},
            {0.25, 0.25, 0.75}, {0.75, 0.25, 0.75},
            {0.75, 0.75, 0.75}, {0.25, 0.75, 0.75}
        };
        // Add edges and faces for tool cube
        toolBody = SolidBody::FromTopology(toolTopology);
    }

    std::shared_ptr<SolidBody> mainBody;
    std::shared_ptr<SolidBody> toolBody;
    SubtractTool subtractTool;
};

TEST_F(SubtractToolTests, ThrowsOnNullMainBody) {
    EXPECT_THROW({
        subtractTool.SetMainBody(nullptr);
    }, Error);
}

TEST_F(SubtractToolTests, ThrowsOnEmptyToolBodies) {
    std::vector<std::shared_ptr<SolidBody>> emptyBodies;
    EXPECT_THROW({
        subtractTool.SetToolBodies(emptyBodies);
    }, Error);
}

TEST_F(SubtractToolTests, ThrowsOnNullToolBody) {
    std::vector<std::shared_ptr<SolidBody>> bodies = {nullptr};
    EXPECT_THROW({
        subtractTool.SetToolBodies(bodies);
    }, Error);
}

TEST_F(SubtractToolTests, ExecuteWithoutIntersection) {
    // Create a tool body that doesn't intersect with main body
    Topology separateTopology;
    separateTopology.vertices = {
        {2, 2, 2}, {3, 2, 2}, {3, 3, 2}, {2, 3, 2},
        {2, 2, 3}, {3, 2, 3}, {3, 3, 3}, {2, 3, 3}
    };
    auto separateBody = SolidBody::FromTopology(separateTopology);
    
    subtractTool.SetMainBody(mainBody);
    subtractTool.SetToolBodies({separateBody});
    
    auto result = subtractTool.Execute();
    ASSERT_NE(result, nullptr);
    
    // Result should be identical to main body when no intersection
    const auto& resultTopo = result->GetTopology();
    const auto& mainTopo = mainBody->GetTopology();
    EXPECT_EQ(resultTopo.vertices.size(), mainTopo.vertices.size());
    EXPECT_EQ(resultTopo.faces.size(), mainTopo.faces.size());
}

TEST_F(SubtractToolTests, ExecuteWithIntersection) {
    subtractTool.SetMainBody(mainBody);
    subtractTool.SetToolBodies({toolBody});
    
    auto result = subtractTool.Execute();
    ASSERT_NE(result, nullptr);
    
    // Result should have more faces than original due to cut
    const auto& resultTopo = result->GetTopology();
    const auto& mainTopo = mainBody->GetTopology();
    EXPECT_GT(resultTopo.faces.size(), mainTopo.faces.size());
    
    // Volume should be reduced
    // TODO: Add volume calculation and comparison
}

TEST_F(SubtractToolTests, ExecuteWithMultipleToolBodies) {
    // Create second tool body
    Topology tool2Topology;
    tool2Topology.vertices = {
        {0.4, 0.4, 0}, {0.6, 0.4, 0},
        {0.6, 0.6, 0}, {0.4, 0.6, 0},
        {0.4, 0.4, 1}, {0.6, 0.4, 1},
        {0.6, 0.6, 1}, {0.4, 0.6, 1}
    };
    auto tool2Body = SolidBody::FromTopology(tool2Topology);
    
    subtractTool.SetMainBody(mainBody);
    subtractTool.SetToolBodies({toolBody, tool2Body});
    
    auto result = subtractTool.Execute();
    ASSERT_NE(result, nullptr);
    EXPECT_TRUE(result->GetTopology().IsValid());
}

TEST_F(SubtractToolTests, PreviewDoesNotThrow) {
    subtractTool.SetMainBody(mainBody);
    subtractTool.SetToolBodies({toolBody});
    
    auto graphics = std::make_shared<RebelCAD::Graphics::GraphicsSystem>();
    EXPECT_NO_THROW(subtractTool.Preview(graphics));
}
