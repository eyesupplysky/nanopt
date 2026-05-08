//! Scene aggregate implementation

#include "scene/scene.hpp"

#include <utility>

namespace nanopt {

const Bsdf* Scene::addBsdf(std::unique_ptr<Bsdf> bsdf) {
    const Bsdf* raw = bsdf.get();
    bsdfs_.push_back(std::move(bsdf));
    return raw;
}

void Scene::addPrimitive(std::unique_ptr<Primitive> primitive) {
    world_.add(primitive.get());
    primitives_.push_back(std::move(primitive));
}

void Scene::addLight(std::unique_ptr<Light> light) {
    lights_.push_back(std::move(light));
}

void Scene::setCamera(std::unique_ptr<Camera> camera) {
    camera_ = std::move(camera);
}

}  // namespace nanopt
