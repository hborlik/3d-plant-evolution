#include <renderer/model.h>

namespace ev2::renderer {

void VertexBuffer::add_accessors_from_layout(int buffer_id, const VertexBufferLayout& layout) {
    assert(buffers.find(buffer_id) != buffers.end());
    // map the attributes defined in the layout
    for (auto& attr : layout.attributes) {
        accessors.emplace(std::make_pair(attr.attribute, VertexBufferAccessor{
            buffer_id, // id of the target buffer
            0,
            false,
            attr.type,
            attr.count,
            layout.stride})
        );
    }
}

std::pair<VertexBuffer, int32_t> VertexBuffer::vbInitArrayVertexData(const std::vector<float>& vertices, const std::vector<float>& normals, const std::vector<float>& vertex_colors) {
    VertexBuffer vb;

    GLuint gl_vao;
    glGenVertexArrays(1, &gl_vao);
    glBindVertexArray(gl_vao);

    vb.buffers.emplace(mat_spec::VERTEX_BINDING_LOCATION, Buffer{gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, vertices});
    vb.buffers.at(mat_spec::VERTEX_BINDING_LOCATION).Bind();
    glEnableVertexAttribArray(mat_spec::VERTEX_BINDING_LOCATION);
    glVertexAttribPointer(mat_spec::VERTEX_BINDING_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);

    vb.buffers.emplace(mat_spec::NORMAL_BINDING_LOCATION, Buffer{gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, normals});
    vb.buffers.at(mat_spec::NORMAL_BINDING_LOCATION).Bind();
    glEnableVertexAttribArray(mat_spec::NORMAL_BINDING_LOCATION);
    glVertexAttribPointer(mat_spec::NORMAL_BINDING_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)(sizeof(float) * 3));

    vb.buffers.emplace(mat_spec::COLOR_BINDING_LOCATION, Buffer{gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, vertex_colors});
    vb.buffers.at(mat_spec::COLOR_BINDING_LOCATION).Bind();
    glEnableVertexAttribArray(mat_spec::COLOR_BINDING_LOCATION);
    glVertexAttribPointer(mat_spec::COLOR_BINDING_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)(sizeof(float) * 6));

    vb.buffers.at(mat_spec::COLOR_BINDING_LOCATION).Unbind();

    glBindVertexArray(0);

    return std::make_pair(std::move(vb), gl_vao);
}

VertexBuffer VertexBuffer::vbInitArrayVertexData(const std::vector<float>& buffer) {
    VertexBuffer vb;
    // pos(3float), normal(3float), color(3float), texcoord(2float)

    constexpr std::size_t vec3Size = sizeof(glm::vec3);

    vb.add_accessor(VertexAttributeLabel::Vertex,   0, 0         , false, gl::DataType::FLOAT, 3, sizeof(float) * 11);
    vb.add_accessor(VertexAttributeLabel::Normal,   0, vec3Size*1, false, gl::DataType::FLOAT, 3, sizeof(float) * 11);
    vb.add_accessor(VertexAttributeLabel::Color,    0, vec3Size*2, false, gl::DataType::FLOAT, 3, sizeof(float) * 11);
    vb.add_accessor(VertexAttributeLabel::Texcoord, 0, vec3Size*3, false, gl::DataType::FLOAT, 2, sizeof(float) * 11);

    vb.buffers.emplace(0, Buffer{gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, buffer});
    return std::move(vb);
}

VertexBuffer VertexBuffer::vbInitArrayVertexSpecIndexed(const std::vector<float>& buffer, const std::vector<unsigned int>& indexBuffer, const VertexBufferLayout& layout) {
    VertexBuffer vb;
    
    vb.buffers.emplace(0, Buffer{gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, buffer});
    vb.add_accessors_from_layout(0, layout);

    vb.buffers.emplace(1, Buffer{gl::BindingTarget::ELEMENT_ARRAY, gl::Usage::STATIC_DRAW, indexBuffer});
    vb.indexed = 1;
    return std::move(vb);
}

VertexBuffer VertexBuffer::vbInitSphereArrayVertexData(const std::vector<float>& buffer, const std::vector<unsigned int>& indexBuffer) {
    VertexBuffer vb;

    vb.buffers.emplace(0, Buffer{gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, buffer});
    vb.buffers.emplace(1, Buffer{gl::BindingTarget::ELEMENT_ARRAY, gl::Usage::STATIC_DRAW, indexBuffer});

    vb.indexed = 1; // id of index buffer

    // pos(3float), normal(3float), color(3float), texcoord(2float)

    vb.add_accessor(VertexAttributeLabel::Vertex, 0, 0, false, gl::DataType::FLOAT, 3, sizeof(float) * 8);
    vb.add_accessor(VertexAttributeLabel::Normal, 0, sizeof(float) * 3, false, gl::DataType::FLOAT, 3, sizeof(float) * 8);
    vb.add_accessor(VertexAttributeLabel::Texcoord, 0, sizeof(float) * 6, false, gl::DataType::FLOAT, 2, sizeof(float) * 8);

    // vb.buffers.emplace(2, Buffer{gl::BindingTarget::ARRAY, gl::Usage::DYNAMIC_DRAW});
    // // mat4 instance info, note: max size for a vertex attribute is a vec4
    // constexpr std::size_t vec4Size = sizeof(glm::vec4);
    // glEnableVertexAttribArray(mat_spec::INSTANCE_BINDING_LOCATION);
    // glVertexAttribPointer(mat_spec::INSTANCE_BINDING_LOCATION, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);

    // glEnableVertexAttribArray(mat_spec::INSTANCE_BINDING_LOCATION + 1);
    // glVertexAttribPointer(mat_spec::INSTANCE_BINDING_LOCATION + 1, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size));

    // glEnableVertexAttribArray(mat_spec::INSTANCE_BINDING_LOCATION + 2);
    // glVertexAttribPointer(mat_spec::INSTANCE_BINDING_LOCATION + 2, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size * 2));

    // glEnableVertexAttribArray(mat_spec::INSTANCE_BINDING_LOCATION + 3);
    // glVertexAttribPointer(mat_spec::INSTANCE_BINDING_LOCATION + 3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size * 3));

    // glVertexAttribDivisor(mat_spec::INSTANCE_BINDING_LOCATION  , 1);
    // glVertexAttribDivisor(mat_spec::INSTANCE_BINDING_LOCATION+1, 1);
    // glVertexAttribDivisor(mat_spec::INSTANCE_BINDING_LOCATION+2, 1);
    // glVertexAttribDivisor(mat_spec::INSTANCE_BINDING_LOCATION+3, 1);


    return std::move(vb);
}

