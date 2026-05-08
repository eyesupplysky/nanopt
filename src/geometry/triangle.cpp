//! Triangle intersection implementation

#include "geometry/triangle.hpp"

#include <cmath>
#include <optional>

#include "math/frame.hpp"
#include "math/vec3.hpp"

namespace nanopt {

Triangle::Triangle(Point3 v0, Point3 v1, Point3 v2, const Bsdf* bsdf,
                   const AreaLight* areaLight)
    : v0_(v0), v1_(v1), v2_(v2), n0_{}, n1_{}, n2_{},
      hasVertexNormals_(false), bsdf_(bsdf), areaLight_(areaLight) {}

Triangle::Triangle(Point3 v0, Point3 v1, Point3 v2, Vec3 n0, Vec3 n1, Vec3 n2,
                   const Bsdf* bsdf, const AreaLight* areaLight)
    : v0_(v0), v1_(v1), v2_(v2), n0_(n0), n1_(n1), n2_(n2),
      hasVertexNormals_(true), bsdf_(bsdf), areaLight_(areaLight) {}

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
    const Normal3 geom = normalize(cross(e1, e2));
    hit.geometricNormal = geom;

    if (hasVertexNormals_) {
        const float w = 1.0f - u - v;
        const Vec3 raw = n0_ * w + n1_ * u + n2_ * v;
        Normal3 shading = lengthSquared(raw) > 0.0f ? normalize(raw) : geom;
        // Face-forward: interpolated shading normal must stay in geometric upper hemisphere.
        if (dot(shading, geom) < 0.0f) {
            shading = -shading;
        }
        hit.frame = Frame::fromNormal(shading);
    } else {
        hit.frame = Frame::fromNormal(geom);
    }

    hit.bsdf = bsdf_;
    hit.areaLight = areaLight_;
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
