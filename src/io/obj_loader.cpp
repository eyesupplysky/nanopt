//! OBJ loader implementation

#include "io/obj_loader.hpp"

#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace nanopt {

namespace {

/// Parse a single "v/vt/vn", "v/vt", "v//vn", or "v" face vertex into the
/// 0-based position index. Returns false on parse failure.
bool parseFaceVertex(const std::string& token, std::int64_t& positionIndex) {
    const std::size_t slash = token.find('/');
    const std::string head = token.substr(0, slash);
    if (head.empty()) {
        return false;
    }
    try {
        positionIndex = std::stoll(head);
    } catch (...) {
        return false;
    }
    return true;
}

bool resolveIndex(std::int64_t obj1Based, std::size_t positionCount, std::uint32_t& out) {
    if (obj1Based > 0) {
        const std::int64_t zero = obj1Based - 1;
        if (zero < 0 || static_cast<std::size_t>(zero) >= positionCount) {
            return false;
        }
        out = static_cast<std::uint32_t>(zero);
        return true;
    }
    if (obj1Based < 0) {
        const std::int64_t zero =
            static_cast<std::int64_t>(positionCount) + obj1Based;  // -1 -> last
        if (zero < 0 || static_cast<std::size_t>(zero) >= positionCount) {
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

        if (directive == "usemtl") {
            std::string name;
            tokens >> name;
            currentMaterial = std::move(name);
            continue;
        }

        if (directive == "f") {
            std::vector<std::uint32_t> faceIndices;
            std::string token;
            while (tokens >> token) {
                std::int64_t obj1Based = 0;
                if (!parseFaceVertex(token, obj1Based)) {
                    return std::nullopt;
                }
                std::uint32_t resolved = 0;
                if (!resolveIndex(obj1Based, mesh.positions.size(), resolved)) {
                    return std::nullopt;
                }
                faceIndices.push_back(resolved);
            }
            if (faceIndices.size() < 3) {
                return std::nullopt;
            }
            // Fan triangulation around the first vertex; handles quads and n-gons.
            for (std::size_t i = 1; i + 1 < faceIndices.size(); ++i) {
                ObjFace face;
                face.indices = {faceIndices[0], faceIndices[i], faceIndices[i + 1]};
                face.material = currentMaterial;
                mesh.faces.push_back(std::move(face));
            }
            continue;
        }

        // vt, vn, mtllib, o, g, s, l, p — accepted but ignored.
    }

    if (!stream.eof() && stream.fail()) {
        return std::nullopt;
    }
    return mesh;
}

}  // namespace nanopt
