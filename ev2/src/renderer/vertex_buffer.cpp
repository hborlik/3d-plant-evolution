#include <renderer/vertex_buffer.h>

namespace ev2 {

VertexBuffer VertexBuffer::vbInitArrayVertexData(const std::vector<float>& vertices, const std::vector<float>& normals, const std::vector<float>& vertex_colors) {
    VertexBuffer vb;
    glGenVertexArrays(1, &vb.gl_vao);
    glBindVertexArray(vb.gl_vao);

    vb.buffers.emplace(gl::VERTEX_BINDING_LOCATION, Buffer{gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, vertices});
    vb.buffers.at(gl::VERTEX_BINDING_LOCATION).Bind();
    glEnableVertexAttribArray(gl::VERTEX_BINDING_LOCATION);
    glVertexAttribPointer(gl::VERTEX_BINDING_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);

    vb.buffers.emplace(gl::NORMAL_BINDING_LOCATION, Buffer{gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, normals});
    vb.buffers.at(gl::NORMAL_BINDING_LOCATION).Bind();
    glEnableVertexAttribArray(gl::NORMAL_BINDING_LOCATION);
    glVertexAttribPointer(gl::NORMAL_BINDING_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)(sizeof(float) * 3));

    vb.buffers.emplace(gl::COLOR_BINDING_LOCATION, Buffer{gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, vertex_colors});
    vb.buffers.at(gl::COLOR_BINDING_LOCATION).Bind();
    glEnableVertexAttribArray(gl::COLOR_BINDING_LOCATION);
    glVertexAttribPointer(gl::COLOR_BINDING_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)(sizeof(float) * 6));

    vb.buffers.at(gl::COLOR_BINDING_LOCATION).Unbind();

    glBindVertexArray(0);

    return std::move(vb);
}

VertexBuffer VertexBuffer::vbInitArrayVertexData(const std::vector<float>& buffer, bool instanced) {
    VertexBuffer vb;
    // pos(3float), normal(3float), color(3float), texcoord(2float)
    glGenVertexArrays(1, &vb.gl_vao);
    glBindVertexArray(vb.gl_vao);

    vb.buffers.emplace(0, Buffer{gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, buffer});
    vb.buffers.at(0).Bind();

    constexpr std::size_t vec3Size = sizeof(glm::vec3);
    glEnableVertexAttribArray(gl::VERTEX_BINDING_LOCATION);
    glVertexAttribPointer(gl::VERTEX_BINDING_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 11, (void*)0);

    glEnableVertexAttribArray(gl::NORMAL_BINDING_LOCATION);
    glVertexAttribPointer(gl::NORMAL_BINDING_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 11, (void*)(vec3Size));

    glEnableVertexAttribArray(gl::COLOR_BINDING_LOCATION);
    glVertexAttribPointer(gl::COLOR_BINDING_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 11, (void*)(vec3Size * 2));

    glEnableVertexAttribArray(gl::TEXCOORD_BINDING_LOCATION);
    glVertexAttribPointer(gl::TEXCOORD_BINDING_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 11, (void*)(vec3Size * 3));

    vb.buffers.at(0).Unbind();

    if (instanced) {
        vb.buffers.emplace(1, Buffer{gl::BindingTarget::ARRAY, gl::Usage::DYNAMIC_DRAW});
        vb.buffers.at(1).Bind();
        vb.instanced = 1;
        // mat4 instance info, note: max size for a vertex attribute is a vec4
        constexpr std::size_t vec4Size = sizeof(glm::vec4);
        glEnableVertexAttribArray(gl::INSTANCE_BINDING_LOCATION);
        glVertexAttribPointer(gl::INSTANCE_BINDING_LOCATION, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);

        glEnableVertexAttribArray(gl::INSTANCE_BINDING_LOCATION + 1);
        glVertexAttribPointer(gl::INSTANCE_BINDING_LOCATION + 1, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size));

        glEnableVertexAttribArray(gl::INSTANCE_BINDING_LOCATION + 2);
        glVertexAttribPointer(gl::INSTANCE_BINDING_LOCATION + 2, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size * 2));

        glEnableVertexAttribArray(gl::INSTANCE_BINDING_LOCATION + 3);
        glVertexAttribPointer(gl::INSTANCE_BINDING_LOCATION + 3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size * 3));

        glVertexAttribDivisor(gl::INSTANCE_BINDING_LOCATION, 1);
        glVertexAttribDivisor(gl::INSTANCE_BINDING_LOCATION+1, 1);
        glVertexAttribDivisor(gl::INSTANCE_BINDING_LOCATION+2, 1);
        glVertexAttribDivisor(gl::INSTANCE_BINDING_LOCATION+3, 1);

        vb.buffers.at(1).Unbind();
    }

    glBindVertexArray(0);

    return std::move(vb);
}

