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

    glEnableVertexAttribArray(gl::VERTEX_BINDING_LOCATION);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 11, (void*)0);

    glEnableVertexAttribArray(gl::NORMAL_BINDING_LOCATION);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 11, (void*)(sizeof(float) * 3));

    glEnableVertexAttribArray(gl::COLOR_BINDING_LOCATION);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 11, (void*)(sizeof(float) * 6));

    glEnableVertexAttribArray(gl::TEXCOORD_BINDING_LOCATION);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 11, (void*)(sizeof(float) * 9));

    vb.buffers[vb.buffers.size() - 1].Unbind();

    glBindVertexArray(0);

    return std::move(vb);
}

void Model::draw() {
    vb.bind();
    glCullFace(GL_BACK);
    // TODO: support index buffers
    for (auto& m : meshes) {
        glDrawArrays(GL_TRIANGLE_STRIP, m.start_index, m.num_elements);
    }
    vb.unbind();
}

}