#include "sketching/BezierCurve.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <numeric>

namespace RebelCAD {
namespace Sketching {

BezierCurve::BezierCurve() : color_(Graphics::Color(0.0f, 0.0f, 0.0f)) {}

BezierCurve::BezierCurve(const std::vector<std::pair<double, double>>& controlPoints)
    : controlPoints_(controlPoints), color_(Graphics::Color(0.0f, 0.0f, 0.0f)) {
    if (controlPoints.empty()) {
        throw std::invalid_argument("Control points vector cannot be empty");
    }
}

void BezierCurve::addControlPoint(double x, double y) {
    controlPoints_.push_back(std::make_pair(x, y));
}

bool BezierCurve::removeControlPoint(size_t index) {
    if (index >= controlPoints_.size()) {
        return false;
    }
    controlPoints_.erase(controlPoints_.begin() + index);
    return true;
}

bool BezierCurve::moveControlPoint(size_t index, double newX, double newY) {
    if (index >= controlPoints_.size()) {
        return false;
    }
    controlPoints_[index] = std::make_pair(newX, newY);
    return true;
}

const std::vector<std::pair<double, double>>& BezierCurve::getControlPoints() const {
    return controlPoints_;
}

std::vector<std::pair<double, double>> BezierCurve::calculateCurvePoints(size_t numPoints) const {
    if (controlPoints_.empty()) {
        return std::vector<std::pair<double, double>>();
    }

    std::vector<std::pair<double, double>> curvePoints;
    curvePoints.reserve(numPoints);

    for (size_t i = 0; i < numPoints; ++i) {
        double t = static_cast<double>(i) / (numPoints - 1);
        curvePoints.push_back(evaluatePoint(t));
    }

    return curvePoints;
}

std::pair<double, double> BezierCurve::evaluatePoint(double t) const {
    validateParameterT(t);
    
    if (controlPoints_.empty()) {
        throw std::runtime_error("Cannot evaluate curve with no control points");
    }

    const unsigned int n = getDegree();
    double x = 0.0, y = 0.0;

    for (unsigned int i = 0; i <= n; ++i) {
        double basis = bernsteinPolynomial(n, i, t);
        x += controlPoints_[i].first * basis;
        y += controlPoints_[i].second * basis;
    }

    return std::make_pair(x, y);
}

std::pair<double, double> BezierCurve::evaluateFirstDerivative(double t) const {
    validateParameterT(t);
    
    if (controlPoints_.size() < 2) {
        throw std::runtime_error("Cannot evaluate derivative with less than 2 control points");
    }

    const unsigned int n = getDegree();
    double dx = 0.0, dy = 0.0;

    for (unsigned int i = 0; i < n; ++i) {
        double coeff = n * bernsteinPolynomial(n - 1, i, t);
        dx += (controlPoints_[i + 1].first - controlPoints_[i].first) * coeff;
        dy += (controlPoints_[i + 1].second - controlPoints_[i].second) * coeff;
    }

    return std::make_pair(dx, dy);
}

std::pair<double, double> BezierCurve::evaluateSecondDerivative(double t) const {
    validateParameterT(t);
    
    if (controlPoints_.size() < 3) {
        throw std::runtime_error("Cannot evaluate second derivative with less than 3 control points");
    }

    const unsigned int n = getDegree();
    double ddx = 0.0, ddy = 0.0;

    for (unsigned int i = 0; i < n - 1; ++i) {
        double coeff = n * (n - 1) * bernsteinPolynomial(n - 2, i, t);
        double dx1 = controlPoints_[i + 1].first - controlPoints_[i].first;
        double dx2 = controlPoints_[i + 2].first - controlPoints_[i + 1].first;
        double dy1 = controlPoints_[i + 1].second - controlPoints_[i].second;
        double dy2 = controlPoints_[i + 2].second - controlPoints_[i + 1].second;
        ddx += (dx2 - dx1) * coeff;
        ddy += (dy2 - dy1) * coeff;
    }

    return std::make_pair(ddx, ddy);
}

unsigned int BezierCurve::getDegree() const {
    return controlPoints_.empty() ? 0 : static_cast<unsigned int>(controlPoints_.size() - 1);
}

void BezierCurve::setColor(const Graphics::Color& color) {
    color_ = color;
}

const Graphics::Color& BezierCurve::getColor() const {
    return color_;
}

void BezierCurve::render(const std::shared_ptr<Graphics::GraphicsSystem>& graphics,
                        const Graphics::Color& color,
                        float thickness,
                        const std::vector<float>* dashPattern) {
    if (controlPoints_.empty()) {
        return;
    }

    // Calculate points along the curve for rendering
    auto points = calculateCurvePoints(100); // Use 100 points for smooth rendering
    
    // Convert points to format needed by graphics system
    std::vector<float> vertices;
    vertices.reserve(points.size() * 2);
    for (const auto& point : points) {
        vertices.push_back(static_cast<float>(point.first));
        vertices.push_back(static_cast<float>(point.second));
    }

    // Set up rendering state
    graphics->setColor(color);
    graphics->setLineThickness(thickness);
    if (dashPattern) {
        graphics->setLineDashPattern(*dashPattern);
    }

    // Render the curve as a line strip
    graphics->beginPreview();
    graphics->renderTriangleMesh(vertices.data(), vertices.size());
    graphics->endPreview();

    // Optionally render control points and polygon
    graphics->setColor(Graphics::Color(0.5f, 0.5f, 0.5f)); // Gray for control elements
    graphics->setLineThickness(1.0f);
    graphics->setLineDashPattern(std::vector<float>{2.0f, 2.0f}); // Dashed lines

    // Render control polygon
    for (size_t i = 0; i < controlPoints_.size() - 1; ++i) {
        graphics->drawLine(
            static_cast<float>(controlPoints_[i].first),
            static_cast<float>(controlPoints_[i].second),
            static_cast<float>(controlPoints_[i + 1].first),
            static_cast<float>(controlPoints_[i + 1].second)
        );
    }

    // Reset line style
    graphics->setLineDashPattern(std::vector<float>());
}

std::pair<BezierCurve, BezierCurve> BezierCurve::split(double t) const {
    validateParameterT(t);
    
    if (controlPoints_.empty()) {
        throw std::runtime_error("Cannot split curve with no control points");
    }

    // de Casteljau's algorithm for splitting
    std::vector<std::pair<double, double>> left, right;
    std::vector<std::pair<double, double>> points = controlPoints_;
    
    while (!points.empty()) {
        left.push_back(points.front());
        right.insert(right.begin(), points.back());
        
        std::vector<std::pair<double, double>> next;
        for (size_t i = 0; i < points.size() - 1; ++i) {
            double x = (1 - t) * points[i].first + t * points[i + 1].first;
            double y = (1 - t) * points[i].second + t * points[i + 1].second;
            next.push_back(std::make_pair(x, y));
        }
        points = next;
    }

    return std::make_pair(BezierCurve(left), BezierCurve(right));
}

BezierCurve BezierCurve::elevateDegree() const {
    if (controlPoints_.empty()) {
        throw std::runtime_error("Cannot elevate degree of empty curve");
    }

    const unsigned int n = getDegree();
    std::vector<std::pair<double, double>> newPoints;
    newPoints.reserve(n + 2);

    // First point remains the same
    newPoints.push_back(controlPoints_[0]);

    // Calculate internal points
    for (unsigned int i = 1; i <= n; ++i) {
        double x = (i * controlPoints_[i - 1].first + (n + 1 - i) * controlPoints_[i].first) / (n + 1);
        double y = (i * controlPoints_[i - 1].second + (n + 1 - i) * controlPoints_[i].second) / (n + 1);
        newPoints.push_back(std::make_pair(x, y));
    }

    // Last point remains the same
    newPoints.push_back(controlPoints_[n]);

    return BezierCurve(newPoints);
}

BezierCurve BezierCurve::reduceDegree(double tolerance) const {
    if (controlPoints_.size() <= 2) {
        return *this; // Cannot reduce degree of linear or point curves
    }

    // Attempt degree reduction using least squares approximation
    const unsigned int n = getDegree();
    std::vector<std::pair<double, double>> reducedPoints;
    reducedPoints.reserve(n);

    // Calculate reduced control points
    for (unsigned int i = 0; i < n; ++i) {
        double x = (n * controlPoints_[i].first - i * controlPoints_[i - 1].first) / (n - i);
        double y = (n * controlPoints_[i].second - i * controlPoints_[i - 1].second) / (n - i);
        reducedPoints.push_back(std::make_pair(x, y));
    }

    // Verify error is within tolerance
    BezierCurve reduced(reducedPoints);
    const size_t numTestPoints = 100;
    for (size_t i = 0; i < numTestPoints; ++i) {
        double t = static_cast<double>(i) / (numTestPoints - 1);
        auto p1 = evaluatePoint(t);
        auto p2 = reduced.evaluatePoint(t);
        double error = std::hypot(p1.first - p2.first, p1.second - p2.second);
        if (error > tolerance) {
            return *this; // Reduction not possible within tolerance
        }
    }

    return reduced;
}

std::vector<std::pair<double, double>> BezierCurve::calculateConvexHull() const {
    if (controlPoints_.size() < 3) {
        return controlPoints_;
    }

    // Graham scan algorithm for convex hull
    std::vector<std::pair<double, double>> hull = controlPoints_;
    
    // Find point with lowest y-coordinate (and leftmost if tied)
    auto pivot = std::min_element(hull.begin(), hull.end(),
        [](const auto& a, const auto& b) {
            return a.second < b.second || (a.second == b.second && a.first < b.first);
        });
    
    // Sort points by polar angle with respect to pivot
    std::swap(*hull.begin(), *pivot);
    std::sort(hull.begin() + 1, hull.end(),
        [&](const auto& a, const auto& b) {
            double cross = (a.first - hull[0].first) * (b.second - hull[0].second) -
                         (b.first - hull[0].first) * (a.second - hull[0].second);
            return cross > 0 || (cross == 0 && 
                std::hypot(a.first - hull[0].first, a.second - hull[0].second) <
                std::hypot(b.first - hull[0].first, b.second - hull[0].second));
        });

    // Build convex hull
    std::vector<std::pair<double, double>> result;
    for (const auto& point : hull) {
        while (result.size() >= 2) {
            auto& p1 = result[result.size() - 2];
            auto& p2 = result[result.size() - 1];
            double cross = (p2.first - p1.first) * (point.second - p1.second) -
                         (point.first - p1.first) * (p2.second - p1.second);
            if (cross <= 0) {
                result.pop_back();
            } else {
                break;
            }
        }
        result.push_back(point);
    }

    return result;
}

std::pair<std::pair<double, double>, double> BezierCurve::findClosestPoint(
    double x, double y, double tolerance) const {
    if (controlPoints_.empty()) {
        throw std::runtime_error("Cannot find closest point on empty curve");
    }

    // Newton-Raphson iteration to find parameter value
    double t = 0.5; // Initial guess
    const int maxIterations = 100;
    
    for (int i = 0; i < maxIterations; ++i) {
        auto point = evaluatePoint(t);
        auto derivative = evaluateFirstDerivative(t);
        auto secondDerivative = evaluateSecondDerivative(t);
        
        // Calculate distance vector
        double dx = point.first - x;
        double dy = point.second - y;
        
        // Calculate first derivative of squared distance
        double firstDeriv = 2.0 * (dx * derivative.first + dy * derivative.second);
        
        // Calculate second derivative of squared distance
        double secondDeriv = 2.0 * (derivative.first * derivative.first + 
                                   derivative.second * derivative.second +
                                   dx * secondDerivative.first + 
                                   dy * secondDerivative.second);
        
        // Update parameter value
        double delta = firstDeriv / secondDeriv;
        t -= delta;
        
        // Clamp to valid parameter range
        t = std::max(0.0, std::min(1.0, t));
        
        if (std::abs(delta) < tolerance) {
            break;
        }
    }
    
    auto closestPoint = evaluatePoint(t);
    return std::make_pair(closestPoint, t);
}

unsigned int BezierCurve::binomialCoefficient(unsigned int n, unsigned int k) const {
    if (k > n) {
        return 0;
    }
    if (k == 0 || k == n) {
        return 1;
    }
    
    // Use symmetry to reduce calculations
    k = std::min(k, n - k);
    
    unsigned int result = 1;
    for (unsigned int i = 0; i < k; ++i) {
        result *= (n - i);
        result /= (i + 1);
    }
    
    return result;
}

double BezierCurve::bernsteinPolynomial(unsigned int n, unsigned int i, double t) const {
    return binomialCoefficient(n, i) * std::pow(t, i) * std::pow(1.0 - t, n - i);
}

void BezierCurve::validateParameterT(double t) const {
    if (t < 0.0 || t > 1.0) {
        throw std::invalid_argument("Parameter t must be between 0 and 1");
    }
}

} // namespace Sketching
} // namespace RebelCAD
