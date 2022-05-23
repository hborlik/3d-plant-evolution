
#include <scene.h>

#include <algorithm>

namespace ev2 {

void Scene::update(float dt) {
    _update(dt);
}

void Scene::update_pre_render() {
    _update_pre_render();
}

void Scene::ready() {
    _ready();
}

} // namespace ev2