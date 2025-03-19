#include <gtest/gtest.h>
#include "../../include/modeling/NurbsSurface.hpp"
#include <glm/gtc/epsilon.hpp>

using namespace RebelCAD::Modeling;

class NurbsSurfaceTest : public ::testing::Test {
protected:
    // Helper function to create a simple test surface
    std::shared_ptr<NurbsSurface> CreateTestSurface() {
        std::vector<NurbsSurface::Point3D> controlPoints = {
            {0, 0, 0}, {1, 0, 0}, {2, 0, 0},
            {0, 1, 0}, {1, 1, 1}, {2, 1, 0},
            {0, 2, 0}, {1, 2, 0}, {2, 2, 0}
        };
        return NurbsSurface::Create(controlPoints, 3, 3, 2, 2);
    }

    const double epsilon = 1e-10;
};

TEST_F(NurbsSurfaceTest, Creation) {
    // Valid creation
    EXPECT_NO_THROW(CreateTestSurface());

    // Invalid number of control points
    std::vector<NurbsSurface::Point3D> tooFewPoints = {
        {0, 0, 0}, {1, 0, 0},
        {0, 1, 0}, {1, 1, 0}
    };
    EXPECT_THROW(
        NurbsSurface::Create(tooFewPoints, 3, 3, 2, 2),
        RebelCAD::Error
    );

    // Invalid degrees
    std::vector<NurbsSurface::Point3D> points = {
        {0, 0, 0}, {1, 0, 0}, {2, 0, 0},
        {0, 1, 0}, {1, 1, 0}, {2, 1, 0}
    };
    EXPECT_THROW(
        NurbsSurface::Create(points, 3, 2, 3, 2),  // degree >= num points
        RebelCAD::Error
    );
}

TEST_F(NurbsSurfaceTest, ControlPointAccess) {
    auto surface = CreateTestSurface();

    // Valid access
    auto point = surface->GetControlPoint(1, 1);
    EXPECT_TRUE(glm::all(glm::epsilonEqual(
        point,
        NurbsSurface::Point3D(1, 1, 1),
        epsilon
    )));

    // Out of range access
    EXPECT_THROW(surface->GetControlPoint(3, 1), RebelCAD::Error);
    EXPECT_THROW(surface->GetControlPoint(1, 3), RebelCAD::Error);

    // Valid modification
    EXPECT_NO_THROW(surface->SetControlPoint(1, 1, {2, 2, 2}));
    point = surface->GetControlPoint(1, 1);
    EXPECT_TRUE(glm::all(glm::epsilonEqual(
        point,
        NurbsSurface::Point3D(2, 2, 2),
        epsilon
    )));

    // Out of range modification
    EXPECT_THROW(
        surface->SetControlPoint(3, 1, {0, 0, 0}),
        RebelCAD::Error
    );
}

TEST_F(NurbsSurfaceTest, WeightAccess) {
    auto surface = CreateTestSurface();

    // Valid access
    EXPECT_NEAR(surface->GetWeight(1, 1), 1.0, epsilon);

    // Out of range access
    EXPECT_THROW(surface->GetWeight(3, 1), RebelCAD::Error);
    EXPECT_THROW(surface->GetWeight(1, 3), RebelCAD::Error);

    // Valid modification
    EXPECT_NO_THROW(surface->SetWeight(1, 1, 2.0));
    EXPECT_NEAR(surface->GetWeight(1, 1), 2.0, epsilon);

    // Invalid weight value
    EXPECT_THROW(surface->SetWeight(1, 1, 0.0), RebelCAD::Error);
    EXPECT_THROW(surface->SetWeight(1, 1, -1.0), RebelCAD::Error);
}

TEST_F(NurbsSurfaceTest, SurfaceEvaluation) {
    auto surface = CreateTestSurface();

    // Center point evaluation
    auto point = surface->Evaluate(0.5, 0.5);
    // Center should be influenced by the elevated middle control point
    EXPECT_GT(point.z, 0.0);

    // Corner evaluations
    point = surface->Evaluate(0.0, 0.0);
    EXPECT_TRUE(glm::all(glm::epsilonEqual(
        point,
        NurbsSurface::Point3D(0, 0, 0),
        epsilon
    )));

    point = surface->Evaluate(1.0, 1.0);
    EXPECT_TRUE(glm::all(glm::epsilonEqual(
        point,
        NurbsSurface::Point3D(2, 2, 0),
        epsilon
    )));

    // Out of range parameters
    EXPECT_THROW(surface->Evaluate(-0.1, 0.5), RebelCAD::Error);
    EXPECT_THROW(surface->Evaluate(0.5, 1.1), RebelCAD::Error);
}

TEST_F(NurbsSurfaceTest, SurfaceDerivatives) {
    auto surface = CreateTestSurface();

    // First derivatives at center
    auto du = surface->EvaluateDerivative(0.5, 0.5, 1, 0);
    auto dv = surface->EvaluateDerivative(0.5, 0.5, 0, 1);

    // Due to surface shape, we expect:
    // - Positive X derivative in U direction
    // - Positive Y derivative in V direction
    EXPECT_GT(du.x, 0.0);
    EXPECT_GT(dv.y, 0.0);

    // Invalid derivative orders
    EXPECT_THROW(
        surface->EvaluateDerivative(0.5, 0.5, 3, 0),  // > degree
        RebelCAD::Error
    );
}

TEST_F(NurbsSurfaceTest, Tessellation) {
    auto surface = CreateTestSurface();

    // Valid tessellation
    EXPECT_NO_THROW(surface->ToSolidBody(32, 32));

    // Invalid division counts
    EXPECT_THROW(surface->ToSolidBody(0, 32), RebelCAD::Error);
    EXPECT_THROW(surface->ToSolidBody(32, 0), RebelCAD::Error);
}
