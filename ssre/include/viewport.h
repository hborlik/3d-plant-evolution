/**
 * @file viewport.h
 * @author Hunter Borlik 
 * @brief camera viewport information
 * @version 0.1
 * @date 2019-09-18
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#pragma once
#ifndef SSRE_VIEWPORT_H
#define SSRE_VIEWPORT_H

#include <glm/glm.hpp>

namespace ssre {

/**
 * @brief Camera information
 * 
 */
class Viewport {
public:
    Viewport();

    glm::mat4 GetProjectionTransform() const noexcept {return projectionTransform;}
    glm::mat4 GetViewTransform() const noexcept {return viewTransform;}

    void SetFOV(float radians);

    void SetAspectRatio(float ar);

    void SetPlanes(float near, float far);

private:
    glm::mat4 viewTransform;
    glm::mat4 projectionTransform;

    float fov, aspectRatio, nearPlane, farPlane;

    void updateProjection();

};

} // ssre

#endif // SSRE_VIEWPORT_H