/**
 * @file mesh.h
 * @brief 
 * @date 2022-04-18
 * 
 */
#ifndef EV2_MESH_H
#define EV2_MESH_H

#include <memory>

#include <renderer/buffer.h>
#include <renderer/shader.h>
#include <transform.h>

namespace ev2::renderer {

enum class VertexAttributeType {
    Vertex = 0,
    Normal,
    Color,
    Texcoord
};

struct VertexLayout {
    struct Attribute {
        uint32_t location = 0;
        gl::DataType type = gl::DataType::FLOAT;
        uint16_t count = 0;
        uint16_t element_size = 0;
    };

    VertexLayout& add_attribute(VertexAttributeType type) {
        switch(type) {
            case VertexAttributeType::Vertex:
                attributes.push_back(Attribute{gl::VERTEX_BINDING_LOCATION, gl::DataType::FLOAT, 3, sizeof(float)});
                break;
            case VertexAttributeType::Normal:
                attributes.push_back(Attribute{gl::NORMAL_BINDING_LOCATION, gl::DataType::FLOAT, 3, sizeof(float)});
                break;
            case VertexAttributeType::Color:
                attributes.push_back(Attribute{gl::COLOR_BINDING_LOCATION, gl::DataType::FLOAT, 3, sizeof(float)});
                break;
            case VertexAttributeType::Texcoord:
                attributes.push_back(Attribute{gl::TEXCOORD_BINDING_LOCATION, gl::DataType::FLOAT, 2, sizeof(float)});
                break;
            default:
                break;
        }
        return *this;
    }

    VertexLayout& add_attribute(VertexAttributeType type, gl::DataType data_type, uint16_t count, uint16_t size) {
        uint32_t location = 0;
        switch(type) {
            case VertexAttributeType::Vertex:
                location = gl::VERTEX_BINDING_LOCATION;
                break;
            case VertexAttributeType::Normal:
                location = gl::NORMAL_BINDING_LOCATION;
                break;
            case VertexAttributeType::Color:
                location = gl::COLOR_BINDING_LOCATION;
                break;
            case VertexAttributeType::Texcoord:
                location = gl::TEXCOORD_BINDING_LOCATION;
                break;
            default:
                assert(0);
        }
        attributes.push_back(Attribute{location, data_type, count, size});
        return *this;
    }

    VertexLayout& finalize() {
        uint32_t total_size = 0;
        for (auto& l : attributes) {
            total_size += l.element_size * l.count;
        }
        stride = total_size;
        return *this;
    }

    std::vector<Attribute> attributes;
    uint32_t stride = 0;
};

struct VertexBufferAccessor {
    int buffer_id = -1;     // buffer in VertexBuffer
    size_t byte_offset = 0;
    bool normalized = false;
    gl::DataType type = gl::DataType::FLOAT;
    size_t count = 0;       // required
    size_t stride = 0;
};

class Mesh {
public:
    Mesh() = default;
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&& o) : buffers{std::move(o.buffers)} {
        std::swap(gl_vao, o.gl_vao);
        std::swap(indexed, o.indexed);
        std::swap(instanced, o.instanced);
    }
    Mesh& operator=(Mesh&& o) = delete;

    ~Mesh() {
        if (gl_vao != 0)
            glDeleteVertexArrays(1, &gl_vao);
    }

    void bind() const {glBindVertexArray(gl_vao);}
    void unbind() const {glBindVertexArray(0);}
    
    int get_indexed() const {return indexed;}
    int get_instanced() const {return instanced;}

    void add_buffer(uint32_t buffer_id, Buffer&& buffer) {
        if (buffer.target == gl::BindingTarget::ELEMENT_ARRAY) {
            indexed = buffer_id;
        }
        buffers.emplace(buffer_id, std::move(buffer));
    }

    Buffer& get_buffer(uint32_t buffer_id) {
        return buffers.at(buffer_id);
    }

    void add_accessor(uint32_t accessor_id, uint32_t buffer_id, size_t byte_offset, bool normalized, gl::DataType type, size_t count, size_t stride) {
        accessors.insert_or_assign(
            accessor_id,
            VertexBufferAccessor{(int)buffer_id, byte_offset, normalized, type, count, stride}
        );
    }

    VertexBufferAccessor& get_accessor(uint32_t id) {
        return accessors.at(id);
    }

    /**
     * @brief create a vertex array object using stored accessors and locations specified in map
     * 
     * @param attributes map where key is binding location for attribute and value is the accessor it is targeting in this 
     *  vertex_buffer
     * @return GLuint 
     */
    GLuint gen_vao_for_attributes(const std::map<int, int>& attributes);

    static Mesh vbInitArrayVertexData(const std::vector<float>& vertices, const std::vector<float>& normals, const std::vector<float>& vertex_colors);
    
    /**
     * @brief buffer format pos(3float), normal(3float), color(3float), texcoord(2float)
     * 
     * @param buffer 
     * @return VertexBuffer 
     */
    static Mesh vbInitArrayVertexData(const std::vector<float>& buffer, bool instanced = false);
    static Mesh vbInitSphereArrayVertexData(const std::vector<float>& buffer, const std::vector<unsigned int>& indexBuffer, bool instanced = false);

    /**
     * @brief init vertex buffer for a screen space triangle (vertices only)
     * 
     * @return VertexBuffer 
     */
    static Mesh vbInitSST();

    /**
     * @brief buffer format pos(3float), normal(3float), color(3float), texcoord(2float). 
     *  Instanced array contains mat4 model matrices
     * 
     * @param buffer 
     * @return VertexBuffer 
     */
    static Mesh vbInitVertexDataInstanced(const std::vector<float>& buffer, const VertexLayout& layout);

    static Mesh vbInitArrayVertexSpec(const std::vector<float>& buffer, const VertexLayout& layout);
    static Mesh vbInitArrayVertexSpecIndexed(const std::vector<float>& buffer, const std::vector<unsigned int>& indexBuffer, const VertexLayout& layout);

    static Mesh vbInitDefault();

private:
    std::unordered_map<uint32_t, Buffer> buffers;
    std::unordered_map<uint32_t, VertexBufferAccessor> accessors;
    GLuint gl_vao = 0;

    int indexed = -1;

    int instanced = -1;
};

}

#endif // EV2_MESH_H