//! M2 entry point — loads cornell-box.obj and renders a path-traced Cornell room to out.ppm

#include <cstdio>
#include <memory>
#include <numbers>
#include <string>
#include <unordered_map>

#include "camera/pinhole_camera.hpp"
#include "geometry/triangle.hpp"
#include "image/framebuffer.hpp"
#include "image/ppm.hpp"
#include "integrator/path_integrator.hpp"
#include "io/obj_loader.hpp"
#include "light/area_light.hpp"
#include "material/lambertian.hpp"
#include "math/point3.hpp"
#include "math/ray.hpp"
#include "math/vec3.hpp"
#include "sampler/uniform_sampler.hpp"
#include "scene/scene.hpp"
#include "spectrum/cie.hpp"
#include "spectrum/rgb_spectrum.hpp"
#include "spectrum/sampled_spectrum.hpp"
#include "spectrum/spectrum.hpp"
#include "spectrum/wavelength.hpp"

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

/// Build a downward-facing rectangular ceiling light from two triangles.
/// Winding chosen so geometric normal points -y (into the room).
void addCeilingLight(Scene& scene, const Bsdf* emitterBsdf, RgbSpectrum emissionRgb) {
    constexpr float kHalf = 0.1f;
    constexpr float kY = 0.999f;
    const Point3 a{-kHalf, kY, -kHalf};
    const Point3 b{ kHalf, kY, -kHalf};
    const Point3 c{ kHalf, kY,  kHalf};
    const Point3 d{-kHalf, kY,  kHalf};

    auto light0 = std::make_unique<AreaLight>(a, b, d, emissionRgb);
    auto light1 = std::make_unique<AreaLight>(b, c, d, emissionRgb);
    const AreaLight* l0 = light0.get();
    const AreaLight* l1 = light1.get();
    scene.addLight(std::move(light0));
    scene.addLight(std::move(light1));
    scene.addPrimitive(std::make_unique<Triangle>(a, b, d, emitterBsdf, l0));
    scene.addPrimitive(std::make_unique<Triangle>(b, c, d, emitterBsdf, l1));
}

}  // namespace

int main() {
    constexpr int kWidth = 512;
    constexpr int kHeight = 512;
    constexpr int kSamplesPerPixel = 256;

    auto mesh = loadObj(kCornellPath);
    if (!mesh.has_value()) {
        std::fprintf(stderr, "Failed to load %s\n", kCornellPath);
        return 1;
    }

    Scene scene;
    const Bsdf* white =
        scene.addBsdf(std::make_unique<LambertianBsdf>(RgbSpectrum{0.73f, 0.73f, 0.73f}));
    const Bsdf* red =
        scene.addBsdf(std::make_unique<LambertianBsdf>(RgbSpectrum{0.65f, 0.05f, 0.05f}));
    const Bsdf* green =
        scene.addBsdf(std::make_unique<LambertianBsdf>(RgbSpectrum{0.12f, 0.45f, 0.15f}));
    // Black BSDF placeholder for emitter primitives — never sampled because the path terminates on emitter hits.
    const Bsdf* black =
        scene.addBsdf(std::make_unique<LambertianBsdf>(RgbSpectrum{0.0f, 0.0f, 0.0f}));

    const std::unordered_map<std::string, const Bsdf*> materials{
        {"white", white},
        {"red", red},
        {"green", green},
    };
    if (!addObjToScene(scene, *mesh, materials, white)) {
        return 1;
    }

    addCeilingLight(scene, black, RgbSpectrum{15.0f, 15.0f, 15.0f});

    scene.setCamera(std::make_unique<PinholeCamera>(
        Point3{0.0f, 0.5f, 1.6f},
        Point3{0.0f, 0.5f, 0.0f},
        Vec3{0.0f, 1.0f, 0.0f},
        std::numbers::pi_v<float> / 4.0f,
        static_cast<float>(kWidth) / static_cast<float>(kHeight)));

    scene.build();

    PathIntegrator integrator{};
    UniformSampler sampler{42};
    Framebuffer framebuffer{kWidth, kHeight};

    const float invSpp = 1.0f / static_cast<float>(kSamplesPerPixel);
    for (int y = 0; y < kHeight; ++y) {
        for (int x = 0; x < kWidth; ++x) {
            sampler.startPixel(x, y);
            Xyz sumXyz{};
            for (int s = 0; s < kSamplesPerPixel; ++s) {
                const Sample2D jitter = sampler.get2D();
                const float lambdaJitter = sampler.get1D();
                const SampledWavelengths lambdas =
                    SampledWavelengths::sampleHeroStratified(lambdaJitter);
                const float u = (static_cast<float>(x) + jitter.u) / static_cast<float>(kWidth);
                const float v =
                    1.0f - (static_cast<float>(y) + jitter.v) / static_cast<float>(kHeight);
                const Ray ray = scene.camera().generateRay(u, v);
                const Spectrum L = integrator.Li(ray, scene, sampler, lambdas);
                const Xyz xyz = spectrumToXyz(L, lambdas);
                sumXyz.x += xyz.x;
                sumXyz.y += xyz.y;
                sumXyz.z += xyz.z;
            }
            framebuffer.setPixel(x, y, Xyz{sumXyz.x * invSpp, sumXyz.y * invSpp, sumXyz.z * invSpp});
        }
        if ((y % 32) == 0) {
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
