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

class Player : public ev2::RigidBody {
public:
    Player(const std::string& name) : RigidBody{name} {}

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

    void on_process(float dt) override {
        ev2::RigidBody::on_process(dt);

        reset_force();

        glm::vec3 move_input{};
        if (ev2::input::GetKeyDown(ev2::input::Key::KeyW))
            move_input.y += 1.0f;
        if (ev2::input::GetKeyDown(ev2::input::Key::KeyS))
            move_input.y += -1.0f;
        if (ev2::input::GetKeyDown(ev2::input::Key::KeyA))
            move_input.x += -1.0f;
        if (ev2::input::GetKeyDown(ev2::input::Key::KeyD))
            move_input.x += 1.0f;

        const glm::vec3 velocity = get_velocity();
        float vel_p = glm::length(velocity) / max_vel;

        auto& material = get_collider(0)->getMaterial();
        cam_first_person->transform.rotation = glm::rotate(glm::rotate(glm::identity<glm::quat>(), (float)cam_x, glm::vec3{0, 1, 0}), (float)cam_y, glm::vec3{1, 0, 0});
        if (glm::length(move_input) > 0.0f) {
            const glm::vec2 input = glm::normalize(move_input);
            const glm::vec3 cam_forward = glm::normalize(cam_first_person->get_camera().get_forward() * glm::vec3{1, 0, 1});
            const glm::vec3 cam_right = glm::normalize(cam_first_person->get_camera().get_right() * glm::vec3{1, 0, 1});
            const glm::vec3 target_vel = (cam_forward * input.y + cam_right * input.x) * 20.0f;
            float cv_d_av = glm::dot(glm::normalize(velocity), glm::normalize(target_vel));
            glm::vec3 correction{};
            if (vel_p > 0.0f)
                correction = velocity;
            apply_force(
                0.5f * ((1.f - vel_p) + (1.f - cv_d_av*cv_d_av)) * (target_vel - correction)
            ); // force camera movement on y plane
            material.setFrictionCoefficient(0.05f);
        } else {
            material.setFrictionCoefficient(0.9f);
        }

        // const ev2::Camera& cam = cam_first_person->get_camera();
        // glm::vec2 scr_size = ev2::window::getWindowSize();
        // glm::vec2 s_pos = ev2::window::getCursorPosition() / scr_size;
        
        // if ((left_mouse_down || ev2::window::getMouseCaptured()))
        // {
        //     ev2::Ray cast = cam.screen_pos_to_ray(s_pos);
        //     auto si = ev2::Physics::get_singleton().raycast_scene(cast, 200.0f);
        //     if (si) {
        //         std::cout << si->hit.ref_cast<ev2::Node>()->get_path() << std::endl; 
                    
        //         if (strstr(si->hit.ref_cast<ev2::Node>()->get_path().c_str(), std::string("Tree").c_str()))
        //         {
        //         }
        //         // marker->transform.position = si->point + glm::vec3{0, .25f, 0};
        //     }
        // }
    }

    ev2::Ref<ev2::CameraNode> cam_first_person{};
    float cam_x = 0, cam_y = 0;
    bool left_mouse_down = false;
    const float max_vel = 10.0f;
};

#endif // GAME_PLAYER_H