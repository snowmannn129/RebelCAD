#include "modeling/LoftTool.h"
#include "core/Log.h"
#include <cmath>
#include <algorithm>

namespace RebelCAD {
namespace Modeling {

// Helper functions for the LoftTool class
namespace {
    std::pair<float, float> CalculateProfileNormal(const std::vector<std::pair<double, double>>& points) {
        // Calculate average normal vector for the profile
        float nx = 0.0f, ny = 0.0f;
        for (size_t i = 0; i < points.size() - 1; ++i) {
            float dx = static_cast<float>(points[i+1].first - points[i].first);
            float dy = static_cast<float>(points[i+1].second - points[i].second);
            float length = std::sqrt(dx*dx + dy*dy);
            if (length > 1e-6f) {
                nx += -dy/length;
                ny += dx/length;
            }
        }
        float norm = std::sqrt(nx*nx + ny*ny);
        if (norm > 1e-6f) {
            return {nx/norm, ny/norm};
        }
        return {0.0f, 1.0f};
    }

    std::pair<float, float> CalculateGuideTangent(const std::shared_ptr<Sketching::Spline>& guide, float t) {
        auto points = guide->calculateCurvePoints();
        size_t n = points.size();
        if (n < 2) return {0.0f, 1.0f};
        
        // Find segment containing parameter t
        float segment_t = t * (n - 1);
        size_t i = static_cast<size_t>(segment_t);
        if (i >= n - 1) i = n - 2;
        
        float dx = static_cast<float>(points[i+1].first - points[i].first);
        float dy = static_cast<float>(points[i+1].second - points[i].second);
        float length = std::sqrt(dx*dx + dy*dy);
        
        if (length > 1e-6f) {
            return {dx/length, dy/length};
        }
        return {0.0f, 1.0f};
    }
}

class LoftTool::Implementation {
public:
    Implementation() 
        : is_configured(false) {
    }

