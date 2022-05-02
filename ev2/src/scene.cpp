
#include <scene.h>

#include <algorithm>

namespace ev2 {

void Node::add_child(Ref<Node> node) {
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

} // namespace ev2