VertexBuffer VertexBuffer::vbInitSphereArrayVertexData(const std::vector<float>& buffer, const std::vector<unsigned int>& indexBuffer, bool instanced) {
    VertexBuffer vb;

    vb.buffers.emplace(0, Buffer{gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, buffer});
    vb.buffers.emplace(1, Buffer{gl::BindingTarget::ELEMENT_ARRAY, gl::Usage::STATIC_DRAW, indexBuffer});

    vb.indexed = 1; // id of index buffer

    // pos(3float), normal(3float), color(3float), texcoord(2float)
    glGenVertexArrays(1, &vb.gl_vao);
    glBindVertexArray(vb.gl_vao);
    vb.buffers.at(0).Bind();
    vb.buffers.at(1).Bind();

    glEnableVertexAttribArray(gl::VERTEX_BINDING_LOCATION);
    glVertexAttribPointer(gl::VERTEX_BINDING_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)0);

    glEnableVertexAttribArray(gl::NORMAL_BINDING_LOCATION);
    glVertexAttribPointer(gl::NORMAL_BINDING_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3));

    glEnableVertexAttribArray(gl::TEXCOORD_BINDING_LOCATION);
    glVertexAttribPointer(gl::TEXCOORD_BINDING_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 6));

    vb.buffers.at(0).Unbind();
    vb.buffers.at(1).Unbind();

    if (instanced) {
        vb.buffers.emplace(2, Buffer{gl::BindingTarget::ARRAY, gl::Usage::DYNAMIC_DRAW});
        vb.buffers.at(2).Bind();
        vb.instanced = 2;
        // mat4 instance info, note: max size for a vertex attribute is a vec4
        constexpr std::size_t vec4Size = sizeof(glm::vec4);
        glEnableVertexAttribArray(gl::INSTANCE_BINDING_LOCATION);
        glVertexAttribPointer(gl::INSTANCE_BINDING_LOCATION, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);

        glEnableVertexAttribArray(gl::INSTANCE_BINDING_LOCATION + 1);
        glVertexAttribPointer(gl::INSTANCE_BINDING_LOCATION + 1, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size));

        glEnableVertexAttribArray(gl::INSTANCE_BINDING_LOCATION + 2);
        glVertexAttribPointer(gl::INSTANCE_BINDING_LOCATION + 2, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size * 2));

        glEnableVertexAttribArray(gl::INSTANCE_BINDING_LOCATION + 3);
        glVertexAttribPointer(gl::INSTANCE_BINDING_LOCATION + 3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size * 3));

        glVertexAttribDivisor(gl::INSTANCE_BINDING_LOCATION  , 1);
        glVertexAttribDivisor(gl::INSTANCE_BINDING_LOCATION+1, 1);
        glVertexAttribDivisor(gl::INSTANCE_BINDING_LOCATION+2, 1);
        glVertexAttribDivisor(gl::INSTANCE_BINDING_LOCATION+3, 1);

        vb.buffers.at(2).Unbind();
    }

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

    vb.buffers.emplace(0, Buffer{gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, buffer});
    vb.buffers.at(0).Bind();

    glEnableVertexAttribArray(gl::VERTEX_BINDING_LOCATION);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);

    vb.buffers.at(0).Unbind();

    glBindVertexArray(0);

    return std::move(vb);
}

