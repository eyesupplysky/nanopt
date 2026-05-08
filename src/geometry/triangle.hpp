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
class AreaLight;

class Triangle : public Primitive {
public:
    /// Flat-shaded triangle — Hit::frame.n equals the geometric normal everywhere on the face
    Triangle(Point3 v0, Point3 v1, Point3 v2, const Bsdf* bsdf,
             const AreaLight* areaLight = nullptr);

    /// Smooth-shaded triangle — per-vertex normals are barycentric-interpolated at hit time
    Triangle(Point3 v0, Point3 v1, Point3 v2, Vec3 n0, Vec3 n1, Vec3 n2,
             const Bsdf* bsdf, const AreaLight* areaLight = nullptr);

    std::optional<Hit> intersect(const Ray& ray) const override;
    Aabb bounds() const override;

private:
    Point3 v0_;
    Point3 v1_;
    Point3 v2_;
    Vec3 n0_;
    Vec3 n1_;
    Vec3 n2_;
    bool hasVertexNormals_;
    const Bsdf* bsdf_;
    const AreaLight* areaLight_;
};

}  // namespace nanopt
