//! Triangle intersection implementation

#include "geometry/triangle.hpp"

#include <cmath>
#include <optional>

#include "math/vec3.hpp"

namespace nanopt {

Triangle::Triangle(Point3 v0, Point3 v1, Point3 v2, const Bsdf* bsdf)
    : v0_(v0), v1_(v1), v2_(v2), bsdf_(bsdf) {}

std::optional<Hit> Triangle::intersect(const Ray& ray) const {
    // Möller-Trumbore: solve O + tD = v0 + u(v1-v0) + v(v2-v0) for (t, u, v).
    const Vec3 e1 = v1_ - v0_;
    const Vec3 e2 = v2_ - v0_;
    const Vec3 pvec = cross(ray.direction, e2);
    const float det = dot(e1, pvec);

    constexpr float kParallelEpsilon = 1e-8f;
    if (std::fabs(det) < kParallelEpsilon) {
        return std::nullopt;
    }
    const float invDet = 1.0f / det;

    const Vec3 tvec = ray.origin - v0_;
    const float u = dot(tvec, pvec) * invDet;
    if (u < 0.0f || u > 1.0f) {
        return std::nullopt;
    }

    const Vec3 qvec = cross(tvec, e1);
    const float v = dot(ray.direction, qvec) * invDet;
    if (v < 0.0f || u + v > 1.0f) {
        return std::nullopt;
    }

    const float t = dot(e2, qvec) * invDet;
    constexpr float kEpsilon = 1e-4f;
    if (t < kEpsilon || t > ray.tMax) {
        return std::nullopt;
    }

    Hit hit;
    hit.t = t;
    hit.position = ray.at(t);
    hit.normal = normalize(cross(e1, e2));
    hit.bsdf = bsdf_;
    return hit;
}

Aabb Triangle::bounds() const {
    Aabb box;
    box = aabbExtend(box, v0_);
    box = aabbExtend(box, v1_);
    box = aabbExtend(box, v2_);
    return box;
}

}  // namespace nanopt
