/**
 * @file renderer.h
 * @brief 
 * @date 2022-04-27
 * 
 */
#ifndef EV2_RENDERER_H
#define EV2_RENDERER_H

#include <unordered_map>
#include <memory>
#include <filesystem>

#include <singleton.h>
#include <mesh.h>
#include <shader.h>
#include <texture.h>
#include <camera.h>
#include <resource.h>


namespace ev2 {

/**
 * @brief light id
 * 
 */
struct LID {
    enum LightType {
        Point,
        Directional
    } _type;
    int32_t _v = -1;

    bool is_valid() const noexcept {return _v != -1;}
};

struct Light {
    glm::vec3 color;
    glm::vec3 position;
};

struct DirectionalLight {
    glm::vec3 color     = {1, 1, 1};
    glm::vec3 ambient   = {0.05, 0.05, 0.05};
    glm::vec3 direction = {0, -1, 0};
};

/**
 * @brief instance id
 * 
 */
struct IID {
    int32_t v = -1;

    bool is_valid() const noexcept {return v != -1;}
};

struct Drawable {
    Drawable(VertexBuffer&& vb, std::vector<Mesh> meshes, glm::vec3 bmin, glm::vec3 bmax, gl::CullMode cull, gl::FrontFacing ff) : 
        vertex_buffer{std::move(vb)}, meshes{std::move(meshes)}, bmin{bmin}, bmax{bmax}, cull_mode{cull}, front_facing{ff} {}
    VertexBuffer vertex_buffer;
    std::vector<Mesh> meshes;

    glm::vec3 bmin, bmax;

    gl::CullMode cull_mode = gl::CullMode::BACK;
    gl::FrontFacing front_facing = gl::FrontFacing::CCW;

    int32_t material_offset = 0;

    void draw(const Program& prog, int32_t material_override);
};

struct MaterialData {
    glm::vec3 diffuse   = {0.5, 0.4, 0.0};
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

    static MaterialData from_material(const Material& mat) {
        MaterialData nMat{};
        nMat.diffuse = mat.diffuse;
        nMat.specular = mat.shininess;
        return nMat;
    }
};

struct ModelInstance {
    glm::mat4   transform = glm::identity<glm::mat4>();
    Drawable*   drawable = nullptr;
    int32_t     material_override = -1;
};

class Renderer : public Singleton<Renderer> {
public:
    Renderer(uint32_t width, uint32_t height, const std::filesystem::path& asset_path);

    void update_material(int32_t material_id, const MaterialData& material);
    int32_t create_material(const MaterialData& material);

    LID create_light();
    LID create_directional_light();
    void set_light_position(LID lid, const glm::vec3& position);
    void destroy_light(LID lid);

    MID create_model(std::shared_ptr<Model> model);
    MID create_model(std::shared_ptr<Drawable> d);

    IID create_model_instance();
    void set_instance_model(IID iid, MID mid);
    void set_instance_transform(IID iid, const glm::mat4& transform);

    /**
     * @brief Set the instance material override id. Note: this will set materials for all shapes in model
     * 
     * @param iid instance id
     * @param material_override id of material to use when rendering, -1 for model defaults
     */
    void set_instance_material_override(IID iid, int32_t material_override);
    
    void destroy_instance(IID iid);

    void render(const Camera &camera);

    void set_wireframe(bool enable);

    void set_resolution(uint32_t width, uint32_t height);
    float get_aspect_ratio() const noexcept {return width / (float)height;}

    void draw_screen_space_triangle();

private:
    std::unordered_map<MID, std::shared_ptr<Drawable>> models;
    uint32_t next_model_id = 1;

    std::vector<MaterialData> materials;

    std::unordered_map<int32_t, ModelInstance> model_instances;
    uint32_t next_instance_id = 100;

    int next_free_mat = 1;

    std::unordered_map<uint32_t, Light> point_lights;
    std::unordered_map<uint32_t, DirectionalLight> directional_lights;
    uint32_t next_light_id = 1000;

    Program geometry_program;
    int gp_m_location;
    int gp_g_location;

    Program directional_lighting_program;
    int lp_p_location, lp_n_location, lp_as_location, lp_mt_location;

    FBO g_buffer;
    
    VertexBuffer sst_vb;

    std::shared_ptr<Texture> material_tex;
    std::shared_ptr<Texture> albedo_spec;
    std::shared_ptr<Texture> normals;
    std::shared_ptr<Texture> position;

    Buffer shader_globals;
    ProgramUniformBlockDescription globals_desc;

    Buffer lighting_materials;
    ProgramUniformBlockDescription lighting_materials_desc;

    uint32_t width, height;
    bool wireframe = false;

};

}

#endif // EV2_RENDERER_H
