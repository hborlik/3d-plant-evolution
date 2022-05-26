/**
 * @file game.h
 * @brief 
 * @date 2022-05-21
 * 
 */
#ifndef PLANT_GAME_H
#define PLANT_GAME_H

#include <scene.h>
#include <resource.h>
#include <physics.h>
#include <player.h>
#include <tree.h>

using namespace ev2;

class GameState {
public:
    GameState();
    Ref<Scene> scene;
    ev2::Ref<ev2::MaterialResource> tree_bark;
    ev2::Ref<ev2::MaterialResource> highlight_material;

    ev2::Ref<ev2::CameraNode> cam_first_person{};
    ev2::Ref<ev2::VisualInstance> marker{};
    ev2::Ref<Player> player;

    Ref<DirectionalLightNode> sun_light;

    Ref<RigidBody> ground_plane;

    ev2::Ref<TreeNode> selected_tree_1;
    ev2::Ref<TreeNode> selected_tree_2;

    bool startedA = false;
    bool startedB = false;

    float time_accumulator = 0.0f;
    float time_day = 0.0f;
    float time_speed = 1.0f;
    const float DayLength = 60.0f;

    void update(float dt);

    void spawn_tree(const glm::vec3& position, float rotation, const std::map<std::string, float>& params, int iterations, glm::vec3 color_0, glm::vec3 color_1, float starting_growth);
    void spawn_random_tree(const glm::vec3& position, float range_extent, int iterations);
    void spawn_box(const glm::vec3& position);
    void spawn_player(const glm::vec3& position);
    void spawn_cross(const glm::vec3& position, float rotation, int iterations);
};  

#endif // PLANT_GAME_H