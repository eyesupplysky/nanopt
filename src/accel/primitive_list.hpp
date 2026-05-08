//! Linear primitive list — replaced by BVH at M2

#pragma once

#include <optional>
#include <vector>

#include "geometry/aabb.hpp"
#include "geometry/hit.hpp"
#include "geometry/primitive.hpp"
#include "math/ray.hpp"

namespace nanopt {

class PrimitiveList : public Primitive {
public:
    void add(const Primitive* primitive);

    std::optional<Hit> intersect(const Ray& ray) const override;
    Aabb bounds() const override;

private:
    std::vector<const Primitive*> primitives_;
};

}  // namespace nanopt