    std::vector<std::shared_ptr<Sketching::Spline>> profiles;
    std::vector<std::shared_ptr<Sketching::Spline>> guideCurves;
    std::vector<float> generated_mesh;
    bool is_configured;
    LoftOptions current_options;
};

LoftTool::LoftTool() : impl(std::make_unique<Implementation>()) {}
LoftTool::~LoftTool() = default;

Error LoftTool::AddProfile(const std::shared_ptr<Sketching::Spline>& profile) {
    if (!profile) {
        return Error(rebel::core::ErrorCode::GeometryError, "Invalid profile: null pointer provided");
    }

    // Get points defining the profile
    auto points = profile->calculateCurvePoints();
    
    // Validate profile is a closed contour by checking if start and end points match
    if (points.size() < 3 || 
        std::abs(points.front().first - points.back().first) > 1e-6 ||
        std::abs(points.front().second - points.back().second) > 1e-6) {
        return Error(rebel::core::ErrorCode::GeometryError, "Profile must be a closed contour");
    }

    impl->profiles.push_back(profile);
    impl->is_configured = false; // Reset configuration when profile changes
    return Error(rebel::core::ErrorCode::None, "Profile added successfully");
}

Error LoftTool::AddGuideCurve(const std::shared_ptr<Sketching::Spline>& guide) {
    if (!guide) {
        return Error(rebel::core::ErrorCode::GeometryError, "Invalid guide curve: null pointer provided");
    }

    impl->guideCurves.push_back(guide);
    impl->is_configured = false; // Reset configuration when guides change
    return Error(rebel::core::ErrorCode::None, "Guide curve added successfully");
}

void LoftTool::Reset() {
    impl->profiles.clear();
    impl->guideCurves.clear();
    impl->generated_mesh.clear();
    impl->is_configured = false;
}

size_t LoftTool::GetProfileCount() const {
    return impl->profiles.size();
}

size_t LoftTool::GetGuideCurveCount() const {
    return impl->guideCurves.size();
}

Error LoftTool::ValidateInput() const {
    if (impl->profiles.size() < 2) {
        return Error(rebel::core::ErrorCode::CADError, "At least two profiles are required for lofting");
    }

    // Get points for first profile to establish baseline
    auto basePoints = impl->profiles[0]->calculateCurvePoints();
    size_t basePointCount = basePoints.size();

    // Verify all profiles have similar point count (within tolerance)
    for (size_t i = 1; i < impl->profiles.size(); ++i) {
        auto points = impl->profiles[i]->calculateCurvePoints();
        if (std::abs(static_cast<int>(points.size()) - static_cast<int>(basePointCount)) > 5) {
            return Error(rebel::core::ErrorCode::CADError, 
                "All profiles must have similar number of points");
        }
    }

    // For guide curves, we'll validate during lofting since intersection
    // checking requires more complex 3D geometry calculations

    return Error(rebel::core::ErrorCode::None, "Input validation successful");
}

Error LoftTool::Execute(const LoftOptions& options) {
    Error validation = ValidateInput();
    if (validation.code() != rebel::core::ErrorCode::None) {
        return validation;
    }

    impl->current_options = options;
    
    Error err = CreateLoftGeometry(options);
    if (err.code() != rebel::core::ErrorCode::None) {
        return err;
    }

    impl->is_configured = true;
    return Error(rebel::core::ErrorCode::None, "Loft operation executed successfully");
}

Error LoftTool::Preview(std::shared_ptr<Graphics::GraphicsSystem> graphics) {
    if (!graphics) {
        return Error(rebel::core::ErrorCode::CADError, "Invalid graphics system provided");
    }

    if (impl->profiles.size() < 2) {
        return Error(rebel::core::ErrorCode::CADError, "Cannot preview: insufficient profiles");
    }

    Error err = CreateLoftGeometry(impl->current_options);
    if (err.code() != rebel::core::ErrorCode::None) {
        return err;
    }

    // Set preview rendering state
    graphics->setRenderMode(Graphics::RenderMode::Preview);
    graphics->setColor(Graphics::Color(0.7f, 0.7f, 1.0f, 0.6f)); // Semi-transparent blue

    // Render the generated mesh as triangles
    if (!impl->generated_mesh.empty()) {
        // Each triangle consists of 3 vertices, each vertex has 3 coordinates
        const size_t verticesPerTriangle = 3;
        const size_t coordsPerVertex = 3;
        const size_t stride = verticesPerTriangle * coordsPerVertex;

        // Create vertex buffer for rendering
        std::vector<float> renderBuffer;
        renderBuffer.reserve(impl->generated_mesh.size());

        // Convert quad strips to triangles for rendering
        for (size_t i = 0; i < impl->generated_mesh.size() - stride; i += stride) {
            // First triangle of quad
            renderBuffer.push_back(impl->generated_mesh[i]);
            renderBuffer.push_back(impl->generated_mesh[i + 1]);
            renderBuffer.push_back(impl->generated_mesh[i + 2]);
            
            renderBuffer.push_back(impl->generated_mesh[i + 3]);
            renderBuffer.push_back(impl->generated_mesh[i + 4]);
            renderBuffer.push_back(impl->generated_mesh[i + 5]);
            
            renderBuffer.push_back(impl->generated_mesh[i + 6]);
            renderBuffer.push_back(impl->generated_mesh[i + 7]);
            renderBuffer.push_back(impl->generated_mesh[i + 8]);

            // Second triangle of quad
            renderBuffer.push_back(impl->generated_mesh[i + 3]);
            renderBuffer.push_back(impl->generated_mesh[i + 4]);
            renderBuffer.push_back(impl->generated_mesh[i + 5]);
            
            renderBuffer.push_back(impl->generated_mesh[i + 6]);
            renderBuffer.push_back(impl->generated_mesh[i + 7]);
            renderBuffer.push_back(impl->generated_mesh[i + 8]);
            
            renderBuffer.push_back(impl->generated_mesh[i + 9]);
            renderBuffer.push_back(impl->generated_mesh[i + 10]);
            renderBuffer.push_back(impl->generated_mesh[i + 11]);
        }

        // Render the preview mesh
        graphics->beginPreview();
        graphics->renderTriangleMesh(renderBuffer.data(), renderBuffer.size() / 3);
        graphics->endPreview();
    }

    return Error(rebel::core::ErrorCode::None, "Preview rendered successfully");
}

void LoftTool::Cancel() {
    impl->is_configured = false;
    impl->generated_mesh.clear();
}

Error LoftTool::CreateLoftGeometry(const LoftOptions& options) {
    impl->generated_mesh.clear();

    // Get points for each profile
    std::vector<std::vector<std::pair<double, double>>> profile_points;
    size_t min_points = SIZE_MAX;
    
    for (const auto& profile : impl->profiles) {
        auto points = profile->calculateCurvePoints();
        profile_points.push_back(points);
        min_points = std::min(min_points, points.size());
    }

    size_t profile_count = impl->profiles.size();

    // Reserve space for vertices (3 coordinates per point)
    impl->generated_mesh.reserve(min_points * profile_count * 3);

    if (options.transitionType == TransitionType::Ruled) {
        // Simple linear interpolation between corresponding points
        for (size_t i = 0; i < profile_count - 1; ++i) {
            const auto& points1 = profile_points[i];
            const auto& points2 = profile_points[i + 1];

            for (size_t v = 0; v < min_points; ++v) {
                // First profile point
                impl->generated_mesh.push_back(static_cast<float>(points1[v].first));   // x
                impl->generated_mesh.push_back(static_cast<float>(points1[v].second));  // y
                impl->generated_mesh.push_back(static_cast<float>(i));                  // z

                // Second profile point
                impl->generated_mesh.push_back(static_cast<float>(points2[v].first));   // x
                impl->generated_mesh.push_back(static_cast<float>(points2[v].second));  // y
                impl->generated_mesh.push_back(static_cast<float>(i + 1));             // z
            }
        }
    } else {
        // Smooth interpolation using guide curves and tangency conditions
        std::vector<std::vector<float>> interpolated_points;
        
        // Calculate interpolation parameters based on guide curves
        std::vector<float> t_params(profile_count);
        if (!impl->guideCurves.empty()) {
            // Use guide curve length distribution for parameterization
            float total_length = 0.0f;
            auto guide_points = impl->guideCurves[0]->calculateCurvePoints();
            for (size_t i = 1; i < guide_points.size(); ++i) {
                float dx = guide_points[i].first - guide_points[i-1].first;
                float dy = guide_points[i].second - guide_points[i-1].second;
                total_length += std::sqrt(dx*dx + dy*dy);
            }
            
            // Normalize parameters
            float current_length = 0.0f;
            t_params[0] = 0.0f;
            for (size_t i = 1; i < profile_count; ++i) {
                current_length += total_length / (profile_count - 1);
                t_params[i] = current_length / total_length;
            }
        } else {
            // Uniform parameterization if no guide curves
            for (size_t i = 0; i < profile_count; ++i) {
                t_params[i] = static_cast<float>(i) / (profile_count - 1);
            }
        }

        // Calculate tangent vectors based on conditions
        std::vector<std::pair<float, float>> start_tangents, end_tangents;
        for (size_t i = 0; i < profile_count; ++i) {
            std::pair<float, float> tangent;
            
            switch (options.startCondition) {
                case TangencyCondition::Normal:
                    // Use profile normal as tangent
                    tangent = CalculateProfileNormal(profile_points[i]);
                    break;
                case TangencyCondition::Custom:
                    // TODO: Allow custom tangent specification
                    tangent = {0.0f, 1.0f};
                    break;
                case TangencyCondition::Natural:
                default:
                    // Use guide curve tangent or default
                    tangent = !impl->guideCurves.empty() ? 
                        CalculateGuideTangent(impl->guideCurves[0], t_params[i]) :
                        std::make_pair(0.0f, 1.0f);
                    break;
            }
            start_tangents.push_back(tangent);
            
            // Similar for end conditions
            switch (options.endCondition) {
                case TangencyCondition::Normal:
                    tangent = CalculateProfileNormal(profile_points[i]);
                    break;
                case TangencyCondition::Custom:
                    tangent = {0.0f, 1.0f};
                    break;
                case TangencyCondition::Natural:
                default:
                    tangent = !impl->guideCurves.empty() ? 
                        CalculateGuideTangent(impl->guideCurves[0], t_params[i]) :
                        std::make_pair(0.0f, 1.0f);
                    break;
            }
            end_tangents.push_back(tangent);
        }

        // Generate smooth interpolation for each corresponding point set
        for (size_t v = 0; v < min_points; ++v) {
            std::vector<float> x_coords(profile_count);
            std::vector<float> y_coords(profile_count);
            
            // Collect corresponding points from all profiles
            for (size_t i = 0; i < profile_count; ++i) {
                x_coords[i] = static_cast<float>(profile_points[i][v].first);
                y_coords[i] = static_cast<float>(profile_points[i][v].second);
            }
            
            // Generate interpolated points using Catmull-Rom spline
            const size_t num_segments = 8; // Points per segment
            for (size_t i = 0; i < profile_count - 1; ++i) {
                for (size_t j = 0; j <= num_segments; ++j) {
                    float t = static_cast<float>(j) / num_segments;
                    float h00 = 2*t*t*t - 3*t*t + 1;
                    float h10 = t*t*t - 2*t*t + t;
                    float h01 = -2*t*t*t + 3*t*t;
                    float h11 = t*t*t - t*t;
                    
                    float x = h00 * x_coords[i] + 
                             h10 * start_tangents[i].first + 
                             h01 * x_coords[i+1] + 
                             h11 * end_tangents[i+1].first;
                             
                    float y = h00 * y_coords[i] + 
                             h10 * start_tangents[i].second + 
                             h01 * y_coords[i+1] + 
                             h11 * end_tangents[i+1].second;
                    
                    float z = t_params[i] + t * (t_params[i+1] - t_params[i]);
                    
                    impl->generated_mesh.push_back(x);
                    impl->generated_mesh.push_back(y);
                    impl->generated_mesh.push_back(z);
                }
            }
        }
    }

    // Handle closed loft if specified
    if (options.closed && profile_count > 2) {
        const auto& first_points = profile_points[0];
        const auto& last_points = profile_points[profile_count - 1];

        for (size_t v = 0; v < min_points; ++v) {
            // Last profile point
            impl->generated_mesh.push_back(static_cast<float>(last_points[v].first));   // x
            impl->generated_mesh.push_back(static_cast<float>(last_points[v].second));  // y
            impl->generated_mesh.push_back(static_cast<float>(profile_count - 1));      // z

            // First profile point (to close the loop)
            impl->generated_mesh.push_back(static_cast<float>(first_points[v].first));  // x
            impl->generated_mesh.push_back(static_cast<float>(first_points[v].second)); // y
            impl->generated_mesh.push_back(0.0f);                                       // z
        }
    }

    return Error(rebel::core::ErrorCode::None, "Geometry created successfully");
}

} // namespace Modeling
} // namespace RebelCAD
