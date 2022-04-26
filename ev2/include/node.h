/**
 * @file node.h
 * @brief 
 * @date 2022-04-21
 * 
 */
#ifndef EV2_NODE_H
#define EV2_NODE_H

#include <string>
#include <vector>
#include <list>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <reference_counted.h>

namespace ev2 {

struct Transform {
    glm::vec3 position;
    glm::quat rotation = glm::identity<glm::quat>();
    glm::vec3 scale{1, 1, 1};

    glm::mat4 get_transform() const noexcept {
        glm::mat4 tr = glm::mat4_cast(rotation) * glm::scale(glm::identity<glm::mat4>(), scale);
        tr[3] = glm::vec4{position, 1.0f};
        return tr;
    }

    glm::mat4 get_linear_transform() const noexcept {
        glm::mat4 tr = glm::mat4_cast(rotation);
        tr[3] = glm::vec4{position, 1.0f};
        return tr;
    }
};

class Node : public ReferenceCounted<Node> {
public:
    Node() = default;
    explicit Node(const std::string& name) : name{name} {}
    virtual ~Node() = default;

    virtual void on_init() {}
    virtual void on_ready() {}
    virtual void on_process(float delta) {}
    virtual void on_destroy() {}

    void add_child(Ref<Node> node);
    void remove_child(Ref<Node> node);

    /**
     * @brief Get the child at index
     * 
     * @param index 
     * @return Ref<Node> 
     */
    Ref<Node> get_child(int index);

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

    glm::mat4 get_transform() const {
        glm::mat4 tr = transform.get_transform();
        if (parent)
            tr = parent->get_transform() * tr;
        return tr;
    }

    Transform transform{};
    
private:
    const std::string name = "Node";

    std::list<Ref<Node>> children;
    Node* parent = nullptr;
};

}

#endif // EV2_NODE_H