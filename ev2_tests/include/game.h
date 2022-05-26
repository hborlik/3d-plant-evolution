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

using namespace ev2;

float randomFloatTo(float limit);
float randomFloatRange(float low, float high);

class GameState {
public:
    GameState();
    
    glm::vec3 sunset_color{253/255.0f, 94/255.0, 83/255.0};
    glm::vec3 night_ambient{0.13, 0.16, 0.21};

    Ref<Scene> scene;
    ev2::Ref<ev2::MaterialResource> tree_bark;
    ev2::Ref<ev2::MaterialResource> highlight_material;

    ev2::Ref<ev2::CameraNode> cam_first_person{};
    ev2::Ref<ev2::VisualInstance> marker{};
    ev2::Ref<Player> player;

    Ref<DirectionalLightNode> sun_light;

    Ref<RigidBody> ground_plane;

    float time_day = 0.0f;
    float time_speed = 1.0f;
    const float DayLength = 60.0f;

    void update(float dt);

    void spawn_tree(const glm::vec3& position, float rotation, const std::map<std::string, float>& params, int iterations);
    void spawn_random_tree(const glm::vec3& position, float range_extent, int iterations);
    void spawn_box(const glm::vec3& position);
    void spawn_player(const glm::vec3& position);
};

#endif // PLANT_GAME_H