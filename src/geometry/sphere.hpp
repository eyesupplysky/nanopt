//! Sphere primitive — analytic ray-sphere intersection

#pragma once

#include <optional>

#include "geometry/aabb.hpp"
#include "geometry/hit.hpp"
#include "geometry/primitive.hpp"
#include "math/point3.hpp"
#include "math/ray.hpp"

namespace nanopt {

class Bsdf;

class Sphere : public Primitive {
public:
    Sphere(Point3 center, float radius, const Bsdf* bsdf);

    std::optional<Hit> intersect(const Ray& ray) const override;
    Aabb bounds() const override;

private:
    Point3 center_;
    float radius_;
    const Bsdf* bsdf_;
};

}  // namespace nanopt
