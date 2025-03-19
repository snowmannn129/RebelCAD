#include "modeling/PatternTool.hpp"
#include "core/GarbageCollector.hpp"
#include "core/MemoryPool.hpp"
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <stdexcept>
#include <type_traits>
#include <algorithm>

namespace RebelCAD {
namespace Modeling {

PatternTool::PatternTool()
    : m_type(PatternType::Linear)
    , m_isValid(false)
    , m_previewCache(std::make_unique<PreviewCache>())
{
    // Register for preview update events
    auto& eventSystem = Core::EventSystem::getInstance();
    m_previewUpdateHandle = eventSystem.registerHandler(
        "PreviewUpdate",
        [this](const Core::Event& event) {
            this->updatePreview();
        }
    );
}

PatternTool::~PatternTool() {
    clearPreview();
    Core::EventSystem::getInstance().unregisterHandler(m_previewUpdateHandle);
}

void PatternTool::initialize(PatternType type) {
    m_type = type;
    m_isValid = false;
    clearPreview();
}

void PatternTool::setParameters(std::shared_ptr<PatternParameters> params) {
    m_parameters = params;
    m_isValid = validateParameters();
    if (m_isValid && m_targetGeometry) {
        generatePreview();
    }
}

void PatternTool::setTargetGeometry(std::shared_ptr<Mesh> geometry) {
    m_targetGeometry = geometry;
    m_isValid = validateGeometry() && validateParameters();
    if (m_isValid) {
        generatePreview();
    }
}

bool PatternTool::generatePreview() {
    if (!m_isValid || !m_targetGeometry || !m_parameters) {
        return false;
    }

    // Check if we can reuse existing preview
    if (m_previewMesh && !hasParametersChanged()) {
        return true;
    }

    // Pre-calculate total mesh size for all instances
    size_t totalVertices = 0;
    size_t totalFaces = 0;
    size_t instanceCount = calculateInstanceCount();
    
    if (instanceCount > 0) {
        totalVertices = m_targetGeometry->getVertexCount() * instanceCount;
        totalFaces = m_targetGeometry->getFaceCount() * instanceCount;
    }

    // Create new preview mesh with pre-allocated capacity
    auto newPreview = std::make_shared<Mesh>();
    newPreview->reserveVertices(totalVertices);
    newPreview->reserveFaces(totalFaces);

    bool success = false;
    switch (m_type) {
        case PatternType::Linear:
            success = generateLinearPattern(newPreview);
            break;
        case PatternType::Circular:
            success = generateCircularPattern(newPreview);
            break;
        case PatternType::Mirror:
            success = generateMirrorPattern(newPreview);
            break;
    }

    if (success) {
        clearPreview();
        m_previewMesh = newPreview;
        m_lastParameters = m_parameters;
        updatePreview();
    }

    return success;
}

size_t PatternTool::calculateInstanceCount() const {
    switch (m_type) {
        case PatternType::Linear: {
            auto params = std::dynamic_pointer_cast<LinearPatternParameters>(m_parameters);
            return params ? params->count : 0;
        }
        case PatternType::Circular: {
            auto params = std::dynamic_pointer_cast<CircularPatternParameters>(m_parameters);
            return params ? params->count : 0;
        }
        case PatternType::Mirror:
            return 2; // Original + mirrored
        default:
            return 0;
    }
}

bool PatternTool::hasParametersChanged() const {
    if (!m_parameters) return true;
    
    switch (m_type) {
        case PatternType::Linear: {
            auto current = std::dynamic_pointer_cast<LinearPatternParameters>(m_parameters);
            auto last = std::dynamic_pointer_cast<LinearPatternParameters>(m_lastParameters);
            return !current || !last || 
                   current->count != last->count ||
                   current->spacing != last->spacing ||
                   current->direction != last->direction ||
                   current->includeOriginal != last->includeOriginal;
        }
        case PatternType::Circular: {
            auto current = std::dynamic_pointer_cast<CircularPatternParameters>(m_parameters);
            auto last = std::dynamic_pointer_cast<CircularPatternParameters>(m_lastParameters);
            return !current || !last ||
                   current->count != last->count ||
                   current->angle != last->angle ||
                   current->axis != last->axis ||
                   current->center != last->center ||
                   current->includeOriginal != last->includeOriginal;
        }
        case PatternType::Mirror: {
            auto current = std::dynamic_pointer_cast<MirrorPatternParameters>(m_parameters);
            auto last = std::dynamic_pointer_cast<MirrorPatternParameters>(m_lastParameters);
            return !current || !last ||
                   current->normal != last->normal ||
                   current->point != last->point ||
                   current->includeOriginal != last->includeOriginal;
        }
        default:
            return true;
    }
}

bool PatternTool::apply() {
    if (!m_isValid || !m_previewMesh) {
        return false;
    }

    // Transfer ownership of preview mesh to final result
    auto result = std::move(m_previewMesh);
    
    // Clear preview state
    clearPreview();
    m_isValid = false;

    // Notify completion
    Core::EventSystem::getInstance().dispatchEvent(Core::Event("OperationComplete"));

    return true;
}

void PatternTool::cancel() {
    clearPreview();
    m_isValid = false;
}

std::shared_ptr<Mesh> PatternTool::getPreview() const {
    return m_previewMesh;
}

bool PatternTool::validateParameters() const {
    switch (m_type) {
        case PatternType::Linear:
            return validateLinearParameters();
        case PatternType::Circular:
            return validateCircularParameters();
        case PatternType::Mirror:
            return validateMirrorParameters();
        default:
            return false;
    }
}

bool PatternTool::validateGeometry() const {
    return m_targetGeometry && m_targetGeometry->isValid();
}

bool PatternTool::generateLinearPattern(std::shared_ptr<Mesh> preview) {
    auto params = std::dynamic_pointer_cast<LinearPatternParameters>(m_parameters);
    if (!params) {
        return false;
    }

    // Pre-allocate memory from pool
    auto& memoryPool = MemoryPool::getInstance();
    size_t vertexCount = m_targetGeometry->getVertexCount();
    size_t faceCount = m_targetGeometry->getFaceCount();
    
    // Add original geometry if requested
    if (params->includeOriginal) {
        preview->merge(*m_targetGeometry);
    }

    // Calculate optimal parallel threshold based on instance size
    const size_t INSTANCE_SIZE_THRESHOLD = 1000; // vertices
    const size_t dynamicThreshold = std::max<size_t>(
        2UL, 
        INSTANCE_SIZE_THRESHOLD / std::max<size_t>(1UL, vertexCount)
    );

    // Pre-calculate transforms to avoid redundant calculations
    std::vector<glm::mat4> transforms;
    transforms.reserve(params->count - 1);
    for (int i = 1; i < params->count; ++i) {
        float distance = params->spacing * i;
        transforms.push_back(glm::translate(params->direction * distance));
    }

    if (params->count >= dynamicThreshold) {
        // Allocate instances vector before parallel region
        std::vector<Mesh> instances;
        instances.reserve(params->count - 1);
        
        // Create instance meshes using memory pool
        for (int i = 0; i < params->count - 1; ++i) {
            instances.emplace_back();
            instances.back().reserveFromPool(vertexCount, faceCount);
        }

        // Process instances in parallel
        #pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < params->count - 1; ++i) {
            instances[i] = *m_targetGeometry;
            instances[i].transformOptimized(transforms[i]); // Use optimized transform
        }

        // Merge instances using parallel reduction
        const int MERGE_CHUNK_SIZE = 4;
        #pragma omp parallel for schedule(dynamic, MERGE_CHUNK_SIZE)
        for (int i = 0; i < instances.size(); i += MERGE_CHUNK_SIZE) {
            int end = std::min(i + MERGE_CHUNK_SIZE, static_cast<int>(instances.size()));
            for (int j = i; j < end; ++j) {
                #pragma omp critical
                preview->merge(std::move(instances[j]));
            }
        }
    } else {
        // Sequential processing for small counts
        Mesh instance;
        instance.reserveFromPool(vertexCount, faceCount);
        
        for (int i = 0; i < params->count - 1; ++i) {
            instance = *m_targetGeometry;
            instance.transformOptimized(transforms[i]); // Use optimized transform
            preview->merge(std::move(instance));
        }
    }

