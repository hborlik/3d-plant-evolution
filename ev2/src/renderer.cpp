#include <renderer.h>

#include <random>
#include <cmath>

#include <resource.h>

namespace ev2 {

void Drawable::draw(const Program& prog, int32_t material_override) {
    vertex_buffer.bind();
    if (cull_mode == gl::CullMode::NONE) {
        glDisable(GL_CULL_FACE);
    } else {
        glEnable(GL_CULL_FACE);
        glCullFace((GLenum)cull_mode);
    }
    glFrontFace((GLenum)front_facing);
    // TODO: support for multiple index buffers
    if (vertex_buffer.get_indexed() != -1) {
        // draw indexed arrays
        for (auto& m : meshes) {
            // prog.applyMaterial(materials[m.material_id]);

            // TODO does this go here?
            int loc = prog.getUniformInfo("materialId").Location;
            if (loc != -1) {
                GL_CHECKED_CALL(glUniform1ui(loc, material_override > -1 ? material_override : m.material_id + material_offset));
            }

            loc = prog.getUniformInfo("vertex_color_weight").Location;
            if (loc != -1) {
                GL_CHECKED_CALL(glUniform1f(loc, vertex_color_weight));
            }

            vertex_buffer.get_buffer(vertex_buffer.get_indexed()).Bind(); // bind index buffer (again, @Windows)
            glDrawElements(GL_TRIANGLES, m.num_elements, GL_UNSIGNED_INT, (void*)0);
        }
    } else {
        for (auto& m : meshes) {
            // prog.applyMaterial(materials[m.material_id]);

            // TODO does this go here?
            int loc = prog.getUniformInfo("materialId").Location;
            if (loc != -1) {
                GL_CHECKED_CALL(glUniform1ui(loc, material_override > -1 ? material_override : m.material_id + material_offset));
            }

            loc = prog.getUniformInfo("vertex_color_weight").Location;
            if (loc != -1) {
                GL_CHECKED_CALL(glUniform1f(loc, vertex_color_weight));
            }

            glDrawArrays(GL_TRIANGLES, m.start_index, m.num_elements);
        }
    }
    vertex_buffer.unbind();
}

Renderer::Renderer(uint32_t width, uint32_t height, const std::filesystem::path& asset_path) : 
    models{},
    model_instances{},
    geometry_program{"Geometry Program"},
    directional_lighting_program{"Lighting Program"},
    g_buffer{gl::FBOTarget::RW},
    ssao_buffer{gl::FBOTarget::RW},
    sst_vb{VertexBuffer::vbInitSST()},
    shader_globals{gl::BindingTarget::UNIFORM, gl::Usage::DYNAMIC_DRAW},
    lighting_materials{gl::BindingTarget::UNIFORM, gl::Usage::DYNAMIC_DRAW},
    ssao_kernel_buffer{gl::BindingTarget::UNIFORM, gl::Usage::DYNAMIC_DRAW},
    width{width}, 
    height{height} {

    // precomputed (static) data
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
    std::default_random_engine generator;
    std::vector<glm::vec3> ssaoKernel;
    auto lerp = [](float a, float b, float f) -> float
    {
        return a + f * (b - a);
    };

    for (unsigned int i = 0; i < 64; ++i)
    {
        glm::vec3 sample(
            randomFloats(generator) * 2.0 - 1.0, 
            randomFloats(generator) * 2.0 - 1.0, 
            randomFloats(generator)
        );
        sample  = glm::normalize(sample);
        sample *= randomFloats(generator);
        // bias samples towards the central pixel
        float scale = (float)i / 32.0;
        scale   = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }
    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++)
    {
        glm::vec3 noise(
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator) * 2.0 - 1.0,
            0.0f); 
        ssaoNoise.push_back(noise);
    }

    // ssao tiling noise texture
    ssao_kernel_noise = std::make_shared<Texture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST);
    ssao_kernel_noise->set_data2D(gl::TextureInternalFormat::RGBA16F, 4, 4, gl::PixelFormat::RGB, gl::PixelType::FLOAT, (unsigned char*)ssaoNoise.data());
    ssao_kernel_noise->set_wrap_mode(gl::TextureParamWrap::TEXTURE_WRAP_S, gl::TextureWrapMode::REPEAT);
    ssao_kernel_noise->set_wrap_mode(gl::TextureParamWrap::TEXTURE_WRAP_T, gl::TextureWrapMode::REPEAT);

    ssao_kernel_color = std::make_shared<Texture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST);
    ssao_kernel_color->set_data2D(gl::TextureInternalFormat::RED, width, height, gl::PixelFormat::RED, gl::PixelType::FLOAT, nullptr);

    ssao_buffer.attach(ssao_kernel_color, gl::FBOAttachment::COLOR0, 0);

    if (!ssao_buffer.check())
        throw engine_exception{"Framebuffer is not complete"};

    // set up FBO textures
    material_tex = std::make_shared<Texture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST);
    material_tex->set_data2D(gl::TextureInternalFormat::R8UI, width, height, gl::PixelFormat::RED_INTEGER, gl::PixelType::UNSIGNED_BYTE, nullptr);
    g_buffer.attach(material_tex, gl::FBOAttachment::COLOR3, 3);

    albedo_spec = std::make_shared<Texture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST);
    albedo_spec->set_data2D(gl::TextureInternalFormat::RGBA, width, height, gl::PixelFormat::RGBA, gl::PixelType::UNSIGNED_BYTE, nullptr);
    g_buffer.attach(albedo_spec, gl::FBOAttachment::COLOR2, 2);

    normals = std::make_shared<Texture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST);
    normals->set_data2D(gl::TextureInternalFormat::RGBA16F, width, height, gl::PixelFormat::RGBA, gl::PixelType::FLOAT, nullptr);
    g_buffer.attach(normals, gl::FBOAttachment::COLOR1, 1);

    position = std::make_shared<Texture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST);
    position->set_data2D(gl::TextureInternalFormat::RGBA16F, width, height, gl::PixelFormat::RGBA, gl::PixelType::FLOAT, nullptr);
    g_buffer.attach(position, gl::FBOAttachment::COLOR0, 0);

    g_buffer.attach_renderbuffer(gl::RenderBufferInternalFormat::DEPTH_COMPONENT, width, height, gl::FBOAttachment::DEPTH);

    if (!g_buffer.check())
        throw engine_exception{"Framebuffer is not complete"};


    // set up programs

    ev2::ShaderPreprocessor prep{asset_path / "shader"};
    prep.load_includes();

    geometry_program.loadShader(gl::GLSLShaderType::VERTEX_SHADER, "geometry.glsl.vert", prep);
    geometry_program.loadShader(gl::GLSLShaderType::FRAGMENT_SHADER, "geometry.glsl.frag", prep);
    geometry_program.link();

    gp_m_location = geometry_program.getUniformInfo("M").Location;
    gp_g_location = geometry_program.getUniformInfo("G").Location;


    directional_lighting_program.loadShader(gl::GLSLShaderType::VERTEX_SHADER, "sst.glsl.vert", prep);
    directional_lighting_program.loadShader(gl::GLSLShaderType::FRAGMENT_SHADER, "directional.glsl.frag", prep);
    directional_lighting_program.link();

    lp_p_location = directional_lighting_program.getUniformInfo("gPosition").Location;
    lp_n_location = directional_lighting_program.getUniformInfo("gNormal").Location;
    lp_as_location = directional_lighting_program.getUniformInfo("gAlbedoSpec").Location;
    lp_mt_location = directional_lighting_program.getUniformInfo("gMaterialTex").Location;
    lp_gao_location = directional_lighting_program.getUniformInfo("gAO").Location;


    point_lighting_program.loadShader(gl::GLSLShaderType::VERTEX_SHADER, "point_lighting.glsl.vert", prep);
    point_lighting_program.loadShader(gl::GLSLShaderType::FRAGMENT_SHADER, "point_lighting.glsl.frag", prep);
    point_lighting_program.link();

    plp_p_location = point_lighting_program.getUniformInfo("gPosition").Location;
    plp_n_location = point_lighting_program.getUniformInfo("gNormal").Location;
    plp_as_location = point_lighting_program.getUniformInfo("gAlbedoSpec").Location;
    plp_mt_location = point_lighting_program.getUniformInfo("gMaterialTex").Location;
    plp_m_location = point_lighting_program.getUniformInfo("M").Location;
    plp_light_p_location = point_lighting_program.getUniformInfo("lightPos").Location;
    plp_light_c_location = point_lighting_program.getUniformInfo("lightColor").Location;
    plp_k_c_loc = point_lighting_program.getUniformInfo("k_c").Location;
    plp_k_l_loc = point_lighting_program.getUniformInfo("k_l").Location;
    plp_k_q_loc = point_lighting_program.getUniformInfo("k_q").Location;


    ssao_program.loadShader(gl::GLSLShaderType::VERTEX_SHADER, "sst.glsl.vert", prep);
    ssao_program.loadShader(gl::GLSLShaderType::FRAGMENT_SHADER, "ssao.glsl.frag", prep);
    ssao_program.link();

    ssao_p_loc = ssao_program.getUniformInfo("gPosition").Location;
    ssao_n_loc = ssao_program.getUniformInfo("gNormal").Location;
    ssao_tex_noise_loc = ssao_program.getUniformInfo("texNoise").Location;
    ssao_radius_loc = ssao_program.getUniformInfo("radius").Location;
    ssao_bias_loc = ssao_program.getUniformInfo("bias").Location;
    ssao_nSamples_loc = ssao_program.getUniformInfo("nSamples").Location;

    // program block inputs
    globals_desc = geometry_program.getUniformBlockInfo("Globals");
    shader_globals.Allocate(globals_desc.block_size);

    lighting_materials_desc = directional_lighting_program.getUniformBlockInfo("MaterialsInfo");
    lighting_materials.Allocate(lighting_materials_desc.block_size);

    ssao_kernel_desc = ssao_program.getUniformBlockInfo("Samples");
    ssao_kernel_buffer.Allocate(ssao_kernel_desc.block_size);
    auto tgt_layout = ssao_kernel_desc.getLayout("samples[0]");
    ssao_kernel_buffer.SubData(ssaoKernel, tgt_layout.Offset, tgt_layout.ArrayStride);

    materials = std::vector<MaterialData>(100, MaterialData{});
    
    int i = 0;
    for (auto& m : materials) {
        update_material(i, m);
    }

    // light geometry
    std::shared_ptr<Model> sphere = loadObj("sphere.obj", asset_path / "models", nullptr);
    point_light_geometry_id = create_model(sphere);
    glm::vec3 scaling = glm::vec3{2} / (sphere->bmax - sphere->bmin);
    point_light_geom_tr = glm::scale(glm::identity<glm::mat4>(), scaling);
    auto itr = models.find(point_light_geometry_id);
    point_light_drawable = itr->second.get();
    point_light_drawable->front_facing = gl::FrontFacing::CW; // render back facing only
}

