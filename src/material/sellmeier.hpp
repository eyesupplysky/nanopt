//! Sellmeier 3-term dispersion equation for glass IOR — wavelength-dependent refractive index

#pragma once

#include <cmath>

namespace nanopt {

/// Coefficients for the Sellmeier equation: n²(λ) = 1 + Σᵢ Bᵢ·λ²/(λ² − Cᵢ), with λ in μm and Cᵢ in μm²
struct SellmeierGlass {
    float B[3];
    float C[3];
};

/// Schott BK7 — borosilicate crown, low dispersion, the canonical "ordinary" glass
inline constexpr SellmeierGlass kBk7{
    {1.03961212f, 0.231792344f, 1.01046945f},
    {0.00600069867f, 0.0200179144f, 103.560653f},
};

/// Schott SF10 — dense flint, high dispersion (used for visible-spectrum prism demonstrations)
inline constexpr SellmeierGlass kSf10{
    {1.62153902f, 0.256287842f, 1.64447552f},
    {0.0122241457f, 0.0595736775f, 147.468793f},
};

/// Index of refraction at the supplied wavelength in nanometers
inline float iorAtNm(SellmeierGlass glass, float lambda_nm) {
    const float lambda_um = lambda_nm * 1e-3f;
    const float l2 = lambda_um * lambda_um;
    float n2_minus_1 = 0.0f;
    for (int i = 0; i < 3; ++i) {
        n2_minus_1 += (glass.B[i] * l2) / (l2 - glass.C[i]);
    }
    return std::sqrt(1.0f + n2_minus_1);
}

}  // namespace nanopt
