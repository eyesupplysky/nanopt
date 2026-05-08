//! Ray with origin, direction, and a maximum parametric distance

#pragma once

#include <limits>

#include "math/point3.hpp"
#include "math/vec3.hpp"

namespace nanopt {

struct Ray {
    Point3 origin;
    Vec3 direction;
    float tMax = std::numeric_limits<float>::infinity();

    constexpr Ray() = default;
    Ray(Point3 origin_, Vec3 direction_,
        float tMax_ = std::numeric_limits<float>::infinity())
        : origin(origin_), direction(direction_), tMax(tMax_) {}

    /// Point along the ray at parametric distance t
    Point3 at(float t) const { return origin + direction * t; }
};

}  // namespace nanopt
