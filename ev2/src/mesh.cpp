#include <mesh.h>

namespace ev2 {

VertexBuffer VertexBuffer::vbInitArrayVertexData(const std::vector<float>& vertices, const std::vector<float>& normals, const std::vector<float>& vertex_colors) {
    VertexBuffer vb;
    glGenVertexArrays(1, &vb.gl_vao);
    glBindVertexArray(vb.gl_vao);

    vb.buffers.push_back(Buffer{gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, vertices});
    vb.buffers[vb.buffers.size() - 1].Bind();
    glEnableVertexAttribArray(gl::VERTEX_BINDING_LOCATION);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);

    vb.buffers.push_back(Buffer{gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, normals});
    vb.buffers[vb.buffers.size() - 1].Bind();
    glEnableVertexAttribArray(gl::NORMAL_BINDING_LOCATION);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)(sizeof(float) * 3));

    vb.buffers.push_back(Buffer{gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, vertex_colors});
    vb.buffers[vb.buffers.size() - 1].Bind();
    glEnableVertexAttribArray(gl::COLOR_BINDING_LOCATION);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)(sizeof(float) * 6));

    vb.buffers[vb.buffers.size() - 1].Unbind();

    glBindVertexArray(0);

    return std::move(vb);
}

VertexBuffer VertexBuffer::vbInitArrayVertexData(const std::vector<float>& buffer) {
    VertexBuffer vb;
    // pos(3float), normal(3float), color(3float), texcoord(2float)
    glGenVertexArrays(1, &vb.gl_vao);
    glBindVertexArray(vb.gl_vao);

    vb.buffers.push_back(Buffer{gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, buffer});
    vb.buffers[vb.buffers.size() - 1].Bind();

    constexpr std::size_t vec3Size = sizeof(glm::vec3);
    glEnableVertexAttribArray(gl::VERTEX_BINDING_LOCATION);
    glVertexAttribPointer(gl::VERTEX_BINDING_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 11, (void*)0);

    glEnableVertexAttribArray(gl::NORMAL_BINDING_LOCATION);
    glVertexAttribPointer(gl::NORMAL_BINDING_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 11, (void*)(vec3Size));

    glEnableVertexAttribArray(gl::COLOR_BINDING_LOCATION);
    glVertexAttribPointer(gl::COLOR_BINDING_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 11, (void*)(vec3Size * 2));

    glEnableVertexAttribArray(gl::TEXCOORD_BINDING_LOCATION);
    glVertexAttribPointer(gl::TEXCOORD_BINDING_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 11, (void*)(vec3Size * 3));

    vb.buffers[vb.buffers.size() - 1].Unbind();

    glBindVertexArray(0);

    return std::move(vb);
}

VertexBuffer VertexBuffer::vbInitSphereArrayVertexData(const std::vector<float>& buffer, const std::vector<unsigned int>& indexBuffer) {
    VertexBuffer vb;

    vb.buffers.push_back(Buffer{gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, buffer});
    vb.buffers.push_back(Buffer{gl::BindingTarget::ELEMENT_ARRAY, gl::Usage::STATIC_DRAW, indexBuffer});

    vb.indexed = 1;

    // pos(3float), normal(3float), color(3float), texcoord(2float)
    glGenVertexArrays(1, &vb.gl_vao);
    glBindVertexArray(vb.gl_vao);
    vb.buffers[0].Bind();
    vb.buffers[1].Bind();

    glEnableVertexAttribArray(gl::VERTEX_BINDING_LOCATION);
    glVertexAttribPointer(gl::VERTEX_BINDING_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)0);

    glEnableVertexAttribArray(gl::NORMAL_BINDING_LOCATION);
    glVertexAttribPointer(gl::NORMAL_BINDING_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3));

    glEnableVertexAttribArray(gl::TEXCOORD_BINDING_LOCATION);
    glVertexAttribPointer(gl::TEXCOORD_BINDING_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 6));

    vb.buffers[0].Unbind();
    vb.buffers[1].Unbind();

    glBindVertexArray(0);

    return std::move(vb);
}

VertexBuffer VertexBuffer::vbInitSST() {
    VertexBuffer vb;
    vb.indexed = -1;

    // pos(3float), normal(3float), color(3float), texcoord(2float)
    glGenVertexArrays(1, &vb.gl_vao);
    glBindVertexArray(vb.gl_vao);

    std::vector<float> buffer{0, 0, 2, 0, 0, 2};

    vb.buffers.push_back(Buffer{gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, buffer});
    vb.buffers[vb.buffers.size() - 1].Bind();

    glEnableVertexAttribArray(gl::VERTEX_BINDING_LOCATION);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);

    vb.buffers[vb.buffers.size() - 1].Unbind();

    glBindVertexArray(0);

    return std::move(vb);
}

VertexBuffer VertexBuffer::vbInitArrayVertexDataInstanced(const std::vector<float>& buffer) {
    VertexBuffer vb;
    // pos(3float), normal(3float), color(3float), texcoord(2float)
    glGenVertexArrays(1, &vb.gl_vao);
    glBindVertexArray(vb.gl_vao);

    vb.buffers.push_back(Buffer{gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, buffer});
    vb.buffers[vb.buffers.size() - 1].Bind();

    constexpr std::size_t vec3Size = sizeof(glm::vec3);
    glEnableVertexAttribArray(gl::VERTEX_BINDING_LOCATION);
    glVertexAttribPointer(gl::VERTEX_BINDING_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 11, (void*)0);

    glEnableVertexAttribArray(gl::NORMAL_BINDING_LOCATION);
    glVertexAttribPointer(gl::NORMAL_BINDING_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 11, (void*)(vec3Size));

    glEnableVertexAttribArray(gl::COLOR_BINDING_LOCATION);
    glVertexAttribPointer(gl::COLOR_BINDING_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 11, (void*)(vec3Size * 2));

    glEnableVertexAttribArray(gl::TEXCOORD_BINDING_LOCATION);
    glVertexAttribPointer(gl::TEXCOORD_BINDING_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 11, (void*)(vec3Size * 3));

    // mat4 instance info, note: max size for a vertex attribute is a vec4
    constexpr std::size_t vec4Size = sizeof(glm::vec4);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);

    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size));

    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size * 2));

    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size * 3));

    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);
    glVertexAttribDivisor(7, 1);

    vb.buffers[vb.buffers.size() - 1].Unbind();

    glBindVertexArray(0);

    return std::move(vb);
}


void Model::draw(const Program& prog) {
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
            prog.applyMaterial(materials[m.material_id]);

            vb.buffers[vb.getIndexed()].Bind(); // bind index buffer (again, @Windows)
            glDrawElements(GL_TRIANGLES, m.num_elements, GL_UNSIGNED_INT, (void*)0);
        }
    } else {
        for (auto& m : meshes) {
            prog.applyMaterial(materials[m.material_id]);

            glDrawArrays(GL_TRIANGLES, m.start_index, m.num_elements);
        }
    }
    vb.unbind();
}

}