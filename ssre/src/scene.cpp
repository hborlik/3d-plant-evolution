/**
 * @file scene.cpp
 * @author Hunter Borlik 
 * @brief 
 * @version 0.1
 * @date 2019-11-13
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include <scene.h>
#include <renderer.h>
#include <ssre_gl.h>
#include <material.h>
#include <shader.h>
#include <mesh.h>
#include <camera.h>

#include <glm/gtc/matrix_transform.hpp>

using namespace ssre;

Node::Node(std::string name) noexcept : name{std::move(name)} {

}

Node::~Node() {

}

void Node::addChild(const std::shared_ptr<Node>& node) noexcept {
    if(node != nullptr) {
        children.push_back(node);
    }
}

void Node::rotate(const glm::vec3& axis, float w) {
    rotation = glm::rotate(rotation, w, axis);
}

void Node::update(float delta) {
    
}

void Node::pre_render(const glm::mat4& parentModelMatrix) {
    glm::mat4 transform = rotation;
    transform = parentModelMatrix * glm::translate(glm::mat4{1.f}, position) * rotation * glm::translate(glm::mat4{1.f}, offset);
    for(auto& child : children) {
        child->pre_render(transform);
    }
    model_matrix = transform * glm::scale(glm::mat4{1.f}, scale);
}

///////////////////////////////////////////////////////////////////////

Scene::Scene(std::shared_ptr<Node> root) : rootNode{std::move(root)} {
    SSRE_CHECK_THROW(rootNode, "Scene root must not be null");
    addNodeToList(rootNode);
}

void Scene::pre_render() {
    rootNode->pre_render(glm::mat4{1.f});
}

void Scene::update(float delta) {
    for(auto& n : nodes) {
        n.second->update(delta);
    }
}

void Scene::setCamera(const std::shared_ptr<Camera>& cam) {
    camera = cam;
    // only update the camera parent if it is invalid
    addNode(std::dynamic_pointer_cast<Node, Camera>(cam), (!cam->parent.expired() ? cam->parent.lock() : rootNode));
}

bool Scene::addNode(const std::shared_ptr<Node>& node, const std::shared_ptr<Node>& parent) {
    if(addNodeToList(node)) {
        if(parent != nullptr) {
            node->parent = parent;
            parent->addChild(node);
        } else {
            node->parent = rootNode;
            rootNode->addChild(node);
        }
        return true;
    }
    std::cout << "Failed to add node: " << (node ? node->name : "null") << std::endl;
    return false;
}

bool Scene::addNodeToList(const std::shared_ptr<Node>& node) {
    if(node != nullptr && nodes.find(node->name) == nodes.end()) {
        nodes.insert(std::make_pair(node->name, node));
        return true;
    }
    return false;
}