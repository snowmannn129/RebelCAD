#include "modeling/NURBSCurve.h"
#include "MockGraphicsSystem.h"
#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>

namespace RebelCAD {
namespace Modeling {

class NURBSCurveTest : public ::testing::Test {
protected:
    NURBSCurve curve;

    // Helper to verify point is on curve
    void VerifyPointOnCurve(const glm::vec3& point, float t) {
        glm::vec3 curve_point = curve.Evaluate(t);
        EXPECT_NEAR(point.x, curve_point.x, 1e-6f);
        EXPECT_NEAR(point.y, curve_point.y, 1e-6f);
        EXPECT_NEAR(point.z, curve_point.z, 1e-6f);
    }

    // Helper to verify curve geometry
    void VerifyGeometry(int expected_segments) {
        const auto& vertices = curve.GetResultVertices();
        const auto& tangents = curve.GetResultTangents();
        const auto& indices = curve.GetResultIndices();

        // Check vertex count
        ASSERT_EQ(vertices.size(), (expected_segments + 1) * 3);
        ASSERT_EQ(tangents.size(), (expected_segments + 1) * 3);
        ASSERT_EQ(indices.size(), expected_segments * 2);

        // Check indices are valid
        for (unsigned int idx : indices) {
            EXPECT_LT(idx * 3 + 2, vertices.size());
        }

        // Check tangents are normalized
        for (size_t i = 0; i < tangents.size(); i += 3) {
            float length = std::sqrt(
                tangents[i] * tangents[i] +
                tangents[i+1] * tangents[i+1] +
                tangents[i+2] * tangents[i+2]
            );
            EXPECT_NEAR(length, 1.0f, 1e-6f);
        }
    }

    // Helper to create circle control points
    std::vector<glm::vec3> CreateCircleControlPoints() {
        return {
            glm::vec3(1.0f, 0.0f, 0.0f),   // P0
            glm::vec3(1.0f, 1.0f, 0.0f),   // P1
            glm::vec3(0.0f, 1.0f, 0.0f),   // P2
            glm::vec3(-1.0f, 1.0f, 0.0f),  // P3
            glm::vec3(-1.0f, 0.0f, 0.0f),  // P4
            glm::vec3(-1.0f, -1.0f, 0.0f), // P5
            glm::vec3(0.0f, -1.0f, 0.0f),  // P6
            glm::vec3(1.0f, -1.0f, 0.0f),  // P7
            glm::vec3(1.0f, 0.0f, 0.0f)    // P8 = P0
        };
    }

    // Helper to create circle weights
    std::vector<float> CreateCircleWeights() {
        float w = std::sqrt(2.0f) / 2.0f;
        return {
            1.0f,  // w0
            w,     // w1
            1.0f,  // w2
            w,     // w3
            1.0f,  // w4
            w,     // w5
            1.0f,  // w6
            w,     // w7
            1.0f   // w8
        };
    }
};

TEST_F(NURBSCurveTest, Initialization) {
    // Test invalid inputs
    EXPECT_EQ(curve.SetControlPoints({}), NURBSCurve::ErrorCode::InvalidMesh);
    EXPECT_EQ(curve.SetWeights({}), NURBSCurve::ErrorCode::InvalidMesh);

    // Test invalid points
    EXPECT_EQ(
        curve.SetControlPoints({glm::vec3(INFINITY)}),
        NURBSCurve::ErrorCode::InvalidMesh
    );

    // Test invalid weights
    EXPECT_EQ(
        curve.SetWeights({-1.0f}),
        NURBSCurve::ErrorCode::InvalidMesh
    );

    // Test invalid configuration
    NURBSCurveParams invalid_params;
    invalid_params.degree = 0;
    EXPECT_EQ(curve.Configure(invalid_params), NURBSCurve::ErrorCode::InvalidMesh);

    invalid_params.degree = 3;
    invalid_params.tolerance = -1.0f;
    EXPECT_EQ(curve.Configure(invalid_params), NURBSCurve::ErrorCode::InvalidMesh);

    // Test valid initialization
    std::vector<glm::vec3> points = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(2.0f, 0.0f, 0.0f)
    };
    EXPECT_EQ(curve.SetControlPoints(points), NURBSCurve::ErrorCode::None);

