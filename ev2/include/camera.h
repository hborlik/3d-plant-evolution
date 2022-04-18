/**
 * @file camera.h
 * @brief 
 * @date 2022-04-18
 * 
 */
#ifndef EV2_CAMERA_H
#define EV2_CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <geometry.h>

namespace ev2 {

class Camera {
public:
    Frustum extractFrustum() const noexcept {
        using namespace glm;
        const mat4& comp = composite;
        Frustum f{};
        auto& Left = f.left();
        auto& Right = f.right();
        auto& Top = f.top();
        auto& Bottom = f.bottom();
        auto& Near = f.near();
        auto& Far = f.far();

        Left.x = comp[0][3] + comp[0][0]; 
        Left.y = comp[1][3] + comp[1][0]; 
        Left.z = comp[2][3] + comp[2][0]; 
        Left.w = comp[3][3] + comp[3][0];
        Left /= length(vec3{Left});

        Right.x = comp[0][3] - comp[0][0];
        Right.y = comp[1][3] - comp[1][0];
        Right.z = comp[2][3] - comp[2][0];
        Right.w = comp[3][3] - comp[3][0];
        Right /= length(vec3{Right});

        Bottom.x = comp[0][3] + comp[0][1];
        Bottom.y = comp[1][3] + comp[1][1];
        Bottom.z = comp[2][3] + comp[2][1];
        Bottom.w = comp[3][3] + comp[3][1];
        Bottom /= length(vec3{Bottom});

        Top.x = comp[0][3] - comp[0][1];
        Top.y = comp[1][3] - comp[1][1];
        Top.z = comp[2][3] - comp[2][1];
        Top.w = comp[3][3] - comp[3][1];
        Top /= length(vec3{Top});

        Near.x = comp[0][3] + comp[0][2];
        Near.y = comp[1][3] + comp[1][2];
        Near.z = comp[2][3] + comp[2][2];
        Near.w = comp[3][3] + comp[3][2];
        Near /= length(vec3{Near});

        Far.x = comp[0][3] - comp[0][2];
        Far.y = comp[1][3] - comp[1][2];
        Far.z = comp[2][3] - comp[2][2];
        Far.w = comp[3][3] - comp[3][2];
        Far /= length(vec3{Far});

        return f;
    }

private:
    glm::mat4 projection, view;
    glm::mat4 composite;
};

} // namespace ev

#endif // EV2_CAMERA_H