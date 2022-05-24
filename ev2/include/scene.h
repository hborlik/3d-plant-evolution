/**
 * @file scene.h
 * @brief 
 * @date 2022-04-21
 * 
 */
#ifndef EV2_SCENE_H
#define EV2_SCENE_H

#include <string>
#include <vector>
#include <list>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <reference_counted.h>
#include <node.h>
#include <visual_nodes.h>

namespace ev2 {

class Scene : public Node {
public:
    explicit Scene(const std::string& name) : Node{name} {
        scene = this; // we are the scene
    }

    void add_node(Ref<Node> n, Ref<Node> parent = nullptr);

    template<typename T, typename... Args>
    Ref<T> create_node(Args&&... args);

    void update(float dt);
    void update_pre_render();

    void ready();

private:
    friend class Node;

    /**
     * @brief used by nodes under this scene heirarchy to notify that a child was added
     * 
     * @param child 
     */
    void _notify_child_added(Ref<Node> child);

    /**
     * @brief used by nodes under this scene heirarchy to notify that a child was removed
     * 
     * @param child 
     */
    void _notify_child_removed(Ref<Node> child);

    bool is_ready = false;
};

template<typename T, typename... Args>
Ref<T> Scene::create_node(Args&&... args) {
    Ref<T> node = Node::create_node<T>(std::forward<Args>(args)...);
    node->scene = this;
    add_child(node);
    return node;
}

}

#endif // EV2_SCENE_H