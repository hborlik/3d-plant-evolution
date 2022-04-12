/**
 * @file camera.h
 * @author Hunter Borlik 
 * @brief basic camera
 * @version 0.1
 * @date 2019-11-15
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#pragma once
#ifndef SSRE_CAMERA_H
#define SSRE_CAMERA_H

#include <scene.h>

namespace ssre {

class Camera : public Node {
public:
    Camera(std::string name) : Node{std::move(name)} {}

    void update(float delta) override;

    void Move(const glm::vec3& dir);

    glm::mat4 getViewMatrix();
protected:
    float x_angle = 0.f, y_angle = 0.f;
};

}

#endif // SSRE_CAMERA_H