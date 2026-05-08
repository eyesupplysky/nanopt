//! M2 entry point — loads cornell-box.obj and renders direct-lit Cornell room to out.ppm

#include <cstdio>
#include <memory>
#include <numbers>
#include <string>
#include <unordered_map>

#include "camera/pinhole_camera.hpp"
#include "geometry/triangle.hpp"
#include "image/framebuffer.hpp"
#include "image/ppm.hpp"
#include "integrator/direct_integrator.hpp"
#include "io/obj_loader.hpp"
#include "light/point_light.hpp"
#include "material/lambertian.hpp"
#include "math/point3.hpp"
#include "math/ray.hpp"
#include "math/vec3.hpp"
#include "sampler/uniform_sampler.hpp"
#include "scene/scene.hpp"
#include "spectrum/spectrum.hpp"

using namespace nanopt;

namespace {

constexpr const char* kCornellPath = "assets/cornell-box.obj";

bool addObjToScene(Scene& scene, const ObjMesh& mesh,
                   const std::unordered_map<std::string, const Bsdf*>& materials,
                   const Bsdf* fallback) {
    for (const ObjFace& face : mesh.faces) {
        const Bsdf* bsdf = fallback;
        if (auto it = materials.find(face.material); it != materials.end()) {
            bsdf = it->second;
        }
        if (bsdf == nullptr) {
            std::fprintf(stderr, "No BSDF available for material '%s'\n", face.material.c_str());
            return false;
        }
        const Point3 v0 = mesh.positions[face.indices[0]];
        const Point3 v1 = mesh.positions[face.indices[1]];
        const Point3 v2 = mesh.positions[face.indices[2]];
        scene.addPrimitive(std::make_unique<Triangle>(v0, v1, v2, bsdf));
    }
    return true;
}

}  // namespace

int main() {
    constexpr int kWidth = 512;
    constexpr int kHeight = 512;

    auto mesh = loadObj(kCornellPath);
    if (!mesh.has_value()) {
        std::fprintf(stderr, "Failed to load %s\n", kCornellPath);
        return 1;
    }

    Scene scene;
    const Bsdf* white =
        scene.addBsdf(std::make_unique<LambertianBsdf>(Spectrum{0.73f, 0.73f, 0.73f}));
    const Bsdf* red =
        scene.addBsdf(std::make_unique<LambertianBsdf>(Spectrum{0.65f, 0.05f, 0.05f}));
    const Bsdf* green =
        scene.addBsdf(std::make_unique<LambertianBsdf>(Spectrum{0.12f, 0.45f, 0.15f}));

    const std::unordered_map<std::string, const Bsdf*> materials{
        {"white", white},
        {"red", red},
        {"green", green},
    };
    if (!addObjToScene(scene, *mesh, materials, white)) {
        return 1;
    }

    // Stand-in for the ceiling area light at M2 — point light just below the ceiling.
    scene.addLight(
        std::make_unique<PointLight>(Point3{0.0f, 0.95f, 0.0f}, Spectrum{1.5f}));

    scene.setCamera(std::make_unique<PinholeCamera>(
        Point3{0.0f, 0.5f, 1.6f},
        Point3{0.0f, 0.5f, 0.0f},
        Vec3{0.0f, 1.0f, 0.0f},
        std::numbers::pi_v<float> / 4.0f,
        static_cast<float>(kWidth) / static_cast<float>(kHeight)));

    scene.build();

    DirectIntegrator integrator{Spectrum{0.0f}};
    UniformSampler sampler{42};
    Framebuffer framebuffer{kWidth, kHeight};

    for (int y = 0; y < kHeight; ++y) {
        for (int x = 0; x < kWidth; ++x) {
            const float u = (static_cast<float>(x) + 0.5f) / static_cast<float>(kWidth);
            const float v =
                1.0f - (static_cast<float>(y) + 0.5f) / static_cast<float>(kHeight);
            const Ray ray = scene.camera().generateRay(u, v);
            const Spectrum color = integrator.Li(ray, scene, sampler);
            framebuffer.setPixel(x, y, color);
        }
        if ((y % 64) == 0) {
            std::printf("Row %d/%d\n", y, kHeight);
        }
    }

    if (!writePpm(framebuffer, "out.ppm")) {
        std::fprintf(stderr, "Failed to write out.ppm\n");
        return 1;
    }
    std::printf("Wrote out.ppm\n");
    return 0;
}
