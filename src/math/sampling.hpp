//! Geometric sampling helpers — concentric disk, cosine hemisphere, uniform triangle

#pragma once

#include <cmath>
#include <numbers>

#include "math/vec3.hpp"
#include "sampler/sampler.hpp"

namespace nanopt {

inline Sample2D concentricDiskSample(Sample2D u) {
    const float ux = 2.0f * u.u - 1.0f;
    const float vy = 2.0f * u.v - 1.0f;
    if (ux == 0.0f && vy == 0.0f) {
        return {0.0f, 0.0f};
    }
    constexpr float kPiOver4 = std::numbers::pi_v<float> / 4.0f;
    constexpr float kPiOver2 = std::numbers::pi_v<float> / 2.0f;
    float r = 0.0f;
    float theta = 0.0f;
    if (std::fabs(ux) > std::fabs(vy)) {
        r = ux;
        theta = kPiOver4 * (vy / ux);
    } else {
        r = vy;
        theta = kPiOver2 - kPiOver4 * (ux / vy);
    }
    return {r * std::cos(theta), r * std::sin(theta)};
}

inline Vec3 cosineHemisphereLocal(Sample2D u) {
    const Sample2D d = concentricDiskSample(u);
    const float zSq = 1.0f - d.u * d.u - d.v * d.v;
    const float z = zSq > 0.0f ? std::sqrt(zSq) : 0.0f;
    return Vec3{d.u, d.v, z};
}

/// PDF (solid angle) of a cosine-weighted hemisphere sample whose local +z component is cosTheta
inline float cosineHemispherePdf(float cosTheta) {
    return cosTheta * std::numbers::inv_pi_v<float>;
}

inline Sample2D uniformTriangleBarycentric(Sample2D u) {
    const float su = std::sqrt(u.u);
    return {1.0f - su, u.v * su};
}

/// Veach power-2 MIS heuristic: weight for strategy A combined with strategy B.
/// Returns 0 when both pdfs are zero (caller should skip the contribution anyway).
inline float powerHeuristic(float pdfA, float pdfB) {
    const float a2 = pdfA * pdfA;
    const float b2 = pdfB * pdfB;
    const float denom = a2 + b2;
    return denom > 0.0f ? a2 / denom : 0.0f;
}

}  // namespace nanopt
