#include "modeling/NURBSCurve.h"
#include <benchmark/benchmark.h>
#include <glm/glm.hpp>
#include <vector>
#include <random>

namespace RebelCAD {
namespace Modeling {
namespace {

// Helper to create random control points
std::vector<glm::vec3> CreateRandomControlPoints(size_t count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-10.0f, 10.0f);

    std::vector<glm::vec3> points;
    points.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        points.emplace_back(dist(gen), dist(gen), dist(gen));
    }
    return points;
}

// Helper to create random weights
std::vector<float> CreateRandomWeights(size_t count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.1f, 2.0f);

    std::vector<float> weights;
    weights.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        weights.push_back(dist(gen));
    }
    return weights;
}

static void BM_CurveEvaluation(benchmark::State& state) {
    const size_t num_points = state.range(0);
    const int degree = state.range(1);
    
    // Setup curve
    NURBSCurve curve;
    curve.SetControlPoints(CreateRandomControlPoints(num_points));
    
    NURBSCurveParams params;
    params.degree = degree;
    curve.Configure(params);

    // Run benchmark
    for (auto _ : state) {
        for (int i = 0; i <= 100; ++i) {
            float t = static_cast<float>(i) / 100.0f;
            benchmark::DoNotOptimize(curve.Evaluate(t));
        }
    }
}

static void BM_RationalCurveEvaluation(benchmark::State& state) {
    const size_t num_points = state.range(0);
    const int degree = state.range(1);
    
    // Setup curve
    NURBSCurve curve;
    curve.SetControlPoints(CreateRandomControlPoints(num_points));
    curve.SetWeights(CreateRandomWeights(num_points));
    
    NURBSCurveParams params;
    params.degree = degree;
    params.rational = true;
    curve.Configure(params);

    // Run benchmark
    for (auto _ : state) {
        for (int i = 0; i <= 100; ++i) {
            float t = static_cast<float>(i) / 100.0f;
            benchmark::DoNotOptimize(curve.Evaluate(t));
        }
    }
}

static void BM_DegreeElevation(benchmark::State& state) {
    const size_t num_points = state.range(0);
    const int start_degree = state.range(1);
    const int elevation = 2;  // Elevate by 2 degrees
    
    for (auto _ : state) {
        state.PauseTiming();
        // Setup fresh curve for each iteration
        NURBSCurve curve;
        curve.SetControlPoints(CreateRandomControlPoints(num_points));
        
        NURBSCurveParams params;
        params.degree = start_degree;
        curve.Configure(params);
        state.ResumeTiming();

        // Perform degree elevation
        benchmark::DoNotOptimize(curve.ElevateDegree(start_degree + elevation));
    }
}

static void BM_DegreeReduction(benchmark::State& state) {
    const size_t num_points = state.range(0);
    const int start_degree = state.range(1);
    const int reduction = 1;  // Reduce by 1 degree
    
    for (auto _ : state) {
        state.PauseTiming();
        // Setup fresh curve for each iteration
        NURBSCurve curve;
        curve.SetControlPoints(CreateRandomControlPoints(num_points));
        
        NURBSCurveParams params;
        params.degree = start_degree;
        params.tolerance = 0.01f;
        curve.Configure(params);
        state.ResumeTiming();

        // Perform degree reduction
        benchmark::DoNotOptimize(curve.ReduceDegree(start_degree - reduction));
    }
}

static void BM_KnotInsertion(benchmark::State& state) {
    const size_t num_points = state.range(0);
    const int degree = state.range(1);
    
    for (auto _ : state) {
        state.PauseTiming();
        // Setup fresh curve for each iteration
        NURBSCurve curve;
        curve.SetControlPoints(CreateRandomControlPoints(num_points));
        
        NURBSCurveParams params;
        params.degree = degree;
        curve.Configure(params);
        state.ResumeTiming();

        // Insert knot at midpoint
        benchmark::DoNotOptimize(curve.InsertKnot(0.5f, 2));
    }
}

static void BM_KnotRemoval(benchmark::State& state) {
    const size_t num_points = state.range(0);
    const int degree = state.range(1);
    
    for (auto _ : state) {
        state.PauseTiming();
        // Setup fresh curve for each iteration
        NURBSCurve curve;
        curve.SetControlPoints(CreateRandomControlPoints(num_points));
        
        NURBSCurveParams params;
        params.degree = degree;
        params.tolerance = 0.01f;
        curve.Configure(params);

        // Insert knot to remove
        curve.InsertKnot(0.5f, 2);
        state.ResumeTiming();

        // Remove knot
        benchmark::DoNotOptimize(curve.RemoveKnot(0.5f, 1));
    }
}

// Register benchmarks with different control point counts and degrees
BENCHMARK(BM_CurveEvaluation)
    ->Args({10, 3})   // 10 points, cubic
    ->Args({20, 3})   // 20 points, cubic
    ->Args({50, 3})   // 50 points, cubic
    ->Args({10, 5})   // 10 points, degree 5
    ->Args({20, 5})   // 20 points, degree 5
    ->Args({50, 5});  // 50 points, degree 5

