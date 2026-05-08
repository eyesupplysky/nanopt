//! Geometric sampling helpers — concentric disk, cosine hemisphere, uniform triangle, ONB

#pragma once

#include <cmath>
#include <numbers>

#include "math/normal3.hpp"
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

/// Branchless orthonormal basis around n, after Duff et al. ("Building an Orthonormal Basis, Revisited")
inline void buildOrthonormalBasis(Normal3 n, Vec3& tangent, Vec3& bitangent) {
    const float sign = std::copysignf(1.0f, n.z);
    const float a = -1.0f / (sign + n.z);
    const float b = n.x * n.y * a;
    tangent = Vec3{1.0f + sign * n.x * n.x * a, sign * b, -sign * n.x};
    bitangent = Vec3{b, sign + n.y * n.y * a, -n.y};
}

/// Transform a vector from a local tangent frame (z aligned with n) into world space
inline Vec3 fromTangentSpace(Vec3 local, Vec3 tangent, Vec3 bitangent, Normal3 n) {
    return tangent * local.x + bitangent * local.y + n * local.z;
}

inline Sample2D uniformTriangleBarycentric(Sample2D u) {
    const float su = std::sqrt(u.u);
    return {1.0f - su, u.v * su};
}

}  // namespace nanopt