    NURBSCurveParams params;
    params.degree = 2;
    EXPECT_EQ(curve.Configure(params), NURBSCurve::ErrorCode::None);
}

TEST_F(NURBSCurveTest, QuadraticCurve) {
    // Create quadratic curve
    std::vector<glm::vec3> points = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(2.0f, 0.0f, 0.0f)
    };
    ASSERT_EQ(curve.SetControlPoints(points), NURBSCurve::ErrorCode::None);

    NURBSCurveParams params;
    params.degree = 2;
    ASSERT_EQ(curve.Configure(params), NURBSCurve::ErrorCode::None);

    // Test curve points
    VerifyPointOnCurve(points[0], 0.0f);  // Start point
    VerifyPointOnCurve(points[2], 1.0f);  // End point

    // Generate geometry
    ASSERT_EQ(curve.GenerateGeometry(10), NURBSCurve::ErrorCode::None);
    VerifyGeometry(10);
}

TEST_F(NURBSCurveTest, RationalCircle) {
    // Create rational circle (degree 2)
    ASSERT_EQ(curve.SetControlPoints(CreateCircleControlPoints()), NURBSCurve::ErrorCode::None);
    ASSERT_EQ(curve.SetWeights(CreateCircleWeights()), NURBSCurve::ErrorCode::None);

    NURBSCurveParams params;
    params.degree = 2;
    params.rational = true;
    ASSERT_EQ(curve.Configure(params), NURBSCurve::ErrorCode::None);

    // Test points on circle
    for (int i = 0; i <= 8; ++i) {
        float t = static_cast<float>(i) / 8.0f;
        float angle = t * 2.0f * glm::pi<float>();
        glm::vec3 expected(std::cos(angle), std::sin(angle), 0.0f);
        VerifyPointOnCurve(expected, t);
    }

    // Generate geometry
    ASSERT_EQ(curve.GenerateGeometry(32), NURBSCurve::ErrorCode::None);
    VerifyGeometry(32);
}

TEST_F(NURBSCurveTest, Derivatives) {
    // Create cubic curve
    std::vector<glm::vec3> points = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(2.0f, 1.0f, 0.0f),
        glm::vec3(3.0f, 0.0f, 0.0f)
    };
    ASSERT_EQ(curve.SetControlPoints(points), NURBSCurve::ErrorCode::None);

    NURBSCurveParams params;
    params.degree = 3;
    ASSERT_EQ(curve.Configure(params), NURBSCurve::ErrorCode::None);

    // Test derivatives
    auto derivatives = curve.EvaluateDerivatives(0.5f, 2);
    ASSERT_EQ(derivatives.size(), 3);  // 0th to 2nd derivatives

    // First derivative should be tangent
    glm::vec3 tangent = glm::normalize(derivatives[1]);
    EXPECT_NEAR(glm::length(tangent), 1.0f, 1e-6f);

    // Test invalid inputs
    EXPECT_TRUE(curve.EvaluateDerivatives(INFINITY, 1).empty());
    EXPECT_TRUE(curve.EvaluateDerivatives(0.5f, -1).empty());
    EXPECT_TRUE(curve.EvaluateDerivatives(0.5f, 4).empty());  // > degree
}

TEST_F(NURBSCurveTest, DegreeElevation) {
    // Create quadratic curve
    std::vector<glm::vec3> points = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(2.0f, 0.0f, 0.0f)
    };
    ASSERT_EQ(curve.SetControlPoints(points), NURBSCurve::ErrorCode::None);

    NURBSCurveParams params;
    params.degree = 2;
    ASSERT_EQ(curve.Configure(params), NURBSCurve::ErrorCode::None);

    // Sample original curve
    std::vector<glm::vec3> original_points;
    for (int i = 0; i <= 10; ++i) {
        float t = static_cast<float>(i) / 10.0f;
        original_points.push_back(curve.Evaluate(t));
    }

    // Elevate to cubic
    ASSERT_EQ(curve.ElevateDegree(3), NURBSCurve::ErrorCode::None);
    EXPECT_EQ(curve.GetDegree(), 3);

    // Verify curve shape preserved
    for (int i = 0; i <= 10; ++i) {
        float t = static_cast<float>(i) / 10.0f;
        glm::vec3 elevated = curve.Evaluate(t);
        EXPECT_NEAR(glm::length(elevated - original_points[i]), 0.0f, 1e-6f);
    }

    // Test invalid elevation
    EXPECT_EQ(curve.ElevateDegree(2), NURBSCurve::ErrorCode::InvalidMesh);  // Lower degree
    EXPECT_EQ(curve.ElevateDegree(3), NURBSCurve::ErrorCode::InvalidMesh);  // Same degree
}