std::pair<VertexBuffer, int32_t> VertexBuffer::vbInitSST() {
    VertexBuffer vb;
    vb.indexed = -1;

    // pos(3float), normal(3float), color(3float), texcoord(2float)
    GLuint gl_vao;
    glGenVertexArrays(1, &gl_vao);
    glBindVertexArray(gl_vao);

    std::vector<float> buffer{0, 0, 2, 0, 0, 2};

    vb.buffers.emplace(0, Buffer{gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, buffer});
    vb.buffers.at(0).Bind();

    glEnableVertexAttribArray(mat_spec::VERTEX_BINDING_LOCATION);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);

    vb.buffers.at(0).Unbind();

    glBindVertexArray(0);

    return std::make_pair(std::move(vb), gl_vao);
}

VertexBuffer VertexBuffer::vbInitArrayVertexSpec(const std::vector<float>& buffer, const VertexBufferLayout& layout) {
    assert(layout.finalized());
    VertexBuffer vb;
    vb.buffers.emplace(0, Buffer{gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW, buffer});

    // map the attributes defined in the layout
    vb.add_accessors_from_layout(0, layout);

    return std::move(vb);
}

std::pair<VertexBuffer, int32_t> VertexBuffer::vbInitDefault() {
    VertexBuffer vb;
    GLuint gl_vao;
    glGenVertexArrays(1, &gl_vao);
    return std::make_pair(std::move(vb), gl_vao);
}

GLuint VertexBuffer::gen_vao_for_attributes(const std::unordered_map<VertexAttributeLabel, uint32_t>& attributes, const Buffer* instance_buffer) {
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    for (const auto& l : attributes) {
        const int binding = l.second;
        const VertexAttributeLabel accessor_id = l.first;

        auto accessor_itr = accessors.find(accessor_id);
        if(accessor_itr != accessors.end()) {
            VertexBufferAccessor& vba = accessor_itr->second;
        
            auto buffer_itr= buffers.find(vba.buffer_id);
            assert(buffer_itr != buffers.end());

            buffer_itr->second.Bind();
            glEnableVertexAttribArray(binding);
            glVertexAttribPointer(binding, vba.count, (GLenum)vba.type, vba.normalized ? GL_TRUE : GL_FALSE, vba.stride, (void*)vba.byte_offset);
            buffer_itr->second.Unbind();
        } else 
            std::cout << "could not find accessor for " << (int)accessor_id << std::endl;
    }

    if (instance_buffer != nullptr) {
        // bind instance buffer
        instance_buffer->Bind();

        // mat4 instance info, note: max size for a vertex attribute is a vec4
        constexpr std::size_t vec4Size = sizeof(glm::vec4);
        glEnableVertexAttribArray(mat_spec::INSTANCE_BINDING_LOCATION);
        glVertexAttribPointer(mat_spec::INSTANCE_BINDING_LOCATION, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);

        glEnableVertexAttribArray(mat_spec::INSTANCE_BINDING_LOCATION + 1);
        glVertexAttribPointer(mat_spec::INSTANCE_BINDING_LOCATION + 1, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size));

        glEnableVertexAttribArray(mat_spec::INSTANCE_BINDING_LOCATION + 2);
        glVertexAttribPointer(mat_spec::INSTANCE_BINDING_LOCATION + 2, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size * 2));

        glEnableVertexAttribArray(mat_spec::INSTANCE_BINDING_LOCATION + 3);
        glVertexAttribPointer(mat_spec::INSTANCE_BINDING_LOCATION + 3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size * 3));

        glVertexAttribDivisor(mat_spec::INSTANCE_BINDING_LOCATION,  1);
        glVertexAttribDivisor(mat_spec::INSTANCE_BINDING_LOCATION+1, 1);
        glVertexAttribDivisor(mat_spec::INSTANCE_BINDING_LOCATION+2, 1);
        glVertexAttribDivisor(mat_spec::INSTANCE_BINDING_LOCATION+3, 1);

        instance_buffer->Unbind();
    }

    glBindVertexArray(0);
    return vao_id;
}

}