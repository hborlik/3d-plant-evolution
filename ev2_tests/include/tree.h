/**
 * @file tree.h
 * @brief 
 * @date 2022-05-04
 * 
 */
#ifndef PLANT_GAME_TREE_H
#define PLANT_GAME_TREE_H

#include <visual_nodes.h>

#include <procedural_tree.hpp>

class TreeNode : public ev2::VisualInstance {
public:
    explicit TreeNode(const std::string& name) : ev2::VisualInstance{name} {}

    void generate(int iterations);
};

#endif // PLANT_GAME_TREE_H