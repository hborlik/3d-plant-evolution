#include <renderer.h>

#include <random>
#include <cmath>
#include <chrono>

#include <resource.h>

namespace ev2::renderer {

void Renderer::draw(Drawable* dr, const Program& prog, bool use_materials, int32_t material_override) {
    if (dr->cull_mode == gl::CullMode::NONE) {
        glDisable(GL_CULL_FACE);
    } else {
        glEnable(GL_CULL_FACE);
        glCullFace((GLenum)dr->cull_mode);
    }
    glFrontFace((GLenum)dr->front_facing);
    int mat_loc = prog.getUniformInfo("materialId").Location;
    int vert_col_w_loc = prog.getUniformInfo("vertex_color_weight").Location;
    int diffuse_sampler_loc = prog.getUniformInfo("diffuse_tex").Location;
    bool indexed = dr->vertex_buffer.get_indexed() != -1;
    // TODO: support for multiple index buffers
    dr->vertex_buffer.bind();
    for (auto& m : dr->primitives) {
        Material* material_ptr = nullptr;
        if (use_materials) {
            mat_id_t material_slot = 0;
            if (material_override < 0) {
                material_ptr = &materials.at(dr->materials[m.material_ind]->material_id);
            } else {
                material_ptr = &materials.at(material_override);
            }
            material_slot = material_ptr->slot;

            if (diffuse_sampler_loc >= 0) {
                glActiveTexture(GL_TEXTURE0);
                gl::glUniformSampler(0, diffuse_sampler_loc);
            }
            if (material_ptr->diffuse_tex) {
                material_ptr->diffuse_tex->bind();
            } else {
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            // TODO does this go here?
            if (mat_loc >= 0) {
                GL_CHECKED_CALL(glUniform1ui(mat_loc, material_slot));
            }

            if (vert_col_w_loc >= 0) {
                GL_CHECKED_CALL(glUniform1f(vert_col_w_loc, dr->vertex_color_weight));
            }
        }

        if (indexed) {
            dr->vertex_buffer.get_buffer(dr->vertex_buffer.get_indexed()).Bind(); // bind index buffer (again, @Windows)
            glDrawElements(GL_TRIANGLES, m.num_elements, GL_UNSIGNED_INT, (void*)0);
        } else {
            glDrawArrays(GL_TRIANGLES, m.start_index, m.num_elements);
        }

        if (material_ptr && diffuse_sampler_loc >= 0 && material_ptr->diffuse_tex) {
            material_ptr->diffuse_tex->unbind();
        }
    }
    dr->vertex_buffer.unbind();
}

Renderer::Renderer(uint32_t width, uint32_t height) : 
    models{},
    model_instances{},
    geometry_program{"Geometry Program"},
    directional_lighting_program{"Lighting Program"},
    g_buffer{gl::FBOTarget::RW},
    ssao_buffer{gl::FBOTarget::RW},
    lighting_buffer{gl::FBOTarget::RW},
    d_buffer{gl::FBOTarget::RW},
    sst_vb{VertexBuffer::vbInitSST()},
    shader_globals{gl::BindingTarget::UNIFORM, gl::Usage::DYNAMIC_DRAW},
    lighting_materials{gl::BindingTarget::UNIFORM, gl::Usage::DYNAMIC_DRAW},
    ssao_kernel_buffer{gl::BindingTarget::UNIFORM, gl::Usage::DYNAMIC_DRAW},
    width{width}, 
    height{height} {

    // material id queue
    for (mat_id_t i = 1; i < MAX_N_MATERIALS; i++) {
        free_material_slots.push(i);
    }

    // create default material
    materials[0] = {};

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

    // set up FBO textures
    shadow_depth_tex = std::make_shared<Texture>(gl::TextureType::TEXTURE_2D, gl::TextureFilterMode::NEAREST);
    shadow_depth_tex->set_data2D(gl::TextureInternalFormat::DEPTH_COMPONENT16, ShadowMapWidth, ShadowMapHeight, gl::PixelFormat::DEPTH_COMPONENT, gl::PixelType::FLOAT, nullptr);
    shadow_depth_tex->set_wrap_mode(gl::TextureParamWrap::TEXTURE_WRAP_S, gl::TextureWrapMode::CLAMP_TO_BORDER);
    shadow_depth_tex->set_wrap_mode(gl::TextureParamWrap::TEXTURE_WRAP_T, gl::TextureWrapMode::CLAMP_TO_BORDER);
    shadow_depth_tex->set_border_color(glm::vec4{1.0f});
    d_buffer.attach(shadow_depth_tex, gl::FBOAttachment::DEPTH);
    if (!d_buffer.check())
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

    // set up programs

    ev2::ShaderPreprocessor prep{ResourceManager::get_singleton().asset_path / "shader"};
    prep.load_includes();


    geometry_program.loadShader(gl::GLSLShaderType::VERTEX_SHADER, "geometry.glsl.vert", prep);
    geometry_program.loadShader(gl::GLSLShaderType::FRAGMENT_SHADER, "geometry.glsl.frag", prep);
    geometry_program.link();

    gp_m_location = geometry_program.getUniformInfo("M").Location;
    gp_g_location = geometry_program.getUniformInfo("G").Location;

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
    plp_k_radius_loc = point_lighting_program.getUniformInfo("radius").Location;


    ssao_program.loadShader(gl::GLSLShaderType::VERTEX_SHADER, "sst.glsl.vert", prep);
    ssao_program.loadShader(gl::GLSLShaderType::FRAGMENT_SHADER, "ssao.glsl.frag", prep);
    ssao_program.link();

    ssao_p_loc = ssao_program.getUniformInfo("gPosition").Location;
    ssao_n_loc = ssao_program.getUniformInfo("gNormal").Location;
    ssao_tex_noise_loc = ssao_program.getUniformInfo("texNoise").Location;
    ssao_radius_loc = ssao_program.getUniformInfo("radius").Location;
    ssao_bias_loc = ssao_program.getUniformInfo("bias").Location;
    ssao_nSamples_loc = ssao_program.getUniformInfo("nSamples").Location;


    sky_program.loadShader(gl::GLSLShaderType::VERTEX_SHADER, std::filesystem::path("sky") / "sky.glsl.vert", prep);
    sky_program.loadShader(gl::GLSLShaderType::FRAGMENT_SHADER, std::filesystem::path("sky") / "sky.glsl.frag", prep);
    sky_program.link();
    sky_time_loc = sky_program.getUniformInfo("time").Location;
    sky_cirrus_loc = sky_program.getUniformInfo("cirrus").Location;
    sky_cumulus_loc = sky_program.getUniformInfo("cumulus").Location;
    sky_sun_position_loc = sky_program.getUniformInfo("sun_position").Location;

    post_fx_program.loadShader(gl::GLSLShaderType::VERTEX_SHADER, "sst.glsl.vert", prep);
    post_fx_program.loadShader(gl::GLSLShaderType::FRAGMENT_SHADER, "post_fx.glsl.frag", prep);
    post_fx_program.link();
    post_fx_gamma_loc = post_fx_program.getUniformInfo("gamma").Location;
    post_fx_exposure_loc = post_fx_program.getUniformInfo("exposure").Location;
    post_fx_hdrt_loc = post_fx_program.getUniformInfo("hdrBuffer").Location;

    // program block inputs
    globals_desc = geometry_program.getUniformBlockInfo("Globals");
    shader_globals.Allocate(globals_desc.block_size);

    lighting_materials_desc = directional_lighting_program.getUniformBlockInfo("MaterialsInfo");
    lighting_materials.Allocate(lighting_materials_desc.block_size);

    ssao_kernel_desc = ssao_program.getUniformBlockInfo("Samples");
    ssao_kernel_buffer.Allocate(ssao_kernel_desc.block_size);
    auto tgt_layout = ssao_kernel_desc.getLayout("samples[0]");
    ssao_kernel_buffer.SubData(ssaoKernel, tgt_layout.Offset, tgt_layout.ArrayStride);
    
    int i = 0;
    for (auto& m : material_data_buffer) {
        update_material(i, m);
    }

    // light geometry
    point_light_geometry_id = ResourceManager::get_singleton().get_model(std::filesystem::path("models") / "sphere.obj", false);
    auto itr = models.find(point_light_geometry_id);
    point_light_drawable = itr->second.get();
    point_light_drawable->front_facing = gl::FrontFacing::CW; // render back facing only
    glm::vec3 scaling = glm::vec3{2} / (point_light_drawable->bmax - point_light_drawable->bmin);
    point_light_geom_tr = glm::scale(glm::identity<glm::mat4>(), scaling);
}

void Renderer::update_material(mat_id_t material_slot, const MaterialData& material) {
    material_data_buffer[material_slot] = material;
    lighting_materials_desc.setShaderParameter("materials[" + std::to_string(material_slot) + "].diffuse", material.diffuse, lighting_materials);
    lighting_materials_desc.setShaderParameter("materials[" + std::to_string(material_slot) + "].metallic", material.metallic, lighting_materials);
    lighting_materials_desc.setShaderParameter("materials[" + std::to_string(material_slot) + "].subsurface", material.subsurface, lighting_materials);
    lighting_materials_desc.setShaderParameter("materials[" + std::to_string(material_slot) + "].specular", material.specular, lighting_materials);
    lighting_materials_desc.setShaderParameter("materials[" + std::to_string(material_slot) + "].roughness", material.roughness, lighting_materials);
    lighting_materials_desc.setShaderParameter("materials[" + std::to_string(material_slot) + "].specularTint", material.specularTint, lighting_materials);
    lighting_materials_desc.setShaderParameter("materials[" + std::to_string(material_slot) + "].clearcoat", material.clearcoat, lighting_materials);
    lighting_materials_desc.setShaderParameter("materials[" + std::to_string(material_slot) + "].clearcoatGloss", material.clearcoatGloss, lighting_materials);
    lighting_materials_desc.setShaderParameter("materials[" + std::to_string(material_slot) + "].anisotropic", material.anisotropic, lighting_materials);
    lighting_materials_desc.setShaderParameter("materials[" + std::to_string(material_slot) + "].sheen", material.sheen, lighting_materials);
    lighting_materials_desc.setShaderParameter("materials[" + std::to_string(material_slot) + "].sheenTint", material.sheenTint, lighting_materials);
}

Material* Renderer::create_material() {
    if (free_material_slots.size() > 0) {
        int32_t id = next_material_id++;
        mat_id_t slot = free_material_slots.front();
        free_material_slots.pop();
        Material* new_material = &materials[id];
        new_material->material_id = id;
        new_material->internal_material = &material_data_buffer[slot];
        new_material->slot = slot;
        return new_material;
    }
    return nullptr;
}

void Renderer::destroy_material(Material* material) {
    assert(material);
    material_data_buffer[material->slot] = {};
    free_material_slots.push(material->slot);
    material->internal_material = nullptr;
    material->slot = 0;
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

void Renderer::set_instance_material_override(IID iid, Material* material) {
    if (!iid.is_valid() || !material)
        return;
    
    auto mi = model_instances.find(iid.v);
    if (mi != model_instances.end()) {
        mi->second.material_id_override = material->material_id;
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

VBID Renderer::create_vertex_buffer() {
    int32_t id = next_vb_id++;
    vertex_buffers.emplace(id, VertexBuffer{});
    return {id};
}

VertexBuffer* Renderer::get_vertex_buffer(VBID vbid) {
    if (vbid.is_valid()) {
        auto itr = vertex_buffers.find(vbid.v);
        if (itr != vertex_buffers.end()) {
            return &(itr->second);
        }
    }
    return nullptr;
}

void Renderer::destroy_vertex_buffer(VBID vbid) {
    if (vbid.is_valid())
        vertex_buffers.erase(vbid.v);
}

MSID Renderer::create_mesh() {
    int32_t id = next_mesh_id++;
    Mesh mesh{};
    meshes.emplace(id, mesh);
    return {id};
}

void Renderer::set_mesh_primitives(MSID mesh_id, const std::vector<MeshPrimitive>& primitives) {
    if (!mesh_id.is_valid())
        return;
    
    auto mi = meshes.find(mesh_id.v);
    if (mi != meshes.end()) {
        mi->second.primitives = primitives;

        for (auto& m_pri : mi->second.primitives) {
            VertexBuffer* vb = get_vertex_buffer(m_pri.vbid);
            if (vb) {
                m_pri.gl_vao = vb->gen_vao_for_attributes(m_pri.attributes);
            }
        }
    }
}

void Renderer::set_mesh_cull_mode(MSID mesh_id, gl::CullMode cull_mode) {
    if (!mesh_id.is_valid())
        return;
    
    auto mi = meshes.find(mesh_id.v);
    if (mi != meshes.end()) {
        mi->second.cull_mode = cull_mode;
    }
}

void Renderer::set_mesh_front_facing(MSID mesh_id, gl::FrontFacing front_facing) {
    if (!mesh_id.is_valid())
        return;
    
    auto mi = meshes.find(mesh_id.v);
    if (mi != meshes.end()) {
        mi->second.front_facing = front_facing;
    }
}

void Renderer::destroy_mesh(MSID mesh_id) {
    if (mesh_id.is_valid())
        meshes.erase(mesh_id.v);
}

MSIID Renderer::create_mesh_instance() {
    int32_t id = next_mesh_instance_id++;
    mesh_instances.emplace(id, MeshInstance{});
    return {id};
}

void Renderer::set_mesh_instance_mesh(MSIID msiid, MSID msid) {
    if (!msiid.is_valid())
        return;
    auto msi = meshes.find(msid.v);
    auto mi = mesh_instances.find(msiid.v);
    if (mi != mesh_instances.end() && msi != meshes.end()) {
        mi->second.mesh = &(msi->second);
    }
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
        material.update_internal();
    }

    for(mat_id_t i = 0; i < MAX_N_MATERIALS; i++) {
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

    // real render
    glm::mat4 light_vp;

    // render all geometry to g buffer
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_DITHER);
    glDisable(GL_STENCIL_TEST);

    if (shadow_directional_light_id >= 0) {
        //set up shadow shader
        depth_program.use();
        d_buffer.bind();
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
        glm::mat4 LO = glm::ortho(minX, maxX, minY, maxY, shadow_near, shadow_far);
        glm::mat4 bias_mat = {
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

                draw(m.drawable, depth_program, false);
            }
        }

        depth_program.unbind();
        d_buffer.unbind();
    }


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

            draw(m.drawable, geometry_program, true, m.material_id_override);
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

    position->unbind();
    normals->unbind();
    ssao_kernel_noise->unbind();

    ssao_program.unbind();
    ssao_buffer.unbind();

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

        glm::mat4 tr = glm::translate(glm::identity<glm::mat4>(), l.position) * glm::scale(point_light_geom_tr, 5.f * glm::vec3{radius});

        gl::glUniform(tr, plp_m_location);
        gl::glUniform(l.color, plp_light_c_location);
        gl::glUniform(l.position, plp_light_p_location);
        gl::glUniformf(constant, plp_k_c_loc);
        gl::glUniformf(linear, plp_k_l_loc);
        gl::glUniformf(quadratic, plp_k_q_loc);
        gl::glUniformf(radius, plp_k_radius_loc);

        draw(point_light_drawable, point_lighting_program, false);
    }

    normals->unbind();
    albedo_spec->unbind();
    material_tex->unbind();

    point_lighting_program.unbind();

    // sky program
    sky_program.use();
    float time = (float)glfwGetTime() - 0.0f;
    gl::glUniformf(time * cloud_speed, sky_time_loc);
    gl::glUniformf(sun_position, sky_sun_position_loc);
    // draw into non lit pixels
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00); // disable writing to the stencil buffer
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDisable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    globals_desc.bind_buffer(shader_globals);

    draw_screen_space_triangle();

    sky_program.unbind();
    lighting_buffer.unbind();

    // post fx pass
    post_fx_program.use();

    glDisable(GL_STENCIL_TEST);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    gl::glUniformf(exposure, post_fx_exposure_loc);
    gl::glUniformf(gamma, post_fx_gamma_loc);

    if (post_fx_hdrt_loc >= 0) {
        glActiveTexture(GL_TEXTURE0);
        hdr_texture->bind();
        gl::glUniformSampler(0, post_fx_hdrt_loc);
    }

    draw_screen_space_triangle();

    hdr_texture->unbind();

    post_fx_program.unbind();

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
}

void Renderer::draw_screen_space_triangle() {
    sst_vb.bind();
    GL_CHECKED_CALL(glDrawArrays(GL_TRIANGLES, 0, 3));
    sst_vb.unbind();
}

}