TEST_F(NURBSCurveTest, DegreeReduction) {
    // Create cubic curve
    std::vector<glm::vec3> points = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 2.0f, 0.0f),
        glm::vec3(2.0f, 2.0f, 0.0f),
        glm::vec3(3.0f, 0.0f, 0.0f)
    };
    ASSERT_EQ(curve.SetControlPoints(points), NURBSCurve::ErrorCode::None);

    NURBSCurveParams params;
    params.degree = 3;
    params.tolerance = 0.01f;  // Allow some deviation
    ASSERT_EQ(curve.Configure(params), NURBSCurve::ErrorCode::None);

    // Sample original curve
    std::vector<glm::vec3> original_points;
    for (int i = 0; i <= 10; ++i) {
        float t = static_cast<float>(i) / 10.0f;
        original_points.push_back(curve.Evaluate(t));
    }

    // Reduce to quadratic
    ASSERT_EQ(curve.ReduceDegree(2), NURBSCurve::ErrorCode::None);
    EXPECT_EQ(curve.GetDegree(), 2);

    // Verify curve shape approximately preserved
    float max_error = 0.0f;
    for (int i = 0; i <= 10; ++i) {
        float t = static_cast<float>(i) / 10.0f;
        glm::vec3 reduced = curve.Evaluate(t);
        max_error = std::max(max_error, glm::length(reduced - original_points[i]));
    }
    EXPECT_LT(max_error, params.tolerance);

    // Test invalid reduction
    EXPECT_EQ(curve.ReduceDegree(0), NURBSCurve::ErrorCode::InvalidMesh);  // Below degree 1
    EXPECT_EQ(curve.ReduceDegree(2), NURBSCurve::ErrorCode::InvalidMesh);  // Same degree
    EXPECT_EQ(curve.ReduceDegree(3), NURBSCurve::ErrorCode::InvalidMesh);  // Higher degree
}

TEST_F(NURBSCurveTest, KnotInsertion) {
    // Create cubic curve
    std::vector<glm::vec3> points = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(2.0f, 1.0f, 0.0f),
        glm::vec3(3.0f, 0.0f, 0.0f)
    };
    ASSERT_EQ(curve.SetControlPoints(points), NURBSCurve::ErrorCode::None);

    NURBSCurveParams params;
    params.degree = 3;
    ASSERT_EQ(curve.Configure(params), NURBSCurve::ErrorCode::None);

    // Sample original curve
    std::vector<glm::vec3> original_points;
    for (int i = 0; i <= 10; ++i) {
        float t = static_cast<float>(i) / 10.0f;
        original_points.push_back(curve.Evaluate(t));
    }

    // Insert knot
    ASSERT_EQ(curve.InsertKnot(0.5f, 2), NURBSCurve::ErrorCode::None);

    // Verify curve shape preserved
    for (int i = 0; i <= 10; ++i) {
        float t = static_cast<float>(i) / 10.0f;
        glm::vec3 after_insert = curve.Evaluate(t);
        EXPECT_NEAR(glm::length(after_insert - original_points[i]), 0.0f, 1e-6f);
    }

    // Test invalid insertion
    EXPECT_EQ(curve.InsertKnot(-1.0f, 1), NURBSCurve::ErrorCode::InvalidMesh);  // Out of range
    EXPECT_EQ(curve.InsertKnot(2.0f, 1), NURBSCurve::ErrorCode::InvalidMesh);   // Out of range
    EXPECT_EQ(curve.InsertKnot(0.5f, 0), NURBSCurve::ErrorCode::InvalidMesh);   // Invalid multiplicity
}

