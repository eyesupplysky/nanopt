//! Geometric primitive interface — supports ray intersection and bounding

#pragma once

#include <optional>

#include "geometry/aabb.hpp"
#include "geometry/hit.hpp"
#include "math/ray.hpp"

namespace nanopt {

class Primitive {
public:
    virtual ~Primitive() = default;

    /// Test the ray against this primitive
    virtual std::optional<Hit> intersect(const Ray& ray) const = 0;

    /// Axis-aligned bounding box in world space
    virtual Aabb bounds() const = 0;
};

}  // namespace nanopt
