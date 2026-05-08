//! Camera interface — generates primary rays from normalized image coordinates

#pragma once

#include "math/ray.hpp"

namespace nanopt {

class Camera {
public:
    virtual ~Camera() = default;

    virtual Ray generateRay(float u, float v) const = 0;
};

}  // namespace nanopt
