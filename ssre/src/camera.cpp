/**
 * @file camera.cpp
 * @author Hunter Borlik 
 * @brief 
 * @version 0.1
 * @date 2019-11-15
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <camera.h>
#include <window.h>

using namespace ssre;

void Camera::update(float delta) {
    glm::vec2 mvel = Window::StaticInst().getMouseVel();

    x_angle += -mvel.y * 0.5f;
    y_angle += -mvel.x * 0.5f;

    x_angle = glm::clamp(x_angle, -glm::pi<float>(), glm::pi<float>());
    y_angle = y_angle - (2.f*glm::pi<float>()*(int)(y_angle / (2.f*glm::pi<float>())));

    rotation = glm::rotate(glm::mat4{1.f}, y_angle, glm::vec3{0, 1, 0});
    rotation = glm::rotate(rotation, x_angle, glm::vec3{1, 0, 0});

    glm::vec3 deltaP{0, 0, 0};

    if(Window::StaticInst().w)
        deltaP.z -= delta * 2.f;
    
    if(Window::StaticInst().s)
        deltaP.z += delta * 2.f;

    if(Window::StaticInst().a)
        deltaP.x -= delta * 2.f;

    if(Window::StaticInst().d)
        deltaP.x += delta * 2.f;

    Move(deltaP);
}

void Camera::Move(const glm::vec3& dir) {
    position += glm::vec3{rotation*glm::vec4{dir, 1}};
}

glm::mat4 Camera::getViewMatrix() {
    return glm::inverse(getModelMatrix());
}