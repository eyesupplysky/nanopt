//! Bounding volume hierarchy — replaces linear primitive list as the world aggregator

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "geometry/aabb.hpp"
#include "geometry/hit.hpp"
#include "geometry/primitive.hpp"
#include "math/ray.hpp"

namespace nanopt {

class Bvh : public Primitive {
public:
    Bvh() = default;

    /// Replace contents with the given primitives and build the SAH BVH over them.
    /// Subsequent calls re-build from scratch.
    void build(std::vector<const Primitive*> primitives);

    std::optional<Hit> intersect(const Ray& ray) const override;
    Aabb bounds() const override;

    int nodeCount() const { return static_cast<int>(nodes_.size()); }
    int primitiveCount() const { return static_cast<int>(primitives_.size()); }

private:
    struct Node {
        Aabb bounds;
        std::int32_t firstPrim = 0;   // valid when primCount > 0 (leaf)
        std::int32_t leftChild = -1;  // valid when primCount == 0 (inner)
        std::int32_t rightChild = -1;
        std::int32_t primCount = 0;
        std::int32_t splitAxis = 0;
    };

    std::int32_t buildRecursive(std::int32_t firstPrim, std::int32_t primCount, int depth);
    Aabb computePrimRange(std::int32_t firstPrim, std::int32_t primCount) const;
    Aabb computeCentroidRange(std::int32_t firstPrim, std::int32_t primCount) const;

    std::vector<const Primitive*> primitives_;
    std::vector<Aabb> primBounds_;        // parallel to primitives_
    std::vector<Point3> primCentroids_;   // parallel to primitives_
    std::vector<Node> nodes_;
};

}  // namespace nanopt
