#include "modeling/OffsetSurfaceTool.h"
#include "modeling/PatchSurface.h"
#include <gtest/gtest.h>
#include <memory>

using namespace RebelCAD::Modeling;

class OffsetSurfaceToolTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a simple test surface (patch surface)
        std::vector<std::vector<Point3D>> controlPoints = {
            {Point3D(0,0,0), Point3D(1,0,0), Point3D(2,0,0), Point3D(3,0,0)},
            {Point3D(0,1,0), Point3D(1,1,1), Point3D(2,1,1), Point3D(3,1,0)},
            {Point3D(0,2,0), Point3D(1,2,1), Point3D(2,2,1), Point3D(3,2,0)},
            {Point3D(0,3,0), Point3D(1,3,0), Point3D(2,3,0), Point3D(3,3,0)}
        };
        testSurface = PatchSurface::CreateBicubic(controlPoints);
        tool = std::make_unique<OffsetSurfaceTool>();
    }

    std::unique_ptr<OffsetSurfaceTool> tool;
    std::shared_ptr<Surface> testSurface;
};

TEST_F(OffsetSurfaceToolTests, InitialState) {
    EXPECT_FALSE(tool->validate());
}

TEST_F(OffsetSurfaceToolTests, SetInputSurface) {
    EXPECT_TRUE(tool->setInputSurface(testSurface));
    EXPECT_TRUE(tool->validate());
}

TEST_F(OffsetSurfaceToolTests, NullInputSurface) {
    EXPECT_FALSE(tool->setInputSurface(nullptr));
    EXPECT_FALSE(tool->validate());
}

TEST_F(OffsetSurfaceToolTests, PositiveOffset) {
    tool->setInputSurface(testSurface);
    tool->setOffsetDistance(1.0);
    auto result = tool->execute();
    EXPECT_NE(result, nullptr);
    
    // Test a point on the offset surface
    Point3D point = result->Evaluate(0.5, 0.5);
    Point3D basePoint = testSurface->Evaluate(0.5, 0.5);
    Vector3D normal = testSurface->EvaluateNormal(0.5, 0.5).Normalize();
    
    // Check if offset point is correct distance from base surface
    double distance = std::sqrt(
        std::pow(point.x - basePoint.x, 2) +
        std::pow(point.y - basePoint.y, 2) +
        std::pow(point.z - basePoint.z, 2)
    );
    EXPECT_NEAR(distance, 1.0, 0.001);
}

TEST_F(OffsetSurfaceToolTests, NegativeOffset) {
    tool->setInputSurface(testSurface);
    tool->setOffsetDistance(-1.0);
    auto result = tool->execute();
    EXPECT_NE(result, nullptr);
    
    // Test that normals are inverted
    Vector3D baseNormal = testSurface->EvaluateNormal(0.5, 0.5);
    Vector3D offsetNormal = result->EvaluateNormal(0.5, 0.5);
    EXPECT_NEAR(baseNormal.Dot(offsetNormal), -1.0, 0.001);
}

TEST_F(OffsetSurfaceToolTests, InvalidTolerance) {
    tool->setInputSurface(testSurface);
    EXPECT_THROW(tool->setTolerance(0.0), std::invalid_argument);
    EXPECT_THROW(tool->setTolerance(-1.0), std::invalid_argument);
}

TEST_F(OffsetSurfaceToolTests, MeshGeneration) {
    tool->setInputSurface(testSurface);
    tool->setOffsetDistance(1.0);
    auto result = tool->execute();
    
    auto mesh = result->ToMesh(20);
    EXPECT_FALSE(mesh->vertices.empty());
    EXPECT_FALSE(mesh->triangles.empty());
    EXPECT_FALSE(mesh->normals.empty());
}
