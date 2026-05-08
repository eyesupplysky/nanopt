//! 3D vector with float components for directions and offsets

#pragma once

#include <cmath>

namespace nanopt {

struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    constexpr Vec3() = default;
    constexpr explicit Vec3(float v) : x(v), y(v), z(v) {}
    constexpr Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    constexpr Vec3 operator+(Vec3 b) const { return {x + b.x, y + b.y, z + b.z}; }
    constexpr Vec3 operator-(Vec3 b) const { return {x - b.x, y - b.y, z - b.z}; }
    constexpr Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    constexpr Vec3 operator/(float s) const { return {x / s, y / s, z / s}; }
    constexpr Vec3 operator-() const { return {-x, -y, -z}; }

    constexpr Vec3& operator+=(Vec3 b) { x += b.x; y += b.y; z += b.z; return *this; }
    constexpr Vec3& operator-=(Vec3 b) { x -= b.x; y -= b.y; z -= b.z; return *this; }
    constexpr Vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
};

constexpr Vec3 operator*(float s, Vec3 v) { return v * s; }

/// Dot product
constexpr float dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

/// Cross product (right-handed)
constexpr Vec3 cross(Vec3 a, Vec3 b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
}

/// Squared length — cheaper than length when only a comparison is needed
constexpr float lengthSquared(Vec3 v) { return dot(v, v); }

/// Vector length
inline float length(Vec3 v) { return std::sqrt(lengthSquared(v)); }

/// Unit-length vector in the same direction
inline Vec3 normalize(Vec3 v) { return v / length(v); }

}  // namespace nanopt
