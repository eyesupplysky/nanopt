//! Right-handed orthonormal shading frame for tangent-space BSDF evaluation

#pragma once

#include <cmath>

#include "math/normal3.hpp"
#include "math/vec3.hpp"

namespace nanopt {

struct Frame {
    Normal3 n;
    Vec3 s;
    Vec3 t;

    constexpr Frame() = default;
    constexpr Frame(Normal3 n_, Vec3 s_, Vec3 t_) : n(n_), s(s_), t(t_) {}

    /// Build an orthonormal frame around a unit-length normal — Duff et al. 2017 branchless basis
    static Frame fromNormal(Normal3 n) {
        const float sign = std::copysignf(1.0f, n.z);
        const float a = -1.0f / (sign + n.z);
        const float b = n.x * n.y * a;
        const Vec3 s{1.0f + sign * n.x * n.x * a, sign * b, -sign * n.x};
        const Vec3 t{b, sign + n.y * n.y * a, -n.y};
        return Frame{n, s, t};
    }

    /// Transform a world-space direction into tangent space — local +z is aligned with n
    constexpr Vec3 toLocal(Vec3 v) const { return Vec3{dot(v, s), dot(v, t), dot(v, n)}; }

    /// Transform a tangent-space direction back to world space
    constexpr Vec3 toWorld(Vec3 v) const { return s * v.x + t * v.y + n * v.z; }
};

}  // namespace nanopt
