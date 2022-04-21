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
    std::string name = "default";

    glm::vec3 ambient = {};
    glm::vec3 diffuse = {1.,.0,.5};
    glm::vec3 specular = {};
    glm::vec3 transmittance = {};
    glm::vec3 emission = {};
    float shininess = 1.0f;
    float ior = 0;
    float dissolve = 0;

    std::string ambient_texname;             // map_Ka
    std::string diffuse_texname;             // map_Kd
    std::string specular_texname;            // map_Ks
    std::string specular_highlight_texname;  // map_Ns
    std::string bump_texname;                // map_bump, map_Bump, bump
    std::string displacement_texname;        // disp
    std::string alpha_texname;               // map_d
    std::string reflection_texname;          // refl

    Material() = default;
};

}

#endif // EV2_MATERIAL_H