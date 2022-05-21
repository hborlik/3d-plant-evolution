/**
 * @file game.h
 * @brief 
 * @date 2022-05-21
 * 
 */
#ifndef PLANT_GAME_H
#define PLANT_GAME_H

#include <scene.h>
#include <renderer.h>
#include <physics.h>

using namespace ev2;

float randomFloatTo(float limit);
float randomFloatRange(float low, float high);

class GameState {
public:
    GameState();

    Ref<Scene> scene;
    std::pair<std::shared_ptr<ev2::Material>, int32_t> tree_bark;
    std::pair<std::shared_ptr<ev2::Material>, int32_t> highlight_material;

    Ref<DirectionalLightNode> sun_light;

    Ref<RigidBody> ground_plane;

    void spawn_tree(const glm::vec3& position, float rotation, const std::map<std::string, float>& params, int iterations);
    void spawn_random_tree(const glm::vec3& position, float range_extent, int iterations);
    void spawn_box(const glm::vec3& position);
    void spawn_player(const glm::vec3& position);
};

#endif // PLANT_GAME_H