TEST_F(NURBSCurveTest, KnotRemoval) {
    // Create cubic curve with multiple knots
    std::vector<glm::vec3> points = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(2.0f, 1.0f, 0.0f),
        glm::vec3(3.0f, 0.0f, 0.0f),
        glm::vec3(4.0f, 0.0f, 0.0f)
    };
    ASSERT_EQ(curve.SetControlPoints(points), NURBSCurve::ErrorCode::None);

    NURBSCurveParams params;
    params.degree = 3;
    params.tolerance = 0.01f;  // Allow some deviation
    ASSERT_EQ(curve.Configure(params), NURBSCurve::ErrorCode::None);

    // Insert knot to remove later
    ASSERT_EQ(curve.InsertKnot(0.5f, 2), NURBSCurve::ErrorCode::None);

    // Sample curve before removal
    std::vector<glm::vec3> original_points;
    for (int i = 0; i <= 10; ++i) {
        float t = static_cast<float>(i) / 10.0f;
        original_points.push_back(curve.Evaluate(t));
    }

    // Remove knot
    ASSERT_EQ(curve.RemoveKnot(0.5f, 1), NURBSCurve::ErrorCode::None);

    // Verify curve shape approximately preserved
    float max_error = 0.0f;
    for (int i = 0; i <= 10; ++i) {
        float t = static_cast<float>(i) / 10.0f;
        glm::vec3 after_remove = curve.Evaluate(t);
        max_error = std::max(max_error, glm::length(after_remove - original_points[i]));
    }
    EXPECT_LT(max_error, params.tolerance);

    // Test invalid removal
    EXPECT_EQ(curve.RemoveKnot(-1.0f, 1), NURBSCurve::ErrorCode::InvalidMesh);  // Out of range
    EXPECT_EQ(curve.RemoveKnot(2.0f, 1), NURBSCurve::ErrorCode::InvalidMesh);   // Out of range
    EXPECT_EQ(curve.RemoveKnot(0.5f, 0), NURBSCurve::ErrorCode::InvalidMesh);   // Invalid multiplicity
}

TEST_F(NURBSCurveTest, RationalDegreeElevation) {
    // Create rational quadratic curve (circle segment)
    std::vector<glm::vec3> points = {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    };
    std::vector<float> weights = {1.0f, 1.0f/std::sqrt(2.0f), 1.0f};
    
    ASSERT_EQ(curve.SetControlPoints(points), NURBSCurve::ErrorCode::None);
    ASSERT_EQ(curve.SetWeights(weights), NURBSCurve::ErrorCode::None);

    NURBSCurveParams params;
    params.degree = 2;
    params.rational = true;
    ASSERT_EQ(curve.Configure(params), NURBSCurve::ErrorCode::None);

    // Sample original curve
    std::vector<glm::vec3> original_points;
    for (int i = 0; i <= 10; ++i) {
        float t = static_cast<float>(i) / 10.0f;
        original_points.push_back(curve.Evaluate(t));
    }

    // Elevate to cubic
    ASSERT_EQ(curve.ElevateDegree(3), NURBSCurve::ErrorCode::None);
    EXPECT_EQ(curve.GetDegree(), 3);

    // Verify curve shape preserved
    for (int i = 0; i <= 10; ++i) {
        float t = static_cast<float>(i) / 10.0f;
        glm::vec3 elevated = curve.Evaluate(t);
        EXPECT_NEAR(glm::length(elevated - original_points[i]), 0.0f, 1e-6f);
    }
}

