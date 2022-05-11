/**
 * @file resource.h
 * @brief disk resource loader
 * @date 2022-04-18
 * 
 */
#ifndef EV2_RESOURCE_H
#define EV2_RESOURCE_H

#include <filesystem>

#include <glm/glm.hpp>

#include <mesh.h>

namespace ev2 {

/**
 * @brief model id
 * 
 */
struct MID {
    MID() = default;

    bool is_valid() const noexcept {return v != -1;}

    int32_t v = -1;
private:
    friend bool operator==(const MID& a, const MID& b) noexcept {
        return a.v == b.v;
    }
};

} // namespace ev2


template<> 
struct std::hash<ev2::MID> {
    std::size_t operator()(ev2::MID const& s) const noexcept {
        std::size_t h1 = std::hash<int>{}(s.v);
        // std::size_t h2 = std::hash<int>{}(s.y);
        // return h1 ^ (h2 << 1);
        return h1;
    }
};

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

class Model {
public:
    Model(std::vector<Mesh> meshes, glm::vec3 bmin, glm::vec3 bmax, std::vector<float> vb, VertexFormat format) : 
        meshes{std::move(meshes)}, bmin{bmin}, bmax{bmax}, buffer{std::move(vb)}, bufferFormat{format} {}
    
    std::vector<Mesh>       meshes;
    std::vector<float>      buffer;

    glm::vec3 bmin, bmax;

    VertexFormat bufferFormat;
};

class ResourceManager {
public:
    struct MaterialLocation {
        int32_t update_internal();

        int32_t material_id = -1;
        std::shared_ptr<Material> material{};
    };

    explicit ResourceManager(const std::filesystem::path& asset_path) : asset_path{asset_path}, model_lookup{} {}

    void pre_render();
    
    /**
     * @brief Get the model object reference id, or load object if not available
     * 
     * @param filename 
     * @return MID 
     */
    MID get_model(const std::filesystem::path& filename);

    std::pair<std::shared_ptr<Material>, int32_t> create_material(const std::string& name);
    std::shared_ptr<Material> get_material(const std::string& name);
    int32_t get_material_id(const std::string& name);
    void push_material_changed(const std::string& name);

    const auto& get_materials_locations() const {return materials;}

private:
    MaterialLocation get_material_internal(const std::string& name) {
        auto itr = materials.find(name);
        if (itr != materials.end()) {
            return itr->second;
        }
        return {};
    }

public:
    std::filesystem::path asset_path;

private:
    std::unordered_map<std::string, MID> model_lookup;
    std::unordered_map<MID, std::shared_ptr<Model>> models;

    std::unordered_map<std::string, MaterialLocation> materials;
};



}

#endif // EV2_RESOURCE_H