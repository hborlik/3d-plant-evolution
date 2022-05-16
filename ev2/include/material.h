/**
 * @file material.h
 * @brief 
 * @date 2022-05-16
 * 
 * 
 */
#ifndef EV2_MATERIAL_H
#define EV2_MATERIAL_H

#include <string>

#include <glm/glm.hpp>

namespace ev2 {

struct Material {
    const std::string name = "default";

    glm::vec3 diffuse   = {1.00f,0.10f,0.85f};
    float metallic       = 0;
    float subsurface     = 0;
    float specular       = .5f;
    float roughness      = .5f;
    float specularTint   = 0;
    float clearcoat      = 0;
    float clearcoatGloss = 1.f;
    float anisotropic    = 0;
    float sheen          = 0;
    float sheenTint      = .5f;

    std::string ambient_texname;             // map_Ka
    std::string diffuse_texname;             // map_Kd
    std::string specular_texname;            // map_Ks
    std::string specular_highlight_texname;  // map_Ns
    std::string bump_texname;                // map_bump, map_Bump, bump
    std::string displacement_texname;        // disp
    std::string alpha_texname;               // map_d
    std::string reflection_texname;          // refl

    Material() = default;
    Material(std::string name) : name{std::move(name)} {}
};

}

#endif // EV2_MATERIAL_H