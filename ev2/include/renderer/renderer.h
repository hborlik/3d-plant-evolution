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
#include <array>
#include <queue>

#include <singleton.h>
#include <renderer/model.h>
#include <renderer/shader.h>
#include <renderer/texture.h>
#include <renderer/camera.h>


namespace ev2::renderer {

constexpr uint16_t MAX_N_MATERIALS = 255;

using mat_id_t = uint8_t;

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
    glm::vec3 color{};
    glm::vec3 position{};
    glm::vec3 k{0.0, 0.0, 2.5}; // k_c, k_l, k_q
};

struct DirectionalLight {
    glm::vec3 color     = {1, 1, 1};
    glm::vec3 ambient   = {0.05, 0.05, 0.05};
    glm::vec3 position = {0, -1, 0};
};

/**
 * @brief material data internal
 * 
 */
struct MaterialData {
    glm::vec3 diffuse       = {0.5, 0.4, 0.0};
    glm::vec3 emissive      = {};
    float metallic          = 0;
    float subsurface        = 0;
    float specular          = .5f;
    float roughness         = .5f;
    float specularTint      = 0;
    float clearcoat         = 0;
    float clearcoatGloss    = 1.f;
    float anisotropic       = 0;
    float sheen             = 0;
    float sheenTint         = .5f;

    GLint diffuse_offset        = 0;
    GLint emissive_offset       = 0;
    GLint metallic_offset       = 0;
    GLint subsurface_offset     = 0;
    GLint specular_offset       = 0;
    GLint roughness_offset      = 0;
    GLint specularTint_offset   = 0;
    GLint clearcoat_offset      = 0;
    GLint clearcoatGloss_offset = 0;
    GLint anisotropic_offset    = 0;
    GLint sheen_offset          = 0;
    GLint sheenTint_offset      = 0;

    bool changed = false;

    void update_from(Material* material) noexcept {
        if (material) {
            if (
                material->diffuse          != diffuse ||
                material->emissive         != emissive ||
                material->metallic         != metallic ||
                material->subsurface       != subsurface ||
                material->specular         != specular ||
                material->roughness        != roughness ||
                material->specularTint     != specularTint ||
                material->clearcoat        != clearcoat ||
                material->clearcoatGloss   != clearcoatGloss ||
                material->anisotropic      != anisotropic ||
                material->sheen            != sheen ||
                material->sheenTint        != sheenTint
            ) {
                changed = true;
                diffuse          = material->diffuse;
                emissive         = material->emissive;
                metallic         = material->metallic;
                subsurface       = material->subsurface;
                specular         = material->specular;
                roughness        = material->roughness;
                specularTint     = material->specularTint;
                clearcoat        = material->clearcoat;
                clearcoatGloss   = material->clearcoatGloss;
                anisotropic      = material->anisotropic;
                sheen            = material->sheen;
                sheenTint        = material->sheenTint;
            }
        }
    }
};

struct MSIID { // mesh instance id
    int32_t v = -1;

    bool is_valid() const noexcept {return v != -1;}
};

struct MeshPrimitive {
    MeshPrimitive() noexcept = default;
    MeshPrimitive(VertexBuffer* vb, int32_t material_id, int32_t indices = -1)
        : vb{vb}, indices{indices}, material_id{material_id} {}
    
    ~MeshPrimitive() {
        if (gl_vao != 0)
            glDeleteVertexArrays(1, &gl_vao);
    }

    std::map<int, int>      attributes;         // map of attribute location (engine value like VERTEX_BINDING_LOCATION to a buffer in vb)
    VertexBuffer*           vb{};               // id of used vertex buffer
    int32_t                 indices = -1;       // ind of index buffer in vb
    int32_t                 material_id = 0;

private:
    friend class Renderer;
    friend class RenderObj;
    GLuint gl_vao = 0;
};

struct RenderObj {
    gl::CullMode                    cull_mode = gl::CullMode::NONE;
    gl::FrontFacing                 front_facing = gl::FrontFacing::CCW;

    void set_mesh_primitives(const std::vector<MeshPrimitive>& primitives);

    RenderObj() = default;

private:
    friend class Renderer;
    int32_t id = -1;

    std::vector<MeshPrimitive> primitives{};
};

struct MeshInstance {
    glm::mat4               transform = glm::identity<glm::mat4>();
    RenderObj*              mesh = nullptr;
};

// instance drawable primitive setup

struct Primitive {
    size_t      start_index = 0;
    size_t      num_elements = 0;
    int32_t     material_ind = 0; // material index in model material list
};

/**
 * @brief Renderer Drawable structure
 * 
 */
