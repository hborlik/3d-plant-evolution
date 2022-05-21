/**
 * @file game.h
 * @brief 
 * @date 2022-05-21
 * 
 */
#ifndef PLANT_GAME_H
#define PLANT_GAME_H

#include <scene.h>

using namespace ev2;

class GameState {
public:

    Ref<Scene> scene;

    void spawn_tree(const glm::vec3& position);
    void spawn_box(const glm::vec3& position);
    void spawn_player(const glm::vec3& position);
};

#endif // PLANT_GAME_H