TEST_F(NURBSCurveTest, RationalDegreeReduction) {
    // Create rational cubic curve
    std::vector<glm::vec3> points = {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(-1.0f, 1.0f, 0.0f)
    };
    std::vector<float> weights = {1.0f, 0.8f, 0.8f, 1.0f};
    
    ASSERT_EQ(curve.SetControlPoints(points), NURBSCurve::ErrorCode::None);
    ASSERT_EQ(curve.SetWeights(weights), NURBSCurve::ErrorCode::None);

    NURBSCurveParams params;
    params.degree = 3;
    params.rational = true;
    params.tolerance = 0.01f;  // Allow some deviation
    ASSERT_EQ(curve.Configure(params), NURBSCurve::ErrorCode::None);

    // Sample original curve
    std::vector<glm::vec3> original_points;
    for (int i = 0; i <= 10; ++i) {
        float t = static_cast<float>(i) / 10.0f;
        original_points.push_back(curve.Evaluate(t));
    }

    // Reduce to quadratic
    ASSERT_EQ(curve.ReduceDegree(2), NURBSCurve::ErrorCode::None);
    EXPECT_EQ(curve.GetDegree(), 2);

    // Verify curve shape approximately preserved
    float max_error = 0.0f;
    for (int i = 0; i <= 10; ++i) {
        float t = static_cast<float>(i) / 10.0f;
        glm::vec3 reduced = curve.Evaluate(t);
        max_error = std::max(max_error, glm::length(reduced - original_points[i]));
    }
    EXPECT_LT(max_error, params.tolerance);
}

TEST_F(NURBSCurveTest, RationalKnotOperations) {
    // Create rational cubic curve
    std::vector<glm::vec3> points = {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(-1.0f, 1.0f, 0.0f)
    };
    std::vector<float> weights = {1.0f, 0.8f, 0.8f, 1.0f};
    
    ASSERT_EQ(curve.SetControlPoints(points), NURBSCurve::ErrorCode::None);
    ASSERT_EQ(curve.SetWeights(weights), NURBSCurve::ErrorCode::None);

    NURBSCurveParams params;
    params.degree = 3;
    params.rational = true;
    ASSERT_EQ(curve.Configure(params), NURBSCurve::ErrorCode::None);

    // Sample original curve
    std::vector<glm::vec3> original_points;
    for (int i = 0; i <= 10; ++i) {
        float t = static_cast<float>(i) / 10.0f;
        original_points.push_back(curve.Evaluate(t));
    }

    // Insert and remove knot
    ASSERT_EQ(curve.InsertKnot(0.5f, 2), NURBSCurve::ErrorCode::None);
    ASSERT_EQ(curve.RemoveKnot(0.5f, 1), NURBSCurve::ErrorCode::None);

    // Verify curve shape preserved
    float max_error = 0.0f;
    for (int i = 0; i <= 10; ++i) {
        float t = static_cast<float>(i) / 10.0f;
        glm::vec3 result = curve.Evaluate(t);
        max_error = std::max(max_error, glm::length(result - original_points[i]));
    }
    EXPECT_LT(max_error, params.tolerance);
}

TEST_F(NURBSCurveTest, PreviewNonRationalCurve) {
    // Create quadratic curve
    std::vector<glm::vec3> points = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(2.0f, 0.0f, 0.0f)
    };
    ASSERT_EQ(curve.SetControlPoints(points), NURBSCurve::ErrorCode::None);

    NURBSCurveParams params;
    params.degree = 2;
    ASSERT_EQ(curve.Configure(params), NURBSCurve::ErrorCode::None);

    // Create mock graphics system
    auto graphics = std::make_shared<RebelCAD::Testing::MockGraphicsSystem>();
    ASSERT_EQ(curve.Preview(graphics), NURBSCurve::ErrorCode::None);

    // Verify preview calls
    ASSERT_GE(graphics->draw_calls.size(), 2);  // At least curve and control points
    
    // First call should be curve with tangents
    EXPECT_TRUE(graphics->VerifyDrawCall(0, 33, 64, true));  // 32 segments = 33 points, 32*2 indices
    
    // Second call should be control points
    EXPECT_TRUE(graphics->VerifyDrawCall(1, 3, 4));  // 3 points, 2 line segments
}

