/**
 * @file tree.h
 * @brief 
 * @date 2022-05-04
 * 
 */
#ifndef PLANT_GAME_TREE_H
#define PLANT_GAME_TREE_H

#include <visual_nodes.h>
#include <mesh.h>

#include <procedural_tree.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

class TreeNode : public ev2::VisualInstance {
public:
    explicit TreeNode(const std::string& name);

    void on_init() override;

    void generate(int iterations);

    ptree::Skeleton tree_skeleton;
    ev2::VertexLayout buffer_layout;
    ev2::MID tree_geometry;
    std::shared_ptr<ev2::Drawable> model;
};

#endif // PLANT_GAME_TREE_H