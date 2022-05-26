/**
 * @file tree.h
 * @brief 
 * @date 2022-05-04
 * 
 */
#ifndef PLANT_GAME_TREE_H
#define PLANT_GAME_TREE_H

#include <visual_nodes.h>
#include <vertex_buffer.h>

#include <Sphere.h>

#include <procedural_tree.hpp>

struct PNVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
};

class TreeNode : public ev2::VisualInstance {
public:
    explicit TreeNode(const std::string& name);

    void on_init() override;

    void generate(int iterations);
    void setParams(std::map<std::string, float> params, int iterations);
    std::map<std::string, float> getParams() {return params;}

    float thickness = 1.0f;
    glm::vec3 c0, c1;
    ptree::Skeleton tree_skeleton;
    ptree::Tree tree;
    std::map<std::string, float> params;
    ev2::VertexLayout buffer_layout;
    ev2::renderer::MID tree_geometry;
    std::shared_ptr<ev2::renderer::Drawable> model;
    SuperShapeParams fruit_params{};
};

class Fruit : public ev2::VisualInstance {
public:
    Fruit(const std::string& name, const SuperShapeParams& params);
    Fruit(const std::string& name);

    void on_init() override;

    void generate();

    const float radius_mul = 0.25f;
    SuperSphere supershape{};
    SuperShapeParams params{};
    std::shared_ptr<ev2::renderer::Drawable> model{};
    ev2::renderer::MID geometry{};
};

#endif // PLANT_GAME_TREE_H