TEST_F(NURBSCurveTest, PreviewRationalCurve) {
    // Create rational circle segment
    std::vector<glm::vec3> points = {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    };
    std::vector<float> weights = {1.0f, 1.0f/std::sqrt(2.0f), 1.0f};
    
    ASSERT_EQ(curve.SetControlPoints(points), NURBSCurve::ErrorCode::None);
    ASSERT_EQ(curve.SetWeights(weights), NURBSCurve::ErrorCode::None);

    NURBSCurveParams params;
    params.degree = 2;
    params.rational = true;
    ASSERT_EQ(curve.Configure(params), NURBSCurve::ErrorCode::None);

    // Create mock graphics system
    auto graphics = std::make_shared<RebelCAD::Testing::MockGraphicsSystem>();
    ASSERT_EQ(curve.Preview(graphics), NURBSCurve::ErrorCode::None);

    // Verify preview calls
    ASSERT_GE(graphics->draw_calls.size(), 2);
    
    // Verify curve geometry
    EXPECT_TRUE(graphics->VerifyDrawCall(0, 33, 64, true));
    
    // Verify control points
    EXPECT_TRUE(graphics->VerifyDrawCall(1, 3, 4));
}

TEST_F(NURBSCurveTest, PreviewAfterModification) {
    // Create initial curve
    std::vector<glm::vec3> points = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(2.0f, 0.0f, 0.0f)
    };
    ASSERT_EQ(curve.SetControlPoints(points), NURBSCurve::ErrorCode::None);

    NURBSCurveParams params;
    params.degree = 2;
    ASSERT_EQ(curve.Configure(params), NURBSCurve::ErrorCode::None);

    // Create mock graphics system
    auto graphics = std::make_shared<RebelCAD::Testing::MockGraphicsSystem>();
    
    // First preview
    ASSERT_EQ(curve.Preview(graphics), NURBSCurve::ErrorCode::None);
    size_t initial_calls = graphics->draw_calls.size();

    // Modify curve
    ASSERT_EQ(curve.InsertKnot(0.5f, 1), NURBSCurve::ErrorCode::None);
    graphics->Clear();

    // Preview after modification
    ASSERT_EQ(curve.Preview(graphics), NURBSCurve::ErrorCode::None);
    EXPECT_EQ(graphics->draw_calls.size(), initial_calls);  // Same number of draw calls
    
    // Verify updated geometry
    EXPECT_TRUE(graphics->VerifyDrawCall(0, 33, 64, true));
    EXPECT_TRUE(graphics->VerifyDrawCall(1, 4, 6));  // One more control point after knot insertion
}

TEST_F(NURBSCurveTest, PreviewErrorHandling) {
    // Test uninitialized curve
    auto graphics = std::make_shared<RebelCAD::Testing::MockGraphicsSystem>();
    EXPECT_EQ(curve.Preview(graphics), NURBSCurve::ErrorCode::CADError);
    EXPECT_TRUE(graphics->draw_calls.empty());

    // Test null graphics system
    std::vector<glm::vec3> points = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(2.0f, 0.0f, 0.0f)
    };
    ASSERT_EQ(curve.SetControlPoints(points), NURBSCurve::ErrorCode::None);

    NURBSCurveParams params;
    params.degree = 2;
    ASSERT_EQ(curve.Configure(params), NURBSCurve::ErrorCode::None);

    EXPECT_EQ(curve.Preview(nullptr), NURBSCurve::ErrorCode::InvalidMesh);
}

TEST_F(NURBSCurveTest, SplitNonRationalCurve) {
    // Create cubic curve
    std::vector<glm::vec3> points = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(2.0f, 1.0f, 0.0f),
        glm::vec3(3.0f, 0.0f, 0.0f)
    };
    ASSERT_EQ(curve.SetControlPoints(points), NURBSCurve::ErrorCode::None);

    NURBSCurveParams params;
    params.degree = 3;
    ASSERT_EQ(curve.Configure(params), NURBSCurve::ErrorCode::None);

    // Sample original curve
    std::vector<glm::vec3> original_points;
    for (int i = 0; i <= 10; ++i) {
        float t = static_cast<float>(i) / 10.0f;
        original_points.push_back(curve.Evaluate(t));
    }

    // Split curve at midpoint
    NURBSCurve left, right;
    ASSERT_EQ(curve.Split(0.5f, left, right), NURBSCurve::ErrorCode::None);

    // Verify left curve matches original for t in [0, 0.5]
    for (int i = 0; i <= 5; ++i) {
        float t = static_cast<float>(i) / 10.0f;
        float left_t = t * 2.0f;  // Map [0, 0.5] to [0, 1]
        glm::vec3 left_point = left.Evaluate(left_t);
        EXPECT_NEAR(glm::length(left_point - original_points[i]), 0.0f, 1e-6f);
    }

    // Verify right curve matches original for t in [0.5, 1]
    for (int i = 5; i <= 10; ++i) {
        float t = static_cast<float>(i) / 10.0f;
        float right_t = (t - 0.5f) * 2.0f;  // Map [0.5, 1] to [0, 1]
        glm::vec3 right_point = right.Evaluate(right_t);
        EXPECT_NEAR(glm::length(right_point - original_points[i]), 0.0f, 1e-6f);
    }

    // Test invalid split parameters
    EXPECT_EQ(curve.Split(-0.1f, left, right), NURBSCurve::ErrorCode::InvalidMesh);
    EXPECT_EQ(curve.Split(1.1f, left, right), NURBSCurve::ErrorCode::InvalidMesh);
}

