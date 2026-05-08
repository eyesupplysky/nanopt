//! Minimal OBJ wavefront loader — positions, vertex normals, and faces

#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "math/point3.hpp"
#include "math/vec3.hpp"

namespace nanopt {

struct ObjFace {
    std::array<std::uint32_t, 3> indices{};
    std::array<std::uint32_t, 3> normalIndices{};
    bool hasNormals = false;
    std::string material;  // empty when no usemtl directive precedes the face
};

struct ObjMesh {
    std::vector<Point3> positions;
    std::vector<Vec3> normals;
    std::vector<ObjFace> faces;
};

/// Load a Wavefront OBJ file. Returns std::nullopt on read or parse failure.
/// Supports: v, vn, f directives and usemtl scoping. Texture coordinate indices in
/// "f v/vt/vn" entries are accepted and discarded. Quads are fan-triangulated.
/// Faces with normal indices on every vertex set ObjFace::hasNormals to true.
/// Other directives (vt, mtllib, o, g, s) are ignored.
std::optional<ObjMesh> loadObj(std::string_view path);

/// Compute angle-weighted per-vertex normals from face geometry; replaces any existing
/// mesh.normals and sets every face's hasNormals flag. Use this when the source OBJ
/// has no vn lines but smooth shading is required at render time.
//            normalIndices match its position indices
void computeVertexNormals(ObjMesh& mesh);

}  // namespace nanopt
