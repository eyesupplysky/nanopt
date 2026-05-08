//! Ray-primitive intersection record

#pragma once

#include "math/normal3.hpp"
#include "math/point3.hpp"

namespace nanopt {

class Bsdf;

struct Hit {
    Point3 position;
    Normal3 normal;
    float t = 0.0f;
    const Bsdf* bsdf = nullptr;
};

}  // namespace nanopt