void Renderer::update_material(int32_t material_id, const MaterialData& material) {
    materials[material_id] = material;
    lighting_materials_desc.setShaderParameter("materials[" + std::to_string(material_id) + "].diffuse", material.diffuse, lighting_materials);
    lighting_materials_desc.setShaderParameter("materials[" + std::to_string(material_id) + "].metallic", material.metallic, lighting_materials);
    lighting_materials_desc.setShaderParameter("materials[" + std::to_string(material_id) + "].subsurface", material.subsurface, lighting_materials);
    lighting_materials_desc.setShaderParameter("materials[" + std::to_string(material_id) + "].specular", material.specular, lighting_materials);
    lighting_materials_desc.setShaderParameter("materials[" + std::to_string(material_id) + "].roughness", material.roughness, lighting_materials);
    lighting_materials_desc.setShaderParameter("materials[" + std::to_string(material_id) + "].specularTint", material.specularTint, lighting_materials);
    lighting_materials_desc.setShaderParameter("materials[" + std::to_string(material_id) + "].clearcoat", material.clearcoat, lighting_materials);
    lighting_materials_desc.setShaderParameter("materials[" + std::to_string(material_id) + "].clearcoatGloss", material.clearcoatGloss, lighting_materials);
    lighting_materials_desc.setShaderParameter("materials[" + std::to_string(material_id) + "].anisotropic", material.anisotropic, lighting_materials);
    lighting_materials_desc.setShaderParameter("materials[" + std::to_string(material_id) + "].sheen", material.sheen, lighting_materials);
    lighting_materials_desc.setShaderParameter("materials[" + std::to_string(material_id) + "].sheenTint", material.sheenTint, lighting_materials);

}

