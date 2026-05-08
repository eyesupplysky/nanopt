//! BVH construction and traversal

#include "accel/bvh.hpp"

#include <algorithm>
#include <array>
#include <cstdint>

#include "math/vec3.hpp"

namespace nanopt {

namespace {

constexpr int kSahBins = 12;
constexpr int kMaxLeafSize = 4;
constexpr int kMaxDepth = 64;
constexpr float kTraversalCost = 1.0f;
constexpr float kIntersectionCost = 1.0f;

inline float axisComponent(Point3 p, int axis) {
    return axis == 0 ? p.x : (axis == 1 ? p.y : p.z);
}

}  // namespace

void Bvh::build(std::vector<const Primitive*> primitives) {
    primitives_ = std::move(primitives);
    primBounds_.clear();
    primCentroids_.clear();
    nodes_.clear();

    primBounds_.reserve(primitives_.size());
    primCentroids_.reserve(primitives_.size());
    for (const Primitive* prim : primitives_) {
        const Aabb b = prim->bounds();
        primBounds_.push_back(b);
        primCentroids_.push_back(centroid(b));
    }

    if (primitives_.empty()) {
        return;
    }

    nodes_.reserve(2 * primitives_.size());
    buildRecursive(0, static_cast<std::int32_t>(primitives_.size()), 0);
}

Aabb Bvh::computePrimRange(std::int32_t firstPrim, std::int32_t primCount) const {
    Aabb box;
    for (std::int32_t i = 0; i < primCount; ++i) {
        box = aabbUnion(box, primBounds_[firstPrim + i]);
    }
    return box;
}

Aabb Bvh::computeCentroidRange(std::int32_t firstPrim, std::int32_t primCount) const {
    Aabb box;
    for (std::int32_t i = 0; i < primCount; ++i) {
        box = aabbExtend(box, primCentroids_[firstPrim + i]);
    }
    return box;
}

std::int32_t Bvh::buildRecursive(std::int32_t firstPrim, std::int32_t primCount, int depth) {
    const std::int32_t nodeIndex = static_cast<std::int32_t>(nodes_.size());
    nodes_.push_back(Node{});

    const Aabb nodeBounds = computePrimRange(firstPrim, primCount);
    nodes_[nodeIndex].bounds = nodeBounds;

    const auto makeLeaf = [&]() {
        nodes_[nodeIndex].firstPrim = firstPrim;
        nodes_[nodeIndex].primCount = primCount;
    };

    if (primCount <= kMaxLeafSize || depth >= kMaxDepth) {
        makeLeaf();
        return nodeIndex;
    }

    const Aabb centroidBounds = computeCentroidRange(firstPrim, primCount);
    const int axis = largestAxis(centroidBounds);
    const float axisMin = axisComponent(centroidBounds.min, axis);
    const float axisMax = axisComponent(centroidBounds.max, axis);
    if (axisMax <= axisMin) {
        // All centroids coincide along every axis — splitting cannot help.
        makeLeaf();
        return nodeIndex;
    }

    struct Bin {
        Aabb bounds;
        std::int32_t count = 0;
    };
    std::array<Bin, kSahBins> bins{};

    const float invExtent = static_cast<float>(kSahBins) / (axisMax - axisMin);
    for (std::int32_t i = 0; i < primCount; ++i) {
        const float c = axisComponent(primCentroids_[firstPrim + i], axis);
        int b = static_cast<int>((c - axisMin) * invExtent);
        if (b < 0) b = 0;
        if (b >= kSahBins) b = kSahBins - 1;
        bins[b].bounds = aabbUnion(bins[b].bounds, primBounds_[firstPrim + i]);
        bins[b].count += 1;
    }

    // Sweep prefix/suffix to evaluate the (kSahBins - 1) candidate splits.
    std::array<Aabb, kSahBins - 1> leftBoxes{};
    std::array<std::int32_t, kSahBins - 1> leftCounts{};
    Aabb leftAccum;
    std::int32_t leftCount = 0;
    for (int i = 0; i < kSahBins - 1; ++i) {
        leftAccum = aabbUnion(leftAccum, bins[i].bounds);
        leftCount += bins[i].count;
        leftBoxes[i] = leftAccum;
        leftCounts[i] = leftCount;
    }

    const float parentSa = surfaceArea(nodeBounds);
    const float invParentSa = parentSa > 0.0f ? 1.0f / parentSa : 0.0f;
    const float leafCost = static_cast<float>(primCount) * kIntersectionCost;

    int bestSplit = -1;
    float bestCost = leafCost;

    Aabb rightAccum;
    std::int32_t rightCount = 0;
    for (int i = kSahBins - 1; i >= 1; --i) {
        rightAccum = aabbUnion(rightAccum, bins[i].bounds);
        rightCount += bins[i].count;
        const int splitIndex = i - 1;
        const std::int32_t lc = leftCounts[splitIndex];
        const std::int32_t rc = rightCount;
        if (lc == 0 || rc == 0) {
            continue;
        }
        const float cost = kTraversalCost +
                           kIntersectionCost * invParentSa *
                               (surfaceArea(leftBoxes[splitIndex]) * static_cast<float>(lc) +
                                surfaceArea(rightAccum) * static_cast<float>(rc));
        if (cost < bestCost) {
            bestCost = cost;
            bestSplit = splitIndex;
        }
    }

    if (bestSplit < 0) {
        makeLeaf();
        return nodeIndex;
    }

    // Partition primitives in place by the chosen bin boundary.
    const float boundary = axisMin + (static_cast<float>(bestSplit + 1) / static_cast<float>(kSahBins)) *
                                          (axisMax - axisMin);
    auto begin = primitives_.begin() + firstPrim;
    auto end = begin + primCount;
    auto bbegin = primBounds_.begin() + firstPrim;
    auto cbegin = primCentroids_.begin() + firstPrim;

    std::int32_t mid = 0;
    for (std::int32_t i = 0; i < primCount; ++i) {
        if (axisComponent(*(cbegin + i), axis) < boundary) {
            std::swap(*(begin + mid), *(begin + i));
            std::swap(*(bbegin + mid), *(bbegin + i));
            std::swap(*(cbegin + mid), *(cbegin + i));
            ++mid;
        }
    }
    (void)end;

    // Degenerate partition (every centroid landed on the same side after binning rounding).
    if (mid == 0 || mid == primCount) {
        makeLeaf();
        return nodeIndex;
    }

    const std::int32_t leftIndex = buildRecursive(firstPrim, mid, depth + 1);
    const std::int32_t rightIndex = buildRecursive(firstPrim + mid, primCount - mid, depth + 1);

    nodes_[nodeIndex].leftChild = leftIndex;
    nodes_[nodeIndex].rightChild = rightIndex;
    nodes_[nodeIndex].splitAxis = axis;
    nodes_[nodeIndex].primCount = 0;
    return nodeIndex;
}

std::optional<Hit> Bvh::intersect(const Ray& ray) const {
    if (nodes_.empty()) {
        return std::nullopt;
    }

    const Vec3 invDir{1.0f / ray.direction.x, 1.0f / ray.direction.y, 1.0f / ray.direction.z};
    const std::array<int, 3> dirIsNeg{ray.direction.x < 0.0f, ray.direction.y < 0.0f,
                                      ray.direction.z < 0.0f};

    std::optional<Hit> closest;
    Ray clipped = ray;

    std::array<std::int32_t, kMaxDepth + 1> stack{};
    int stackTop = 0;
    stack[stackTop++] = 0;

    while (stackTop > 0) {
        const std::int32_t nodeIndex = stack[--stackTop];
        const Node& node = nodes_[nodeIndex];
        if (!intersectAabb(node.bounds, clipped, invDir).has_value()) {
            continue;
        }

        if (node.primCount > 0) {
            for (std::int32_t i = 0; i < node.primCount; ++i) {
                const Primitive* prim = primitives_[node.firstPrim + i];
                auto hit = prim->intersect(clipped);
                if (hit.has_value()) {
                    clipped.tMax = hit->t;
                    closest = hit;
                }
            }
            continue;
        }

        // Visit nearer child first by pushing the farther child onto the stack first.
        const bool flip = dirIsNeg[static_cast<std::size_t>(node.splitAxis)] != 0;
        const std::int32_t first = flip ? node.rightChild : node.leftChild;
        const std::int32_t second = flip ? node.leftChild : node.rightChild;
        if (stackTop + 2 <= static_cast<int>(stack.size())) {
            stack[stackTop++] = second;
            stack[stackTop++] = first;
        }
    }

    return closest;
}

Aabb Bvh::bounds() const {
    if (nodes_.empty()) {
        return Aabb{};
    }
    return nodes_.front().bounds;
}

}  // namespace nanopt
