//! 3D point in world or object space — distinct from Vec3 to keep affine arithmetic honest

#pragma once

#include "math/vec3.hpp"

namespace nanopt {

struct Point3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    constexpr Point3() = default;
    constexpr Point3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    constexpr Point3 operator+(Vec3 v) const { return {x + v.x, y + v.y, z + v.z}; }
    constexpr Point3 operator-(Vec3 v) const { return {x - v.x, y - v.y, z - v.z}; }
    constexpr Vec3 operator-(Point3 b) const { return {x - b.x, y - b.y, z - b.z}; }

    constexpr Point3& operator+=(Vec3 v) { x += v.x; y += v.y; z += v.z; return *this; }
};

}  // namespace nanopt