    return true;
}

bool PatternTool::generateCircularPattern(std::shared_ptr<Mesh> preview) {
    auto params = std::dynamic_pointer_cast<CircularPatternParameters>(m_parameters);
    if (!params) {
        return false;
    }

    // Pre-allocate memory from pool
    auto& memoryPool = MemoryPool::getInstance();
    size_t vertexCount = m_targetGeometry->getVertexCount();
    size_t faceCount = m_targetGeometry->getFaceCount();
    
    // Add original geometry if requested
    if (params->includeOriginal) {
        preview->merge(*m_targetGeometry);
    }

    // Calculate angle step
    float angleStep = params->angle / (params->count - 1);

    // Calculate optimal parallel threshold based on instance size
    const size_t INSTANCE_SIZE_THRESHOLD = 1000; // vertices
    const size_t dynamicThreshold = std::max<size_t>(
        2UL, 
        INSTANCE_SIZE_THRESHOLD / std::max<size_t>(1UL, vertexCount)
    );

    // Pre-calculate transforms to avoid redundant calculations
    std::vector<glm::mat4> transforms;
    transforms.reserve(params->count - 1);
    for (int i = 1; i < params->count; ++i) {
        float angle = angleStep * i;
        transforms.push_back(
            glm::translate(params->center) *
            glm::rotate(glm::radians(angle), params->axis) *
            glm::translate(-params->center)
        );
    }

    if (params->count >= dynamicThreshold) {
        // Allocate instances vector before parallel region
        std::vector<Mesh> instances;
        instances.reserve(params->count - 1);
        
        // Create instance meshes using memory pool
        for (int i = 0; i < params->count - 1; ++i) {
            instances.emplace_back();
            instances.back().reserveFromPool(vertexCount, faceCount);
        }

        // Process instances in parallel
        #pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < params->count - 1; ++i) {
            instances[i] = *m_targetGeometry;
            instances[i].transformOptimized(transforms[i]); // Use optimized transform
        }

        // Merge instances using parallel reduction
        const int MERGE_CHUNK_SIZE = 4;
        #pragma omp parallel for schedule(dynamic, MERGE_CHUNK_SIZE)
        for (int i = 0; i < instances.size(); i += MERGE_CHUNK_SIZE) {
            int end = std::min(i + MERGE_CHUNK_SIZE, static_cast<int>(instances.size()));
            for (int j = i; j < end; ++j) {
                #pragma omp critical
                preview->merge(std::move(instances[j]));
            }
        }
    } else {
        // Sequential processing for small counts
        Mesh instance;
        instance.reserveFromPool(vertexCount, faceCount);
        
        for (int i = 0; i < params->count - 1; ++i) {
            instance = *m_targetGeometry;
            instance.transformOptimized(transforms[i]); // Use optimized transform
            preview->merge(std::move(instance));
        }
    }

