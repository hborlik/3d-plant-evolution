/**
 * @file camera.h
 * @brief 
 * @date 2022-04-18
 * 
 */
#ifndef EV2_CAMERA_H
#define EV2_CAMERA_H

#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <geometry.h>


namespace ev2 {

class Camera {
public:
    Frustum extract_frustum() const noexcept {
        using namespace glm;
        if (dirty)
            force_update_internal();
        const mat4& comp = p_v;
        Frustum f{};
        auto& Left = f.left();
        auto& Right = f.right();
        auto& Top = f.top();
        auto& Bottom = f.bottom();
        auto& Near = f.near();
        auto& Far = f.far();

        Left.p.x = comp[0][3] + comp[0][0]; 
        Left.p.y = comp[1][3] + comp[1][0]; 
        Left.p.z = comp[2][3] + comp[2][0]; 
        Left.p.w = comp[3][3] + comp[3][0];
        Left.normalize();

        Right.p.x = comp[0][3] - comp[0][0];
        Right.p.y = comp[1][3] - comp[1][0];
        Right.p.z = comp[2][3] - comp[2][0];
        Right.p.w = comp[3][3] - comp[3][0];
        Right.normalize();

        Bottom.p.x = comp[0][3] + comp[0][1];
        Bottom.p.y = comp[1][3] + comp[1][1];
        Bottom.p.z = comp[2][3] + comp[2][1];
        Bottom.p.w = comp[3][3] + comp[3][1];
        Bottom.normalize();

        Top.p.x = comp[0][3] - comp[0][1];
        Top.p.y = comp[1][3] - comp[1][1];
        Top.p.z = comp[2][3] - comp[2][1];
        Top.p.w = comp[3][3] - comp[3][1];
        Top.normalize();

        Near.p.x = comp[0][3] + comp[0][2];
        Near.p.y = comp[1][3] + comp[1][2];
        Near.p.z = comp[2][3] + comp[2][2];
        Near.p.w = comp[3][3] + comp[3][2];
        Near.normalize();

        Far.p.x = comp[0][3] - comp[0][2];
        Far.p.y = comp[1][3] - comp[1][2];
        Far.p.z = comp[2][3] - comp[2][2];
        Far.p.w = comp[3][3] - comp[3][2];
        Far.normalize();

        return f;
    }
    

    std::array<glm::vec3, 8> extract_frustum_corners() const noexcept {
        using namespace glm;
        if (dirty)
            force_update_internal();
        std::array<glm::vec3, 8> ndcPoints = {
            glm::vec3(-1, -1, 1),
            glm::vec3(1, -1, 1),
            glm::vec3(-1, 1, 1),
            glm::vec3(1, 1, 1),
            glm::vec3(-1, -1, -1),
            glm::vec3(1, -1, -1),
            glm::vec3(-1, 1, -1),
            glm::vec3(1, 1, -1)
        };    
        std::array<glm::vec3, 8> worldPoints;
        for (int i = 0; i < 8; i++)
        {
            worldPoints[0] = inv_pv() * vec4(ndcPoints[0], 1.0);
        }
        return worldPoints;
        // TODO
    }

    /**
     * @brief Force internal View and Projection * View matrices to be updated.
     * 
     */
    void force_update_internal() const {
        using namespace glm;
        view = inverse(translate(identity<glm::mat4>(), position) * mat4_cast(rotation));
        p_v = projection * view;
        dirty = false;
    }

    /**
     * @brief Get the View Matrix for Camera
     * 
     * @return glm::mat4 
     */
    glm::mat4 get_view() const {
        if (dirty)
            force_update_internal();
        return view;
    }

    /**
     * @brief get inverse Projection * View matrix
     * 
     * @return glm::mat4 
     */
    glm::mat4 inv_pv() const {
        if (dirty)
            force_update_internal();
        return glm::inverse(p_v);
    }

    glm::mat4 get_projection() const {return projection;}
    glm::vec3 get_position() const {return position;}
    glm::quat get_rotation() const {return rotation;}

    void set_projection(const glm::mat4& p) {
        projection = p;
        dirty = true;
    }

    void set_position(const glm::vec3& p) {
        position = p;
        dirty = true;
    }

    void set_rotation(const glm::quat& q) {
        rotation = q;
        dirty = true;
    }

    /**
     * @brief Move camera in a local direction
     * 
     * @param direction 
     */
    void move(const glm::vec3 &direction) {
        using namespace glm;
        vec3 dir = glm::mat3(rotation) * direction;
        position += dir;
    }

    glm::vec3 get_forward() const {
        return -glm::mat4_cast(rotation)[2];
    }

    glm::vec3 get_up() const {
        return glm::mat4_cast(rotation)[1];
    }

    glm::vec3 get_right() const {
        return glm::mat4_cast(rotation)[0];
    }

    Ray screen_pos_to_ray(glm::vec2 s_pos) const {
        glm::vec2 s_pos_ndc = 2.f * (s_pos) - glm::vec2{1, 1};
        s_pos_ndc.y *= -1;
        glm::mat4 pv_inv = inv_pv();
        glm::vec4 pos = pv_inv * glm::vec4{s_pos_ndc, -1, 1.0};

        pos.x /= pos.w;
        pos.y /= pos.w;
        pos.z /= pos.w;

        glm::vec3 c_pos = get_position();
        return Ray{c_pos, glm::vec3(pos) - c_pos};
    }

private:
    glm::vec3 position{};
    glm::quat rotation = glm::identity<glm::quat>();

    glm::mat4 projection = glm::identity<glm::mat4>();

    mutable glm::mat4 view = glm::identity<glm::mat4>();
    mutable glm::mat4 p_v = glm::identity<glm::mat4>();
    mutable bool dirty = true;
};

} // namespace ev

#endif // EV2_CAMERA_H