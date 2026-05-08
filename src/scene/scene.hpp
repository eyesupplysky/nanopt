//! Scene aggregate — owns materials, primitives, lights, and the camera

#pragma once

#include <memory>
#include <vector>

#include "accel/primitive_list.hpp"
#include "camera/camera.hpp"
#include "geometry/primitive.hpp"
#include "light/light.hpp"
#include "material/bsdf.hpp"

namespace nanopt {

class Scene {
public:
    /// Take ownership of a BSDF and return a non-owning pointer for primitives to reference
    const Bsdf* addBsdf(std::unique_ptr<Bsdf> bsdf);

    /// Take ownership of a primitive and add it to the world intersector
    void addPrimitive(std::unique_ptr<Primitive> primitive);

    /// Take ownership of a light
    void addLight(std::unique_ptr<Light> light);

    /// Set the camera (replaces any previous camera)
    void setCamera(std::unique_ptr<Camera> camera);

    const Primitive& world() const { return world_; }
    const std::vector<std::unique_ptr<Light>>& lights() const { return lights_; }
    const Camera& camera() const { return *camera_; }

private:
    std::vector<std::unique_ptr<Bsdf>> bsdfs_;
    std::vector<std::unique_ptr<Primitive>> primitives_;
    std::vector<std::unique_ptr<Light>> lights_;
    std::unique_ptr<Camera> camera_;
    PrimitiveList world_;
};

}  // namespace nanopt
