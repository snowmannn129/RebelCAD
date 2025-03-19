#include <gtest/gtest.h>
#include "modeling/RevolveTool.h"
#include <memory>
#include <vector>

namespace RebelCAD {
namespace Graphics {
class GraphicsSystem;
}

namespace Modeling {

using ErrorCode = rebel::core::ErrorCode;

class RevolveToolTests : public ::testing::Test {
protected:
    RevolveTool tool;
    
    // Simple rectangular profile for testing
    std::vector<float> CreateTestProfile() {
        return {
            1.0f, 0.0f, 0.0f,  // Point 1
            2.0f, 0.0f, 0.0f,  // Point 2
            2.0f, 1.0f, 0.0f,  // Point 3
            1.0f, 1.0f, 0.0f   // Point 4
        };
    }

    RevolveTool::RevolveParams CreateDefaultParams() {
        RevolveTool::RevolveParams params;
        params.angle = 360.0f;
        params.isSubtractive = false;
        params.axis_start[0] = params.axis_start[1] = params.axis_start[2] = 0.0f;
        params.axis_end[0] = 0.0f;
        params.axis_end[1] = 1.0f;
        params.axis_end[2] = 0.0f;
        params.full_revolution = true;
        params.segments = 32;
        return params;
    }
};

TEST_F(RevolveToolTests, ValidProfileIsAccepted) {
    auto profile = CreateTestProfile();
    EXPECT_EQ(tool.SetProfile(profile), ErrorCode::None);
}

TEST_F(RevolveToolTests, EmptyProfileIsRejected) {
    std::vector<float> empty_profile;
    EXPECT_EQ(tool.SetProfile(empty_profile), ErrorCode::InvalidMesh);
}

TEST_F(RevolveToolTests, InvalidProfileSizeIsRejected) {
    std::vector<float> invalid_profile = {1.0f, 2.0f}; // Not multiple of 3
    EXPECT_EQ(tool.SetProfile(invalid_profile), ErrorCode::InvalidMesh);
}

TEST_F(RevolveToolTests, ValidParamsAreAccepted) {
    auto params = CreateDefaultParams();
    EXPECT_EQ(tool.Configure(params), ErrorCode::None);
}

TEST_F(RevolveToolTests, InvalidAngleIsRejected) {
    auto params = CreateDefaultParams();
    params.angle = -45.0f;
    EXPECT_EQ(tool.Configure(params), ErrorCode::InvalidMesh);
    
    params.angle = 361.0f;
    EXPECT_EQ(tool.Configure(params), ErrorCode::InvalidMesh);
}

TEST_F(RevolveToolTests, InvalidSegmentsAreRejected) {
    auto params = CreateDefaultParams();
    params.segments = 3; // Minimum is 4
    EXPECT_EQ(tool.Configure(params), ErrorCode::InvalidMesh);
}

TEST_F(RevolveToolTests, InvalidAxisIsRejected) {
    auto params = CreateDefaultParams();
    // Start and end points are the same
    params.axis_end[0] = params.axis_start[0];
    params.axis_end[1] = params.axis_start[1];
    params.axis_end[2] = params.axis_start[2];
    EXPECT_EQ(tool.Configure(params), ErrorCode::InvalidMesh);
}

TEST_F(RevolveToolTests, ExecuteRequiresProfileAndParams) {
    EXPECT_EQ(tool.Execute(), ErrorCode::CADError);
    
    auto profile = CreateTestProfile();
    EXPECT_EQ(tool.SetProfile(profile), ErrorCode::None);
    EXPECT_EQ(tool.Execute(), ErrorCode::CADError); // Still needs params
    
    auto params = CreateDefaultParams();
    EXPECT_EQ(tool.Configure(params), ErrorCode::None);
    EXPECT_EQ(tool.Execute(), ErrorCode::None); // Now should succeed
}

TEST_F(RevolveToolTests, PreviewRequiresProfileAndParams) {
    auto graphics = std::make_shared<Graphics::GraphicsSystem>();
    EXPECT_EQ(tool.Preview(graphics), ErrorCode::CADError);
    
    auto profile = CreateTestProfile();
    EXPECT_EQ(tool.SetProfile(profile), ErrorCode::None);
    EXPECT_EQ(tool.Preview(graphics), ErrorCode::CADError); // Still needs params
    
    auto params = CreateDefaultParams();
    EXPECT_EQ(tool.Configure(params), ErrorCode::None);
    EXPECT_EQ(tool.Preview(graphics), ErrorCode::None); // Now should succeed
}

TEST_F(RevolveToolTests, ProfileTooCloseToAxisIsRejected) {
    std::vector<float> profile_on_axis = {
        0.0f, 0.0f, 0.0f,  // Point on axis
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f
    };
    
    auto params = CreateDefaultParams();
    EXPECT_EQ(tool.Configure(params), ErrorCode::None);
    EXPECT_EQ(tool.SetProfile(profile_on_axis), ErrorCode::InvalidMesh);
}

TEST_F(RevolveToolTests, PartialRevolutionGeneratesCorrectSegments) {
    auto profile = CreateTestProfile();
    EXPECT_EQ(tool.SetProfile(profile), ErrorCode::None);
    
    auto params = CreateDefaultParams();
    params.angle = 180.0f; // Half revolution
    params.full_revolution = false;
    EXPECT_EQ(tool.Configure(params), ErrorCode::None);
    EXPECT_EQ(tool.Execute(), ErrorCode::None);
}

TEST_F(RevolveToolTests, CancelClearsMeshData) {
    auto profile = CreateTestProfile();
    auto params = CreateDefaultParams();
    
    EXPECT_EQ(tool.SetProfile(profile), ErrorCode::None);
    EXPECT_EQ(tool.Configure(params), ErrorCode::None);
    EXPECT_EQ(tool.Execute(), ErrorCode::None);
    
    tool.Cancel();
    // After cancel, should need to execute again
    EXPECT_EQ(tool.Preview(std::make_shared<Graphics::GraphicsSystem>()), ErrorCode::None);
}

TEST_F(RevolveToolTests, ArbitraryAxisRevolution) {
    auto profile = CreateTestProfile();
    auto params = CreateDefaultParams();
    
    // Test with an arbitrary axis
    params.axis_start[0] = 1.0f;
    params.axis_start[1] = 1.0f;
    params.axis_start[2] = 1.0f;
    params.axis_end[0] = 2.0f;
    params.axis_end[1] = 3.0f;
    params.axis_end[2] = 4.0f;
    
    EXPECT_EQ(tool.SetProfile(profile), ErrorCode::None);
    EXPECT_EQ(tool.Configure(params), ErrorCode::None);
    EXPECT_EQ(tool.Execute(), ErrorCode::None);
}

} // namespace Modeling
} // namespace RebelCAD
