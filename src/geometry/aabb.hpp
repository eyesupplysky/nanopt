//! Axis-aligned bounding box and helpers used by primitives and the BVH

#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>

#include "math/point3.hpp"
#include "math/ray.hpp"
#include "math/vec3.hpp"

namespace nanopt {

struct Aabb {
    Point3 min{std::numeric_limits<float>::infinity(),
               std::numeric_limits<float>::infinity(),
               std::numeric_limits<float>::infinity()};
    Point3 max{-std::numeric_limits<float>::infinity(),
               -std::numeric_limits<float>::infinity(),
               -std::numeric_limits<float>::infinity()};
};

/// Box containing both inputs
constexpr Aabb aabbUnion(Aabb a, Aabb b) {
    return Aabb{
        Point3{std::min(a.min.x, b.min.x), std::min(a.min.y, b.min.y), std::min(a.min.z, b.min.z)},
        Point3{std::max(a.max.x, b.max.x), std::max(a.max.y, b.max.y), std::max(a.max.z, b.max.z)},
    };
}

/// Box extended to contain p
constexpr Aabb aabbExtend(Aabb a, Point3 p) {
    return Aabb{
        Point3{std::min(a.min.x, p.x), std::min(a.min.y, p.y), std::min(a.min.z, p.z)},
        Point3{std::max(a.max.x, p.x), std::max(a.max.y, p.y), std::max(a.max.z, p.z)},
    };
}

/// Center of the box; meaningless for an empty box
constexpr Point3 centroid(Aabb a) {
    return Point3{0.5f * (a.min.x + a.max.x),
                  0.5f * (a.min.y + a.max.y),
                  0.5f * (a.min.z + a.max.z)};
}

/// Axis (0=x, 1=y, 2=z) along which the box is widest
constexpr int largestAxis(Aabb a) {
    const Vec3 diag = a.max - a.min;
    if (diag.x >= diag.y && diag.x >= diag.z) {
        return 0;
    }
    if (diag.y >= diag.z) {
        return 1;
    }
    return 2;
}

/// Surface area; zero for an empty or degenerate box
constexpr float surfaceArea(Aabb a) {
    const Vec3 diag = a.max - a.min;
    if (diag.x < 0.0f || diag.y < 0.0f || diag.z < 0.0f) {
        return 0.0f;
    }
    return 2.0f * (diag.x * diag.y + diag.y * diag.z + diag.x * diag.z);
}

/// Slab test. Returns the entry t in (0, ray.tMax] when the ray hits the box,
/// or std::nullopt when it misses. invDir is the component-wise reciprocal
/// of ray.direction; passing it precomputed lets the BVH amortize the divides
/// across many node tests for a single ray.
inline std::optional<float> intersectAabb(Aabb a, const Ray& ray, Vec3 invDir) {
    const float tx1 = (a.min.x - ray.origin.x) * invDir.x;
    const float tx2 = (a.max.x - ray.origin.x) * invDir.x;
    float tNear = std::min(tx1, tx2);
    float tFar = std::max(tx1, tx2);

    const float ty1 = (a.min.y - ray.origin.y) * invDir.y;
    const float ty2 = (a.max.y - ray.origin.y) * invDir.y;
    tNear = std::max(tNear, std::min(ty1, ty2));
    tFar = std::min(tFar, std::max(ty1, ty2));

    const float tz1 = (a.min.z - ray.origin.z) * invDir.z;
    const float tz2 = (a.max.z - ray.origin.z) * invDir.z;
    tNear = std::max(tNear, std::min(tz1, tz2));
    tFar = std::min(tFar, std::max(tz1, tz2));

    if (tFar < std::max(tNear, 0.0f) || tNear > ray.tMax) {
        return std::nullopt;
    }
    return std::max(tNear, 0.0f);
}

}  // namespace nanopt
