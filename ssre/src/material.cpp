/**
 * @file material.cpp
 * @author Hunter Borlik 
 * @brief 
 * @version 0.1
 * @date 2019-10-06
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include <cassert>

#include <uniform.h>
#include <material.h>
#include <texture.h>
#include <resource.h>

#include <tiny_obj_wrapper.h>

using namespace ssre;

MaterialInfo::MaterialInfo(const tinyobj_wrapper::material_t& other, std::string baseDir) :
    ambient_tex{Resource::StaticInst().load2DTexture(other.mat.ambient_texname.empty() ? "" : baseDir + other.mat.ambient_texname)},
    diffuse_tex{Resource::StaticInst().load2DTexture(other.mat.diffuse_texname.empty() ? "" : baseDir + other.mat.diffuse_texname)},
    specular_tex{Resource::StaticInst().load2DTexture(other.mat.specular_texname.empty() ? "" : baseDir + other.mat.specular_texname)},
    roughness_tex{Resource::StaticInst().load2DTexture(other.mat.roughness_texname.empty() ? "" : baseDir + other.mat.roughness_texname)},
    metallic_tex{Resource::StaticInst().load2DTexture(other.mat.metallic_texname.empty() ? "" : baseDir + other.mat.metallic_texname)},
    sheen_tex{Resource::StaticInst().load2DTexture(other.mat.sheen_texname.empty() ? "" : baseDir + other.mat.sheen_texname)},
    emissive_tex{Resource::StaticInst().load2DTexture(other.mat.emissive_texname.empty() ? "" : baseDir + other.mat.emissive_texname)},
    normal_tex{Resource::StaticInst().load2DTexture(other.mat.normal_texname.empty() ? "" : baseDir + other.mat.normal_texname)} {

}

/////////////////////////////////////////////////////////////////////////////////////

Material::Material(uint32_t pid, UniformInputs inputs, std::vector<TextureInput> tex) : 
    parameters{std::move(inputs)}, textures{std::move(tex)}, program_id{pid} {

}

Material::Material(const Material& other) : textures{other.textures}, program_id{other.program_id} {
    for(const auto& param : other.parameters) {
        parameters.insert(std::make_pair(param.first, param.second->cloneUniform()));
    }
}

bool Material::setTextureInput(const std::string& name, const std::shared_ptr<Texture>& tex) {
    for(auto& p : textures) {
        if(p.name == name) {
            p.texture = tex;
            return true;
        }
    }
    return false;
}