int32_t Renderer::create_material(const MaterialData& material) {
    if (next_free_mat < materials.size()) {
        int32_t mat_id = next_free_mat++;
        update_material(mat_id, material);
        return mat_id;
    } else
        return -1;
}

LID Renderer::create_point_light() {
    uint32_t nlid = next_light_id++;
    Light l{};
    point_lights.insert_or_assign(nlid, l);
    return {LID::Point, nlid};
}

LID Renderer::create_directional_light() {
    uint32_t nlid = next_light_id++;
    DirectionalLight l{};
    directional_lights.insert_or_assign(nlid, l);
    return {LID::Directional, nlid};
}

void Renderer::set_light_position(LID lid, const glm::vec3& position) {
    if (!lid.is_valid())
        return;
    
    switch(lid._type) {
        case LID::Point:
        {
            auto mi = point_lights.find(lid._v);
            if (mi != point_lights.end()) {
                mi->second.position = position;
            }
        }
        break;
        case LID::Directional:
        {
            auto mi = directional_lights.find(lid._v);
            if (mi != directional_lights.end()) {
                mi->second.direction = glm::normalize(position);
            }
        }
        break;
    };
}

void Renderer::set_light_color(LID lid, const glm::vec3& color) {
    if (!lid.is_valid())
        return;
    
    switch(lid._type) {
        case LID::Point:
        {
            auto mi = point_lights.find(lid._v);
            if (mi != point_lights.end()) {
                mi->second.color = color;
            }
        }
        break;
        case LID::Directional:
        {
            auto mi = directional_lights.find(lid._v);
            if (mi != directional_lights.end()) {
                mi->second.color = color;
            }
        }
        break;
    };
}

