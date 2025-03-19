#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <omp.h>
#include <cstring>
#include "../core/SmartResourceManager.hpp"
#include "../core/EventSystem.hpp"
#include "../ui/Events.hpp"
#include "Mesh.hpp"
#include "PreviewCache.hpp"

namespace RebelCAD {
namespace Modeling {


// Pattern type enumeration
enum class PatternType {
    Linear,     // Linear pattern along direction
    Circular,   // Circular pattern around axis
    Mirror      // Mirror pattern across plane
};

// Pattern parameters base class
struct PatternParameters {
    virtual ~PatternParameters() = default;
};

// Linear pattern parameters
struct LinearPatternParameters : public PatternParameters {
    glm::vec3 direction;      // Pattern direction vector
    float spacing;            // Distance between instances
    int count;               // Number of instances
    bool includeOriginal;    // Whether to include original in count
};

// Circular pattern parameters
struct CircularPatternParameters : public PatternParameters {
    glm::vec3 axis;          // Rotation axis
    glm::vec3 center;        // Center point of rotation
    float angle;             // Total angle to distribute instances
    int count;               // Number of instances
    bool includeOriginal;    // Whether to include original in count
};

// Mirror pattern parameters
struct MirrorPatternParameters : public PatternParameters {
    glm::vec3 normal;        // Normal vector of mirror plane
    glm::vec3 point;         // Point on mirror plane
    bool includeOriginal;    // Whether to include original
};

class PatternTool {
public:
    PatternTool();
    ~PatternTool();

    // Initialize pattern tool with specific type
    void initialize(PatternType type);

    // Set parameters based on pattern type
    void setParameters(std::shared_ptr<PatternParameters> params);

    // Set target geometry for pattern
    void setTargetGeometry(std::shared_ptr<Mesh> geometry);

    // Generate pattern preview
    bool generatePreview();

    // Apply pattern operation
    bool apply();

    // Cancel operation
    void cancel();

    // Get current preview
    std::shared_ptr<Mesh> getPreview() const;

    // Validation methods
    bool validateParameters() const;
    bool validateGeometry() const;

private:
    // Pattern generation methods
    bool generateLinearPattern(std::shared_ptr<Mesh> preview);
    bool generateCircularPattern(std::shared_ptr<Mesh> preview);
    bool generateMirrorPattern(std::shared_ptr<Mesh> preview);

    // Helper methods
    void clearPreview();
    void updatePreview();
    bool validateLinearParameters() const;
    bool validateCircularParameters() const;
    bool validateMirrorParameters() const;
    bool hasParametersChanged() const;
    size_t calculateInstanceCount() const;

    // Member variables
    PatternType m_type;
    std::shared_ptr<PatternParameters> m_parameters;
    std::shared_ptr<PatternParameters> m_lastParameters;  // Store last parameters for change detection
    std::shared_ptr<Mesh> m_targetGeometry;
    std::shared_ptr<Mesh> m_previewMesh;
    std::unique_ptr<PreviewCache> m_previewCache;
    bool m_isValid;

    // Event system integration
    Core::EventHandle m_previewUpdateHandle{0};
};

} // namespace Modeling
} // namespace RebelCAD
