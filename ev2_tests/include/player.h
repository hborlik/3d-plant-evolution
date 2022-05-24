/**
 * @file player.h
 * @brief 
 * @date 2022-05-23
 * 
 * 
 */
#ifndef GAME_PLAYER_H
#define GAME_PLAYER_H

#include <physics.h>
#include <renderer.h>
#include <input.h>
#include <visual_nodes.h>

class GameState;

class Player : public ev2::RigidBody {
public:
    Player(const std::string& name, GameState* game) : RigidBody{name}, game{game} {}

    void on_init() override {
        ev2::RigidBody::on_init();
        cam_first_person = create_node<ev2::CameraNode>("FP");
        add_child(cam_first_person);

        add_shape(ev2::make_referenced<ev2::CapsuleShape>(0.5, 2));
        get_body()->setType(reactphysics3d::BodyType::DYNAMIC);
        get_body()->setLinearDamping(0.3f);
        get_body()->setAngularLockAxisFactor(reactphysics3d::Vector3(0, 1, 0));

        auto& material = get_collider(0)->getMaterial();
        material.setBounciness(0.01f);
        material.setFrictionCoefficient(0.5f);
    }

    void on_ready() override {
        ev2::RigidBody::on_ready();
    }

    void on_process(float dt) override;

    ev2::Ref<ev2::CameraNode> cam_first_person{};
    float cam_x = 0, cam_y = 0;
    bool left_mouse_down = false;
    const float max_vel = 10.0f;
    GameState* game;
    glm::vec2 last_mouse_position{};
};

#endif // GAME_PLAYER_H