struct Drawable {
    VertexBuffer            vertex_buffer;
    std::vector<Primitive>  primitives;
    std::vector<Material*>  materials;

    glm::vec3 bmin, bmax;

    gl::CullMode    cull_mode = gl::CullMode::BACK;
    gl::FrontFacing front_facing = gl::FrontFacing::CCW;

    float vertex_color_weight = 0.f;

private:
    friend class Renderer;

    Drawable(VertexBuffer &&vb,
                 std::vector<Primitive> primitives,
                 std::vector<Material *> materials,
                 glm::vec3 bmin,
                 glm::vec3 bmax,
                 gl::CullMode cull,
                 gl::FrontFacing ff,
                 uint32_t id) : vertex_buffer{std::move(vb)},
                                primitives{std::move(primitives)},
                                materials{std::move(materials)},
                                bmin{bmin},
                                bmax{bmax},
                                cull_mode{cull},
                                front_facing{ff},
                                id{id}
    {
    }

    uint32_t id = 0;
};

struct ModelInstance {
    glm::mat4   transform = glm::identity<glm::mat4>();

    void set_material_override(Material* material);

    void set_drawable(Drawable* drawable);

    ModelInstance() = default;

    ~ModelInstance() {
        if (gl_vao != 0)
            glDeleteVertexArrays(1, &gl_vao);
    }

private:
    friend class Renderer;

    int32_t     material_id_override = -1;
    int32_t     id = -1;
    Drawable*   drawable = nullptr;
    GLuint      gl_vao = 0;
};

struct InstancedDrawable {
    glm::mat4               instance_world_transform = glm::identity<glm::mat4>();
    std::unique_ptr<Buffer> instance_transform_buffer{};
    uint32_t                n_instances;

    void set_drawable(Drawable* drawable);

    InstancedDrawable() = default;
    InstancedDrawable(InstancedDrawable &&o) : instance_world_transform{std::move(o.instance_world_transform)},
                                               instance_transform_buffer{std::move(o.instance_transform_buffer)}
    {
        std::swap(n_instances, o.n_instances);
        std::swap(id, o.id);
        std::swap(drawable, o.drawable);
        std::swap(gl_vao, o.gl_vao);
    }

    ~InstancedDrawable() {
        if (gl_vao != 0)
            glDeleteVertexArrays(1, &gl_vao);
    }

private:
    friend class Renderer;

    int32_t     id = -1;
    Drawable*   drawable = nullptr;
    GLuint      gl_vao = 0;
};

class Renderer : public Singleton<Renderer> {
public:
    Renderer(uint32_t width, uint32_t height);
    ~Renderer();

    void init();

    Material* create_material();
    void destroy_material(Material* material);

    LID create_point_light();
    LID create_directional_light();
    void set_light_position(LID lid, const glm::vec3& position);
    void set_light_color(LID lid, const glm::vec3& color);
    void set_light_ambient(LID lid, const glm::vec3& color);
    void destroy_light(LID lid);

    Drawable* create_model(VertexBuffer &&vb,
                           std::vector<Primitive> primitives,
                           std::vector<Material *> materials,
                           glm::vec3 bmin,
                           glm::vec3 bmax,
                           gl::CullMode cull,
                           gl::FrontFacing ff);
    void destroy_model(Drawable* d);

    ModelInstance* create_model_instance();
    void destroy_model_instance(ModelInstance* model);

    InstancedDrawable* create_instanced_drawable();
    void destroy_instanced_drawable(InstancedDrawable* drawable);

    RenderObj* create_render_obj();
    void destroy_render_obj(RenderObj* render_obj);

    MSIID create_mesh_instance();
    void set_mesh_instance_mesh(MSIID msiid, RenderObj* render_object);
    void set_mesh_instance_transform(MSIID msiid, const glm::mat4& transform);
    void destroy_mesh_instance(MSIID msiid);
    

    void render(const Camera &camera);

    void set_wireframe(bool enable);

    void set_resolution(uint32_t width, uint32_t height);
    float get_aspect_ratio() const noexcept {return width / (float)height;}

    void draw_screen_space_triangle();

    float ssao_radius = 0.5f;
    float ssao_bias = 0.025f;
    uint32_t ssao_kernel_samples = 32;

    float exposure      = .8f;
    float gamma         = 2.2f;
    float bloom_falloff = 0.9f;

    float sky_brightness = .22f;
    float sun_position  = .0f;
    float cloud_speed   = .1f;

    const uint32_t ShadowMapWidth = 4096;
    const uint32_t ShadowMapHeight = 4096;
    float shadow_bias_world = 0.17f;

    int32_t bloom_iterations = 2;
    float bloom_threshold = 2.17f;

private:

