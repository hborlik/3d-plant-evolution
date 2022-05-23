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
#include <vertex_buffer.h>
#include <shader.h>
#include <texture.h>
#include <camera.h>
#include <material.h>


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
    glm::vec3 k{0.0, 0.2, 1.8}; // k_c, k_l, k_q
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

struct VBID { // vertex buffer id
    int32_t v = -1;

    bool is_valid() const noexcept {return v != -1;}
};

struct MSID { // mesh id
    int32_t v = -1;

    bool is_valid() const noexcept {return v != -1;}
};

struct MSIID { // mesh instance id
    int32_t v = -1;

    bool is_valid() const noexcept {return v != -1;}
};

struct MeshPrimitive {
    MeshPrimitive() noexcept = default;
    MeshPrimitive(VBID vbid, int32_t material_id, int32_t indices = -1)
        : vbid{vbid}, indices{indices}, material_id{material_id} {}

    ~MeshPrimitive() {
        if (gl_vao != 0)
            glDeleteVertexArrays(1, &gl_vao);
    }

    std::map<int, int>      attributes;         // map of attribute location (engine value like VERTEX_BINDING_LOCATION to a buffer in vb)
    VBID                    vbid{};             // id of used vertex buffer
    int32_t                 indices = -1;       // ind of index buffer in vb
    int32_t                 material_id = 0;

    GLuint gl_vao = 0;                          // vao for primitive (internal use)
};

struct Mesh {
    std::vector<MeshPrimitive>      primitives{};
    gl::CullMode                    cull_mode = gl::CullMode::NONE;
    gl::FrontFacing                 front_facing = gl::FrontFacing::CCW;
};

struct MeshInstance {
    glm::mat4               transform = glm::identity<glm::mat4>();
    Mesh*                   mesh = nullptr;
};

struct Primitive {
    size_t  start_index = 0;
    size_t  num_elements = 0;
    int32_t material_id = 0;
};

struct Drawable {
    Drawable(VertexBuffer&& vb, std::vector<Primitive> primitives, glm::vec3 bmin, glm::vec3 bmax, gl::CullMode cull, gl::FrontFacing ff) : 
        vertex_buffer{std::move(vb)}, primitives{std::move(primitives)}, bmin{bmin}, bmax{bmax}, cull_mode{cull}, front_facing{ff} {}
    VertexBuffer vertex_buffer;
    std::vector<Primitive> primitives;

    glm::vec3 bmin, bmax;

    gl::CullMode cull_mode = gl::CullMode::BACK;
    gl::FrontFacing front_facing = gl::FrontFacing::CCW;

    int32_t material_offset = 0;
    float vertex_color_weight = 0.f;

    void draw(const Program& prog, int32_t material_override = -1);
};

/**
 * @brief material data as represented on gpu
 * 
 */
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
        nMat.metallic = mat.metallic;
        nMat.subsurface = mat.subsurface;
        nMat.specular = mat.specular;
        nMat.roughness = mat.roughness;
        nMat.specularTint = mat.specularTint;
        nMat.clearcoat = mat.clearcoat;
        nMat.clearcoatGloss = mat.clearcoatGloss;
        nMat.anisotropic = mat.anisotropic;
        nMat.sheen = mat.sheen;
        nMat.sheenTint = mat.sheenTint;
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
    Renderer(uint32_t width, uint32_t height);

    void init();

    void update_material(int32_t material_id, const MaterialData& material);
    int32_t create_material(const MaterialData& material);

    LID create_point_light();
    LID create_directional_light();
    void set_light_position(LID lid, const glm::vec3& position);
    void set_light_color(LID lid, const glm::vec3& color);
    void set_light_ambient(LID lid, const glm::vec3& color);
    void destroy_light(LID lid);

    MID create_model(std::shared_ptr<Drawable> d);
    void set_model_vertex_color_diffuse_weight(MID mid, float weight);

    IID create_model_instance();
    void set_instance_model(IID iid, MID mid);
    void set_instance_transform(IID iid, const glm::mat4& transform);


    VBID create_vertex_buffer();
    VertexBuffer* get_vertex_buffer(VBID vbid);
    void destroy_vertex_buffer(VBID vbid);

    MSID create_mesh();
    void set_mesh_primitives(MSID mesh_id, const std::vector<MeshPrimitive>& primitives);
    void set_mesh_cull_mode(MSID mesh_id, gl::CullMode cull_mode);
    void set_mesh_front_facing(MSID mesh_id, gl::FrontFacing front_facing);
    void destroy_mesh(MSID mesh_id);

    MSIID create_mesh_instance();
    void set_mesh_instance_mesh(MSIID msiid, MSID msid);
    void set_mesh_instance_transform(MSIID msiid, const glm::mat4& transform);
    void destroy_mesh_instance(MSIID msiid);

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

    float ssao_radius = 0.5f;
    float ssao_bias = 0.025f;
    uint32_t ssao_kernel_samples = 32;

