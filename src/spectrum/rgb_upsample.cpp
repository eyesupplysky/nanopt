//! RGB→spectrum upsampling implementation — Smits 1999 An RGB to Spectrum Conversion for Reflectances

#include "spectrum/rgb_upsample.hpp"
#include "spectrum/cie.hpp"
#include "spectrum/rgb_spectrum.hpp"
#include "spectrum/sampled_spectrum.hpp"
#include "spectrum/wavelength.hpp"

#include <array>

namespace nanopt {

namespace {

constexpr int kBasisSamples = 10;
constexpr float kBasisLambdaMin = 380.0f;
constexpr float kBasisLambdaStep = 38.0f;  // (722 - 380) / 9, covering 380..722 nm in 10 bins

constexpr std::array<float, kBasisSamples> kSmitsWhite   = { 1.0000f, 1.0000f, 0.9999f, 0.9993f, 0.9992f, 0.9998f, 1.0000f, 1.0000f, 1.0000f, 1.0000f };
constexpr std::array<float, kBasisSamples> kSmitsCyan    = { 0.9710f, 0.9426f, 1.0007f, 1.0007f, 1.0007f, 1.0007f, 0.1564f, 0.0000f, 0.0000f, 0.0000f };
constexpr std::array<float, kBasisSamples> kSmitsMagenta = { 1.0000f, 1.0000f, 0.9685f, 0.2229f, 0.0000f, 0.0458f, 0.8369f, 1.0000f, 1.0000f, 0.9959f };
constexpr std::array<float, kBasisSamples> kSmitsYellow  = { 0.0001f, 0.0000f, 0.1088f, 0.6651f, 1.0000f, 1.0000f, 0.9996f, 0.9586f, 0.9685f, 0.9840f };
constexpr std::array<float, kBasisSamples> kSmitsRed     = { 0.1012f, 0.0515f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.8325f, 1.0149f, 1.0149f, 1.0149f };
constexpr std::array<float, kBasisSamples> kSmitsGreen   = { 0.0000f, 0.0000f, 0.0273f, 0.7937f, 1.0000f, 0.9418f, 0.1719f, 0.0000f, 0.0000f, 0.0025f };
constexpr std::array<float, kBasisSamples> kSmitsBlue    = { 1.0000f, 1.0000f, 0.8916f, 0.3323f, 0.0000f, 0.0000f, 0.0003f, 0.0369f, 0.0483f, 0.0496f };

float sampleBasis(const std::array<float, kBasisSamples>& basis, float lambda) {
    if (lambda <= kBasisLambdaMin) return basis.front();
    constexpr float maxLambda = kBasisLambdaMin + static_cast<float>(kBasisSamples - 1) * kBasisLambdaStep;
    if (lambda >= maxLambda) return basis.back();
    const float t = (lambda - kBasisLambdaMin) / kBasisLambdaStep;
    const int idx = static_cast<int>(t);
    const float f = t - static_cast<float>(idx);
    return basis[idx] * (1.0f - f) + basis[idx + 1] * f;
}

struct SmitsCoefficients {
    float white;
    float cyan;
    float magenta;
    float yellow;
    float red;
    float green;
    float blue;
};

SmitsCoefficients smitsDecompose(RgbSpectrum rgb) {
    SmitsCoefficients c{};
    const float R = rgb.r;
    const float G = rgb.g;
    const float B = rgb.b;
    if (R <= G && R <= B) {
        c.white = R;
        if (G <= B) {
            c.cyan = G - R;
            c.blue = B - G;
        } else {
            c.cyan = B - R;
            c.green = G - B;
        }
    } else if (G <= R && G <= B) {
        c.white = G;
        if (R <= B) {
            c.magenta = R - G;
            c.blue = B - R;
        } else {
            c.magenta = B - G;
            c.red = R - B;
        }
    } else {
        c.white = B;
        if (R <= G) {
            c.yellow = R - B;
            c.green = G - R;
        } else {
            c.yellow = G - B;
            c.red = R - G;
        }
    }
    return c;
}

SampledSpectrum smitsEvaluate(RgbSpectrum rgb, const SampledWavelengths& lambdas) {
    const SmitsCoefficients c = smitsDecompose(rgb);
    SampledSpectrum out;
    for (int i = 0; i < kSpectrumSamples; ++i) {
        const float lambda = lambdas.lambda[i];
        out.values[i] =
              c.white   * sampleBasis(kSmitsWhite,   lambda)
            + c.cyan    * sampleBasis(kSmitsCyan,    lambda)
            + c.magenta * sampleBasis(kSmitsMagenta, lambda)
            + c.yellow  * sampleBasis(kSmitsYellow,  lambda)
            + c.red     * sampleBasis(kSmitsRed,     lambda)
            + c.green   * sampleBasis(kSmitsGreen,   lambda)
            + c.blue    * sampleBasis(kSmitsBlue,    lambda);
    }
    return out;
}

}  // namespace

SampledSpectrum sampledReflectanceFromRgb(RgbSpectrum rgb, const SampledWavelengths& lambdas) {
    return smitsEvaluate(rgb, lambdas);
}

SampledSpectrum sampledIlluminantFromRgb(RgbSpectrum rgb, const SampledWavelengths& lambdas) {
    SampledSpectrum smits = smitsEvaluate(rgb, lambdas);
    for (int i = 0; i < kSpectrumSamples; ++i) {
        smits.values[i] *= cieD65Spectrum(lambdas.lambda[i]);
    }
    return smits;
}

}  // namespace nanopt
