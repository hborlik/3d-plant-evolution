/**
 * @file mesh.h
 * @brief 
 * @date 2022-04-18
 * 
 */
#ifndef EV2_MESH_H
#define EV2_MESH_H

#include <memory>

#include <buffer.h>
#include <shader.h>
#include <transform.h>

namespace ev2 {

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

enum class VertexFormat {
    Array = 0,
    Indexed
};

class VertexBuffer {
public:
    VertexBuffer() = default;

    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    VertexBuffer(VertexBuffer&& o) : buffers{std::move(o.buffers)} {
        std::swap(gl_vao, o.gl_vao);
        indexed = o.indexed;
    }
    VertexBuffer& operator=(VertexBuffer&& o) = delete;

    ~VertexBuffer() {
        if (gl_vao != 0)
            glDeleteVertexArrays(1, &gl_vao);
    }

    void bind() const {glBindVertexArray(gl_vao);}
    void unbind() const {glBindVertexArray(0);}
    
    static VertexBuffer vbInitArrayVertexData(const std::vector<float>& vertices, const std::vector<float>& normals, const std::vector<float>& vertex_colors);
    
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
    static VertexBuffer vbInitSST();

    /**
     * @brief buffer format pos(3float), normal(3float), color(3float), texcoord(2float). 
     *  Instanced array contains mat4 model matrices
     * 
     * @param buffer 
     * @return VertexBuffer 
     */
    static VertexBuffer vbInitArrayVertexDataInstanced(const std::vector<float>& buffer);

    static VertexBuffer vbInitArrayVertexSpec(const std::vector<float>& buffer, const VertexLayout& layout);
    static VertexBuffer vbInitArrayVertexSpecIndexed(const std::vector<float>& buffer, const std::vector<unsigned int>& indexBuffer, const VertexLayout& layout);

    std::vector<Buffer> buffers;

    int get_indexed() {return indexed;}
private:
    GLuint gl_vao = 0;
    int indexed = -1;
};

struct Mesh {
    size_t  start_index;
    size_t  num_elements;
    int32_t material_id;
};

class Model {
public:
    Model(std::vector<Mesh> meshes, glm::vec3 bmin, glm::vec3 bmax, std::vector<float> vb, VertexFormat format) : 
        meshes{std::move(meshes)}, bmin{bmin}, bmax{bmax}, buffer{std::move(vb)}, bufferFormat{format} {}
    
    std::vector<Mesh>       meshes;
    std::vector<float>      buffer;

    glm::vec3 bmin, bmax;

    VertexFormat bufferFormat;
};

}

#endif // EV2_MESH_H