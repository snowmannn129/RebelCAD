#pragma once

#include "modeling/Geometry.h"
#include "core/Error.h"
#include <memory>
#include <vector>

namespace RebelCAD {
namespace Modeling {

/**
 * @brief The ShellTool creates hollow objects by removing material from solid models
 * 
 * The shell operation creates a hollow object by removing material from a solid body
 * while maintaining a specified wall thickness. It can optionally remove selected faces
 * to create openings in the resulting shell.
 */
class ShellTool {
public:
    /**
     * @brief Constructs a new Shell Tool
     */
    ShellTool();
    ~ShellTool();

    /**
     * @brief Configures the shell operation with input parameters
     * 
     * @param body The input solid body to shell
     * @param thickness The desired wall thickness of the shell
     * @param removedFaces Optional vector of face indices to remove (create openings)
     * @return rebel::core::Error Success or failure with description
     */
    rebel::core::Error Configure(
        const std::shared_ptr<SolidBody>& body,
        double thickness,
        const std::vector<int>& removedFaces = std::vector<int>()
    );

    /**
     * @brief Executes the shell operation with configured parameters
     * 
     * @return rebel::core::Error Success or failure with description
     */
    rebel::core::Error Execute();

private:
    /**
     * @brief Implementation of the ShellTool
     */
    class Implementation;
    std::unique_ptr<Implementation> impl;

    /**
     * @brief Validates the shell parameters before execution
     * 
     * @return rebel::core::Error Success or failure with description
     */
    rebel::core::Error validateParameters() const;

    /**
     * @brief Creates an offset surface for each face of the solid
     * 
     * @return std::vector<std::shared_ptr<SolidBody>> Offset surfaces
     */
    std::vector<std::shared_ptr<SolidBody>> createOffsetSurfaces() const;

    /**
     * @brief Trims and connects offset surfaces to create the shell
     * 
     * @param offsetSurfaces The offset surfaces to connect
     * @return std::shared_ptr<SolidBody> The final shelled body
     */
    std::shared_ptr<SolidBody> connectOffsetSurfaces(
        const std::vector<std::shared_ptr<SolidBody>>& offsetSurfaces
    ) const;
};

} // namespace Modeling
} // namespace RebelCAD
