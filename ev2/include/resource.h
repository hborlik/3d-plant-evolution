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

#include <vertex_buffer.h>
#include <renderer.h>
#include <scene.h>
#include <texture.h>
#include <gltf.h>

namespace ev2 {

struct DrawObject {
    size_t start;
    size_t numTriangles;
    size_t material_id;
};

class MaterialResource : public Object {
public:
    MaterialResource() {
        material = renderer::Renderer::get_singleton().create_material();
    }
    virtual ~MaterialResource() {
        renderer::Renderer::get_singleton().destroy_material(material);
    }

    MaterialResource(const MaterialResource&) = delete;
    MaterialResource(MaterialResource&&) = delete;
    MaterialResource& operator=(const MaterialResource&) = delete;
    MaterialResource& operator=(MaterialResource&&) = delete;

    renderer::Material* get_material() {return material;}

private:
    renderer::Material* material = nullptr;
};

// single buffer model
class Model {
public:
    Model(const std::string &name,
          std::vector<DrawObject> draw_objects,
          std::vector<Ref<MaterialResource>> materials,
          glm::vec3 bmin,
          glm::vec3 bmax,
          std::vector<float> vb) : name{name},
                                   draw_objects{std::move(draw_objects)},
                                   materials{std::move(materials)},
                                   bmin{bmin},
                                   bmax{bmax},
                                   buffer{std::move(vb)} {}

    std::string             name;
    std::vector<DrawObject> draw_objects;
    std::vector<Ref<MaterialResource>>  materials;
    std::vector<float>      buffer;

    glm::vec3 bmin, bmax;
};

class ResourceManager : public Singleton<ResourceManager> {
public:
    struct MaterialLocation {
        int32_t update_internal();

        int32_t material_id = -1;
        
    };

    explicit ResourceManager(const std::filesystem::path& asset_path) : asset_path{asset_path}, model_lookup{} {}
    ~ResourceManager();

    void pre_render();

    /**
     * @brief Get the model object reference id, or load object if not available
     * 
     * @param filename 
     * @return renderer::MID 
     */
    renderer::Drawable* get_model(const std::filesystem::path& filename, bool cache = true);

    renderer::Drawable* create_model(std::shared_ptr<Model> model);

    std::shared_ptr<Texture> get_texture(const std::filesystem::path& filename);

    Ref<GLTFScene> loadGLTF(const std::filesystem::path& filename, bool normalize = false);

    Ref<MaterialResource> get_material(const std::string& name);
    int32_t get_material_id(const std::string& name);

    const auto& get_materials() const {return materials;}

public:
    std::filesystem::path asset_path;

private:
    std::unordered_map<std::string, renderer::Drawable*> model_lookup;

    std::unordered_map<std::string, Ref<MaterialResource>> materials;

    std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
};

std::unique_ptr<Model> loadObj(const std::filesystem::path& filename, const std::filesystem::path& base_dir, ResourceManager* rm = nullptr);

std::unique_ptr<Texture> load_texture2D(const std::filesystem::path& filename);

}

#endif // EV2_RESOURCE_H