    void draw(Drawable* dr, const Program& prog, bool use_materials, GLuint gl_vao, int32_t material_override = -1, const Buffer* instance_buffer = nullptr, int32_t n_instances = -1);

    void update_material(mat_id_t material_slot, const MaterialData& material);

    // model, instance vertex data
    std::unordered_map<uint32_t, Drawable> models;
    uint32_t next_model_id = 1;

    // material management
    int32_t next_material_id = 1234;
    std::unordered_map<int32_t, Material> materials;
    std::array<MaterialData, MAX_N_MATERIALS> material_data_buffer; // cpu side material data array
    std::queue<mat_id_t> free_material_slots;

    // single instance of a drawable
    std::unordered_map<int32_t, ModelInstance> model_instances;
    uint32_t next_model_instance_id = 100;

    // instances of a drawable with instanced draw calls
    std::unordered_map<int32_t, InstancedDrawable> instanced_drawables;
    uint32_t next_instanced_drawable_id = 100;

    std::unordered_map<int32_t, RenderObj> meshes;
    uint32_t next_mesh_id = 1;

    std::unordered_map<int32_t, MeshInstance> mesh_instances;
    uint32_t next_mesh_instance_id = 1;

    // lights
    std::unordered_map<uint32_t, Light> point_lights;
    std::unique_ptr<Buffer> point_light_instance_buffer;
    std::unordered_map<uint32_t, DirectionalLight> directional_lights;
    uint32_t next_light_id = 1000;
    int32_t shadow_directional_light_id = -1;

    Program geometry_program;
    int gp_m_location;
    int gp_g_location;

    Program geometry_program_instanced;
    int gpi_m_location;

    Program depth_program;
    int sdp_m_location;
    int sdp_lpv_location;

    Program directional_lighting_program;
    int lp_p_location, lp_n_location, lp_as_location, lp_mt_location, lp_gao_location, lp_ls_location, lp_sdt_location;

    Program point_lighting_program;
    int plp_p_location, plp_n_location, plp_as_location, plp_mt_location, plp_m_location, plp_light_p_location, plp_light_c_location, plp_k_c_loc, plp_k_l_loc, plp_k_q_loc, plp_k_radius_loc;

    Program ssao_program;
    int ssao_p_loc, ssao_n_loc, ssao_tex_noise_loc, ssao_radius_loc, ssao_bias_loc, ssao_nSamples_loc;

    Program sky_program;
    int sky_time_loc, sky_cirrus_loc, sky_cumulus_loc, sky_sun_position_loc, sky_output_mul_loc;

    Program post_fx_bloom_combine_program;
    int post_fx_bc_hdrt_loc, post_fx_bc_emist_loc, post_fx_bc_thresh_loc;

    Program post_fx_bloom_blur;
    int post_fx_bb_hor_loc, post_fx_bb_bloom_in_loc;

    Program post_fx_program;
    int post_fx_gamma_loc, post_fx_exposure_loc, post_fx_bloom_falloff_loc, post_fx_hdrt_loc, post_fx_bloomt_loc;


    FBO g_buffer;
    FBO ssao_buffer;
    FBO lighting_buffer;
    FBO depth_buffer;
    FBO bloom_thresh_combine;
    std::array<FBO, 2> bloom_blur_swap_fbo;
    
    std::pair<VertexBuffer, GLuint> sst_vb;

    std::shared_ptr<Texture> shadow_depth_tex;

    std::shared_ptr<Texture> material_tex;
    std::shared_ptr<Texture> albedo_spec;
    std::shared_ptr<Texture> normals;
    std::shared_ptr<Texture> position;
    std::shared_ptr<Texture> emissive;

    std::shared_ptr<Texture> ssao_kernel_noise;
    std::shared_ptr<Texture> ssao_kernel_color;

    std::shared_ptr<Texture> one_p_black_tex;

    std::shared_ptr<Texture> hdr_texture;
    std::shared_ptr<Texture> hdr_combined;

    // texture in the 0 index is used for the bloom combined output
    std::array<std::shared_ptr<Texture>, 2> bloom_blur_swap_tex;

    Buffer shader_globals;
    ProgramUniformBlockDescription globals_desc;

    Buffer lighting_materials;
    ProgramUniformBlockDescription lighting_materials_desc;

    Buffer ssao_kernel_buffer;
    ProgramUniformBlockDescription ssao_kernel_desc;

    uint32_t width, height;
    bool wireframe = false;

    glm::mat4 point_light_geom_tr;
    Drawable* point_light_drawable;
    GLuint point_light_gl_vao = 0;

    int32_t default_material_id = 0;
};

}

#endif // EV2_RENDERER_H
