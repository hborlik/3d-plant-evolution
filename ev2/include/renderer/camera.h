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

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 2; j++) {
                f.planes[i * 2 + j].p.x = comp[0][3] + (j == 0 ? comp[0][j] : -comp[0][j]);
                f.planes[i * 2 + j].p.y = comp[1][3] + (j == 0 ? comp[1][j] : -comp[1][j]);
                f.planes[i * 2 + j].p.z = comp[2][3] + (j == 0 ? comp[2][j] : -comp[2][j]);
                f.planes[i * 2 + j].p.w = comp[3][3] + (j == 0 ? comp[3][j] : -comp[3][j]);
                f.planes[i * 2 + j].normalize();
            }

        return f;
    }
    
    /**
     * @brief get the world space frustum corners. 
     * 
     * @param frustum_extent_far percentage of the frustum far extent
     * @return std::array<glm::vec3, 8> 
     */
    std::array<glm::vec3, 8> extract_frustum_corners(float frustum_extent_far = 1.0f) const noexcept {
        using namespace glm;
        if (dirty)
            force_update_internal();
        const float f_n = m_far - m_near;
        const float f_view = f_n * frustum_extent_far + m_near;
        const float ndc_far = (m_far + m_near) / f_n + 2 * m_far * m_near / (f_n * -f_view);
        std::array<glm::vec3, 8> ndcPoints = {
            glm::vec3(-1, -1,  ndc_far),
            glm::vec3( 1, -1,  ndc_far),
            glm::vec3(-1,  1,  ndc_far),
            glm::vec3( 1,  1,  ndc_far),
            glm::vec3(-1, -1, -1),
            glm::vec3( 1, -1, -1),
            glm::vec3(-1,  1, -1),
            glm::vec3( 1,  1, -1)
        };    
        std::array<glm::vec3, 8> worldPoints;
        for (int i = 0; i < 8; i++)
        {
            glm::vec4 pos = inv_pv() * vec4(ndcPoints[i], 1.0);
            pos = pos / pos.w;
            worldPoints[i] = glm::vec3(pos);
        }
        return worldPoints;
    }

    /**
     * @brief Force internal View and Projection * View matrices to be updated.
     * 
     */
    void force_update_internal() const noexcept {
        using namespace glm;
        viewInv = translate(identity<glm::mat4>(), position) * mat4_cast(rotation);
        view = inverse(viewInv);
        p_v = projection * view;
        dirty = false;
    }

    /**
     * @brief Get the View Matrix for Camera
     * 
     * @return glm::mat4 
     */
    glm::mat4 get_view() const noexcept {
        if (dirty)
            force_update_internal();
        return view;
    }

    /**
     * @brief Get the Inverse View Matrix for Camera
     * 
     * @return glm::mat4 
     */
    glm::mat4 get_view_inv() const noexcept {
        if (dirty)
            force_update_internal();
        return viewInv;
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

    void set_projection(float _fov, float _aspect, float _near, float _far) {
        fov = _fov;
        aspect = _aspect;
        m_near = _near;
        m_far = _far;
        projection = glm::perspective(glm::radians(fov), aspect, m_near, m_far);
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
    mutable glm::mat4 viewInv = glm::identity<glm::mat4>();
    mutable glm::mat4 p_v = glm::identity<glm::mat4>();
    mutable bool dirty = true;

    float fov = 60.0f;
    float m_near = 0.1f, m_far = 500.0f;
    float aspect = 1.0f;
};

} // namespace ev

#endif // EV2_CAMERA_H