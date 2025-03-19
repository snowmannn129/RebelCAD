#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include "constraints/Constraint.h"
#include "constraints/AutoConstraintDetector.h"

// Forward declarations
namespace RebelCAD {
    class AutoConstraintDetector;
}

namespace RebelCAD {
namespace Constraints {

/**
 * @brief Manages the user interface for geometric and dimensional constraints
 */
class ConstraintManagerUI {
public:
    /**
     * @brief Constructs a new Constraint Manager UI
     * @param autoDetector Pointer to the auto constraint detector
     */
    explicit ConstraintManagerUI(std::shared_ptr<AutoConstraintDetector> autoDetector);

    /**
     * @brief Renders the constraint manager interface
     * @return true if any changes were made to constraints
     */
    bool render();

    /**
     * @brief Adds a constraint to be managed
     * @param constraint Pointer to the constraint to add
     * @return true if successfully added
     */
    bool addConstraint(std::shared_ptr<Constraint> constraint);

    /**
     * @brief Removes a constraint from management
     * @param constraint Pointer to the constraint to remove
     * @return true if successfully removed
     */
    bool removeConstraint(std::shared_ptr<Constraint> constraint);

    /**
     * @brief Enables or disables auto-constraint detection
     * @param enabled Whether auto-constraint detection should be enabled
     */
    void setAutoConstraintEnabled(bool enabled);

    /**
     * @brief Gets whether auto-constraint detection is enabled
     * @return true if auto-constraint detection is enabled
     */
    bool isAutoConstraintEnabled() const;

    /**
     * @brief Updates the constraint list after changes
     */
    void refresh();

private:
    /**
     * @brief Renders the auto-constraint settings section
     * @return true if any settings were changed
     */
    bool renderAutoConstraintSettings();

    /**
     * @brief Renders the active constraints list
     * @return true if any constraints were modified
     */
    bool renderConstraintList();

    /**
     * @brief Renders the constraint type filter options
     */
    void renderConstraintFilters();

    std::shared_ptr<AutoConstraintDetector> m_autoDetector;
    std::vector<std::shared_ptr<Constraint>> m_constraints;
    std::unordered_map<std::string, bool> m_constraintTypeFilters;
    bool m_autoConstraintEnabled;
};

} // namespace Constraints
} // namespace RebelCAD
