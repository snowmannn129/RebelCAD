#pragma once

#include "modeling/SubdivisionSurface.h"
#include "modeling/BooleanTool.h"
#include "modeling/SolidBody.hpp"
#include "core/Error.h"
#include <memory>
#include <vector>

namespace RebelCAD {
namespace Modeling {

/**
 * @brief Class for handling boolean operations between subdivision surfaces
 * 
 * Provides functionality to perform boolean operations (union, subtract, intersect)
 * between subdivision surfaces while maintaining sharp features and proper topology.
 */
class SubdivisionBooleanOperations {
public:
    /**
     * @brief Perform a boolean operation between two subdivision surfaces
     * @param first First subdivision surface
     * @param second Second subdivision surface
     * @param operation Type of boolean operation to perform
     * @param params Optional boolean operation parameters
     * @return Resulting subdivision surface
     */
    static std::shared_ptr<SubdivisionSurface> PerformOperation(
        std::shared_ptr<SubdivisionSurface> first,
        std::shared_ptr<SubdivisionSurface> second,
        BooleanOperationType operation,
        const BooleanParams& params = BooleanParams());

    /**
     * @brief Preview a boolean operation between two subdivision surfaces
     * @param first First subdivision surface
     * @param second Second subdivision surface
     * @param operation Type of boolean operation to perform
     * @param graphics Graphics system for rendering
     * @param params Optional boolean operation parameters
     * @return Error code indicating success or failure
     */
    static Core::ErrorCode PreviewOperation(
        std::shared_ptr<SubdivisionSurface> first,
        std::shared_ptr<SubdivisionSurface> second,
        BooleanOperationType operation,
        std::shared_ptr<Graphics::GraphicsSystem> graphics,
        const BooleanParams& params = BooleanParams());

private:
    /**
     * @brief Convert subdivision surface to solid body for boolean operations
     * @param surface Subdivision surface to convert
     * @param subdivisionLevel Level of subdivision to apply before conversion
     * @return Solid body representation
     */
    static std::shared_ptr<SolidBody> ConvertToSolidBody(
        std::shared_ptr<SubdivisionSurface> surface,
        size_t subdivisionLevel = 2);

    /**
     * @brief Convert solid body back to subdivision surface
     * @param body Solid body to convert
     * @param detectFeatures Whether to detect and preserve sharp features
     * @return Subdivision surface representation
     */
    static std::shared_ptr<SubdivisionSurface> ConvertToSubdivisionSurface(
        std::shared_ptr<SolidBody> body,
        bool detectFeatures = true);

    /**
     * @brief Detect sharp features in a mesh
     * @param vertices Mesh vertices
     * @param triangles Mesh triangles
     * @param angleTolerance Angle threshold for sharp edge detection
     * @return Vector of indices of detected sharp edges
     */
    static std::vector<size_t> DetectSharpFeatures(
        const std::vector<SolidBody::Vertex>& vertices,
        const std::vector<SolidBody::Triangle>& triangles,
        float angleTolerance = 30.0f);

    /**
     * @brief Optimize topology after boolean operation
     * @param surface Surface to optimize
     * @return Optimized surface
     */
    static std::shared_ptr<SubdivisionSurface> OptimizeTopology(
        std::shared_ptr<SubdivisionSurface> surface);

    /**
     * @brief Converts SolidBody data to raw vertex and index arrays
     * @param body Input solid body
     * @param vertices Output vertex array (x,y,z coordinates)
     * @param indices Output triangle index array
     */
    static void ConvertSolidBodyToRawData(
        std::shared_ptr<SolidBody> body,
        std::vector<float>& vertices,
        std::vector<unsigned int>& indices);

    /**
     * @brief Converts raw vertex and index data to SolidBody format
     * @param vertices Input vertex array
     * @param indices Input index array
     * @param outVertices Output SolidBody vertices
     * @param outTriangles Output SolidBody triangles
     */
    static void ConvertRawDataToSolidBody(
        const std::vector<float>& vertices,
        const std::vector<unsigned int>& indices,
        std::vector<SolidBody::Vertex>& outVertices,
        std::vector<SolidBody::Triangle>& outTriangles);
};

} // namespace Modeling
} // namespace RebelCAD