TEST_F(NURBSCurveTest, SplitRationalCurve) {
    // Create rational circle segment
    std::vector<glm::vec3> points = {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    };
    std::vector<float> weights = {1.0f, 1.0f/std::sqrt(2.0f), 1.0f};
    
    ASSERT_EQ(curve.SetControlPoints(points), NURBSCurve::ErrorCode::None);
    ASSERT_EQ(curve.SetWeights(weights), NURBSCurve::ErrorCode::None);

    NURBSCurveParams params;
    params.degree = 2;
    params.rational = true;
    ASSERT_EQ(curve.Configure(params), NURBSCurve::ErrorCode::None);

    // Sample original curve
    std::vector<glm::vec3> original_points;
    for (int i = 0; i <= 10; ++i) {
        float t = static_cast<float>(i) / 10.0f;
        original_points.push_back(curve.Evaluate(t));
    }

    // Split curve at midpoint
    NURBSCurve left, right;
    ASSERT_EQ(curve.Split(0.5f, left, right), NURBSCurve::ErrorCode::None);

    // Verify left curve matches original for t in [0, 0.5]
    for (int i = 0; i <= 5; ++i) {
        float t = static_cast<float>(i) / 10.0f;
        float left_t = t * 2.0f;  // Map [0, 0.5] to [0, 1]
        glm::vec3 left_point = left.Evaluate(left_t);
        EXPECT_NEAR(glm::length(left_point - original_points[i]), 0.0f, 1e-6f);
    }

    // Verify right curve matches original for t in [0.5, 1]
    for (int i = 5; i <= 10; ++i) {
        float t = static_cast<float>(i) / 10.0f;
        float right_t = (t - 0.5f) * 2.0f;  // Map [0.5, 1] to [0, 1]
        glm::vec3 right_point = right.Evaluate(right_t);
        EXPECT_NEAR(glm::length(right_point - original_points[i]), 0.0f, 1e-6f);
    }
}

TEST_F(NURBSCurveTest, JoinNonRationalCurves) {
    // Create first curve
    std::vector<glm::vec3> points1 = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(2.0f, 0.0f, 0.0f)
    };
    ASSERT_EQ(curve.SetControlPoints(points1), NURBSCurve::ErrorCode::None);

    NURBSCurveParams params;
    params.degree = 2;
    ASSERT_EQ(curve.Configure(params), NURBSCurve::ErrorCode::None);

    // Create second curve
    std::vector<glm::vec3> points2 = {
        glm::vec3(2.0f, 0.0f, 0.0f),
        glm::vec3(3.0f, -1.0f, 0.0f),
        glm::vec3(4.0f, 0.0f, 0.0f)
    };
    NURBSCurve curve2;
    ASSERT_EQ(curve2.SetControlPoints(points2), NURBSCurve::ErrorCode::None);
    ASSERT_EQ(curve2.Configure(params), NURBSCurve::ErrorCode::None);

    // Join curves
    ASSERT_EQ(curve.Join(curve2), NURBSCurve::ErrorCode::None);

    // Verify joined curve
    EXPECT_EQ(curve.Evaluate(0.0f), points1[0]);
    EXPECT_EQ(curve.Evaluate(1.0f), points2.back());

    // Test invalid joins
    NURBSCurve invalid_curve;
    EXPECT_EQ(curve.Join(invalid_curve), NURBSCurve::ErrorCode::CADError);  // Unconfigured curve

    params.degree = 3;
    ASSERT_EQ(invalid_curve.SetControlPoints(points2), NURBSCurve::ErrorCode::None);
    ASSERT_EQ(invalid_curve.Configure(params), NURBSCurve::ErrorCode::None);
    EXPECT_EQ(curve.Join(invalid_curve), NURBSCurve::ErrorCode::InvalidMesh);  // Different degree
}

