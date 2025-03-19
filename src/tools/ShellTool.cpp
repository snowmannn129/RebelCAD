#include "modeling/ShellTool.h"
#include "core/Error.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace RebelCAD {
namespace Modeling {

class ShellTool::Implementation {
public:
    Implementation() 
        : is_configured(false)
        , body(nullptr)
        , thickness(0.0)
    {}

    std::shared_ptr<SolidBody> body;
    double thickness;
    std::vector<int> removedFaces;
    bool is_configured;
};

ShellTool::ShellTool() : impl(std::make_unique<Implementation>()) {}
ShellTool::~ShellTool() = default;

rebel::core::Error ShellTool::Configure(
    const std::shared_ptr<SolidBody>& body,
    double thickness,
    const std::vector<int>& removedFaces
) {
    if (!body) {
        return rebel::core::Error(
            rebel::core::ErrorCode::CADError,
            "Input body is null"
        );
    }

    if (thickness <= 0) {
        return rebel::core::Error(
            rebel::core::ErrorCode::CADError,
            "Shell thickness must be positive"
        );
    }

    impl->body = body;
    impl->thickness = thickness;
    impl->removedFaces = removedFaces;
    impl->is_configured = true;

    return rebel::core::Error(rebel::core::ErrorCode::None, "Configuration successful");
}

rebel::core::Error ShellTool::Execute() {
    if (!impl->is_configured) {
        return rebel::core::Error(
            rebel::core::ErrorCode::CADError,
            "Shell tool not configured"
        );
    }

    try {
        // Validate parameters
        auto validationResult = validateParameters();
        if (validationResult.code() != rebel::core::ErrorCode::None) {
            return validationResult;
        }

        // Create offset surfaces for each face
        auto offsetSurfaces = createOffsetSurfaces();

        // Connect offset surfaces to create the shell
        auto result = connectOffsetSurfaces(offsetSurfaces);

        if (!result) {
            return rebel::core::Error(
                rebel::core::ErrorCode::CADError,
                "Failed to create shell geometry"
            );
        }

        impl->body = result;
        return rebel::core::Error(rebel::core::ErrorCode::None, "Shell operation successful");
    }
    catch (const std::exception& e) {
        return rebel::core::Error(
            rebel::core::ErrorCode::CADError,
            std::string("Shell operation failed: ") + e.what()
        );
    }
}

rebel::core::Error ShellTool::validateParameters() const {
    const auto& topology = impl->body->GetTopology();
    
    // Check if any removed face indices are invalid
    for (int faceIndex : impl->removedFaces) {
        if (faceIndex < 0 || faceIndex >= static_cast<int>(topology.faces.size())) {
            return rebel::core::Error(
                rebel::core::ErrorCode::CADError,
                "Invalid face index in removedFaces"
            );
        }
    }

    // Find smallest body dimension to validate thickness
    double minDimension = std::numeric_limits<double>::max();
    const auto& vertices = topology.vertices;
    
    for (size_t i = 0; i < vertices.size(); ++i) {
        for (size_t j = i + 1; j < vertices.size(); ++j) {
            double dx = vertices[j].x - vertices[i].x;
            double dy = vertices[j].y - vertices[i].y;
            double dz = vertices[j].z - vertices[i].z;
            double distance = std::sqrt(dx*dx + dy*dy + dz*dz);
            minDimension = std::min(minDimension, distance);
        }
    }

    if (impl->thickness >= minDimension / 2) {
        return rebel::core::Error(
            rebel::core::ErrorCode::CADError,
            "Shell thickness too large for body dimensions"
        );
    }

    return rebel::core::Error(rebel::core::ErrorCode::None, "Parameters valid");
}

std::vector<std::shared_ptr<SolidBody>> ShellTool::createOffsetSurfaces() const {
    std::vector<std::shared_ptr<SolidBody>> offsetSurfaces;
    const auto& topology = impl->body->GetTopology();

    // Create offset surface for each face
    for (size_t i = 0; i < topology.faces.size(); ++i) {
        const auto& face = topology.faces[i];
        
        // Create new topology for offset face
        Topology offsetTopology;
        
        // Copy vertices with offset along face normal
        for (const auto& vertex : topology.vertices) {
            // TODO: Calculate face normal and offset vertex
            // This is a placeholder - actual implementation would need to:
            // 1. Calculate face normal at vertex
            // 2. Offset vertex along normal by thickness
            // 3. Add new vertex to offset topology
            offsetTopology.vertices.push_back(vertex); // Placeholder
        }
        
        // Copy face edges to maintain topology
        offsetTopology.edges = topology.edges;
        offsetTopology.faces.push_back(face);
        
        // Create solid body from offset topology
        auto offsetBody = SolidBody::FromTopology(offsetTopology);
        if (offsetBody) {
            offsetSurfaces.push_back(offsetBody);
        }
    }

    return offsetSurfaces;
}

std::shared_ptr<SolidBody> ShellTool::connectOffsetSurfaces(
    const std::vector<std::shared_ptr<SolidBody>>& offsetSurfaces
) const {
    if (offsetSurfaces.empty()) {
        return nullptr;
    }

    std::vector<std::shared_ptr<SolidBody>> shellBodies;
    
    // Filter out removed faces
    for (size_t i = 0; i < offsetSurfaces.size(); ++i) {
        if (std::find(impl->removedFaces.begin(), impl->removedFaces.end(), static_cast<int>(i)) 
            == impl->removedFaces.end()) {
            shellBodies.push_back(offsetSurfaces[i]);
        }
    }

    // Create compound body from all offset surfaces
    return SolidBody::CreateCompound(shellBodies);
}

} // namespace Modeling
} // namespace RebelCAD
