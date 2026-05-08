//! Sampled spectrum — kSpectrumSamples parallel radiance/reflectance values at the wavelengths in SampledWavelengths

#pragma once

#include "spectrum/wavelength.hpp"

#include <array>

namespace nanopt {

struct SampledSpectrum {
    std::array<float, kSpectrumSamples> values{};

    constexpr SampledSpectrum() = default;
    constexpr explicit SampledSpectrum(float v) {
        for (int i = 0; i < kSpectrumSamples; ++i) values[i] = v;
    }

    constexpr SampledSpectrum operator+(SampledSpectrum o) const {
        SampledSpectrum r;
        for (int i = 0; i < kSpectrumSamples; ++i) r.values[i] = values[i] + o.values[i];
        return r;
    }
    constexpr SampledSpectrum operator*(SampledSpectrum o) const {
        SampledSpectrum r;
        for (int i = 0; i < kSpectrumSamples; ++i) r.values[i] = values[i] * o.values[i];
        return r;
    }
    constexpr SampledSpectrum operator*(float s) const {
        SampledSpectrum r;
        for (int i = 0; i < kSpectrumSamples; ++i) r.values[i] = values[i] * s;
        return r;
    }
    constexpr SampledSpectrum operator/(float s) const {
        SampledSpectrum r;
        for (int i = 0; i < kSpectrumSamples; ++i) r.values[i] = values[i] / s;
        return r;
    }

    constexpr SampledSpectrum& operator+=(SampledSpectrum o) {
        for (int i = 0; i < kSpectrumSamples; ++i) values[i] += o.values[i];
        return *this;
    }

    constexpr bool isBlack() const {
        for (int i = 0; i < kSpectrumSamples; ++i) {
            if (values[i] != 0.0f) return false;
        }
        return true;
    }

    /// Largest sample value — used for Russian-roulette throughput thresholds
    constexpr float maxComponent() const {
        float m = values[0];
        for (int i = 1; i < kSpectrumSamples; ++i) {
            if (values[i] > m) m = values[i];
        }
        return m;
    }
};

constexpr SampledSpectrum operator*(float s, SampledSpectrum c) { return c * s; }

}  // namespace nanopt