void Renderer::set_light_ambient(LID lid, const glm::vec3& color) {
    if (!lid.is_valid())
        return;
    
    switch(lid._type) {
        case LID::Directional:
        {
            auto mi = directional_lights.find(lid._v);
            if (mi != directional_lights.end()) {
                mi->second.ambient = color;
            }
        }
        break;
    };
}

void Renderer::destroy_light(LID lid) {
    if (!lid.is_valid())
        return;
    directional_lights.erase(lid._v);
    point_lights.erase(lid._v);
}

MID Renderer::create_model(std::shared_ptr<Model> model) {
    assert(model->bufferFormat == VertexFormat::Array);

    auto meshes = model->meshes;
    // for (auto& m : meshes) {
    //     m.material_id = m.material_id - 1;
    // }

    std::shared_ptr<Drawable> d = std::make_shared<Drawable>(
        VertexBuffer::vbInitArrayVertexData(model->buffer),
        std::move(meshes),
        model->bmin,
        model->bmax,
        gl::CullMode::BACK,
        gl::FrontFacing::CCW
    );
    d->material_offset = 0;

    return create_model(d);
}

MID Renderer::create_model(std::shared_ptr<Drawable> d) {
    MID nmid = {next_model_id++};
    models.insert_or_assign(nmid, d);
    return nmid;
}

void Renderer::set_model_vertex_color_diffuse_weight(MID mid, float weight) {
    if (!mid.is_valid())
        return;
    auto model = models.find(mid);
    if (model != models.end()) {
        model->second->vertex_color_weight = weight;
    }
}

IID Renderer::create_model_instance() {
    int32_t id = next_instance_id++;
    ModelInstance mi{};
    model_instances.emplace(id, mi);

    return {id};
}

void Renderer::set_instance_model(IID iid, MID mid) {
    if (!iid.is_valid())
        return;
    auto model = models.find(mid);
    if (model != models.end()) {
        model_instances[iid.v].drawable = (*model).second.get();
    }
}

void Renderer::set_instance_material_override(IID iid, int32_t material_override) {
    if (!iid.is_valid())
        return;
    
    auto mi = model_instances.find(iid.v);
    if (mi != model_instances.end()) {
        mi->second.material_override = material_override;
    }
}

void Renderer::destroy_instance(IID iid) {
    if (!iid.is_valid())
        return;
    model_instances.erase(iid.v);
}

void Renderer::set_instance_transform(IID iid, const glm::mat4& transform) {
    if (!iid.is_valid())
        return;
    
    auto mi = model_instances.find(iid.v);
    if (mi != model_instances.end()) {
        mi->second.transform = transform;
    }
}

