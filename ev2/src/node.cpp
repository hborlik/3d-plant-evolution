#include <node.h>
#include <scene.h>

#include <algorithm>

namespace ev2 {

void Node::add_child(Ref<Node> node) {
    if (node.get() == this)
        return;
    if (node->parent)
        node->parent->remove_child(node);
    int ind = children.size();
    children.push_back(node);
    node->parent = this;
    node->scene = scene;

    if (scene)
        scene->_notify_child_added(node);

    on_child_added(node, ind);
}

void Node::remove_child(Ref<Node> node) {
    auto itr = std::find(children.begin(), children.end(), node);
    if (itr != children.end()) {
        (*itr)->parent = nullptr;
        children.erase(itr);
    } else {
        throw engine_exception{"Node: " + name + " does not have child " + node->name};
    }

    if (scene)
        scene->_notify_child_removed(node);

    on_child_removed(node);
}

void Node::destroy() {
    // cleanup children first
    for (auto& c : children) {
        c->destroy();
    }

    on_destroy();
}

void Node::_update(float dt) {
    on_process(dt);

    for (auto& c : children) {
        c->_update(dt);
    }
}

void Node::_ready() {
    on_ready();

    for (auto& c : children) {
        c->_ready();
    }
}

void Node::_update_pre_render() {
    pre_render();

    for (auto& c : children) {
        c->_update_pre_render();
    }
}

} // namespace ev2