/**
 * @file model.h
 * @brief 
 * @date 2022-04-18
 * 
 */
#ifndef EV2_MODEL_H
#define EV2_MODEL_H

#include <memory>

#include <renderer/buffer.h>
#include <renderer/shader.h>
#include <renderer/texture.h>
#include <transform.h>

namespace ev2::renderer {

struct VertexBufferAccessor {
    int         buffer_id   = -1;     // buffer in VertexBuffer
    size_t      byte_offset = 0;
    bool        normalized  = false;
    gl::DataType type       = gl::DataType::FLOAT;
    size_t      count       = 0;       // required
    size_t      stride      = 0;
};

struct VertexBufferLayout {
    struct Attribute {
        VertexAttributeLabel attribute   = VertexAttributeLabel::Vertex;
        gl::DataType        type        = gl::DataType::FLOAT;
        uint16_t            count       = 0;
        uint16_t            element_size= 0;
        uint16_t            offset = 0;    // calculated offset. This is populated by finalize()
    };

    VertexBufferLayout& add_attribute(VertexAttributeLabel type) {
        // Layout should not be finalized
        assert(!finalized());
        switch(type) {
            case VertexAttributeLabel::Vertex:
            case VertexAttributeLabel::Normal:
            case VertexAttributeLabel::Color:
                attributes.push_back(Attribute{type, gl::DataType::FLOAT, 3, sizeof(float), 0});
                break;
            case VertexAttributeLabel::Texcoord:
                attributes.push_back(Attribute{type, gl::DataType::FLOAT, 2, sizeof(float), 0});
                break;
            default:
                break;
        }
        return *this;
    }

    VertexBufferLayout& add_attribute(VertexAttributeLabel type, gl::DataType data_type, uint16_t count, uint16_t size) {
        // Layout should not be finalized
        assert(!finalized());
        attributes.push_back(Attribute{type, data_type, count, size, 0});
        return *this;
    }

    void set_attribute_divisor(uint16_t divisor) {
        this->divisor = divisor;
    }

    VertexBufferLayout& finalize() {
        uint32_t total_size = 0;
        for (auto& l : attributes) {
            l.offset = total_size;
            total_size += l.element_size * l.count;
        }
        stride = total_size;
        return *this;
    }

    bool finalized() const noexcept {
        return stride != 0;
    }

    std::vector<Attribute> attributes;
    uint32_t stride = 0;
    uint16_t divisor = 0;
};

class VertexBuffer {
public:
    VertexBuffer() = default;
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    VertexBuffer(VertexBuffer&& o) noexcept { *this = std::move(o); }
    
    VertexBuffer& operator=(VertexBuffer&& o) noexcept {
        std::swap(buffers, o.buffers);
        std::swap(indexed, o.indexed);
        std::swap(accessors, o.accessors);
        return *this;
    }
    
    int get_indexed() const {return indexed;}

    void add_buffer(uint32_t buffer_id, Buffer&& buffer) {
        if (buffer.target == gl::BindingTarget::ELEMENT_ARRAY) {
            indexed = buffer_id;
        }
        buffers.emplace(buffer_id, std::move(buffer));
    }

    Buffer& get_buffer(uint32_t buffer_id) {
        return buffers.at(buffer_id);
    }

    inline void add_accessor(VertexAttributeLabel accessor, uint32_t buffer_id, size_t byte_offset, bool normalized, gl::DataType type, size_t count, size_t stride) {
        accessors.insert_or_assign(
            accessor,
            VertexBufferAccessor{(int)buffer_id, byte_offset, normalized, type, count, stride}
        );
    }

    inline VertexBufferAccessor& get_accessor(VertexAttributeLabel accessor) {
        return accessors.at(accessor);
    }

    /**
     * @brief Set the accessors from layout for a specific buffer in Vertex buffer set
     * 
     * @param buffer_id buffer that is the target of the layout. Buffer should be in buffers map
     * @param layout vertex buffer layout
     */
    void add_accessors_from_layout(int buffer_id, const VertexBufferLayout& layout);

    /**
     * @brief create a vertex array object using stored accessors and locations specified in map
     * 
     * @param attributes map where key is the attribute identifier and value is the binding location in the shader 
     *  vertex_buffer
     * @return GLuint 
     */
    GLuint gen_vao_for_attributes(const std::unordered_map<VertexAttributeLabel, uint32_t>& attributes, const Buffer* instance_buffer=nullptr);

    static std::pair<VertexBuffer, int32_t> vbInitArrayVertexData(const std::vector<float>& vertices, const std::vector<float>& normals, const std::vector<float>& vertex_colors);
    
    /**
     * @brief buffer format pos(3float), normal(3float), color(3float), texcoord(2float)
     * 
     * @param buffer 
     * @return VertexBuffer 
     */
    static VertexBuffer vbInitArrayVertexData(const std::vector<float>& buffer);
    static VertexBuffer vbInitSphereArrayVertexData(const std::vector<float>& buffer, const std::vector<unsigned int>& indexBuffer);

    /**
     * @brief init vertex buffer for a screen space triangle (vertices only)
     * 
     * @return VertexBuffer 
     */
    static std::pair<VertexBuffer, int32_t> vbInitSST();

    /**
     * @brief buffer format pos(3float), normal(3float), color(3float), texcoord(2float). 
     *  Instanced array contains mat4 model matrices
     * 
     * @param buffer 
     * @param instance_buffer instance buffer to be used with the returned VAO
     * @return VertexBuffer 
     */
    static std::pair<VertexBuffer, int32_t> vbInitVertexDataInstanced(const std::vector<float>& buffer, const Buffer& instance_buffer, const VertexBufferLayout& layout);

    static VertexBuffer vbInitArrayVertexSpec(const std::vector<float>& buffer, const VertexBufferLayout& layout);
    static VertexBuffer vbInitArrayVertexSpecIndexed(const std::vector<float>& buffer, const std::vector<unsigned int>& indexBuffer, const VertexBufferLayout& layout);

    static std::pair<VertexBuffer, int32_t> vbInitDefault();

    // buffer accessors
    std::map<VertexAttributeLabel, VertexBufferAccessor> accessors;

private:
    std::unordered_map<int, Buffer> buffers;

    int indexed = -1;
};

}

#endif // EV2_MESH_H