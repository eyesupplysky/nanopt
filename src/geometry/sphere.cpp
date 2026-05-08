//! Sphere intersection implementation

#include "geometry/sphere.hpp"

#include <cmath>
#include <optional>

#include "math/frame.hpp"
#include "math/vec3.hpp"

namespace nanopt {

Sphere::Sphere(Point3 center, float radius, const Bsdf* bsdf)
    : center_(center), radius_(radius), bsdf_(bsdf) {}

std::optional<Hit> Sphere::intersect(const Ray& ray) const {
    // Solve |O + t*D - C|^2 = r^2 with the half-b form for numerical stability.
    const Vec3 oc = ray.origin - center_;
    const float a = lengthSquared(ray.direction);
    const float halfB = dot(oc, ray.direction);
    const float c = lengthSquared(oc) - radius_ * radius_;
    const float discriminant = halfB * halfB - a * c;

    if (discriminant < 0.0f) {
        return std::nullopt;
    }

    constexpr float kEpsilon = 1e-4f;
    const float sqrtDisc = std::sqrt(discriminant);
    float t = (-halfB - sqrtDisc) / a;
    if (t < kEpsilon || t > ray.tMax) {
        t = (-halfB + sqrtDisc) / a;
        if (t < kEpsilon || t > ray.tMax) {
            return std::nullopt;
        }
    }

    Hit hit;
    hit.t = t;
    hit.position = ray.at(t);
    const Normal3 n = normalize(hit.position - center_);
    hit.geometricNormal = n;
    hit.frame = Frame::fromNormal(n);
    hit.bsdf = bsdf_;
    return hit;
}

Aabb Sphere::bounds() const {
    const Vec3 r{radius_, radius_, radius_};
    return Aabb{center_ - r, center_ + r};
}

}  // namespace nanopt
