/**
 * @file material.h
 * @brief 
 * @date 2022-04-17
 * 
 */
#ifndef EV2_MATERIAL_H
#define EV2_MATERIAL_H

#include <glm/glm.hpp>

namespace ev2 {

struct Material {
    glm::vec3 spectral;
    glm::vec3 diffuse;
    glm::vec3 ambient;
};

}

#endif // EV2_MATERIAL_H