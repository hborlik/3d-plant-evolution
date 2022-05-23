/**
 * @file node.h
 * @brief 
 * @date 2022-05-04
 * 
 */
#ifndef EV2_NODE_H
#define EV2_NODE_H

#include <string>
#include <vector>
#include <list>
#include <memory>

#include <transform.h>
#include <ev.h>
#include <reference_counted.h>

namespace ev2 {

class Node : public Object {
protected:
    Node() = default;
    explicit Node(const std::string& name) : name{name} {}
public:
    virtual ~Node() = default;

    template<typename T, typename... Args>
    static Ref<T> create_node(Args&&... args) {
        Ref<T> node{new T(std::forward<Args&&>(args)...)};
        node->on_init();
        return node;
    }

    /**
     * @brief node initialization
     * 
     */
    virtual void on_init() {}

    /**
     * @brief node has been added to scene
     * 
     */
    virtual void on_ready() {}

    /**
     * @brief per frame update function
     * 
     * @param delta 
     */
    virtual void on_process(float delta) {}

    /**
     * @brief called before node is removed from the scene
     * 
     */
    virtual void on_destroy() {}

    virtual void on_child_added(Ref<Node> child, int index) {}

    virtual void on_child_removed(Ref<Node> child) {}

    /**
     * @brief call just before scene is rendered. Used to push changes to rendering server
     * 
     */
    virtual void pre_render() {};

    void add_child(Ref<Node> node);
    void remove_child(Ref<Node> node);

    /**
     * @brief Get the child at index
     * 
     * @param index 
     * @return Ref<Node> 
     */
    Ref<Node> get_child(int index);
    size_t get_n_children() const noexcept {return children.size();}

    Ref<Node> get_parent() const {
        if (parent)
            return parent->get_ref().ref_cast<Node>();
        return{};
    }

    class Scene* get_scene() {
        return scene;
    }

    /**
     * @brief Trigger destruction events and remove node from scene
     * 
     */
    void destroy();

    std::string get_path() const {
        std::string path = "/" + name;
        if (parent)
            path = parent->get_path() + path;
        return path;
    }

    virtual glm::mat4 get_transform() const {
        glm::mat4 tr = transform.get_transform();
        if (parent)
            tr = parent->get_transform() * tr;
        return tr;
    }

    glm::vec3 get_world_position() const {return glm::vec3(get_transform()[3]);}

    const std::string name = "Node";
    Transform transform{};

private:
    friend class Scene;

    void _update(float dt);
    void _ready();
    void _update_pre_render();
    
    std::list<Ref<Node>> children;
    Node* parent = nullptr;
    class Scene* scene = nullptr;
};

}

#endif // EV2_NODE_H