//! Triangle primitive — Möller-Trumbore ray intersection

#pragma once

#include <optional>

#include "geometry/aabb.hpp"
#include "geometry/hit.hpp"
#include "geometry/primitive.hpp"
#include "math/point3.hpp"
#include "math/ray.hpp"
#include "math/vec3.hpp"

namespace nanopt {

class Bsdf;

class Triangle : public Primitive {
public:
    Triangle(Point3 v0, Point3 v1, Point3 v2, const Bsdf* bsdf);

    std::optional<Hit> intersect(const Ray& ray) const override;
    Aabb bounds() const override;

private:
    Point3 v0_;
    Point3 v1_;
    Point3 v2_;
    const Bsdf* bsdf_;
};

}  // namespace nanopt
