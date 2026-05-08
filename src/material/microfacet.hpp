//! GGX microfacet distribution, Smith masking-shadowing, and VNDF sampling

#pragma once

#include <algorithm>
#include <cmath>
#include <numbers>

#include "math/vec3.hpp"
#include "sampler/sampler.hpp"

namespace nanopt {

/// GGX (Trowbridge-Reitz) normal distribution evaluated in the local shading frame
inline float ggxD(Vec3 h, float alpha) {
    if (h.z <= 0.0f) return 0.0f;
    const float a2 = alpha * alpha;
    const float cos2 = h.z * h.z;
    const float denomPart = cos2 * (a2 - 1.0f) + 1.0f;
    return a2 / (std::numbers::pi_v<float> * denomPart * denomPart);
}

/// Smith Λ for GGX — auxiliary for masking-shadowing (Heitz 2014 eq. 73)
inline float ggxLambda(Vec3 w, float alpha) {
    const float cos2 = w.z * w.z;
    if (cos2 >= 1.0f) return 0.0f;
    const float sin2 = 1.0f - cos2;
    const float tan2 = sin2 / cos2;
    return 0.5f * (-1.0f + std::sqrt(1.0f + alpha * alpha * tan2));
}

/// Smith G1 (single-direction masking) for GGX
inline float ggxG1(Vec3 w, float alpha) {
    return 1.0f / (1.0f + ggxLambda(w, alpha));
}

/// Height-correlated Smith G2 for GGX (Heitz 2014 eq. 99) — preferred over the separable form
inline float ggxG2(Vec3 wi, Vec3 wo, float alpha) {
    return 1.0f / (1.0f + ggxLambda(wi, alpha) + ggxLambda(wo, alpha));
}

/// Sample the GGX visible-normal distribution (Heitz 2018, isotropic α) given an outgoing direction in local frame
inline Vec3 sampleGgxVndf(Vec3 wo, float alpha, Sample2D u) {
    const Vec3 Vh = normalize(Vec3{alpha * wo.x, alpha * wo.y, wo.z});
    const float lensq = Vh.x * Vh.x + Vh.y * Vh.y;
    const Vec3 T1 = lensq > 0.0f
                        ? Vec3{-Vh.y, Vh.x, 0.0f} * (1.0f / std::sqrt(lensq))
                        : Vec3{1.0f, 0.0f, 0.0f};
    const Vec3 T2 = cross(Vh, T1);

    const float r = std::sqrt(std::clamp(u.u, 0.0f, 1.0f));
    const float phi = 2.0f * std::numbers::pi_v<float> * u.v;
    const float t1 = r * std::cos(phi);
    float t2 = r * std::sin(phi);
    const float s = 0.5f * (1.0f + Vh.z);
    t2 = (1.0f - s) * std::sqrt(std::max(0.0f, 1.0f - t1 * t1)) + s * t2;
    const float t3 = std::sqrt(std::max(0.0f, 1.0f - t1 * t1 - t2 * t2));
    const Vec3 Nh = T1 * t1 + T2 * t2 + Vh * t3;
    return normalize(Vec3{alpha * Nh.x, alpha * Nh.y, std::max(0.0f, Nh.z)});
}

}  // namespace nanopt
