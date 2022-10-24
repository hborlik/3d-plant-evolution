#include <renderer/renderer.h>

#include <random>
#include <cmath>
#include <chrono>

#include <resource.h>

namespace ev2::renderer {

void ModelInstance::set_material_override(Material* material) {
    if (material == nullptr) 
        material_id_override = -1;
}

void ModelInstance::set_drawable(Drawable* drawable) {
    this->drawable = drawable;

    if (gl_vao != 0)
        glDeleteVertexArrays(1, &gl_vao);
    
    gl_vao = drawable->vertex_buffer.gen_vao_for_attributes(mat_spec::DefaultBindings);
}

void Renderer::draw(Drawable* dr, const Program& prog, bool use_materials, GLuint gl_vao, int32_t material_override, const Buffer* instance_buffer, int32_t n_instances) {
    if (instance_buffer != nullptr) {
        assert(n_instances > 0);
    }
    
    if (dr->cull_mode == gl::CullMode::NONE) {
        glDisable(GL_CULL_FACE);
    } else {
        glEnable(GL_CULL_FACE);
        glCullFace((GLenum)dr->cull_mode);
    }
    glFrontFace((GLenum)dr->front_facing);
    const int mat_loc = prog.getUniformInfo("materialId").Location;
    const int vert_col_w_loc = prog.getUniformInfo("vertex_color_weight").Location;
    const int diffuse_sampler_loc = prog.getUniformInfo("diffuse_tex").Location;
    const bool indexed = dr->vertex_buffer.get_indexed() != -1;

    glBindVertexArray(gl_vao);
    for (auto& m : dr->primitives) {
        Material* material_ptr = nullptr;
        if (use_materials) {
            mat_id_t material_slot = 0;
            if (m.material_ind >= 0 && material_override < 0) {
                material_ptr = &materials.at(dr->materials[m.material_ind]->material_id);
            } else if (material_override >= 0) {
                material_ptr = &materials.at(material_override);
            } else { // use default if no material is set
                material_ptr = &materials.at(default_material_id);
            }
            material_slot = material_ptr->material_slot;

            // TODO does this go here?
            if (mat_loc >= 0) {
                GL_CHECKED_CALL(glUniform1ui(mat_loc, material_slot));
            }

            if (vert_col_w_loc >= 0) {
                GL_CHECKED_CALL(glUniform1f(vert_col_w_loc, dr->vertex_color_weight));
            }
        }

        // textures
        if (diffuse_sampler_loc >= 0) {
            glActiveTexture(GL_TEXTURE0);
            if (use_materials && material_ptr && material_ptr->diffuse_tex)
                material_ptr->diffuse_tex->bind();
            else
                one_p_black_tex->bind();

            gl::glUniformSampler(0, diffuse_sampler_loc);
        } else {
            // glBindTexture(GL_TEXTURE_2D, 0);
        }

        if (indexed) {
            Buffer& el_buf = dr->vertex_buffer.get_buffer(dr->vertex_buffer.get_indexed());
            el_buf.Bind(); // bind index buffer (again, @Windows)
            if (instance_buffer) {
                glDrawElementsInstanced(GL_TRIANGLES, m.num_elements, GL_UNSIGNED_INT, (void*)0, n_instances);
            } else {
                glDrawElements(GL_TRIANGLES, m.num_elements, GL_UNSIGNED_INT, (void*)0);
            }
            el_buf.Unbind();
        } else {
            if (instance_buffer) {
                glDrawArraysInstanced(GL_TRIANGLES, m.start_index, m.num_elements, n_instances);
            } else {
                glDrawArrays(GL_TRIANGLES, m.start_index, m.num_elements);
            }
        }

        // glBindTexture(GL_TEXTURE_2D, 0);
    }
    glBindVertexArray(0);
}

Renderer::Renderer(uint32_t width, uint32_t height) : 
    models{},
    model_instances{},
    geometry_program{"Geometry Program"},
    directional_lighting_program{"Lighting Program"},
    g_buffer{gl::FBOTarget::RW},
    ssao_buffer{gl::FBOTarget::RW},
    lighting_buffer{gl::FBOTarget::RW},
    depth_buffer{gl::FBOTarget::RW},
    bloom_thresh_combine{gl::FBOTarget::RW},
    bloom_blur_swap_fbo{FBO{gl::FBOTarget::RW}, FBO{gl::FBOTarget::RW}},
    sst_vb{VertexBuffer::vbInitSST()},
    shader_globals{gl::BindingTarget::UNIFORM, gl::Usage::DYNAMIC_DRAW},
    lighting_materials{gl::BindingTarget::UNIFORM, gl::Usage::DYNAMIC_DRAW},
    ssao_kernel_buffer{gl::BindingTarget::UNIFORM, gl::Usage::DYNAMIC_DRAW},
    width{width}, 
    height{height} {

    // material id queue
    for (mat_id_t i = 0; i < MAX_N_MATERIALS; i++) {
        free_material_slots.push(i);
    }

    // create default material
    default_material_id = create_material()->material_id;
    materials[default_material_id].diffuse = {1.00,0.00,1.00};

    for (auto& mat : material_data_buffer) {
        mat = {};
    }
}

Renderer::~Renderer() {

}

void Renderer::init() {
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

    // black texture
    one_p_black_tex = std::make_shared<Texture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST);
    one_p_black_tex->set_data2D(gl::TextureInternalFormat::RED, 1, 1, gl::PixelFormat::RED, gl::PixelType::UNSIGNED_BYTE, nullptr);
    one_p_black_tex->set_wrap_mode(gl::TextureParamWrap::TEXTURE_WRAP_S, gl::TextureWrapMode::REPEAT);
    one_p_black_tex->set_wrap_mode(gl::TextureParamWrap::TEXTURE_WRAP_T, gl::TextureWrapMode::REPEAT);
    one_p_black_tex->generate_mips();

