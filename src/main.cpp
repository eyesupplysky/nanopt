//! M5 entry point — renders the Cornell room with a glass dragon, gold sphere, and a Veach-style row of equal-flux ceiling lights at varying sizes (small/intense → large/dim) to exercise direct-lighting MIS

#include <algorithm>
#include <cstdio>
#include <limits>
#include <memory>
#include <numbers>
#include <string>
#include <unordered_map>

#include "camera/pinhole_camera.hpp"
#include "geometry/sphere.hpp"
#include "geometry/triangle.hpp"
#include "image/framebuffer.hpp"
#include "image/ppm.hpp"
#include "integrator/path_integrator.hpp"
#include "io/obj_loader.hpp"
#include "light/area_light.hpp"
#include "material/conductor.hpp"
#include "material/dielectric.hpp"
#include "material/lambertian.hpp"
#include "material/sellmeier.hpp"
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

constexpr const char* kCornellPath = "assets/cornell-empty.obj";
constexpr const char* kDragonPath = "assets/dragon.obj";

/// Scale + translate a mesh's positions in place so its y-min sits on the floor and its
/// horizontal centre rests at the requested xz coordinate. Returns the resulting bbox.
struct Bbox {
    Point3 min;
    Point3 max;
};

Bbox computeBbox(const ObjMesh& mesh) {
    constexpr float kInf = std::numeric_limits<float>::infinity();
    Bbox box{Point3{kInf, kInf, kInf}, Point3{-kInf, -kInf, -kInf}};
    for (const Point3& p : mesh.positions) {
        box.min.x = std::min(box.min.x, p.x);
        box.min.y = std::min(box.min.y, p.y);
        box.min.z = std::min(box.min.z, p.z);
        box.max.x = std::max(box.max.x, p.x);
        box.max.y = std::max(box.max.y, p.y);
        box.max.z = std::max(box.max.z, p.z);
    }
    return box;
}

void fitMeshOnFloor(ObjMesh& mesh, float targetHeight, float centreX, float centreZ) {
    const Bbox box = computeBbox(mesh);
    const float h = box.max.y - box.min.y;
    if (h <= 0.0f) {
        return;
    }
    const float scale = targetHeight / h;
    const float preCx = (box.min.x + box.max.x) * 0.5f;
    const float preCz = (box.min.z + box.max.z) * 0.5f;
    const float preMinY = box.min.y;
    for (Point3& p : mesh.positions) {
        p.x = (p.x - preCx) * scale + centreX;
        p.y = (p.y - preMinY) * scale;
        p.z = (p.z - preCz) * scale + centreZ;
    }
}

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
        if (face.hasNormals) {
            const Vec3 n0 = mesh.normals[face.normalIndices[0]];
            const Vec3 n1 = mesh.normals[face.normalIndices[1]];
            const Vec3 n2 = mesh.normals[face.normalIndices[2]];
            scene.addPrimitive(std::make_unique<Triangle>(v0, v1, v2, n0, n1, n2, bsdf));
        } else {
            scene.addPrimitive(std::make_unique<Triangle>(v0, v1, v2, bsdf));
        }
    }
    return true;
}

/// Build a downward-facing rectangular ceiling light from two triangles centred at (cx, cz).
/// Winding chosen so geometric normal points -y (into the room).
void addCeilingQuadLight(Scene& scene, const Bsdf* emitterBsdf, RgbSpectrum emissionRgb,
                         float cx, float cz, float halfX, float halfZ) {
    constexpr float kY = 0.999f;
    const Point3 a{cx - halfX, kY, cz - halfZ};
    const Point3 b{cx + halfX, kY, cz - halfZ};
    const Point3 c{cx + halfX, kY, cz + halfZ};
    const Point3 d{cx - halfX, kY, cz + halfZ};

    auto light0 = std::make_unique<AreaLight>(a, b, d, emissionRgb);
    auto light1 = std::make_unique<AreaLight>(b, c, d, emissionRgb);
    const AreaLight* l0 = light0.get();
    const AreaLight* l1 = light1.get();
    scene.addLight(std::move(light0));
    scene.addLight(std::move(light1));
    scene.addPrimitive(std::make_unique<Triangle>(a, b, d, emitterBsdf, l0));
    scene.addPrimitive(std::make_unique<Triangle>(b, c, d, emitterBsdf, l1));
}

/// Veach-style row of four equal-flux area lights at increasing size (decreasing intensity).
/// Sizes/intensities are tuned so the integrated flux ≈ the M4 single-light reference.
/// Demonstrates MIS variance reduction: small lights favour the light strategy; large lights
/// favour the BSDF strategy. With MIS both regimes converge cleanly.
void addVeachCeilingLights(Scene& scene, const Bsdf* emitterBsdf) {
    struct LightSpec {
        float cx;
        float halfX;
        float halfZ;
        float intensity;
    };
    // Equal-flux row: each light radiates ≈ the original Cornell ceiling light's per-light flux.
    // (intensity × full-area is constant ≈ 0.15 per channel.)
    constexpr LightSpec kLights[] = {
        {-0.36f, 0.020f, 0.020f, 93.75f},
        {-0.12f, 0.040f, 0.040f, 23.4375f},
        { 0.12f, 0.080f, 0.080f,  5.859375f},
        { 0.36f, 0.140f, 0.140f,  1.91326f},
    };
    for (const LightSpec& l : kLights) {
        addCeilingQuadLight(scene, emitterBsdf, RgbSpectrum{l.intensity, l.intensity, l.intensity},
                            l.cx, 0.0f, l.halfX, l.halfZ);
    }
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

    addVeachCeilingLights(scene, black);

    // Stanford XYZ RGB Dragon, glass (SF10) — centred on the floor
    auto dragonMesh = loadObj(kDragonPath);
    if (!dragonMesh.has_value()) {
        std::fprintf(stderr, "Failed to load %s\n", kDragonPath);
        return 1;
    }
    fitMeshOnFloor(*dragonMesh, 0.42f, 0.0f, 0.0f);
    computeVertexNormals(*dragonMesh);
    const Bsdf* glass =
        scene.addBsdf(std::make_unique<DielectricBsdf>(kSf10));
    if (!addObjToScene(scene, *dragonMesh, {}, glass)) {
        return 1;
    }

    // Gold sphere on the floor, off to the right of the dragon
    const Bsdf* gold =
        scene.addBsdf(std::make_unique<ConductorBsdf>(MetalKind::Gold, 0.1f));
    scene.addPrimitive(std::make_unique<Sphere>(Point3{0.35f, 0.1f, 0.25f}, 0.1f, gold));

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
