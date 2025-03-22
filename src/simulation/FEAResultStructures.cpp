#include "simulation/FEAResult.h"
#include <cmath>

// Define M_PI if not already defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace rebel::simulation {

// StressResult implementation
StressResult::StressResult(int elementId, double xx, double yy, double zz, double xy, double yz, double xz)
    : elementId(elementId), xx(xx), yy(yy), zz(zz), xy(xy), yz(yz), xz(xz) {
    
    // Calculate von Mises stress
    vonMises = std::sqrt(0.5 * ((xx - yy) * (xx - yy) + 
                                (yy - zz) * (yy - zz) + 
                                (zz - xx) * (zz - xx) + 
                                6.0 * (xy * xy + yz * yz + xz * xz)));
    
    // Calculate principal stresses
    double I1 = xx + yy + zz;
    double I2 = xx * yy + yy * zz + zz * xx - xy * xy - yz * yz - xz * xz;
    double I3 = xx * yy * zz + 2.0 * xy * yz * xz - xx * yz * yz - yy * xz * xz - zz * xy * xy;
    
    // Solve cubic equation for principal stresses
    double p = I1 * I1 - 3.0 * I2;
    double q = 2.0 * I1 * I1 * I1 - 9.0 * I1 * I2 + 27.0 * I3;
    double r = std::sqrt(std::abs(p));
    
    double phi = std::acos(q / (2.0 * r * r * r));
    
    maxPrincipal = I1 / 3.0 + 2.0 * r * std::cos(phi / 3.0) / 3.0;
    midPrincipal = I1 / 3.0 + 2.0 * r * std::cos((phi + 2.0 * M_PI) / 3.0) / 3.0;
    minPrincipal = I1 / 3.0 + 2.0 * r * std::cos((phi + 4.0 * M_PI) / 3.0) / 3.0;
    
    // Calculate maximum shear stress
    maxShear = (maxPrincipal - minPrincipal) / 2.0;
}

// StrainResult implementation
StrainResult::StrainResult(int elementId, double xx, double yy, double zz, double xy, double yz, double xz)
    : elementId(elementId), xx(xx), yy(yy), zz(zz), xy(xy), yz(yz), xz(xz) {
    
    // Calculate principal strains
    double I1 = xx + yy + zz;
    double I2 = xx * yy + yy * zz + zz * xx - xy * xy - yz * yz - xz * xz;
    double I3 = xx * yy * zz + 2.0 * xy * yz * xz - xx * yz * yz - yy * xz * xz - zz * xy * xy;
    
    // Solve cubic equation for principal strains
    double p = I1 * I1 - 3.0 * I2;
    double q = 2.0 * I1 * I1 * I1 - 9.0 * I1 * I2 + 27.0 * I3;
    double r = std::sqrt(std::abs(p));
    
    double phi = std::acos(q / (2.0 * r * r * r));
    
    maxPrincipal = I1 / 3.0 + 2.0 * r * std::cos(phi / 3.0) / 3.0;
    midPrincipal = I1 / 3.0 + 2.0 * r * std::cos((phi + 2.0 * M_PI) / 3.0) / 3.0;
    minPrincipal = I1 / 3.0 + 2.0 * r * std::cos((phi + 4.0 * M_PI) / 3.0) / 3.0;
    
    // Calculate maximum shear strain
    maxShear = (maxPrincipal - minPrincipal) / 2.0;
}

} // namespace rebel::simulation
