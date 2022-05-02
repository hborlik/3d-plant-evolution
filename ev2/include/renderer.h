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
 * @brief instance id
 * 
 */
struct IID {
    int32_t v = -1;

    bool is_valid() const noexcept {return v != -1;}
};

struct ModelInstance {
    glm::mat4   transform;
    Model*      model;
};

class Renderer : public Singleton<Renderer> {
public:
    Renderer(uint32_t width, uint32_t height, const std::filesystem::path& asset_path);

    void create_model(MID mid, std::shared_ptr<Model> model);

    IID create_model_instance(MID mid);
    void destroy_instance(MID mid);
    void set_instance_transform(int32_t iid, const glm::mat4& transform);

    void render(const Camera &camera);

    void set_wireframe(bool enable);

    void set_resolution(uint32_t width, uint32_t height);

    void draw_screen_space_triangle();

private:
    std::unordered_map<MID, std::shared_ptr<Model>> models;
    std::vector<ModelInstance> model_instances;

    Program geometry_program;
    int gp_m_location;
    int gp_g_location;

    Program lighting_program;
    int lp_p_location, lp_n_location, lp_as_location;

    FBO g_buffer;
    
    VertexBuffer sst_vb;

    std::shared_ptr<Texture> albedo_spec;
    std::shared_ptr<Texture> normals;
    std::shared_ptr<Texture> position;

    Buffer shader_globals;
    ProgramUniformBlockDescription globals_desc;

    uint32_t width, height;
    bool wireframe = false;

};

}

#endif // EV2_RENDERER_H