    // set up FBO textures
    shadow_depth_tex = std::make_shared<Texture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST);
    shadow_depth_tex->set_data2D(gl::TextureInternalFormat::DEPTH_COMPONENT16, ShadowMapWidth, ShadowMapHeight, gl::PixelFormat::DEPTH_COMPONENT, gl::PixelType::FLOAT, nullptr);
    shadow_depth_tex->set_wrap_mode(gl::TextureParamWrap::TEXTURE_WRAP_S, gl::TextureWrapMode::CLAMP_TO_BORDER);
    shadow_depth_tex->set_wrap_mode(gl::TextureParamWrap::TEXTURE_WRAP_T, gl::TextureWrapMode::CLAMP_TO_BORDER);
    shadow_depth_tex->set_border_color(glm::vec4{1.0f});
    depth_buffer.attach(shadow_depth_tex, gl::FBOAttachment::DEPTH);
    if (!depth_buffer.check())
        throw engine_exception{"Framebuffer is not complete"};
    

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

    emissive = std::make_shared<Texture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST);
    emissive->set_data2D(gl::TextureInternalFormat::RGBA16F, width, height, gl::PixelFormat::RGBA, gl::PixelType::FLOAT, nullptr);
    g_buffer.attach(emissive, gl::FBOAttachment::COLOR4, 4);

    g_buffer.attach_renderbuffer(gl::RenderBufferInternalFormat::DEPTH_COMPONENT, width, height, gl::FBOAttachment::DEPTH);

    if (!g_buffer.check())
        throw engine_exception{"Geometry Framebuffer is not complete"};

    // lighting output HDR FBO
    hdr_texture = std::make_shared<Texture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST);
    hdr_texture->set_data2D(gl::TextureInternalFormat::RGBA16F, width, height, gl::PixelFormat::RGBA, gl::PixelType::FLOAT, nullptr);
    lighting_buffer.attach(hdr_texture, gl::FBOAttachment::COLOR0, 0);

    lighting_buffer.attach_renderbuffer(gl::RenderBufferInternalFormat::DEPTH24_STENCIL8, width, height, gl::FBOAttachment::DEPTH_STENCIL);

    if (!lighting_buffer.check())
        throw engine_exception{"Framebuffer is not complete"};

    // blur pass texture and FBOs
    bloom_blur_swap_tex[0] = std::make_shared<Texture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST);
    bloom_blur_swap_tex[0]->set_data2D(gl::TextureInternalFormat::RGBA16F, width, height, gl::PixelFormat::RGBA, gl::PixelType::FLOAT, nullptr);
    bloom_blur_swap_fbo[0].attach(bloom_blur_swap_tex[0], gl::FBOAttachment::COLOR0, 0);

    bloom_blur_swap_tex[1] = std::make_shared<Texture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST);
    bloom_blur_swap_tex[1]->set_data2D(gl::TextureInternalFormat::RGBA16F, width, height, gl::PixelFormat::RGBA, gl::PixelType::FLOAT, nullptr);
    bloom_blur_swap_fbo[1].attach(bloom_blur_swap_tex[1], gl::FBOAttachment::COLOR0, 0);

    // bloom threshold combine output HDR FBO
    hdr_combined = std::make_shared<Texture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST);
    hdr_combined->set_data2D(gl::TextureInternalFormat::RGBA16F, width, height, gl::PixelFormat::RGBA, gl::PixelType::FLOAT, nullptr);
    bloom_thresh_combine.attach(hdr_combined, gl::FBOAttachment::COLOR0, 0);

    bloom_thresh_combine.attach(bloom_blur_swap_tex[0], gl::FBOAttachment::COLOR1, 1);

    if (!bloom_thresh_combine.check())
        throw engine_exception{"Framebuffer is not complete"};

    // set up programs

    ShaderPreprocessor prep{ResourceManager::get_singleton().asset_path / "shader"};
    prep.load_includes();


    geometry_program.loadShader(gl::GLSLShaderType::VERTEX_SHADER, "geometry.glsl.vert", prep);
    geometry_program.loadShader(gl::GLSLShaderType::FRAGMENT_SHADER, "geometry.glsl.frag", prep);
    geometry_program.link();

    gp_m_location = geometry_program.getUniformInfo("M").Location;
    gp_g_location = geometry_program.getUniformInfo("G").Location;

    geometry_program_instanced.loadShader(gl::GLSLShaderType::VERTEX_SHADER, "geometry_instanced.glsl.vert", prep);
    geometry_program_instanced.loadShader(gl::GLSLShaderType::FRAGMENT_SHADER, "geometry.glsl.frag", prep);
    geometry_program_instanced.link();

    gpi_m_location = geometry_program_instanced.getUniformInfo("M").Location;

    // Initialize the GLSL programs
    depth_program.loadShader(gl::GLSLShaderType::VERTEX_SHADER, "simpleDepth.glsl.vert", prep);
    depth_program.loadShader(gl::GLSLShaderType::FRAGMENT_SHADER, "simpleDepth.glsl.frag", prep);
    depth_program.link();

    sdp_m_location = depth_program.getUniformInfo("M").Location;
    sdp_lpv_location = depth_program.getUniformInfo("LPV").Location;


    directional_lighting_program.loadShader(gl::GLSLShaderType::VERTEX_SHADER, "sst.glsl.vert", prep);
    directional_lighting_program.loadShader(gl::GLSLShaderType::FRAGMENT_SHADER, "directional.glsl.frag", prep);
    directional_lighting_program.link();

    lp_p_location = directional_lighting_program.getUniformInfo("gPosition").Location;
    lp_n_location = directional_lighting_program.getUniformInfo("gNormal").Location;
    lp_as_location = directional_lighting_program.getUniformInfo("gAlbedoSpec").Location;
    lp_mt_location = directional_lighting_program.getUniformInfo("gMaterialTex").Location;
    lp_gao_location = directional_lighting_program.getUniformInfo("gAO").Location;
    lp_ls_location = directional_lighting_program.getUniformInfo("LS").Location;
    lp_sdt_location = directional_lighting_program.getUniformInfo("shadowDepth").Location;


    point_lighting_program.loadShader(gl::GLSLShaderType::VERTEX_SHADER, "point_lighting.glsl.vert", prep);
    point_lighting_program.loadShader(gl::GLSLShaderType::FRAGMENT_SHADER, "point_lighting.glsl.frag", prep);
    point_lighting_program.link();

    plp_p_location       = point_lighting_program.getUniformInfo("gPosition").Location;
    plp_n_location       = point_lighting_program.getUniformInfo("gNormal").Location;
    plp_as_location      = point_lighting_program.getUniformInfo("gAlbedoSpec").Location;
    plp_mt_location      = point_lighting_program.getUniformInfo("gMaterialTex").Location;
    plp_m_location       = point_lighting_program.getUniformInfo("M").Location;
    plp_light_p_location = point_lighting_program.getUniformInfo("lightPos").Location;
    plp_light_c_location = point_lighting_program.getUniformInfo("lightColor").Location;
    plp_k_c_loc          = point_lighting_program.getUniformInfo("k_c").Location;
    plp_k_l_loc          = point_lighting_program.getUniformInfo("k_l").Location;
    plp_k_q_loc          = point_lighting_program.getUniformInfo("k_q").Location;
    plp_k_radius_loc     = point_lighting_program.getUniformInfo("radius").Location;


    ssao_program.loadShader(gl::GLSLShaderType::VERTEX_SHADER, "sst.glsl.vert", prep);
    ssao_program.loadShader(gl::GLSLShaderType::FRAGMENT_SHADER, "ssao.glsl.frag", prep);
    ssao_program.link();

    ssao_p_loc         = ssao_program.getUniformInfo("gPosition").Location;
    ssao_n_loc         = ssao_program.getUniformInfo("gNormal").Location;
    ssao_tex_noise_loc = ssao_program.getUniformInfo("texNoise").Location;
    ssao_radius_loc    = ssao_program.getUniformInfo("radius").Location;
    ssao_bias_loc      = ssao_program.getUniformInfo("bias").Location;
    ssao_nSamples_loc  = ssao_program.getUniformInfo("nSamples").Location;


    sky_program.loadShader(gl::GLSLShaderType::VERTEX_SHADER, std::filesystem::path("sky") / "sky.glsl.vert", prep);
    sky_program.loadShader(gl::GLSLShaderType::FRAGMENT_SHADER, std::filesystem::path("sky") / "sky.glsl.frag", prep);
    sky_program.link();
    sky_time_loc = sky_program.getUniformInfo("time").Location;
    sky_cirrus_loc = sky_program.getUniformInfo("cirrus").Location;
    sky_cumulus_loc = sky_program.getUniformInfo("cumulus").Location;
    sky_sun_position_loc = sky_program.getUniformInfo("sun_position").Location;
    sky_output_mul_loc = sky_program.getUniformInfo("output_mul").Location;

    post_fx_bloom_combine_program.loadShader(gl::GLSLShaderType::VERTEX_SHADER, "sst.glsl.vert", prep);
    post_fx_bloom_combine_program.loadShader(gl::GLSLShaderType::FRAGMENT_SHADER, "post_fx_bloom_combine.glsl.frag", prep);
    post_fx_bloom_combine_program.link();
    post_fx_bc_hdrt_loc = post_fx_bloom_combine_program.getUniformInfo("hdrBuffer").Location;
    post_fx_bc_emist_loc = post_fx_bloom_combine_program.getUniformInfo("emissiveBuffer").Location;
    post_fx_bc_thresh_loc = post_fx_bloom_combine_program.getUniformInfo("bloom_threshold").Location;

    post_fx_bloom_blur.loadShader(gl::GLSLShaderType::VERTEX_SHADER, "sst.glsl.vert", prep);
    post_fx_bloom_blur.loadShader(gl::GLSLShaderType::FRAGMENT_SHADER, "post_fx_bloom_blur.glsl.frag", prep);
    post_fx_bloom_blur.link();
    post_fx_bb_hor_loc = post_fx_bloom_blur.getUniformInfo("horizontal").Location;
    post_fx_bb_bloom_in_loc = post_fx_bloom_blur.getUniformInfo("bloom_blur_in").Location;

    post_fx_program.loadShader(gl::GLSLShaderType::VERTEX_SHADER, "sst.glsl.vert", prep);
    post_fx_program.loadShader(gl::GLSLShaderType::FRAGMENT_SHADER, "post_fx.glsl.frag", prep);
    post_fx_program.link();
    post_fx_gamma_loc = post_fx_program.getUniformInfo("gamma").Location;
    post_fx_exposure_loc = post_fx_program.getUniformInfo("exposure").Location;
    post_fx_bloom_falloff_loc = post_fx_program.getUniformInfo("bloom_falloff").Location;
    post_fx_hdrt_loc = post_fx_program.getUniformInfo("hdrBuffer").Location;
    post_fx_bloomt_loc = post_fx_program.getUniformInfo("bloomBuffer").Location;

    // program block inputs
    globals_desc = geometry_program.getUniformBlockInfo("Globals");
    shader_globals.Allocate(globals_desc.block_size);

    lighting_materials_desc = directional_lighting_program.getUniformBlockInfo("MaterialsInfo");
    lighting_materials.Allocate(lighting_materials_desc.block_size);

    // extract all offsets for material buffer
    for (const auto& layout_p : lighting_materials_desc.layouts) {
        const auto& layout = layout_p.second;
        std::string name = layout_p.first;
        std::size_t b_begin = name.find('[');
        std::size_t b_end = name.find(']');
        if (b_begin != std::string::npos) {
            if (b_end == std::string::npos)
                throw engine_exception{"Invalid array name??"};
            b_begin++;
            uint32_t index = std::stoi(name.substr(b_begin, b_end - b_begin));
            std::string var_name = name.substr(b_end + 2);
            MaterialData& data_ref = material_data_buffer[index];
            if (var_name == "diffuse") {
                data_ref.diffuse_offset = layout.Offset;
            } else if (var_name == "emissive") {
                data_ref.emissive_offset = layout.Offset;
            } else if (var_name == "metallic") {
                data_ref.metallic_offset = layout.Offset;
            } else if (var_name == "subsurface") {
                data_ref.subsurface_offset = layout.Offset;
            } else if (var_name == "specular") {
                data_ref.specular_offset = layout.Offset;
            } else if (var_name == "roughness") {
                data_ref.roughness_offset = layout.Offset;
            } else if (var_name == "specularTint") {
                data_ref.specularTint_offset = layout.Offset;
            } else if (var_name == "clearcoat") {
                data_ref.clearcoat_offset = layout.Offset;
            } else if (var_name == "clearcoatGloss") {
                data_ref.clearcoatGloss_offset = layout.Offset;
            } else if (var_name == "anisotropic") {
                data_ref.anisotropic_offset = layout.Offset;
            } else if (var_name == "sheen") {
                data_ref.sheen_offset = layout.Offset;
            } else if (var_name == "sheenTint") {
                data_ref.sheenTint_offset = layout.Offset;
            } else {
                throw engine_exception{"invalid material array name " + var_name};
            }
        }
    }

    ssao_kernel_desc = ssao_program.getUniformBlockInfo("Samples");
    ssao_kernel_buffer.Allocate(ssao_kernel_desc.block_size);
    auto tgt_layout = ssao_kernel_desc.getLayout("samples[0]");
    ssao_kernel_buffer.SubData(ssaoKernel, tgt_layout.Offset, tgt_layout.ArrayStride);
    
    int i = 0;
    for (auto& m : material_data_buffer) {
        update_material(i, m);
    }

    // light geometry
    point_light_drawable = ResourceManager::get_singleton().get_model(std::filesystem::path("models") / "sphere.obj", false);
    point_light_drawable->front_facing = gl::FrontFacing::CW; // render back facing only
    glm::vec3 scaling = glm::vec3{2} / (point_light_drawable->bmax - point_light_drawable->bmin);
    point_light_geom_tr = glm::scale(glm::identity<glm::mat4>(), scaling);

    point_light_gl_vao = point_light_drawable->vertex_buffer.gen_vao_for_attributes(point_lighting_program.getAttributeMap());
}

