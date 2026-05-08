//! CIE 1931 spectral-to-XYZ conversion and XYZ-to-linear-sRGB transform

#pragma once

#include "spectrum/rgb_spectrum.hpp"

namespace nanopt {

struct SampledSpectrum;
struct SampledWavelengths;

struct Xyz {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

/// Wyman et al. 2013 multi-lobe Gaussian fit of the CIE 1931 2° X̄ matching function
float cieMatchingX(float lambda);

/// Wyman et al. 2013 multi-lobe Gaussian fit of the CIE 1931 2° Ȳ matching function
float cieMatchingY(float lambda);

/// Wyman et al. 2013 multi-lobe Gaussian fit of the CIE 1931 2° Z̄ matching function
float cieMatchingZ(float lambda);

float cieD65Spectrum(float lambda);

Xyz spectrumToXyz(const SampledSpectrum& s, const SampledWavelengths& lambdas);

/// Convert CIE XYZ (D65 white point) to linear sRGB. Out-of-gamut components may be negative; caller decides how to clip.
RgbSpectrum xyzToLinearSrgb(Xyz xyz);

}  // namespace nanopt
