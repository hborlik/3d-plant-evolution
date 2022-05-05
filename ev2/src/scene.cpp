
#include <scene.h>

#include <algorithm>

namespace ev2 {

Scene::Scene(std::shared_ptr<ResourceManager> RM) : resource_manager{std::move(RM)} {
    root = make_referenced<Node>("root");
}

void Scene::update(float dt) {
    root->update(dt);
}

} // namespace ev2