void Renderer::update_material(mat_id_t material_slot, const MaterialData& material) {
    material_data_buffer[material_slot] = material;
    lighting_materials.SubData(material.diffuse,        material.diffuse_offset);
    lighting_materials.SubData(material.emissive,       material.emissive_offset);
    lighting_materials.SubData(material.metallic,       material.metallic_offset);
    lighting_materials.SubData(material.subsurface,     material.subsurface_offset);
    lighting_materials.SubData(material.specular,       material.specular_offset);
    lighting_materials.SubData(material.roughness,      material.roughness_offset);
    lighting_materials.SubData(material.specularTint,   material.specularTint_offset);
    lighting_materials.SubData(material.clearcoat,      material.clearcoat_offset);
    lighting_materials.SubData(material.clearcoatGloss, material.clearcoatGloss_offset);
    lighting_materials.SubData(material.anisotropic,    material.anisotropic_offset);
    lighting_materials.SubData(material.sheen,          material.sheen_offset);
    lighting_materials.SubData(material.sheenTint,      material.sheenTint_offset);
}

Material* Renderer::create_material() {
    if (free_material_slots.size() > 0) {
        int32_t id = next_material_id++;
        mat_id_t slot = free_material_slots.front();
        free_material_slots.pop();
        Material* new_material = &materials[id];
        new_material->material_id = id;
        new_material->material_slot = slot;
        return new_material;
    }
    return nullptr;
}