BENCHMARK(BM_RationalCurveEvaluation)
    ->Args({10, 3})
    ->Args({20, 3})
    ->Args({50, 3})
    ->Args({10, 5})
    ->Args({20, 5})
    ->Args({50, 5});

BENCHMARK(BM_DegreeElevation)
    ->Args({10, 3})
    ->Args({20, 3})
    ->Args({50, 3})
    ->Args({10, 5})
    ->Args({20, 5})
    ->Args({50, 5});

BENCHMARK(BM_DegreeReduction)
    ->Args({10, 3})
    ->Args({20, 3})
    ->Args({50, 3})
    ->Args({10, 5})
    ->Args({20, 5})
    ->Args({50, 5});

BENCHMARK(BM_KnotInsertion)
    ->Args({10, 3})
    ->Args({20, 3})
    ->Args({50, 3})
    ->Args({10, 5})
    ->Args({20, 5})
    ->Args({50, 5});

BENCHMARK(BM_KnotRemoval)
    ->Args({10, 3})
    ->Args({20, 3})
    ->Args({50, 3})
    ->Args({10, 5})
    ->Args({20, 5})
    ->Args({50, 5});

static void BM_CurveSplit(benchmark::State& state) {
    const size_t num_points = state.range(0);
    const int degree = state.range(1);
    
    for (auto _ : state) {
        state.PauseTiming();
        // Setup fresh curve for each iteration
        NURBSCurve curve;
        curve.SetControlPoints(CreateRandomControlPoints(num_points));
        
        NURBSCurveParams params;
        params.degree = degree;
        curve.Configure(params);
        state.ResumeTiming();

        // Split curve at midpoint
        NURBSCurve left, right;
        benchmark::DoNotOptimize(curve.Split(0.5f, left, right));
    }
}

static void BM_RationalCurveSplit(benchmark::State& state) {
    const size_t num_points = state.range(0);
    const int degree = state.range(1);
    
    for (auto _ : state) {
        state.PauseTiming();
        // Setup fresh curve for each iteration
        NURBSCurve curve;
        curve.SetControlPoints(CreateRandomControlPoints(num_points));
        curve.SetWeights(CreateRandomWeights(num_points));
        
        NURBSCurveParams params;
        params.degree = degree;
        params.rational = true;
        curve.Configure(params);
        state.ResumeTiming();

        // Split curve at midpoint
        NURBSCurve left, right;
        benchmark::DoNotOptimize(curve.Split(0.5f, left, right));
    }
}

static void BM_CurveJoin(benchmark::State& state) {
    const size_t num_points = state.range(0);
    const int degree = state.range(1);
    
    for (auto _ : state) {
        state.PauseTiming();
        // Setup two fresh curves for each iteration
        NURBSCurve curve1, curve2;
        auto points1 = CreateRandomControlPoints(num_points);
        auto points2 = points1;  // Ensure endpoints match
        curve1.SetControlPoints(points1);
        curve2.SetControlPoints(points2);
        
        NURBSCurveParams params;
        params.degree = degree;
        curve1.Configure(params);
        curve2.Configure(params);
        state.ResumeTiming();

        // Join curves
        benchmark::DoNotOptimize(curve1.Join(curve2));
    }
}

static void BM_RationalCurveJoin(benchmark::State& state) {
    const size_t num_points = state.range(0);
    const int degree = state.range(1);
    
    for (auto _ : state) {
        state.PauseTiming();
        // Setup two fresh curves for each iteration
        NURBSCurve curve1, curve2;
        auto points1 = CreateRandomControlPoints(num_points);
        auto points2 = points1;  // Ensure endpoints match
        auto weights1 = CreateRandomWeights(num_points);
        auto weights2 = weights1;  // Ensure weights match at join
        
        curve1.SetControlPoints(points1);
        curve2.SetControlPoints(points2);
        curve1.SetWeights(weights1);
        curve2.SetWeights(weights2);
        
        NURBSCurveParams params;
        params.degree = degree;
        params.rational = true;
        curve1.Configure(params);
        curve2.Configure(params);
        state.ResumeTiming();

        // Join curves
        benchmark::DoNotOptimize(curve1.Join(curve2));
    }
}

BENCHMARK(BM_CurveSplit)
    ->Args({10, 3})
    ->Args({20, 3})
    ->Args({50, 3})
    ->Args({10, 5})
    ->Args({20, 5})
    ->Args({50, 5});

BENCHMARK(BM_RationalCurveSplit)
    ->Args({10, 3})
    ->Args({20, 3})
    ->Args({50, 3})
    ->Args({10, 5})
    ->Args({20, 5})
    ->Args({50, 5});

BENCHMARK(BM_CurveJoin)
    ->Args({10, 3})
    ->Args({20, 3})
    ->Args({50, 3})
    ->Args({10, 5})
    ->Args({20, 5})
    ->Args({50, 5});

BENCHMARK(BM_RationalCurveJoin)
    ->Args({10, 3})
    ->Args({20, 3})
    ->Args({50, 3})
    ->Args({10, 5})
    ->Args({20, 5})
    ->Args({50, 5});

} // namespace
} // namespace Modeling
} // namespace RebelCAD
