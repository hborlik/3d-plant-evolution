#include <renderer.h>

namespace ev2 {

void Drawable::draw(const Program& prog, int32_t material_override) {
    vb.bind();
    if (cull_mode == gl::CullMode::NONE) {
        glDisable(GL_CULL_FACE);
    } else {
        glEnable(GL_CULL_FACE);
        glCullFace((GLenum)cull_mode);
    }
    glFrontFace((GLenum)front_facing);
    // TODO: support for multiple index buffers
    if (vb.getIndexed() != -1) {
        // draw indexed arrays
        for (auto& m : meshes) {
            // prog.applyMaterial(materials[m.material_id]);

            // TODO does this go here?
            int loc = prog.getUniformInfo("materialId").Location;
            if (loc != -1) {
                GL_CHECKED_CALL(glUniform1ui(loc, material_override > -1 ? material_override : m.material_id + material_offset));
            }

            vb.buffers[vb.getIndexed()].Bind(); // bind index buffer (again, @Windows)
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

            glDrawArrays(GL_TRIANGLES, m.start_index, m.num_elements);
        }
    }
    vb.unbind();
}

Renderer::Renderer(uint32_t width, uint32_t height, const std::filesystem::path& asset_path) : 
    models{},
    model_instances{},
    geometry_program{"Geometry Program"},
    lighting_program{"Lighting Program"},
    g_buffer{gl::FBOTarget::RW},
    sst_vb{VertexBuffer::vbInitSST()},
    shader_globals{gl::BindingTarget::UNIFORM, gl::Usage::DYNAMIC_DRAW},
    lighting_materials{gl::BindingTarget::UNIFORM, gl::Usage::DYNAMIC_DRAW},
    width{width}, 
    height{height} {

    // set up FBO textures
    material_tex = std::make_shared<Texture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST);
    material_tex->set_data2D(gl::TextureInternalFormat::R8UI, width, height, gl::PixelFormat::RED_INTEGER, gl::PixelType::UNSIGNED_BYTE, nullptr);
    g_buffer.attach(material_tex, gl::FBOAttachment::COLOR3);

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

    gp_m_location = geometry_program.getUniformInfo("M").Location;
    gp_g_location = geometry_program.getUniformInfo("G").Location;


    lighting_program.loadShader(gl::GLSLShaderType::VERTEX_SHADER, "sst.glsl.vert", prep);
    lighting_program.loadShader(gl::GLSLShaderType::FRAGMENT_SHADER, "disney.glsl.frag", prep);
    lighting_program.link();

    lp_p_location = lighting_program.getUniformInfo("gPosition").Location;
    lp_n_location = lighting_program.getUniformInfo("gNormal").Location;
    lp_as_location = lighting_program.getUniformInfo("gAlbedoSpec").Location;
    lp_mt_location = lighting_program.getUniformInfo("gMaterialTex").Location;

    // program block inputs
    globals_desc = geometry_program.getUniformBlockInfo("Globals");
    shader_globals.Allocate(globals_desc.block_size);

    lighting_materials_desc = lighting_program.getUniformBlockInfo("MaterialsInfo");
    lighting_materials.Allocate(lighting_materials_desc.block_size);

    materials = std::vector<MaterialData>(100, MaterialData{});
    
    int i = 0;
    for (auto& m : materials) {
        update_material(i, m);
    }

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

MID Renderer::create_model(std::shared_ptr<Model> model) {
    assert(model->bufferFormat == VertexFormat::Array);

    int32_t mat_offset = next_free_mat;

    for (auto mat = model->materials.begin(); mat != model->materials.end(); mat++) {
        create_material(MaterialData::from_material(*mat));
    }

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
    d->material_offset = mat_offset;

    return create_model(d);
}

MID Renderer::create_model(std::shared_ptr<Drawable> d) {
    MID nmid = {next_model_id++};
    models.insert_or_assign(nmid, d);
    return nmid;
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

    for (auto &mPair : model_instances) {
        auto& m = mPair.second;
        if (m.drawable) {
            const glm::mat3 G = glm::inverse(glm::transpose(glm::mat3(m.transform)));

            ev2::gl::glUniform(m.transform, gp_m_location);
            ev2::gl::glUniform(G, gp_g_location);

            m.drawable->draw(geometry_program, m.material_override);
        }
    }

    g_buffer.unbind();
    geometry_program.unbind();

    // glFlush();

    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    lighting_program.use();
    globals_desc.bind_buffer(shader_globals);
    lighting_materials_desc.bind_buffer(lighting_materials);

    gl::glUniform(glm::vec3{-50, 40, 0}, lighting_program.getUniformInfo("lightPos").Location);
    gl::glUniform(glm::vec3{400, 400, 400}, lighting_program.getUniformInfo("lightColor").Location);


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