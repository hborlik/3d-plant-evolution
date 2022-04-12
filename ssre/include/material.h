/**
 * @file material.h
 * @author Hunter Borlik 
 * @brief 
 * @version 0.1
 * @date 2019-10-05
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#pragma once
#ifndef SSRE_MATERIAL_H
#define SSRE_MATERIAL_H

#include <vector>
#include <unordered_map>
#include <memory>

#include <uniform.h>

#include <glm/glm.hpp>

namespace ssre {

class Texture;
class UniformBase;
class RenderInfo;

// forward declare tinyobj_wrapper dependency
namespace tinyobj_wrapper {
struct material_t;
}

/**
 * @brief material information.
 */
struct MaterialInfo {
    MaterialInfo() = default;
    MaterialInfo(const tinyobj_wrapper::material_t& mat, std::string baseDir = "");
    ~MaterialInfo() = default;

    std::shared_ptr<Texture> ambient_tex;             // map_Ka
    std::shared_ptr<Texture> diffuse_tex;             // map_Kd
    std::shared_ptr<Texture> specular_tex;            // map_Ks

    std::shared_ptr<Texture> roughness_tex;  // map_Pr
    std::shared_ptr<Texture> metallic_tex;   // map_Pm
    std::shared_ptr<Texture> sheen_tex;      // map_Ps
    std::shared_ptr<Texture> emissive_tex;   // map_Ke
    std::shared_ptr<Texture> normal_tex;     // norm. For normal mapping.
};

/**
 * @brief Material: shader inputs.
 */
class Material {
public:
    using UniformInputs = std::unordered_map<std::string, std::unique_ptr<UniformBase>>;

    struct TextureInput {
        uint32_t location;                  // texture uniform location;
        std::string name;                   // uniform string name for sampler
        std::shared_ptr<Texture> texture;   // texture to attach
    };

    Material(uint32_t pid, UniformInputs inputs, std::vector<TextureInput> tex);
    Material(const Material& other);
    virtual ~Material() = default;

    Material& operator=(const Material& other) = delete;

    /**
     * @brief Get the Parameters. Contains Uniform names and values to be used in program.
     * 
     * @return const UniformInputSet&
     */
    const UniformInputs& getParameters() const noexcept {return parameters;}

    const std::vector<TextureInput>& getTextureInputs() const noexcept{return textures;}

    uint32_t getProgramId() const noexcept {return program_id;}

    /**
     * @brief Set a Parameter value
     * 
     * @tparam T 
     * @param name 
     * @param value 
     * @return true the parameter exists and was updates
     * @return false the parameter does not exist, or the type was not compatible
     */
    template<typename T>
    bool setParameter(const std::string& name, const T& value);

    /**
     * @brief Set the Texture to be used for input
     * 
     * @param name 
     * @param tex 
     * @return true 
     * @return false Texture input for name does not exist
     */
    bool setTextureInput(const std::string& name, const std::shared_ptr<Texture>& tex);
    
protected:
    /**
     * @brief Set of shader uniform inputs
     */
    UniformInputs parameters;

    /**
     * @brief Textures
     */
    std::vector<TextureInput> textures;

    /**
     * @brief Id of the Program used to generate this material
     * 
     */
    const uint32_t program_id;

    template<typename T>
    void addParameter(const std::string& name, const T& value);

    void swap(Material& o);
};

template<typename T>
bool Material::setParameter(const std::string& name, const T& value) {
    auto itr = parameters.find(name);
    if(itr != parameters.end()) { // update parameter value
        Uniform<T>* param = dynamic_cast<Uniform<T>*>(itr->second);
        if(param != nullptr) {
            param->setValue(value);
        } else
            return false;
        return true;
    }
    return false;
}


template<typename T>
void Material::addParameter(const std::string& name, const T& value) {
    auto itr = parameters.find(name);
    if(itr == parameters.end()) { // parameter does not exist
        parameters.insert(std::make_pair(name, std::make_unique<Uniform<T>>(name, value)));
    }
}

}

#endif // SSRE_MATERIAL_H