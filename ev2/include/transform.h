/**
 * @file transform.h
 * @brief 
 * @date 2022-05-13
 * 
 * 
 */
#ifndef EV2_TRANSFORM_H
#define EV2_TRANSFORM_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace ev2 {

struct Transform {
    /**
     * @brief apply euler roataions
     * 
     * @param xyz 
     */
    void rotate(const glm::vec3& xyz) {
        rotation = glm::rotate(glm::rotate(glm::rotate(rotation, xyz.x, {1, 0, 0}), xyz.y, {0, 1, 0}), xyz.z, {0, 0, 1});
    }

    glm::mat4 get_transform() const noexcept {
        glm::mat4 tr = transform_cache;
        if (!transform_cache_valid) {
            tr = glm::mat4_cast(rotation) * glm::scale(glm::identity<glm::mat4>(), scale);
            tr[3] = glm::vec4{position, 1.0f};

            transform_cache = tr;
            transform_cache_valid = true;
        }
        return tr;
    }

    glm::mat4 get_linear_transform() const noexcept {
        glm::mat4 tr = glm::mat4_cast(rotation);
        tr[3] = glm::vec4{position, 1.0f};
        return tr;
    }

    inline glm::vec3 get_position() const noexcept {return position;}
    inline glm::quat get_rotation() const noexcept {return rotation;}
    inline glm::vec3 get_scale() const noexcept {return scale;}

    inline void set_position(glm::vec3 pos) noexcept {position = pos;transform_cache_valid = false;}
    inline void set_rotation(glm::quat rot) noexcept {rotation = rot;transform_cache_valid = false;}
    inline void set_scale(glm::vec3 s) noexcept { scale = s;transform_cache_valid = false;}

private:
    glm::vec3 position{};
    glm::quat rotation = glm::identity<glm::quat>();
    glm::vec3 scale{1, 1, 1};

    mutable glm::mat4   transform_cache;
    mutable bool        transform_cache_valid = false;
};

}

#endif // 
