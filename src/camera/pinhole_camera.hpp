//! Pinhole camera — perspective projection without a lens

#pragma once

#include "camera/camera.hpp"
#include "math/point3.hpp"
#include "math/ray.hpp"
#include "math/vec3.hpp"

namespace nanopt {

class PinholeCamera : public Camera {
public:
    /// Look-at construction. fov is the vertical field of view in radians.
    PinholeCamera(Point3 origin, Point3 lookAt, Vec3 up, float fov, float aspectRatio);

    Ray generateRay(float u, float v) const override;

private:
    Point3 origin_;
    Vec3 right_;
    Vec3 up_;
    Vec3 forward_;
    float halfWidth_;
    float halfHeight_;
};

}  // namespace nanopt