TEST_F(NURBSCurveTest, JoinRationalCurves) {
    // Create first rational curve
    std::vector<glm::vec3> points1 = {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    };
    std::vector<float> weights1 = {1.0f, 1.0f/std::sqrt(2.0f), 1.0f};
    
    ASSERT_EQ(curve.SetControlPoints(points1), NURBSCurve::ErrorCode::None);
    ASSERT_EQ(curve.SetWeights(weights1), NURBSCurve::ErrorCode::None);

    NURBSCurveParams params;
    params.degree = 2;
    params.rational = true;
    ASSERT_EQ(curve.Configure(params), NURBSCurve::ErrorCode::None);

    // Create second rational curve
    std::vector<glm::vec3> points2 = {
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(-1.0f, 1.0f, 0.0f),
        glm::vec3(-1.0f, 0.0f, 0.0f)
    };
    std::vector<float> weights2 = {1.0f, 1.0f/std::sqrt(2.0f), 1.0f};
    
    NURBSCurve curve2;
    ASSERT_EQ(curve2.SetControlPoints(points2), NURBSCurve::ErrorCode::None);
    ASSERT_EQ(curve2.SetWeights(weights2), NURBSCurve::ErrorCode::None);
    ASSERT_EQ(curve2.Configure(params), NURBSCurve::ErrorCode::None);

    // Join curves
    ASSERT_EQ(curve.Join(curve2), NURBSCurve::ErrorCode::None);

    // Verify joined curve
    EXPECT_EQ(curve.Evaluate(0.0f), points1[0]);
    EXPECT_EQ(curve.Evaluate(1.0f), points2.back());

    // Test joining rational with non-rational
    NURBSCurve non_rational;
    ASSERT_EQ(non_rational.SetControlPoints(points2), NURBSCurve::ErrorCode::None);
    params.rational = false;
    ASSERT_EQ(non_rational.Configure(params), NURBSCurve::ErrorCode::None);
    EXPECT_EQ(curve.Join(non_rational), NURBSCurve::ErrorCode::InvalidMesh);
}

TEST_F(NURBSCurveTest, ErrorHandling) {
    // Test uninitialized state
    EXPECT_EQ(curve.Evaluate(0.5f), glm::vec3(0.0f));
    EXPECT_TRUE(curve.EvaluateDerivatives(0.5f, 1).empty());
    EXPECT_EQ(curve.GenerateGeometry(10), NURBSCurve::ErrorCode::CADError);
    EXPECT_FALSE(curve.IsValid());

    // Initialize with valid data
    std::vector<glm::vec3> points = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(2.0f, 0.0f, 0.0f)
    };
    ASSERT_EQ(curve.SetControlPoints(points), NURBSCurve::ErrorCode::None);

    NURBSCurveParams params;
    params.degree = 2;
    ASSERT_EQ(curve.Configure(params), NURBSCurve::ErrorCode::None);

    // Test invalid parameter values
    EXPECT_EQ(curve.Evaluate(INFINITY), glm::vec3(0.0f));
    EXPECT_EQ(curve.Evaluate(NAN), glm::vec3(0.0f));

    // Test invalid geometry generation
    EXPECT_EQ(curve.GenerateGeometry(0), NURBSCurve::ErrorCode::InvalidMesh);
    EXPECT_EQ(curve.GenerateGeometry(-1), NURBSCurve::ErrorCode::InvalidMesh);
}

} // namespace Modeling
} // namespace RebelCAD
