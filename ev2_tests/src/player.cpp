#include <player.h>

#include <game.h>
#include <tree.h>

void Player::on_process(float dt) {
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

    const glm::vec3 velocity = get_velocity() * glm::vec3{1, 0, 1};
    float vel_p = glm::length(velocity) / max_vel;

    auto& material = get_collider(0)->getMaterial();
    cam_first_person->transform.rotation = glm::rotate(glm::rotate(glm::identity<glm::quat>(), (float)cam_x, glm::vec3{0, 1, 0}), (float)cam_y, glm::vec3{1, 0, 0});
    if (glm::length(move_input) > 0.0f) {
        const glm::vec2 input = glm::normalize(move_input);
        const glm::vec3 cam_forward = glm::normalize(cam_first_person->get_camera().get_forward() * glm::vec3{1, 0, 1});
        const glm::vec3 cam_right = glm::normalize(cam_first_person->get_camera().get_right() * glm::vec3{1, 0, 1});
        const glm::vec3 target_vel = (cam_forward * input.y + cam_right * input.x) * 20.0f;
        float cv_d_av = 0.0f;
        glm::vec3 correction{};
        if (vel_p > 0.0f) {
            correction = velocity;
            cv_d_av = glm::dot(glm::normalize(velocity), glm::normalize(target_vel));
        }
        apply_force(
            0.5f * ((1.f - vel_p) + (1.f - cv_d_av*cv_d_av)) * (target_vel - correction)
        );
        material.setFrictionCoefficient(0.05f);
    } else {
        material.setFrictionCoefficient(0.9f);
    }

    glm::vec2 s_pos = ev2::input::GetMousePosition();
    glm::vec2 delta_mouse = s_pos - last_mouse_position;
    last_mouse_position = s_pos;
    
    ev2::Ray cast = cam_first_person->get_camera().screen_pos_to_ray(s_pos);
    auto si = ev2::Physics::get_singleton().raycast_scene(cast, 200.0f);
    if (si) {
        std::cout << si->hit.ref_cast<ev2::Node>()->get_path() << std::endl; 
        
        ev2::Ref<TreeNode> tree = si->hit.ref_cast<ev2::Node>()->get_child(0).ref_cast<TreeNode>();
        if (tree)
        {

        game->marker->transform.position = si->point + glm::vec3{0, .25f, 0};
        }
    }
}