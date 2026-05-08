//! RGB→spectrum upsampling — converts linear-sRGB scene literals to wavelength-sampled spectra

#pragma once

namespace nanopt {

struct RgbSpectrum;
struct SampledSpectrum;
struct SampledWavelengths;

SampledSpectrum sampledReflectanceFromRgb(RgbSpectrum rgb, const SampledWavelengths& lambdas);

SampledSpectrum sampledIlluminantFromRgb(RgbSpectrum rgb, const SampledWavelengths& lambdas);

}  // namespace nanopt
