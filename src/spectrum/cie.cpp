//! CIE 1931 spectral-to-XYZ + XYZ-to-linear-sRGB implementation

#include "spectrum/cie.hpp"
#include "spectrum/sampled_spectrum.hpp"
#include "spectrum/wavelength.hpp"

#include <array>
#include <cmath>

namespace nanopt {

namespace {

constexpr int kD65Count = 38;
constexpr float kD65LambdaMin = 380.0f;
constexpr float kD65LambdaStep = 10.0f;

// CIE D65 standard illuminant SPD, 10 nm step, normalized so D65(560 nm) = 1.0.
// Source: CIE 015:2018 standard, divided by 100 (the canonical normalization peaks at 100).
constexpr std::array<float, kD65Count> kD65Spd = {
    0.499755f, 0.546482f, 0.827549f, 0.91486f,  0.934318f, 0.866823f,
    1.04865f,  1.17008f,  1.17812f,  1.14861f,  1.15923f,  1.08811f,
    1.09354f,  1.07802f,  1.0479f,   1.07689f,  1.04405f,  1.04046f,
    1.0f,      0.963342f, 0.95788f,  0.886856f, 0.900062f, 0.895991f,
    0.876987f, 0.832886f, 0.836992f, 0.800268f, 0.802146f, 0.822778f,
    0.782842f, 0.697213f, 0.716091f, 0.74349f,  0.61604f,  0.698856f,
    0.75087f,  0.635927f,
};

float gauss(float x, float mu, float sigma1, float sigma2) {
    const float t = x - mu;
    const float sigma = (t < 0.0f) ? sigma1 : sigma2;
    const float r = t / sigma;
    return std::exp(-0.5f * r * r);
}

float computeD65YNormalization() {
    constexpr float dlambda = 1.0f;
    double acc = 0.0;
    for (float lambda = kLambdaMin; lambda <= kLambdaMax; lambda += dlambda) {
        acc += static_cast<double>(cieD65Spectrum(lambda))
             * static_cast<double>(cieMatchingY(lambda));
    }
    return static_cast<float>(acc * static_cast<double>(dlambda));
}

}  // namespace

float cieMatchingX(float lambda) {
    return 0.362f * gauss(lambda, 442.0f, 16.0f, 26.7f)
         + 1.056f * gauss(lambda, 599.8f, 37.9f, 31.0f)
         - 0.065f * gauss(lambda, 501.1f, 20.4f, 26.2f);
}

float cieMatchingY(float lambda) {
    return 0.821f * gauss(lambda, 568.8f, 46.9f, 40.5f)
         + 0.286f * gauss(lambda, 530.9f, 16.3f, 31.1f);
}

float cieMatchingZ(float lambda) {
    return 1.217f * gauss(lambda, 437.0f, 11.8f, 36.0f)
         + 0.681f * gauss(lambda, 459.0f, 26.0f, 13.8f);
}

float cieD65Spectrum(float lambda) {
    if (lambda <= kD65LambdaMin) return kD65Spd.front();
    constexpr float maxLambda =
        kD65LambdaMin + static_cast<float>(kD65Count - 1) * kD65LambdaStep;
    if (lambda >= maxLambda) return kD65Spd.back();
    const float t = (lambda - kD65LambdaMin) / kD65LambdaStep;
    const int idx = static_cast<int>(t);
    const float f = t - static_cast<float>(idx);
    return kD65Spd[idx] * (1.0f - f) + kD65Spd[idx + 1] * f;
}

Xyz spectrumToXyz(const SampledSpectrum& s, const SampledWavelengths& lambdas) {
    static const float kD65YIntegral = computeD65YNormalization();

    Xyz acc;
    int liveSamples = 0;
    for (int i = 0; i < kSpectrumSamples; ++i) {
        const float p = lambdas.pdf[i];
        if (p == 0.0f) continue;
        ++liveSamples;
        const float weight = s.values[i] / p;
        acc.x += cieMatchingX(lambdas.lambda[i]) * weight;
        acc.y += cieMatchingY(lambdas.lambda[i]) * weight;
        acc.z += cieMatchingZ(lambdas.lambda[i]) * weight;
    }
    if (liveSamples == 0) return acc;
    const float scale = 1.0f / (static_cast<float>(liveSamples) * kD65YIntegral);
    acc.x *= scale;
    acc.y *= scale;
    acc.z *= scale;
    return acc;
}

RgbSpectrum xyzToLinearSrgb(Xyz xyz) {
    return {
         3.2404542f * xyz.x - 1.5371385f * xyz.y - 0.4985314f * xyz.z,
        -0.9692660f * xyz.x + 1.8760108f * xyz.y + 0.0415560f * xyz.z,
         0.0556434f * xyz.x - 0.2040259f * xyz.y + 1.0572252f * xyz.z,
    };
}

}  // namespace nanopt
