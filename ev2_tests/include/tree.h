/**
 * @file tree.h
 * @brief 
 * @date 2022-05-04
 * 
 */
#ifndef PLANT_GAME_TREE_H
#define PLANT_GAME_TREE_H

#include <visual_nodes.h>
#include <renderer/vertex_buffer.h>

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

class Fruit : public ev2::InstancedGeometry {
public:
    Fruit(const std::string& name, const SuperShapeParams& params, float growth_dt);
    Fruit(const std::string& name);

    void on_init() override;
    void on_process(float dt) override;

    void generate(float growth);

    void add_fruit(const glm::vec3& pos) {
        ev2::Transform ft{};
        ft.position = pos;
        fruit_transforms.push_back(ft);
    }

    std::vector<ev2::Transform> fruit_transforms;

    const float radius_mul = 0.25f;
    float growth = 0.01f;
    float growth_dt = 0.01f;
    SuperSphere supershape{};
    SuperShapeParams params{};
};

class TreeNode : public ev2::VisualInstance {
public:
    explicit TreeNode(class GameState* game, const std::string& name, bool has_leafs = false, int u_id = -1,
                     ev2::Ref<ev2::MaterialResource> fruit_material = nullptr,
                     ev2::Ref<ev2::MaterialResource> leaf_material = nullptr);

    void on_init() override;
    void on_destroy() override;

    void generate(int iterations);
    void setParams(const std::map<std::string, float>& paramsNew, int iterations, float growth);
    void spawn_fruit(const glm::vec3& position);
    std::map<std::string, float> getParams() {return params;}
    ev2::Ref<ev2::MaterialResource> fruit_material;
    ev2::Ref<ev2::MaterialResource> leaf_material;
    
    bool breedable = true;
    float growth_current = 0;
    float growth_rate = 0.05f;
    float growth_max = 1;
    float fruit_growth_max = 2;
    float fruit_growth_rate = 0.05f;
    float fruit_spawn_rate = 0.01f;
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
    ev2::Ref<Fruit> fruits;
    
    class GameState* const game;
};


struct Particle
{
    Particle()
        : m_Position(0)
        , m_Velocity(0)
        , m_Color(0)
        , m_fRotate(0)
        , m_fAge(0)
        , m_fLifeTime(0)
    {}
 
    glm::vec3   m_Position; // Center point of particle
    glm::vec3   m_Velocity; // Current particle velocity
    glm::vec4   m_Color;    // Particle color
    float       m_fRotate;  // Rotate the particle the center
    float       m_fSize;    // Size of the particle
    float       m_fAge;
    float       m_fLifeTime;

    glm::mat4 particle_transform() const {
        glm::mat4 tr = glm::translate(glm::identity<glm::mat4>(), m_Position) * glm::scale(glm::identity<glm::mat4>(), glm::vec3{m_fSize});
        return tr;
    }
};

class FireFlies : public ev2::Node {
public:
    FireFlies(class GameState* game, const std::string& name, int32_t n_flies);

    void on_init() override;
    void on_process(float dt) override;

    std::vector<Particle> particles;
    const int32_t NFlies;

private:
    ev2::Ref<ev2::InstancedGeometry> flies;
};

#endif // PLANT_GAME_TREE_H