void Renderer::destroy_material(Material* material) {
    assert(material);
    material_data_buffer[material->material_slot] = {};
    free_material_slots.push(material->material_slot);
    material->material_slot = 0;
    material->material_id = -1;
    materials.erase(material->material_id);
}

LID Renderer::create_point_light() {
    uint32_t nlid = next_light_id++;
    Light l{};
    point_lights.insert_or_assign(nlid, l);
    return {LID::Point, nlid};
}

LID Renderer::create_directional_light() {
    uint32_t nlid = next_light_id++;
    if (shadow_directional_light_id < 0)
        shadow_directional_light_id = nlid;
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
                mi->second.position = position;
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

Drawable* Renderer::create_model(VertexBuffer &&vb,
                                 std::vector<Primitive> primitives,
                                 std::vector<Material*> materials,
                                 glm::vec3 bmin,
                                 glm::vec3 bmax,
                                 gl::CullMode cull,
                                 gl::FrontFacing ff)
{
    uint32_t nmid = {next_model_id++};
    auto ins = models.emplace(
        std::make_pair(
            nmid,
            Drawable{
                std::move(vb),
                std::move(primitives),
                std::move(materials),
                std::move(bmin),
                std::move(bmax),
                std::move(cull),
                std::move(ff),
                nmid
            }
        )
    );
    if (ins.second)
        return &(ins.first->second);
    return nullptr;
}

void Renderer::destroy_model(Drawable* d) {
    assert(d);
    auto mitr = models.find(d->id);
    if (mitr != models.end()) {
        models.erase(mitr);
    }
}


ModelInstance* Renderer::create_model_instance() {
    int32_t id = next_model_instance_id++;
    ModelInstance model{};
    model.id = id;
    auto mi = model_instances.emplace(id, std::move(model));
    ModelInstance* new_model = nullptr;
    if (mi.second)
        new_model = &((mi.first)->second);

    return new_model;
}

void Renderer::destroy_model_instance(ModelInstance* model) {
    if (!model)
        return;
    model_instances.erase(model->id);
}

RenderObj* Renderer::create_render_obj() {
    int32_t id = next_mesh_id++;
    auto ro = meshes.emplace(id, RenderObj{});
    RenderObj* out = nullptr;
    if (ro.second)
        out = &(ro.first->second);
    return out;
}

// void RenderObj::set_mesh_primitives(const std::vector<MeshPrimitive>& primitives) {
//     this->primitives = primitives;

//     // generate the VAOs for all primitives in mesh 
//     for (auto& m_pri : this->primitives) {
//         assert(m_pri.vb);
//         m_pri.gl_vao = m_pri.vb->gen_vao_for_attributes(m_pri.attributes);
//     }
// }

void Renderer::destroy_render_obj(RenderObj* render_object) {
    if (render_object) {
        meshes.erase(render_object->id);
    }
}

MSIID Renderer::create_mesh_instance() {
    int32_t id = next_mesh_instance_id++;
    mesh_instances.emplace(id, MeshInstance{});
    return {id};
}

void Renderer::set_mesh_instance_mesh(MSIID msiid, RenderObj* render_object) {
    if (!(msiid.is_valid() || render_object))
        return;
    auto msi = render_object;
    auto mesh_instance = mesh_instances.find(msiid.v);
    if (mesh_instance != mesh_instances.end())
        mesh_instance->second.mesh = render_object;
}

void Renderer::set_mesh_instance_transform(MSIID msiid, const glm::mat4& transform) {
    if (!msiid.is_valid())
        return;
    
    auto mi = mesh_instances.find(msiid.v);
    if (mi != mesh_instances.end()) {
        mi->second.transform = transform;
    }
}

void Renderer::destroy_mesh_instance(MSIID msiid) {
    if (msiid.is_valid())
        mesh_instances.erase(msiid.v);
}

void Renderer::render(const Camera &camera) {
    std::chrono::time_point<std::chrono::system_clock> start, end;

    start = std::chrono::system_clock::now();

    // pre render data updates
    for (auto& m : materials) {
        Material& material = m.second;
        material_data_buffer[material.material_slot].update_from(&material);
    }

    for(mat_id_t i = 0; i < MAX_N_MATERIALS; i++) {
        if (material_data_buffer[i].changed)
            update_material(i, material_data_buffer[i]);
    }

    // update globals buffer with frame info
    glm::mat4 P = camera.get_projection();
    glm::mat4 V = camera.get_view();
    globals_desc.setShaderParameter("P", P, shader_globals);
    globals_desc.setShaderParameter("PInv", glm::inverse(P), shader_globals);
    globals_desc.setShaderParameter("View", V, shader_globals);
    globals_desc.setShaderParameter("VInv", glm::inverse(V), shader_globals);
    globals_desc.setShaderParameter("CameraPos", camera.get_position(), shader_globals);
    globals_desc.setShaderParameter("CameraDir", camera.get_forward(), shader_globals);

    glm::mat4 light_vp;

    // render all geometry to g buffer
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_DITHER);
    glDisable(GL_STENCIL_TEST);

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, __LINE__, -1, "Shadow Pass");

    if (shadow_directional_light_id >= 0) {
        //set up shadow shader
        depth_program.use();
        depth_buffer.bind();
        //set up light's depth map
        glViewport(0, 0, ShadowMapWidth, ShadowMapHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_DEPTH_BUFFER_BIT);

        const glm::vec3 directional_light_pos = camera.get_position() + directional_lights[shadow_directional_light_id].position;
        const float dist_to_camera = glm::length(directional_lights[shadow_directional_light_id].position);
        glm::mat4 LV =  glm::lookAt(directional_light_pos, camera.get_position(), camera.get_forward());
                    //    glm::inverse(glm::translate(glm::identity<glm::mat4>(), directional_light_pos));
        
        std::array<glm::vec3, 8> worldPoints = camera.extract_frustum_corners(0.1f);
        float minX = INFINITY, maxX = -INFINITY, minY = INFINITY, maxY = -INFINITY;
        for (auto &point : worldPoints) {
            glm::vec3 ls_point = LV * glm::vec4{point, 1.0};
            if (ls_point.x < minX)
                minX = ls_point.x;
            if (ls_point.x > maxX)
                maxX = ls_point.x;
            if (ls_point.y < minY)
                minY = ls_point.y;
            if (ls_point.y > maxY)
                maxY = ls_point.y;
        }

        const float shadow_near = 0.1f;
        const float shadow_far = 2.f * dist_to_camera;
        const float shadow_bias = 2.f * shadow_bias_world / (shadow_far - shadow_near);
        const glm::mat4 LO = glm::ortho(minX, maxX, minY, maxY, shadow_near, shadow_far);
        const glm::mat4 bias_mat = {
            glm::vec4{.5f, 0, 0, 0},
            glm::vec4{0, .5f, 0, 0},
            glm::vec4{0, 0, .5f, 0},
            glm::vec4{.5f, .5f, .5f - shadow_bias, 0}
        };
        light_vp = bias_mat * LO * LV;

        //render scene
        ev2::gl::glUniform(LO * LV, sdp_lpv_location);
        for (auto &mPair : model_instances) {
            auto& m = mPair.second;
            if (m.drawable) {
                ev2::gl::glUniform(m.transform, sdp_m_location);

                draw(m.drawable, depth_program, false, m.gl_vao);
            }
        }

        depth_program.unbind();
        depth_buffer.unbind();
    }

    glPopDebugGroup();
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, __LINE__, -1, "Geometry Pass");

    g_buffer.bind();
    geometry_program.use();

    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (wireframe)
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    else
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );


    // bind global shader UBO to shader
    globals_desc.bind_buffer(shader_globals);
    lighting_materials_desc.bind_buffer(lighting_materials);

    for (auto &mPair : model_instances) {
        auto& m = mPair.second;
        if (m.drawable) {
            const glm::mat3 G = glm::inverse(glm::transpose(glm::mat3(m.transform)));

            ev2::gl::glUniform(m.transform, gp_m_location);
            ev2::gl::glUniform(G, gp_g_location);

            draw(m.drawable, geometry_program, true, m.gl_vao, m.material_id_override);
        }
    }
    geometry_program.unbind();

    // render instanced geometry
    geometry_program_instanced.use();
    // bind global shader UBO to shader
    globals_desc.bind_buffer(shader_globals);
    lighting_materials_desc.bind_buffer(lighting_materials);
    for (auto &mPair : instanced_drawables) {
        auto& m = mPair.second;

        ev2::gl::glUniform(m.instance_world_transform, gpi_m_location);

        draw(m.drawable, geometry_program_instanced, true, m.gl_vao);
    }
    geometry_program_instanced.unbind();

    g_buffer.unbind();

    glPopDebugGroup();

    // glFlush();
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, __LINE__, -1, "SSAO");

    // ssao pass
    ssao_buffer.bind();

    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST); // overdraw
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );
    
    ssao_program.use();

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

    position->unbind();
    normals->unbind();
    ssao_kernel_noise->unbind();

    ssao_program.unbind();
    ssao_buffer.unbind();

    glPopDebugGroup();
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, __LINE__, -1, "Lighting Pass");

    // lighting pass
    // glFlush();
    lighting_buffer.bind();
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glDisable(GL_DEPTH_TEST); // overdraw
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    // add all lighting contributions
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);

    // stencil lighting areas
    glEnable(GL_STENCIL_TEST);
    glStencilMask(255);
    glStencilFunc(GL_ALWAYS, 1, 0xFF); // value to write in stencil buffer
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // only write new value when fragment color is written

    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

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

    if (lp_sdt_location >= 0) {
        glActiveTexture(GL_TEXTURE5);
        shadow_depth_tex->bind();
        gl::glUniformSampler(5, lp_sdt_location);      
    }

    gl::glUniform(light_vp, lp_ls_location);

    for (auto& litr : directional_lights) {
        auto& l = litr.second;
        gl::glUniform(glm::normalize(l.position), directional_lighting_program.getUniformInfo("lightDir").Location);
        gl::glUniform(l.color, directional_lighting_program.getUniformInfo("lightColor").Location);
        gl::glUniform(l.ambient, directional_lighting_program.getUniformInfo("lightAmbient").Location);
        
        draw_screen_space_triangle();
    }


    position->unbind();
    normals->unbind();
    albedo_spec->unbind();
    material_tex->unbind();
    ssao_kernel_color->unbind();
    shadow_depth_tex->unbind();

    directional_lighting_program.unbind();

    // pointlight pass
    point_lighting_program.use();
    globals_desc.bind_buffer(shader_globals);

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

        glm::mat4 tr = glm::translate(glm::identity<glm::mat4>(), l.position) * glm::scale(point_light_geom_tr, 2.f * glm::vec3{radius});

        gl::glUniform(tr, plp_m_location);
        gl::glUniform(l.color, plp_light_c_location);
        gl::glUniform(l.position, plp_light_p_location);
        gl::glUniformf(constant, plp_k_c_loc);
        gl::glUniformf(linear, plp_k_l_loc);
        gl::glUniformf(quadratic, plp_k_q_loc);
        gl::glUniformf(radius, plp_k_radius_loc);

        draw(point_light_drawable, point_lighting_program, false, point_light_gl_vao);
    }

    normals->unbind();
    albedo_spec->unbind();
    material_tex->unbind();

    point_lighting_program.unbind();

    // sky program
    // draw into non lit pixels in hdr fbo
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00); // disable writing to the stencil buffer
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDisable(GL_CULL_FACE);
    glFrontFace(GL_CCW);

    sky_program.use();
    globals_desc.bind_buffer(shader_globals);

    float time = (float)glfwGetTime() - 0.0f;
    gl::glUniformf(time * cloud_speed, sky_time_loc);
    gl::glUniformf(sun_position, sky_sun_position_loc);
    gl::glUniformf(sky_brightness, sky_output_mul_loc);

    draw_screen_space_triangle();

    sky_program.unbind();
    lighting_buffer.unbind();

    glPopDebugGroup();
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, __LINE__, -1, "Bloom");

    // bloom threshold and combine
    bloom_thresh_combine.bind();
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST); // sst
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );

    post_fx_bloom_combine_program.use();

    if (post_fx_bc_hdrt_loc >= 0) {
        glActiveTexture(GL_TEXTURE0);
        hdr_texture->bind();
        gl::glUniformSampler(0, post_fx_bc_hdrt_loc);
    }

    if (post_fx_bc_emist_loc >= 0) {
        glActiveTexture(GL_TEXTURE1);
        emissive->bind();
        gl::glUniformSampler(1, post_fx_bc_emist_loc);
    }

    gl::glUniformf(bloom_threshold, post_fx_bc_thresh_loc);

    draw_screen_space_triangle();

    post_fx_bloom_combine_program.unbind();
    bloom_thresh_combine.unbind();

    // bloom blur
    int bloom_output_tex_ind = 0;
    post_fx_bloom_blur.use();
    for (int i = 0; i < bloom_iterations * 2; i++) {
        bloom_output_tex_ind = (i + 1) % 2;

        bloom_blur_swap_fbo[bloom_output_tex_ind].bind();

        glActiveTexture(GL_TEXTURE0);
        bloom_blur_swap_tex[i%2]->bind();
        gl::glUniformSampler(0, post_fx_bb_bloom_in_loc);

        gl::glUniformi(bloom_output_tex_ind, post_fx_bb_hor_loc); // boolean

        draw_screen_space_triangle();

        bloom_blur_swap_fbo[bloom_output_tex_ind].unbind();
    }
    post_fx_bloom_blur.unbind();

    glPopDebugGroup();
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, __LINE__, -1, "Post Pass");

    // post fx pass
    glDisable(GL_STENCIL_TEST);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    post_fx_program.use();

    gl::glUniformf(exposure, post_fx_exposure_loc);
    gl::glUniformf(bloom_falloff, post_fx_bloom_falloff_loc);
    gl::glUniformf(gamma, post_fx_gamma_loc);

    if (post_fx_hdrt_loc >= 0) {
        glActiveTexture(GL_TEXTURE0);
        hdr_combined->bind();
        gl::glUniformSampler(0, post_fx_hdrt_loc);
    }

    if (post_fx_bloomt_loc >= 0) {
        glActiveTexture(GL_TEXTURE1);
        bloom_blur_swap_tex[bloom_output_tex_ind]->bind();
        gl::glUniformSampler(1, post_fx_bloomt_loc);
    }

    draw_screen_space_triangle();

    hdr_texture->unbind();

    post_fx_program.unbind();

    glPopDebugGroup();

    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;

    // std::cout << "render() elapsed time: " << elapsed_seconds.count() << "s\n";
}

void Renderer::set_wireframe(bool enable) {
    wireframe = enable;
}

void Renderer::set_resolution(uint32_t width, uint32_t height) {
    this->width = width;
    this->height = height;

    g_buffer.resize_all(width, height);
    ssao_buffer.resize_all(width, height);
    lighting_buffer.resize_all(width, height);
    bloom_thresh_combine.resize_all(width, height);
    bloom_blur_swap_fbo[0].resize_all(width, height);
    bloom_blur_swap_fbo[1].resize_all(width, height);
}

void Renderer::draw_screen_space_triangle() {
    glBindVertexArray(sst_vb.second);
    GL_CHECKED_CALL(glDrawArrays(GL_TRIANGLES, 0, 3));
    glBindVertexArray(0);
}

}