void Renderer::render(const Camera &camera) {

    // update globals buffer with frame info
    globals_desc.setShaderParameter("P", camera.get_projection(), shader_globals);
    globals_desc.setShaderParameter("View", camera.get_view(), shader_globals);
    globals_desc.setShaderParameter("CameraPos", camera.get_position(), shader_globals);

    // render all geometry to g buffer
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_DITHER);

    if (wireframe)
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    else
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    geometry_program.use();
    g_buffer.bind();
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // bind global shader UBO to shader
    globals_desc.bind_buffer(shader_globals);

    for (auto &mPair : model_instances) {
        auto& m = mPair.second;
        if (m.drawable) {
            const glm::mat3 G = glm::inverse(glm::transpose(glm::mat3(m.transform)));

            ev2::gl::glUniform(m.transform, gp_m_location);
            ev2::gl::glUniform(G, gp_g_location);

            m.drawable->draw(geometry_program, m.material_override);
        }
    }

    geometry_program.unbind();
    g_buffer.unbind();

    // glFlush();

    // ssao pass
    ssao_program.use();
    ssao_buffer.bind();

    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST); // overdraw
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    if (ssao_p_loc >= 0) {
        glActiveTexture(GL_TEXTURE0);
        position->bind();
        gl::glUniformSampler(0, ssao_p_loc);
    }

    if (ssao_n_loc >= 0) {
        glActiveTexture(GL_TEXTURE1);
        normals->bind();
        gl::glUniformSampler(1, ssao_n_loc);
    }

    if (ssao_tex_noise_loc >= 0) {
        glActiveTexture(GL_TEXTURE2);
        ssao_kernel_noise->bind();
        gl::glUniformSampler(2, ssao_tex_noise_loc);
    }

    gl::glUniformf(ssao_bias, ssao_bias_loc);
    gl::glUniformf(ssao_radius, ssao_radius_loc);
    gl::glUniformui(ssao_kernel_samples, ssao_nSamples_loc);

    ssao_kernel_desc.bind_buffer(ssao_kernel_buffer);

    draw_screen_space_triangle();

    ssao_program.unbind();
    ssao_buffer.unbind();

    // lighting pass
    // glFlush();

    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST); // overdraw
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    // add all lighting contributions
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);

    // setup lighting program
    directional_lighting_program.use();
    globals_desc.bind_buffer(shader_globals);
    lighting_materials_desc.bind_buffer(lighting_materials);


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

    if (lp_mt_location >= 0) {
        glActiveTexture(GL_TEXTURE3);
        material_tex->bind();
        gl::glUniformSampler(3, lp_mt_location);
    }

    if (lp_gao_location >= 0) {
        glActiveTexture(GL_TEXTURE4);
        ssao_kernel_color->bind();
        gl::glUniformSampler(4, lp_gao_location);
    }

    for (auto& litr : directional_lights) {
        auto& l = litr.second;
        gl::glUniform(glm::normalize(l.direction), directional_lighting_program.getUniformInfo("lightDir").Location);
        gl::glUniform(l.color, directional_lighting_program.getUniformInfo("lightColor").Location);
        gl::glUniform(l.ambient, directional_lighting_program.getUniformInfo("lightAmbient").Location);
        
        draw_screen_space_triangle();
    }

    directional_lighting_program.unbind();

    // pointlight pass
    point_lighting_program.use();

    if (plp_n_location >= 0) {
        glActiveTexture(GL_TEXTURE1);
        normals->bind();
        gl::glUniformSampler(1, plp_n_location);
    }

    if (plp_as_location >= 0) {
        glActiveTexture(GL_TEXTURE2);
        albedo_spec->bind();
        gl::glUniformSampler(2, plp_as_location);
    }

    if (plp_mt_location >= 0) {
        glActiveTexture(GL_TEXTURE3);
        material_tex->bind();
        gl::glUniformSampler(3, plp_mt_location);
    }

    for (auto& litr : point_lights) {
        auto& l = litr.second;

        // from https://learnopengl.com/Advanced-Lighting/Deferred-Shading
        float constant  = l.k.x; 
        float linear    = l.k.y;
        float quadratic = l.k.z;
        float lightMax  = std::fmaxf(std::fmaxf(l.color.r, l.color.g), l.color.b);
        float radius    = 
        (-linear +  sqrtf(linear * linear - 4 * quadratic * (constant - (256.0 / 5.0) * lightMax))) 
        / (2 * quadratic);

        glm::mat4 tr = glm::translate(glm::identity<glm::mat4>(), l.position) * glm::scale(point_light_geom_tr, 5.f * glm::vec3{radius});

        ev2::gl::glUniform(tr, plp_m_location);
        ev2::gl::glUniform(l.color, plp_light_c_location);
        ev2::gl::glUniform(l.position, plp_light_p_location);
        ev2::gl::glUniformf(constant, plp_k_c_loc);
        ev2::gl::glUniformf(linear, plp_k_l_loc);
        ev2::gl::glUniformf(quadratic, plp_k_q_loc);

        point_light_drawable->draw(point_lighting_program);
    }

    point_lighting_program.unbind();
}

void Renderer::set_wireframe(bool enable) {
    wireframe = enable;
}

void Renderer::set_resolution(uint32_t width, uint32_t height) {
    this->width = width;
    this->height = height;

    g_buffer.resize_all(width, height);
    ssao_buffer.resize_all(width, height);
}

void Renderer::draw_screen_space_triangle() {
    sst_vb.bind();
    GL_CHECKED_CALL(glDrawArrays(GL_TRIANGLES, 0, 3));
    sst_vb.unbind();
}

}