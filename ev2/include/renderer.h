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
    int32_t v = -1;

    bool is_valid() const noexcept {return v != -1;}
};

struct Light {
    glm::vec3 color;
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
        vb{std::move(vb)}, meshes{std::move(meshes)}, bmin{bmin}, bmax{bmax}, cull_mode{cull}, front_facing{ff} {}
    VertexBuffer vb;
    std::vector<Mesh> meshes;

    glm::vec3 bmin, bmax;

    gl::CullMode cull_mode = gl::CullMode::BACK;
    gl::FrontFacing front_facing = gl::FrontFacing::CCW;

    int32_t material_offset = 0;

    void draw(const Program& prog);
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
    glm::mat4   transform;
    Drawable*   drawable;
};

class Renderer : public Singleton<Renderer> {
public:
    Renderer(uint32_t width, uint32_t height, const std::filesystem::path& asset_path);

    void update_material(int32_t material_id, const MaterialData& material);
    int32_t add_material(const MaterialData& material);

    void create_model(MID mid, std::shared_ptr<Model> model);
    void create_model(MID mid, std::shared_ptr<Drawable> d);

    IID create_model_instance(MID mid);
    void destroy_instance(MID mid);
    void set_instance_transform(int32_t iid, const glm::mat4& transform);

    void render(const Camera &camera);

    void set_wireframe(bool enable);

    void set_resolution(uint32_t width, uint32_t height);

    void draw_screen_space_triangle();

private:
    std::unordered_map<MID, std::shared_ptr<Drawable>> models;
    std::vector<MaterialData> materials;
    std::vector<ModelInstance> model_instances;

    int next_free_mat = 1;

    std::vector<Light> point_lights;

    Program geometry_program;
    int gp_m_location;
    int gp_g_location;

    Program lighting_program;
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
