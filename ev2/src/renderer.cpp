#include <renderer.h>

namespace ev2 {

Renderer::Renderer(uint32_t width, uint32_t height, const std::filesystem::path& asset_path) : 
    models{},
    model_instances{},
    geometry_program{"Geometry Program"},
    lighting_program{"Lighting Program"},
    g_buffer{gl::FBOTarget::RW},
    sst_vb{VertexBuffer::vbInitSST()},
    shader_globals{gl::BindingTarget::UNIFORM, gl::Usage::DYNAMIC_DRAW},
    width{width}, 
    height{height} {

    // set up FBO textures
    albedo_spec = std::make_shared<Texture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST);
    albedo_spec->set_data2D(gl::TextureInternalFormat::RGBA, width, height, gl::PixelFormat::RGBA, gl::PixelType::UNSIGNED_BYTE, nullptr);
    g_buffer.attach(albedo_spec, gl::FBOAttachment::COLOR2);

    normals = std::make_shared<Texture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST);
    normals->set_data2D(gl::TextureInternalFormat::RGBA16F, width, height, gl::PixelFormat::RGBA, gl::PixelType::FLOAT, nullptr);
    g_buffer.attach(normals, gl::FBOAttachment::COLOR1);

    position = std::make_shared<Texture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST);
    position->set_data2D(gl::TextureInternalFormat::RGBA16F, width, height, gl::PixelFormat::RGBA, gl::PixelType::FLOAT, nullptr);
    g_buffer.attach(position, gl::FBOAttachment::COLOR0);

    g_buffer.attach_renderbuffer(gl::RenderBufferInternalFormat::DEPTH_COMPONENT, width, height, gl::FBOAttachment::DEPTH);

    if (!g_buffer.check())
        throw engine_exception{"Framebuffer is not complete"};


    // set up programs

    ev2::ShaderPreprocessor prep{asset_path / "shader"};
    prep.load_includes();

    geometry_program.loadShader(gl::GLSLShaderType::VERTEX_SHADER, "geometry.glsl.vert", prep);
    geometry_program.loadShader(gl::GLSLShaderType::FRAGMENT_SHADER, "geometry.glsl.frag", prep);
    geometry_program.link();

    std::cout << geometry_program << std::endl;

    gp_m_location = geometry_program.getUniformInfo("M").Location;
    gp_g_location = geometry_program.getUniformInfo("G").Location;


    lighting_program.loadShader(gl::GLSLShaderType::VERTEX_SHADER, "sst.glsl.vert", prep);
    lighting_program.loadShader(gl::GLSLShaderType::FRAGMENT_SHADER, "lighting.glsl.frag", prep);
    lighting_program.link();

    lp_p_location = lighting_program.getUniformInfo("gPosition").Location;
    lp_n_location = lighting_program.getUniformInfo("gNormal").Location;
    lp_as_location = lighting_program.getUniformInfo("gAlbedoSpec").Location;

    // program globals
    globals_desc = geometry_program.getUniformBlockInfo("Globals");
    shader_globals.Allocate(globals_desc.block_size);
}

void Renderer::create_model(MID mid, std::shared_ptr<Model> model) {
    models.insert_or_assign(mid, model);
}

IID Renderer::create_model_instance(MID mid) {
    std::size_t id = model_instances.size();
    
    auto model = models.find(mid);
    if (model != models.end()) {
        ModelInstance mi{};
        mi.transform = glm::identity<glm::mat4>();
        mi.model = (*model).second.get();
        model_instances.push_back(mi);

        return {id};
    }

    return {-1};
}

void Renderer::set_instance_transform(int32_t iid, const glm::mat4& transform) {
    if (iid < 0 || iid >= model_instances.size())
        return;
    
    ModelInstance& mi = model_instances[iid];
    mi.transform = transform;
}

void Renderer::render(const Camera &camera) {

    // update globals buffer with frame info
    globals_desc.setShaderParameter("P", camera.getProjection(), shader_globals);
    globals_desc.setShaderParameter("V", camera.getView(), shader_globals);
    globals_desc.setShaderParameter("CameraPos", camera.getPosition(), shader_globals);

    // render all geometry to g buffer
    g_buffer.bind();
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    if (wireframe)
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    else
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

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
    geometry_program.unbind();

    // glFlush();

    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    // globals_desc.bind_buffer(shader_globals);
    lighting_program.use();

    if (lp_p_location >= 0) {
        glActiveTexture(GL_TEXTURE0);
        position->bind();
        gl::glUniformSampler(0, lp_p_location);
    }

    if (lp_n_location >= 0) {
        glActiveTexture(GL_TEXTURE1);
        normals->bind();
        gl::glUniformSampler(1, lp_n_location);
    }

    if (lp_as_location >= 0) {
        glActiveTexture(GL_TEXTURE2);
        albedo_spec->bind();
        gl::glUniformSampler(2, lp_as_location);
    }

    draw_screen_space_triangle();

    lighting_program.unbind();
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
    sst_vb.bind();
    GL_CHECKED_CALL(glDrawArrays(GL_TRIANGLES, 0, 3));
    sst_vb.unbind();
}

}