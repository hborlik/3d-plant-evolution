#include <renderer.h>

namespace ev2 {

Renderer::Renderer(uint32_t width, uint32_t height) : 
    models{},
    model_instances{},
    geometry_program{"Geometry Program"},
    lighting_program{"Lighting Program"},
    g_buffer{gl::FBOTarget::RW},
    shader_globals{gl::BindingTarget::UNIFORM, gl::Usage::DYNAMIC_DRAW},
    width{width}, 
    height{height} {

    // set up FBO textures
    albedo_spec = std::make_shared<Texture>(gl::TextureType::TEXTURE_2D);
    albedo_spec->set_data2D(gl::TextureInternalFormat::RGBA16F, width, height, gl::PixelFormat::RGBA, gl::PixelType::UNSIGNED_BYTE, nullptr);
    g_buffer.attach(albedo_spec, gl::FBOAttachment::COLOR0);

    normals = std::make_shared<Texture>(gl::TextureType::TEXTURE_2D);
    normals->set_data2D(gl::TextureInternalFormat::RGBA16F, width, height, gl::PixelFormat::RGBA, gl::PixelType::FLOAT, nullptr);
    g_buffer.attach(normals, gl::FBOAttachment::COLOR1);

    position = std::make_shared<Texture>(gl::TextureType::TEXTURE_2D);
    position->set_data2D(gl::TextureInternalFormat::RGBA16F, width, height, gl::PixelFormat::RGBA, gl::PixelType::FLOAT, nullptr);
    g_buffer.attach(position, gl::FBOAttachment::COLOR2);

    g_buffer.attach_renderbuffer(gl::RenderBufferInternalFormat::DEPTH24_STENCIL8, width, height, gl::FBOAttachment::DEPTH_STENCIL);

    if (!g_buffer.check())
        throw engine_exception{"Framebuffer is not complete"};


    // set up programs

    geometry_program.loadShader(gl::GLSLShaderType::VERTEX_SHADER, "geometry.glsl.vert");
    geometry_program.loadShader(gl::GLSLShaderType::VERTEX_SHADER, "geometry.glsl.frag");
    geometry_program.link();

    gp_m_location = geometry_program.getUniformInfo("M").Location;
    gp_g_location = geometry_program.getUniformInfo("G").Location;


    lighting_program.loadShader(gl::GLSLShaderType::VERTEX_SHADER, "sst.glsl.vert");
    lighting_program.loadShader(gl::GLSLShaderType::VERTEX_SHADER, "lighting.glsl.frag");
    lighting_program.link();

    lp_pos_location = lighting_program.getUniformInfo("gPosition").Location;
    lp_nor_location = lighting_program.getUniformInfo("gNormal").Location;
    lp_als_location = lighting_program.getUniformInfo("gAlbedoSpec").Location;
}

void Renderer::render(const Camera &camera) {

    // update globals buffer with frame info
    globals_desc.setShaderParameter("P", camera.getProjection(), shader_globals);
    globals_desc.setShaderParameter("V", camera.getView(), shader_globals);
    globals_desc.setShaderParameter("CameraPos", camera.getPosition(), shader_globals);

    // render all geometry to g buffer
    g_buffer.bind();
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    geometry_program.use();

    // bind global shader UBO to shader
    globals_desc.bind_buffer(shader_globals);

    for (auto &m : model_instances) {
        const glm::mat3 G = glm::inverse(glm::transpose(glm::mat3(m.transform)));

        ev2::gl::glUniform(m.transform, gp_m_location);
        ev2::gl::glUniform(G, gp_g_location);

        m.model->draw(geometry_program);
    }

    g_buffer.unbind();

    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    lighting_program.use();
    globals_desc.bind_buffer(shader_globals);

    glActiveTexture(GL_TEXTURE0);
    position->bind();
    glActiveTexture(GL_TEXTURE1);
    normals->bind();
    glActiveTexture(GL_TEXTURE2);
    albedo_spec->bind();

    gl::glUniform(position->get_handle(), lp_pos_location);
    gl::glUniform(normals->get_handle(), lp_nor_location);
    gl::glUniform(albedo_spec->get_handle(), lp_als_location);

    draw_screen_space_triangle();
}

void Renderer::set_wireframe(bool enable) {
    wireframe = enable;
}

void Renderer::set_resolution(uint32_t width, uint32_t height) {
    this->width = width;
    this->height = height;

    g_buffer.resize_all(width, height);
}

void Renderer::draw_screen_space_triangle() {
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

}