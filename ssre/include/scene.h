/**
 * @file scene.h
 * @author Hunter Borlik 
 * @brief 
 * @version 0.1
 * @date 2019-11-03
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#pragma once
#ifndef SSRE_SCENE_H
#define SSRE_SCENE_H

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>

namespace ssre {

class Node {
public:
    explicit Node(std::string name) noexcept;
    virtual ~Node();

    // copy
    Node(const Node& other);

    // move
    Node(Node&& other) noexcept;

    Node& operator=(const Node&) = delete;

    /**
     * @brief Set the position of this node in parent local space
     * 
     * @param position 
     */
    void setPosition(const glm::vec3& position) noexcept {this->position = position;}
    glm::vec3 getPosition() const noexcept {return position;}

    void setScale(const glm::vec3& s) noexcept {scale = s;}

    /**
     * @brief Set the offset of the local system, applied before rotation
     * 
     * @param origin 
     */
    void setOrigin(const glm::vec3& origin) noexcept {offset = -origin;}

    /**
     * @brief Rotate Node about local axis
     * 
     * @param axis 
     * @param w 
     */
    void rotate(const glm::vec3& axis, float w);

    /**
     * @brief update step
     * 
     * @param delta 
     */
    virtual void update(float delta);

    const glm::mat4& getModelMatrix() const noexcept {return model_matrix;}

protected:

    const std::string name;

    glm::vec3 position{};
    glm::vec3 offset{};
    glm::vec3 scale{1.f};

    glm::mat4 rotation{1.f};

    std::vector<std::shared_ptr<Node>> children;

private:
    friend class Scene;
    glm::mat4 model_matrix{1.f};

    // parent pointer managed by Scene
    std::weak_ptr<Node> parent;

    /**
     * @brief Add child to list
     * @param parent 
     */
    void addChild(const std::shared_ptr<Node>& node) noexcept;

    /**
     * @brief pre render operation, update children with current transform information
     */
    void pre_render(const glm::mat4& parentModelMatrix);
};

class Mesh;
class Camera;
class Program;

class PointLight : public Node {
public:
    explicit PointLight(std::string name) noexcept : Node{std::move(name)} {}

    PointLight& operator=(const PointLight&) = delete;

    void setRadiantIntensity(const glm::vec3& r) noexcept {radiantIntensity = r;}
    glm::vec3 getRadiantIntensity() const noexcept {return radiantIntensity;}

    void setActive(bool on) noexcept {active = on;}
    bool isAcitve() const noexcept {return active;}

    glm::vec3 pos{};
    float w = 0;
    void update(float delta) override {
        if(pos == glm::vec3{0, 0, 0})
            pos = position;
        w += delta;
        setPosition(pos + glm::vec3{1.f, 0, 0} * (float)sin(w));
    }

protected:
    glm::vec3 radiantIntensity{1.f}; // watts per seradian
    bool active = true;
};

class SkySphere;

class Scene {
public:
    Scene(std::shared_ptr<Node> root);
    ~Scene() = default;

    Scene& operator=(const Scene&) = delete;
    Scene& operator=(Scene&&) = delete;

    /**
     * @brief perform pre render operations on Nodes
     */
    void pre_render();

    void update(float delta);

    void setRootNode(const std::shared_ptr<Node>& root) {rootNode = root; addNodeToList(root);}
    const std::shared_ptr<Node>& getRoot() const noexcept {return rootNode;}

    void setCamera(const std::shared_ptr<Camera>& cam);
    std::shared_ptr<Camera> getCamera() const noexcept {return camera.lock();}

    void setSkysphere(const std::shared_ptr<SkySphere>& skysphere) noexcept {sky = skysphere;}
    std::shared_ptr<SkySphere> getSkySphere() const noexcept {return sky;}

    /**
     * @brief Add a new node
     * 
     * @param node 
     * @return true 
     * @return false node with name already exists
     */
    bool addNode(const std::shared_ptr<Node>& node, const std::shared_ptr<Node>& parent = nullptr);

    const std::unordered_map<std::string, std::shared_ptr<Node>>& getNodes() const {return nodes;}

protected:
    std::unordered_map<std::string, std::shared_ptr<Node>> nodes;
    std::shared_ptr<Node> rootNode;
    std::weak_ptr<Camera> camera;

    std::shared_ptr<SkySphere> sky;

    bool addNodeToList(const std::shared_ptr<Node>& node);
};

}

#endif // SSRE_SCENE_H