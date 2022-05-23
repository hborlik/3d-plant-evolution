
#include <scene.h>

#include <algorithm>

namespace ev2 {

void Scene::add_node(Ref<Node> n, Ref<Node> parent) {
    assert(n);
    Ref<Node> p = parent;
    if (!p)
        p = Ref<Node>(this);
    p->add_child(n);
}

void Scene::update(float dt) {
    _update(dt);
}

void Scene::update_pre_render() {
    _update_pre_render();
}

void Scene::ready() {
    if (!is_ready) {
        is_ready = true;
        _ready();
    }
}

void Scene::_notify_child_added(Ref<Node> child) {
    if (is_ready)
        child->_ready();
}

void Scene::_notify_child_removed(Ref<Node> child) {

}

} // namespace ev2