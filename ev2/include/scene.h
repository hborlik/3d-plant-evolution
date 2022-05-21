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
    explicit Scene(const std::string& name) : Node{name} {}

    void add_node(Ref<Node> n, Ref<Node> parent);

    template<typename T>
    Ref<T> create_node(const std::string& name);

    void destroy_node(Ref<Node> node);

    void set_active_camera(Ref<CameraNode> camera) {active_camera = camera;}
    Ref<CameraNode> get_active_camera() {return active_camera;}

    void update(float dt);
    void update_pre_render();

    void ready();

private:

    Ref<CameraNode> active_camera;
};

template<typename T>
Ref<T> Scene::create_node(const std::string& name) {
    Ref<T> node = make_referenced<T>(name);
    node->scene = this;
    Ref<Node> nnode = node;
    add_child(nnode);
    node->on_init();
    return node;
}

}

#endif // EV2_SCENE_H