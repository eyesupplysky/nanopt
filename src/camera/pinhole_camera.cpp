//! Pinhole camera implementation

#include "camera/pinhole_camera.hpp"

#include <cmath>

namespace nanopt {

PinholeCamera::PinholeCamera(Point3 origin, Point3 lookAt, Vec3 up, float fov, float aspectRatio)
    : origin_(origin) {
    forward_ = normalize(lookAt - origin);
    right_ = normalize(cross(forward_, up));
    up_ = cross(right_, forward_);

    halfHeight_ = std::tan(fov * 0.5f);
    halfWidth_ = halfHeight_ * aspectRatio;
}

Ray PinholeCamera::generateRay(float u, float v) const {
    const float x = (2.0f * u - 1.0f) * halfWidth_;
    const float y = (2.0f * v - 1.0f) * halfHeight_;
    const Vec3 direction = normalize(forward_ + right_ * x + up_ * y);
    return Ray{origin_, direction};
}

}  // namespace nanopt
