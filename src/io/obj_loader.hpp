//! Minimal OBJ wavefront loader — positions and faces only

#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "math/point3.hpp"

namespace nanopt {

struct ObjFace {
    std::array<std::uint32_t, 3> indices{};
    std::string material;  // empty when no usemtl directive precedes the face
};

struct ObjMesh {
    std::vector<Point3> positions;
    std::vector<ObjFace> faces;
};

/// Load a Wavefront OBJ file. Returns std::nullopt on read or parse failure.
/// Supports: v / f directives, usemtl scoping. Texture and normal indices in
/// "f v/vt/vn" entries are accepted and discarded. Quads are fan-triangulated.
/// Other directives (vt, vn, mtllib, o, g, s) are ignored.
std::optional<ObjMesh> loadObj(std::string_view path);

}  // namespace nanopt
