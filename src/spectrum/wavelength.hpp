//! Wavelength sampling for spectral path tracing — hero + secondary stratification

#pragma once

#include <array>

namespace nanopt {

inline constexpr float kLambdaMin = 380.0f;
inline constexpr float kLambdaMax = 750.0f;
inline constexpr int kSpectrumSamples = 5;

struct SampledWavelengths {
    std::array<float, kSpectrumSamples> lambda{};
    std::array<float, kSpectrumSamples> pdf{};

    /// Build a hero+stratified-secondary set covering [kLambdaMin, kLambdaMax) using a single uniform u in [0, 1)
    static SampledWavelengths sampleHeroStratified(float u);

    /// Mark every slot but the hero as terminated. M4 dispersion will call this on a wavelength-dependent surface; M3 leaves all slots live.
    void terminateSecondary();

    /// True when only the hero wavelength carries non-zero pdf
    bool isSecondaryTerminated() const;
};

}  // namespace nanopt
