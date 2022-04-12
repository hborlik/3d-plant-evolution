/**
 * @file viewport.cpp
 * @author Hunter Borlik 
 * @brief 
 * @version 0.1
 * @date 2019-10-14
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include <viewport.h>
#include <glm/gtc/matrix_transform.hpp>

using namespace ssre;

Viewport::Viewport() : viewTransform{1.0f}, projectionTransform{1.0f} , fov{glm::pi<float>() / 4.f}, aspectRatio{1.f} {
    viewTransform = glm::translate(viewTransform, glm::vec3{0.0f, 0.0f, -10.0f});
}

void Viewport::SetFOV(float radians) {
    fov = radians;
    updateProjection();
}

void Viewport::SetAspectRatio(float ar) {
    aspectRatio = ar;
    updateProjection();
}

void Viewport::SetPlanes(float near, float far) {
    assert(near < far);
    nearPlane = near;
    farPlane = far;
    updateProjection();
}

void Viewport::updateProjection() {
    projectionTransform = glm::perspective(fov, aspectRatio, nearPlane, farPlane);
}