    return true;
}

bool PatternTool::generateMirrorPattern(std::shared_ptr<Mesh> preview) {
    auto params = std::dynamic_pointer_cast<MirrorPatternParameters>(m_parameters);
    if (!params) {
        return false;
    }

    // Add original geometry if requested
    if (params->includeOriginal) {
        preview->merge(*m_targetGeometry);
    }

    // Create mirror transformation matrix
    glm::vec3 n = glm::normalize(params->normal);
    glm::mat4 mirror(
        1.0f - 2.0f * n.x * n.x,     -2.0f * n.x * n.y,         -2.0f * n.x * n.z,         0.0f,
        -2.0f * n.y * n.x,           1.0f - 2.0f * n.y * n.y,   -2.0f * n.y * n.z,         0.0f,
        -2.0f * n.z * n.x,           -2.0f * n.z * n.y,         1.0f - 2.0f * n.z * n.z,   0.0f,
        0.0f,                        0.0f,                       0.0f,                       1.0f
    );
    
    // Apply mirror transformation around the reference point
    glm::mat4 transform = glm::translate(params->point) * mirror * glm::translate(-params->point);

    // Create mirrored instance
    Mesh instance = *m_targetGeometry;
    instance.transform(transform);
    preview->merge(instance);

    return true;
}

void PatternTool::clearPreview() {
    m_previewMesh.reset();
    m_previewCache->clear();
}

void PatternTool::updatePreview() {
    if (m_previewMesh) {
        m_previewCache->update(m_previewMesh);
        Core::EventSystem::getInstance().dispatchEvent(Core::Event("PreviewUpdate"));
    }
}

bool PatternTool::validateLinearParameters() const {
    auto params = std::dynamic_pointer_cast<LinearPatternParameters>(m_parameters);
    if (!params) {
        return false;
    }

    // Validate direction vector
    if (glm::length(params->direction) < 1e-6f) {
        return false;
    }

    // Validate spacing and count
    return params->spacing > 0.0f && params->count >= 2;
}

bool PatternTool::validateCircularParameters() const {
    auto params = std::dynamic_pointer_cast<CircularPatternParameters>(m_parameters);
    if (!params) {
        return false;
    }

    // Validate axis vector
    if (glm::length(params->axis) < 1e-6f) {
        return false;
    }

    // Validate angle and count
    return params->angle > 0.0f && params->angle <= 360.0f && params->count >= 2;
}

bool PatternTool::validateMirrorParameters() const {
    auto params = std::dynamic_pointer_cast<MirrorPatternParameters>(m_parameters);
    if (!params) {
        return false;
    }

    // Validate normal vector
    return glm::length(params->normal) > 1e-6f;
}

} // namespace Modeling
} // namespace RebelCAD
