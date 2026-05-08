//! OBJ loader implementation

#include "io/obj_loader.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace nanopt {

namespace {

/// Parse a single "v/vt/vn", "v/vt", "v//vn", or "v" face vertex.
/// Returns the position index in positionIndex (always); writes the normal index
/// to normalIndex when present (otherwise leaves it 0). Returns false on parse failure.
bool parseFaceVertex(const std::string& token, std::int64_t& positionIndex,
                     std::int64_t& normalIndex) {
    normalIndex = 0;
    const std::size_t slash1 = token.find('/');
    const std::string head = token.substr(0, slash1);
    if (head.empty()) {
        return false;
    }
    try {
        positionIndex = std::stoll(head);
    } catch (...) {
        return false;
    }
    if (slash1 == std::string::npos) {
        return true;  // bare "v"
    }
    const std::size_t slash2 = token.find('/', slash1 + 1);
    if (slash2 == std::string::npos) {
        return true;  // "v/vt" — no normal
    }
    const std::string ntail = token.substr(slash2 + 1);
    if (ntail.empty()) {
        return true;  // "v/vt/" — no normal
    }
    try {
        normalIndex = std::stoll(ntail);
    } catch (...) {
        return false;
    }
    return true;
}

bool resolveIndex(std::int64_t obj1Based, std::size_t count, std::uint32_t& out) {
    if (obj1Based > 0) {
        const std::int64_t zero = obj1Based - 1;
        if (zero < 0 || static_cast<std::size_t>(zero) >= count) {
            return false;
        }
        out = static_cast<std::uint32_t>(zero);
        return true;
    }
    if (obj1Based < 0) {
        const std::int64_t zero = static_cast<std::int64_t>(count) + obj1Based;  // -1 -> last
        if (zero < 0 || static_cast<std::size_t>(zero) >= count) {
            return false;
        }
        out = static_cast<std::uint32_t>(zero);
        return true;
    }
    return false;
}

}  // namespace

std::optional<ObjMesh> loadObj(std::string_view path) {
    std::ifstream stream{std::string{path}};
    if (!stream.is_open()) {
        return std::nullopt;
    }

    ObjMesh mesh;
    std::string currentMaterial;

    std::string line;
    while (std::getline(stream, line)) {
        // Strip trailing carriage return (Windows-authored files on Unix tools and vice versa).
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        std::istringstream tokens{line};
        std::string directive;
        if (!(tokens >> directive)) {
            continue;  // blank line
        }
        if (directive.empty() || directive[0] == '#') {
            continue;
        }

        if (directive == "v") {
            float x = 0.0f;
            float y = 0.0f;
            float z = 0.0f;
            if (!(tokens >> x >> y >> z)) {
                return std::nullopt;
            }
            mesh.positions.emplace_back(x, y, z);
            continue;
        }

        if (directive == "vn") {
            float x = 0.0f;
            float y = 0.0f;
            float z = 0.0f;
            if (!(tokens >> x >> y >> z)) {
                return std::nullopt;
            }
            mesh.normals.emplace_back(x, y, z);
            continue;
        }

        if (directive == "usemtl") {
            std::string name;
            tokens >> name;
            currentMaterial = std::move(name);
            continue;
        }

        if (directive == "f") {
            std::vector<std::uint32_t> faceVerts;
            std::vector<std::uint32_t> faceNormals;
            bool faceHasAllNormals = true;
            std::string token;
            while (tokens >> token) {
                std::int64_t obj1Based = 0;
                std::int64_t normal1Based = 0;
                if (!parseFaceVertex(token, obj1Based, normal1Based)) {
                    return std::nullopt;
                }
                std::uint32_t resolved = 0;
                if (!resolveIndex(obj1Based, mesh.positions.size(), resolved)) {
                    return std::nullopt;
                }
                faceVerts.push_back(resolved);

                if (normal1Based != 0) {
                    std::uint32_t resolvedN = 0;
                    if (!resolveIndex(normal1Based, mesh.normals.size(), resolvedN)) {
                        return std::nullopt;
                    }
                    faceNormals.push_back(resolvedN);
                } else {
                    faceHasAllNormals = false;
                }
            }
            if (faceVerts.size() < 3) {
                return std::nullopt;
            }
            const bool emitNormals = faceHasAllNormals && faceNormals.size() == faceVerts.size();
            // Fan triangulation around the first vertex; handles quads and n-gons.
            for (std::size_t i = 1; i + 1 < faceVerts.size(); ++i) {
                ObjFace face;
                face.indices = {faceVerts[0], faceVerts[i], faceVerts[i + 1]};
                if (emitNormals) {
                    face.normalIndices = {faceNormals[0], faceNormals[i], faceNormals[i + 1]};
                    face.hasNormals = true;
                }
                face.material = currentMaterial;
                mesh.faces.push_back(std::move(face));
            }
            continue;
        }

        // vt, mtllib, o, g, s, l, p — accepted but ignored.
    }

    if (!stream.eof() && stream.fail()) {
        return std::nullopt;
    }
    return mesh;
}

void computeVertexNormals(ObjMesh& mesh) {
    mesh.normals.assign(mesh.positions.size(), Vec3{0.0f, 0.0f, 0.0f});

    for (ObjFace& face : mesh.faces) {
        const std::uint32_t i0 = face.indices[0];
        const std::uint32_t i1 = face.indices[1];
        const std::uint32_t i2 = face.indices[2];
        const Point3 p0 = mesh.positions[i0];
        const Point3 p1 = mesh.positions[i1];
        const Point3 p2 = mesh.positions[i2];
        const Vec3 e0 = p1 - p0;
        const Vec3 e1 = p2 - p1;
        const Vec3 e2 = p0 - p2;
        const Vec3 cr = cross(e0, -e2);  // = cross(p1-p0, p2-p0)
        const float area2 = length(cr);
        if (area2 == 0.0f) {
            continue;  // degenerate triangle — contributes no normal
        }
        const Vec3 faceN = cr / area2;

        // Angle-weighted contribution at each vertex (better than area-weighted for thin triangles).
        auto angleAt = [](Vec3 a, Vec3 b) -> float {
            const float la = length(a);
            const float lb = length(b);
            if (la == 0.0f || lb == 0.0f) return 0.0f;
            const float c = std::clamp(dot(a, b) / (la * lb), -1.0f, 1.0f);
            return std::acos(c);
        };
        const float a0 = angleAt(e0, -e2);
        const float a1 = angleAt(e1, -e0);
        const float a2 = angleAt(e2, -e1);
        mesh.normals[i0] += faceN * a0;
        mesh.normals[i1] += faceN * a1;
        mesh.normals[i2] += faceN * a2;
    }

    for (Vec3& n : mesh.normals) {
        if (lengthSquared(n) > 0.0f) {
            n = normalize(n);
        }
    }

    for (ObjFace& face : mesh.faces) {
        face.normalIndices = face.indices;
        face.hasNormals = true;
    }
}

}  // namespace nanopt
