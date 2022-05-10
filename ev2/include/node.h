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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <reference_counted.h>

namespace ev2 {

struct Transform {
    glm::vec3 position;
    glm::quat rotation = glm::identity<glm::quat>();
    glm::vec3 scale{1, 1, 1};

    /**
     * @brief apply euler roataions
     * 
     * @param xyz 
     */
    void rotate(const glm::vec3& xyz) {
        rotation = glm::rotate(glm::rotate(glm::rotate(rotation, xyz.x, {1, 0, 0}), xyz.y, {0, 1, 0}), xyz.z, {0, 0, 1});
    }

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

    void add_child(Ref<Node> node);
    void remove_child(Ref<Node> node);

    /**
     * @brief Get the child at index
     * 
     * @param index 
     * @return Ref<Node> 
     */
    Ref<Node> get_child(int index);

    class Scene& get_scene() {
        return *scene;
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

    glm::mat4 get_transform() const {
        glm::mat4 tr = transform.get_transform();
        if (parent)
            tr = parent->get_transform() * tr;
        return tr;
    }

    glm::vec3 get_world_position() const {return glm::vec3(get_transform()[3]);}

    Transform transform{};

private:
    friend class Scene;

    void update(float dt);
    
    const std::string name = "Node";
    std::list<Ref<Node>> children;
    Node* parent = nullptr;
    class Scene* scene = nullptr;
};

}

#endif // EV2_NODE_H