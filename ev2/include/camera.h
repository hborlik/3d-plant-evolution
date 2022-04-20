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

    void forceUpdateInternal() const {
        view = glm::inverse(glm::translate(glm::identity<glm::mat4>(), position) * glm::mat4_cast(rotation));
        composite = projection * view;
        dirty = false;
    }

    glm::mat4 getView() const {
        if (dirty)
            forceUpdateInternal();
        return view;
    }

    glm::mat4 getProjection() const {return projection;}
    glm::vec3 getPosition() const {return position;}
    glm::quat getRotation() const {return rotation;}

    void setProjection(const glm::mat4& p) {
        projection = p;
        dirty = true;
    }

    void setPosition(const glm::vec3& p) {
        position = p;
        dirty = true;
    }

    void setRotation(const glm::quat& q) {
        rotation = q;
        dirty = true;
    }

private:
    glm::vec3 position;
    glm::quat rotation;

    glm::mat4 projection;

    mutable glm::mat4 view;
    mutable glm::mat4 composite;
    mutable bool dirty = true;
};

} // namespace ev

#endif // EV2_CAMERA_H