private:
    // model, instance vertex data
    std::unordered_map<MID, std::shared_ptr<Drawable>> models;
    uint32_t next_model_id = 1;

    std::vector<MaterialData> materials;
    int next_free_mat = 1;

    std::unordered_map<int32_t, ModelInstance> model_instances;
    uint32_t next_instance_id = 100;

    // Mesh, vb vertex data
    std::unordered_map<int32_t, VertexBuffer> vertex_buffers;
    uint32_t next_vb_id = 1;

    std::unordered_map<int32_t, Mesh> meshes;
    uint32_t next_mesh_id = 1;

    std::unordered_map<int32_t, MeshInstance> mesh_instances;
    uint32_t next_mesh_instance_id = 1;

    // lights
    std::unordered_map<uint32_t, Light> point_lights;
    std::unordered_map<uint32_t, DirectionalLight> directional_lights;
    uint32_t next_light_id = 1000;

    Program geometry_program;
    int gp_m_location;
    int gp_g_location;

    Program DepthProg;

    int sdp_m_location;
    int sdp_lp_location;
    int sdp_lv_location;

//    int gp_m_location;
//    int gp_g_location;

    Program directional_lighting_program;
    int lp_p_location, lp_n_location, lp_as_location, lp_mt_location, lp_gao_location, lp_ls_location, lp_sdt_location;

    Program point_lighting_program;
    int plp_p_location, plp_n_location, plp_as_location, plp_mt_location, plp_m_location, plp_light_p_location, plp_light_c_location, plp_k_c_loc, plp_k_l_loc, plp_k_q_loc;

    Program ssao_program;
    int ssao_p_loc, ssao_n_loc, ssao_tex_noise_loc, ssao_radius_loc, ssao_bias_loc, ssao_nSamples_loc;

    FBO g_buffer;
    FBO ssao_buffer;
    FBO d_buffer;
    
    VertexBuffer sst_vb;

    GLuint depthMapFBO;
	const GLuint S_WIDTH = 1024, S_HEIGHT = 1024;
	GLuint depthMap;

    std::shared_ptr<Texture> shadow_depth_tex;
    std::shared_ptr<Texture> material_tex;
    std::shared_ptr<Texture> albedo_spec;
    std::shared_ptr<Texture> normals;
    std::shared_ptr<Texture> position;

    std::shared_ptr<Texture> ssao_kernel_noise;
    std::shared_ptr<Texture> ssao_kernel_color;

    Buffer shader_globals;
    ProgramUniformBlockDescription globals_desc;

    Buffer lighting_materials;
    ProgramUniformBlockDescription lighting_materials_desc;

    Buffer ssao_kernel_buffer;
    ProgramUniformBlockDescription ssao_kernel_desc;

    uint32_t width, height;
    bool wireframe = false;

    MID point_light_geometry_id;
    glm::mat4 point_light_geom_tr;
    Drawable* point_light_drawable;
};

}

#endif // EV2_RENDERER_H
