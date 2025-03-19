#pragma once

#include "modeling/NURBSCurve.h"
#include "graphics/GraphicsSystem.h"
#include <memory>
#include <glm/glm.hpp>

namespace RebelCAD {
namespace Testing {

/**
 * @brief Helper class for visualizing NURBS curves during testing
 */
class NURBSCurveVisualizer {
public:
    struct Color {
        float r, g, b, a;
        Color(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f)
            : r(r), g(g), b(b), a(a) {}
    };

    NURBSCurveVisualizer(std::shared_ptr<Graphics::GraphicsSystem> graphics)
        : graphics(graphics) {}

    /**
     * @brief Visualize curve with control points
     * @param curve Curve to visualize
     * @param curve_color Color for curve
     * @param control_color Color for control points
     */
    void VisualizeCurve(
        const Modeling::NURBSCurve& curve,
        const Color& curve_color = Color(0.0f, 0.7f, 1.0f),
        const Color& control_color = Color(1.0f, 0.3f, 0.0f)) {
        
        // Set curve color
        graphics->setColor(Graphics::Color(
            curve_color.r, curve_color.g, curve_color.b, curve_color.a));

        // Draw curve
        curve.Preview(graphics);
    }

    /**
     * @brief Visualize split operation
     * @param original Original curve
     * @param left Left portion after split
     * @param right Right portion after split
     * @param t Split parameter
     */
    void VisualizeSplit(
        const Modeling::NURBSCurve& original,
        const Modeling::NURBSCurve& left,
        const Modeling::NURBSCurve& right,
        float t) {
        
        // Draw original curve
        VisualizeCurve(original, Color(0.7f, 0.7f, 0.7f));  // Gray

        // Draw split point
        glm::vec3 split_point = original.Evaluate(t);
        graphics->setColor(Graphics::Color(1.0f, 1.0f, 0.0f, 1.0f));  // Yellow
        graphics->drawLine(
            split_point.x - 0.1f, split_point.y,
            split_point.x + 0.1f, split_point.y);
        graphics->drawLine(
            split_point.x, split_point.y - 0.1f,
            split_point.x, split_point.y + 0.1f);

        // Draw left and right portions
        VisualizeCurve(left, Color(0.0f, 0.7f, 1.0f));   // Blue
        VisualizeCurve(right, Color(1.0f, 0.3f, 0.0f));  // Red
    }

    /**
     * @brief Visualize join operation
     * @param curve1 First curve
     * @param curve2 Second curve
     * @param result Joined curve
     */
    void VisualizeJoin(
        const Modeling::NURBSCurve& curve1,
        const Modeling::NURBSCurve& curve2,
        const Modeling::NURBSCurve& result) {
        
        // Draw original curves
        VisualizeCurve(curve1, Color(0.0f, 0.7f, 1.0f));   // Blue
        VisualizeCurve(curve2, Color(1.0f, 0.3f, 0.0f));   // Red

        // Draw join point
        glm::vec3 join_point = curve1.Evaluate(1.0f);
        graphics->setColor(Graphics::Color(1.0f, 1.0f, 0.0f, 1.0f));  // Yellow
        graphics->drawLine(
            join_point.x - 0.1f, join_point.y,
            join_point.x + 0.1f, join_point.y);
        graphics->drawLine(
            join_point.x, join_point.y - 0.1f,
            join_point.x, join_point.y + 0.1f);

        // Draw result
        VisualizeCurve(result, Color(0.0f, 1.0f, 0.0f));  // Green
    }

    /**
     * @brief Save visualization to file
     * @param filename Output filename
     */
    void SaveToFile(const std::string& filename) {
        // TODO: Implement saving to image file
        // This would require adding image export capability to GraphicsSystem
    }

private:
    std::shared_ptr<Graphics::GraphicsSystem> graphics;
};

} // namespace Testing
} // namespace RebelCAD
