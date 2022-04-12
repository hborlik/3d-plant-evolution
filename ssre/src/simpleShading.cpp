/**
 * @file simpleShading.cpp
 * @author Hunter Borlik 
 * @brief 
 * @version 0.1
 * @date 2019-11-13
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include <string>

#include <simpleShading.h>

#include <material.h>
#include <geometry.h>
#include <resource.h>
#include <uniform.h>
#include <texture.h>

using namespace ssre;

std::unique_ptr<Material> SimpleShading::createMaterial(const MaterialInfo& mcfg) const {
    Material::UniformInputs uis{};
    std::vector<Material::TextureInput> textures;

    //uis.insert(std::make_pair("diffuse", std::make_unique<Uniform<float>>("diffuse", mcfg.diffuse[0], getUniformInfo("diffuse"))));

    int32_t texLoc = getUniformInfo("diffuseTex").Location;
    if(texLoc != -1) {
        textures.push_back(Material::TextureInput {texLoc, "diffuseTex", mcfg.diffuse_tex});
    }

    std::unique_ptr<Material> mat = std::make_unique<Material>(program_id, std::move(uis), std::move(textures));
    return mat;
}

void SimpleShading::onBuilt() {
    // each program has its own mapping to ubo bindings
    // attach this program to the global one.
    setUniformBlockBinding(mat_spec::GUBName, mat_spec::GUBBindingLocation);

    vertexInputLocation = getInputInfo(mat_spec::VertexAttributeName).Location;
    tangentInputLocation = getInputInfo(mat_spec::TangentAttributeName).Location;
    bitangentInputLocation = getInputInfo(mat_spec::BiTangentAttributeName).Location;
    texChoordInputLocation = getInputInfo(mat_spec::TextureAttributeName).Location;
}

/////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<Material> PBRShading::createMaterial(const MaterialInfo& mcfg) const {
    Material::UniformInputs uis{};
    std::vector<Material::TextureInput> textures;

    int32_t texLoc = getUniformInfo("albedoMap").Location;
    if(texLoc != -1) {
        textures.push_back(Material::TextureInput {texLoc, "albedoMap", mcfg.diffuse_tex});
    }

    texLoc = getUniformInfo("normalMap").Location;
    if(texLoc != -1) {
        textures.push_back(Material::TextureInput {texLoc, "normalMap", mcfg.normal_tex});
    }

    texLoc = getUniformInfo("metallicMap").Location;
    if(texLoc != -1) {
        textures.push_back(Material::TextureInput {texLoc, "metallicMap", mcfg.metallic_tex});
    }

    texLoc = getUniformInfo("roughnessMap").Location;
    if(texLoc != -1) {
        textures.push_back(Material::TextureInput {texLoc, "roughnessMap", mcfg.roughness_tex});
    }

    texLoc = getUniformInfo("aoMap").Location;
    if(texLoc != -1) {
        textures.push_back(Material::TextureInput {texLoc, "aoMap", mcfg.ambient_tex});
    }

    return std::make_unique<Material>(program_id, std::move(uis), std::move(textures));
}

void PBRShading::onBuilt() {
    // each program has its own mapping to ubo bindings
    // attach this program to the global one. If the program does not have a global ubo, it is invalid
    SSRE_CHECK_THROW(setUniformBlockBinding(mat_spec::GUBName, mat_spec::GUBBindingLocation), "PBRShading unable to bind global ubo!");

    vertexInputLocation = getInputInfo(mat_spec::VertexAttributeName).Location;
    tangentInputLocation = getInputInfo(mat_spec::TangentAttributeName).Location;
    bitangentInputLocation = getInputInfo(mat_spec::BiTangentAttributeName).Location;
    texChoordInputLocation = getInputInfo(mat_spec::TextureAttributeName).Location;
}