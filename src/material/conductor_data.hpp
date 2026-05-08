//! Measured complex IOR for conductors — McPeak et al. 2015 (CC0 via refractiveindex.info)

#pragma once

#include "spectrum/spectrum.hpp"
#include "spectrum/wavelength.hpp"

namespace nanopt {

/// Identifier for a measured-conductor IOR table
enum class MetalKind {
    Gold,
    Copper,
    Aluminum,
};

/// Per-wavelength complex refractive index sampled at the supplied wavelengths
struct ConductorIor {
    Spectrum eta;
    Spectrum k;
};

/// Sample η(λ) and κ(λ) of the requested metal at the supplied wavelengths via linear interpolation
ConductorIor evalConductorIor(MetalKind metal, const SampledWavelengths& lambdas);

}  // namespace nanopt
