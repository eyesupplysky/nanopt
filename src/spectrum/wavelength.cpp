//! Wavelength sampling — hero+stratified-secondary implementation

#include "spectrum/wavelength.hpp"

namespace nanopt {

SampledWavelengths SampledWavelengths::sampleHeroStratified(float u) {
    SampledWavelengths sw;
    constexpr float range = kLambdaMax - kLambdaMin;
    constexpr float binWidth = range / static_cast<float>(kSpectrumSamples);
    constexpr float pdfPerSample = 1.0f / range;
    const float baseOffset = u * binWidth;
    for (int i = 0; i < kSpectrumSamples; ++i) {
        const float binStart = kLambdaMin + static_cast<float>(i) * binWidth;
        sw.lambda[i] = binStart + baseOffset;
        sw.pdf[i] = pdfPerSample;
    }
    return sw;
}

void SampledWavelengths::terminateSecondary() {
    for (int i = 1; i < kSpectrumSamples; ++i) {
        pdf[i] = 0.0f;
    }
}

bool SampledWavelengths::isSecondaryTerminated() const {
    for (int i = 1; i < kSpectrumSamples; ++i) {
        if (pdf[i] != 0.0f) return false;
    }
    return true;
}

}  // namespace nanopt
