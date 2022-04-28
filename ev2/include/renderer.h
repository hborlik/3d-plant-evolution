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

#include <singleton.h>
#include <mesh.h>
#include <shader.h>
#include <texture.h>
#include <camera.h>

namespace ev2 {

class Renderer : public Singleton<Renderer> {
public:
    Renderer(uint32_t width, uint32_t height);

    void create_model(uint32_t mid, std::shared_ptr<Model> model);
    

    void render(const Camera &camera);

    void set_wireframe(bool enable);

    void set_resolution(uint32_t width, uint32_t height);

    void draw_screen_space_triangle();

private:
    std::unordered_map<uint32_t, std::shared_ptr<Model>> models;
    std::vector<ModelInstance> model_instances;

    Program geometry_program;
    int gp_m_location;
    int gp_g_location;

    Program lighting_program;
    int lp_pos_location, lp_nor_location, lp_als_location;

    FBO g_buffer;

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
