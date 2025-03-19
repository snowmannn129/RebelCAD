#pragma once

#include <vector>
#include <memory>
#include "graphics/Color.h"
#include "graphics/GraphicsSystem.h"

namespace RebelCAD {
namespace Sketching {

/**
 * @brief Represents a Bezier curve in 2D space
 * 
 * This class implements a Bezier curve defined by control points.
 * Bezier curves provide intuitive control for curve design with
 * global influence of control points, making them suitable for
 * certain CAD operations where direct manipulation is desired.
 */
class BezierCurve {
public:
    /**
     * @brief Construct a new Bezier curve object
     */
    BezierCurve();

    /**
     * @brief Construct a Bezier curve with initial control points
     * @param controlPoints Vector of control points defining the curve
     */
    explicit BezierCurve(const std::vector<std::pair<double, double>>& controlPoints);

    /**
     * @brief Add a control point to the curve
     * @param x X coordinate of the control point
     * @param y Y coordinate of the control point
     */
    void addControlPoint(double x, double y);

    /**
     * @brief Remove a control point at the specified index
     * @param index Index of the control point to remove
     * @return true if point was removed, false if index was invalid
     */
    bool removeControlPoint(size_t index);

    /**
     * @brief Move an existing control point to a new position
     * @param index Index of the control point to move
     * @param newX New X coordinate
     * @param newY New Y coordinate
     * @return true if point was moved, false if index was invalid
     */
    bool moveControlPoint(size_t index, double newX, double newY);

    /**
     * @brief Get the current control points
     * @return Vector of control point coordinates
     */
    const std::vector<std::pair<double, double>>& getControlPoints() const;

    /**
     * @brief Calculate points along the Bezier curve for rendering
     * @param numPoints Number of points to generate (default: 100)
     * @return Vector of points defining the curve
     */
    std::vector<std::pair<double, double>> calculateCurvePoints(size_t numPoints = 100) const;

    /**
     * @brief Calculate a point on the curve at parameter value t
     * @param t Parameter value between 0 and 1
     * @return Point coordinates
     */
    std::pair<double, double> evaluatePoint(double t) const;

    /**
     * @brief Calculate the first derivative at parameter value t
     * @param t Parameter value between 0 and 1
     * @return Vector representing the first derivative (dx/dt, dy/dt)
     */
    std::pair<double, double> evaluateFirstDerivative(double t) const;

    /**
     * @brief Calculate the second derivative at parameter value t
     * @param t Parameter value between 0 and 1
     * @return Vector representing the second derivative (d²x/dt², d²y/dt²)
     */
    std::pair<double, double> evaluateSecondDerivative(double t) const;

    /**
     * @brief Get the degree of the Bezier curve
     * @return Degree of the curve (number of control points - 1)
     */
    unsigned int getDegree() const;

    /**
     * @brief Set the color for rendering the curve
     * @param color Color to use for rendering
     */
    void setColor(const Graphics::Color& color);

    /**
     * @brief Get the current rendering color
     * @return Current color
     */
    const Graphics::Color& getColor() const;

    /**
     * @brief Renders the Bezier curve using the graphics system
     * @param graphics Reference to the graphics system
     * @param color Curve color
     * @param thickness Line thickness in pixels
     * @param dashPattern Optional array of dash lengths for dashed lines
     */
    void render(const std::shared_ptr<Graphics::GraphicsSystem>& graphics,
                const Graphics::Color& color,
                float thickness = 1.0f,
                const std::vector<float>* dashPattern = nullptr);

    /**
     * @brief Split the Bezier curve at parameter t
     * @param t Parameter value between 0 and 1
     * @return Pair of Bezier curves representing left and right portions
     */
    std::pair<BezierCurve, BezierCurve> split(double t) const;

    /**
     * @brief Elevate the degree of the Bezier curve
     * @return New Bezier curve of degree n+1 that is geometrically equivalent
     */
    BezierCurve elevateDegree() const;

    /**
     * @brief Reduce the degree of the Bezier curve if possible
     * @param tolerance Error tolerance for degree reduction
     * @return New Bezier curve of degree n-1 that approximates the original
     */
    BezierCurve reduceDegree(double tolerance = 1e-6) const;

    /**
     * @brief Calculate the convex hull of the control points
     * @return Vector of points defining the convex hull
     */
    std::vector<std::pair<double, double>> calculateConvexHull() const;

    /**
     * @brief Find the closest point on the curve to a given point
     * @param x X coordinate of the target point
     * @param y Y coordinate of the target point
     * @param tolerance Convergence tolerance (default: 1e-6)
     * @return Pair containing closest point coordinates and parameter t
     */
    std::pair<std::pair<double, double>, double> findClosestPoint(
        double x, double y, double tolerance = 1e-6) const;

private:
    /**
     * @brief Calculate binomial coefficient (n choose k)
     * @param n Total number of items
     * @param k Number of items to choose
     * @return Binomial coefficient value
     */
    unsigned int binomialCoefficient(unsigned int n, unsigned int k) const;

    /**
     * @brief Calculate the Bernstein polynomial value
     * @param n Degree of the polynomial
     * @param i Index
     * @param t Parameter value
     * @return Bernstein polynomial value
     */
    double bernsteinPolynomial(unsigned int n, unsigned int i, double t) const;

    std::vector<std::pair<double, double>> controlPoints_; ///< Control points defining the curve
    Graphics::Color color_;                               ///< Color for rendering

    /**
     * @brief Validate parameter value t is between 0 and 1
     * @param t Parameter value to validate
     * @throw std::invalid_argument if t is not between 0 and 1
     */
    void validateParameterT(double t) const;
};

} // namespace Sketching
} // namespace RebelCAD
