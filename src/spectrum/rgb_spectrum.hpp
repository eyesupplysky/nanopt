//! Linear RGB triple — placeholder Spectrum representation until M3 swaps to wavelength samples

#pragma once

namespace nanopt {

struct RgbSpectrum {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;

    constexpr RgbSpectrum() = default;
    constexpr explicit RgbSpectrum(float v) : r(v), g(v), b(v) {}
    constexpr RgbSpectrum(float r_, float g_, float b_) : r(r_), g(g_), b(b_) {}

    constexpr RgbSpectrum operator+(RgbSpectrum o) const {
        return {r + o.r, g + o.g, b + o.b};
    }
    constexpr RgbSpectrum operator*(RgbSpectrum o) const {
        return {r * o.r, g * o.g, b * o.b};
    }
    constexpr RgbSpectrum operator*(float s) const { return {r * s, g * s, b * s}; }
    constexpr RgbSpectrum operator/(float s) const { return {r / s, g / s, b / s}; }

    constexpr RgbSpectrum& operator+=(RgbSpectrum o) {
        r += o.r;
        g += o.g;
        b += o.b;
        return *this;
    }

    constexpr bool isBlack() const { return r == 0.0f && g == 0.0f && b == 0.0f; }

    /// Largest component — used for Russian-roulette throughput thresholds
    constexpr float maxComponent() const {
        const float rg = r > g ? r : g;
        return rg > b ? rg : b;
    }
};

constexpr RgbSpectrum operator*(float s, RgbSpectrum c) { return c * s; }

}  // namespace nanopt
