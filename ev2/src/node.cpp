#include <node.h>

#include <algorithm>

namespace ev2 {

void Node::add_child(Ref<Node> node) {
    if (node.get() == this)
        return;
    if (node->parent)
        node->parent->remove_child(node);
    children.push_back(node);
    node->parent = this;
}

void Node::remove_child(Ref<Node> node) {
    auto itr = std::find(children.begin(), children.end(), node);
    if (itr != children.end()) {
        (*itr)->parent = nullptr;
        children.erase(itr);
    }
}

void Node::_update(float dt) {
    on_process(dt);

    for (auto& c : children) {
        c->_update(dt);
    }
}

void Node::_update_pre_render() {
    pre_render();

    for (auto& c : children) {
        c->_update_pre_render();
    }
}

} // namespace ev2