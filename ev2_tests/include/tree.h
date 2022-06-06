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
    explicit TreeNode(class GameState* game, const std::string& name, bool has_leafs = false);

    void on_init() override;
    void on_destroy() override;

    void generate(int iterations);
    void setParams(const std::map<std::string, float>& paramsNew, int iterations, float growth);
    void spawn_fruit(const glm::vec3& position, const SuperShapeParams& params);
    std::map<std::string, float> getParams() {return params;}
    
    bool breedable = true;
    float growth_current = 0;
    float growth_rate = 0.05f;
    float growth_max = 1;
    float fruit_growth_max = 2;
    float fruit_growth_rate = 0.05f;
    bool fruits_spawned = false;

    float thickness = 1.0f;
    float leaf_scale = 5.f;
    bool has_leafs = false;
    glm::vec3 c0, c1;
    ptree::Skeleton tree_skeleton;
    ptree::Tree tree;
    std::map<std::string, float> params;
    ev2::VertexLayout buffer_layout;
    ev2::renderer::Drawable* tree_geometry;

    PlantInfo plantInfo{};
    SuperShapeParams fruit_params{};
    
    ev2::Ref<ev2::InstancedGeometry> leafs;
    
    class GameState* const game;
};

class Fruit : public ev2::VisualInstance {
public:
    Fruit(const std::string& name, const SuperShapeParams& params);
    Fruit(const std::string& name);

    void on_init() override;

    void generate(float growth);

    const float radius_mul = 0.25f;
    SuperSphere supershape{};
    SuperShapeParams params{};
    ev2::renderer::Drawable* geometry{};
};

#endif // PLANT_GAME_TREE_H