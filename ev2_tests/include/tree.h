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

#include <procedural_tree.hpp>

#include <random_generators.h>
#include <Sphere.h>
struct PlantInfo {
    bool selected = false;
    bool parent = false;
    int ID;
    int iterations = 10;

    glm::vec3 position;
    glm::vec3 color;
    glm::quat rot;
    SuperSphere geometry;
    PlantInfo()
    {
        ID = -1;
    }
    PlantInfo(int IDin, glm::vec3 pos, glm::vec3 col, SuperSphere geo) 
    {
        ID = IDin;
        position = pos;
        color = col; 
        geometry = geo; 
        rot = glm::quatLookAt(glm::vec3(randomFloatTo(2) - 1, 0, randomFloatTo(2) - 1), glm::vec3{0, 1, 0});
    }
};

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
    void setParams(std::map<std::string, float> params, int iterations, float growth);
    std::map<std::string, float> getParams() {return params;}
    bool breedable = true;
    float growth_current = 0;
    float growth_rate = 0.1;
    float growth_max = 1;
    

    float thickness = 1.0f;
    glm::vec3 c0, c1;
    ptree::Skeleton tree_skeleton;
    ptree::Tree tree;
    std::map<std::string, float> params;
    ev2::VertexLayout buffer_layout;
    ev2::renderer::MID tree_geometry;
    std::shared_ptr<ev2::renderer::Drawable> model;

    PlantInfo plantInfo{};
};

ev2::Ref<TreeNode> spawnTree(const ev2::Scene& scene, const glm::vec3& position);

#endif // PLANT_GAME_TREE_H