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

#include <material.h>
#include <vertex_buffer.h>
#include <renderer.h>
#include <scene.h>
#include <texture.h>

namespace ev2 {

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

    std::unique_ptr<Scene> loadGLTF(const std::filesystem::path& filename, const std::filesystem::path& base_dir, bool normalize = false);

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

    std::unordered_map<std::string, MaterialLocation> materials;
};

std::unique_ptr<Model> loadObj(const std::filesystem::path& filename, const std::filesystem::path& base_dir, ResourceManager* rm = nullptr);

std::unique_ptr<Texture> load_texture2D(const std::filesystem::path& filename);

}

#endif // EV2_RESOURCE_H