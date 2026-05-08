//! Axis-aligned bounding box — input to BVH construction at M2

#pragma once

#include <limits>

#include "math/point3.hpp"

namespace nanopt {

struct Aabb {
    Point3 min{std::numeric_limits<float>::infinity(),
               std::numeric_limits<float>::infinity(),
               std::numeric_limits<float>::infinity()};
    Point3 max{-std::numeric_limits<float>::infinity(),
               -std::numeric_limits<float>::infinity(),
               -std::numeric_limits<float>::infinity()};
};

}  // namespace nanopt
