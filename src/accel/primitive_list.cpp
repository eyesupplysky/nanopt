//! Linear primitive list implementation

#include "accel/primitive_list.hpp"

namespace nanopt {

void PrimitiveList::add(const Primitive* primitive) {
    primitives_.push_back(primitive);
}

std::optional<Hit> PrimitiveList::intersect(const Ray& ray) const {
    std::optional<Hit> closest;
    Ray clipped = ray;

    for (const Primitive* primitive : primitives_) {
        auto hit = primitive->intersect(clipped);
        if (hit.has_value()) {
            clipped.tMax = hit->t;
            closest = hit;
        }
    }

    return closest;
}

Aabb PrimitiveList::bounds() const {
    // M2: union of child bounds. M1 callers do not query this.
    return Aabb{};
}

}  // namespace nanopt