VertexBuffer VertexBuffer::vbInitVertexDataInstanced(const std::vector<float>& buffer, const VertexLayout& layout) {
    VertexBuffer vb;
    // pos(3float), normal(3float), color(3float), texcoord(2float)
    glGenVertexArrays(1, &vb.gl_vao);
    glBindVertexArray(vb.gl_vao);

    vb.buffers.emplace(0, Buffer{gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, buffer});
    vb.buffers.at(0).Bind();

    std::size_t offset = 0;
    for (const auto& l : layout.attributes) {
        glEnableVertexAttribArray(l.location);
        glVertexAttribPointer(l.location, l.count, (GLenum)l.type, GL_FALSE, layout.stride, (void*)offset);

        offset += l.count * l.element_size;
    }
    
    vb.buffers.at(0).Unbind();

    // instance buffer
    vb.buffers.emplace(1, Buffer{gl::BindingTarget::ARRAY, gl::Usage::DYNAMIC_DRAW});
    vb.buffers.at(1).Bind();
    vb.instanced = 1;

    // mat4 instance info, note: max size for a vertex attribute is a vec4
    constexpr std::size_t vec4Size = sizeof(glm::vec4);
    glEnableVertexAttribArray(gl::INSTANCE_BINDING_LOCATION);
    glVertexAttribPointer(gl::INSTANCE_BINDING_LOCATION, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);

    glEnableVertexAttribArray(gl::INSTANCE_BINDING_LOCATION + 1);
    glVertexAttribPointer(gl::INSTANCE_BINDING_LOCATION + 1, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size));

    glEnableVertexAttribArray(gl::INSTANCE_BINDING_LOCATION + 2);
    glVertexAttribPointer(gl::INSTANCE_BINDING_LOCATION + 2, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size * 2));

    glEnableVertexAttribArray(gl::INSTANCE_BINDING_LOCATION + 3);
    glVertexAttribPointer(gl::INSTANCE_BINDING_LOCATION + 3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size * 3));

    glVertexAttribDivisor(gl::INSTANCE_BINDING_LOCATION, 1);
    glVertexAttribDivisor(gl::INSTANCE_BINDING_LOCATION+1, 1);
    glVertexAttribDivisor(gl::INSTANCE_BINDING_LOCATION+2, 1);
    glVertexAttribDivisor(gl::INSTANCE_BINDING_LOCATION+3, 1);

    glBindVertexArray(0);
    vb.buffers.at(1).Unbind();

    return std::move(vb);
}

VertexBuffer VertexBuffer::vbInitArrayVertexSpec(const std::vector<float>& buffer, const VertexLayout& layout) {
    VertexBuffer vb;
    // pos(3float), normal(3float), color(3float), texcoord(2float)
    glGenVertexArrays(1, &vb.gl_vao);
    glBindVertexArray(vb.gl_vao);

    vb.buffers.emplace(0, Buffer{gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, buffer});
    vb.buffers.at(0).Bind();

    std::size_t offset = 0;
    for (const auto& l : layout.attributes) {
        glEnableVertexAttribArray(l.location);
        glVertexAttribPointer(l.location, l.count, (GLenum)l.type, GL_FALSE, layout.stride, (void*)offset);

        offset += l.count * l.element_size;
    }

    vb.buffers.at(0).Unbind();

    glBindVertexArray(0);

    return std::move(vb);
}

VertexBuffer VertexBuffer::vbInitArrayVertexSpecIndexed(const std::vector<float>& buffer, const std::vector<unsigned int>& indexBuffer, const VertexLayout& layout) {
    VertexBuffer vb;
    // pos(3float), normal(3float), color(3float), texcoord(2float)
    glGenVertexArrays(1, &vb.gl_vao);
    glBindVertexArray(vb.gl_vao);

    vb.buffers.emplace(0, Buffer{gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, buffer});
    vb.buffers.emplace(1, Buffer{gl::BindingTarget::ELEMENT_ARRAY, gl::Usage::STATIC_DRAW, indexBuffer});

    vb.indexed = 1; // id of index buffer

    vb.buffers.at(0).Bind();
    vb.buffers.at(1).Bind();

    std::size_t offset = 0;
    for (const auto& l : layout.attributes) {
        glEnableVertexAttribArray(l.location);
        glVertexAttribPointer(l.location, l.count, (GLenum)l.type, GL_FALSE, layout.stride, (void*)offset);

        offset += l.count * l.element_size;
    }

    vb.buffers.at(0).Unbind();
    vb.buffers.at(1).Unbind();

    glBindVertexArray(0);

    return std::move(vb);
}

VertexBuffer VertexBuffer::vbInitDefault() {
    VertexBuffer vb;
    glGenVertexArrays(1, &vb.gl_vao);
    return std::move(vb);
}

GLuint VertexBuffer::gen_vao_for_attributes(const std::map<int, int>& attributes) {
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    for (const auto& l : attributes) {
        const int binding = l.first;
        const int accessor_id = l.second;

        auto accessor_itr = accessors.find(accessor_id);
        assert(accessor_itr != accessors.end());
        VertexBufferAccessor& vba = accessor_itr->second;
        
        auto buffer_itr= buffers.find(vba.buffer_id);
        assert(buffer_itr != buffers.end());

        buffer_itr->second.Bind();
        glEnableVertexAttribArray(binding);
        glVertexAttribPointer(binding, vba.count, (GLenum)vba.type, vba.normalized ? GL_TRUE : GL_FALSE, vba.stride, (void*)vba.byte_offset);
        buffer_itr->second.Unbind();
    }

    glBindVertexArray(0);
    return vao_id;
}

}