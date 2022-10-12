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
#include <renderer/texture.h>
#include <transform.h>

namespace ev2::renderer {

struct Material {
    std::string name = "default";

    glm::vec3 diffuse   = {1.00f,0.10f,0.85f};
    glm::vec3 emissive  = {};
    float metallic       = 0;
    float subsurface     = 0;
    float specular       = .5f;
    float roughness      = .5f;
    float specularTint   = 0;
    float clearcoat      = 0;
    float clearcoatGloss = 1.f;
    float anisotropic    = 0;
    float sheen          = 0;
    float sheenTint      = .5f;

    std::shared_ptr<Texture> ambient_tex;             // map_Ka
    std::shared_ptr<Texture> diffuse_tex;             // map_Kd
    std::shared_ptr<Texture> specular_tex;            // map_Ks
    std::shared_ptr<Texture> specular_highlight_tex;  // map_Ns
    std::shared_ptr<Texture> bump_tex;                // map_bump, map_Bump, bump
    std::shared_ptr<Texture> displacement_tex;        // disp
    std::shared_ptr<Texture> alpha_tex;               // map_d
    std::shared_ptr<Texture> reflection_tex;          // refl

    Material() = default;
    Material(std::string name) : name{std::move(name)} {}

    Material(const Material&) = default;
    Material(Material&&) = default;
    Material& operator=(const Material&) = default;
    Material& operator=(Material&&) noexcept = default;

private:
    friend class Renderer;

    bool is_registered() noexcept {return material_id != -1 && material_slot != -1;}

    int32_t material_id = -1;
    int32_t material_slot = -1;
};

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

class VertexBuffer {
public:
    VertexBuffer() = default;
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    VertexBuffer(VertexBuffer&& o) : buffers{std::move(o.buffers)} {
        std::swap(indexed, o.indexed);
    }
    VertexBuffer& operator=(VertexBuffer&& o) = delete;
    
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
     * @param attributes map where key is the attribute identifier and value is the binding location in the shader 
     *  vertex_buffer
     * @return GLuint 
     */
    GLuint gen_vao_for_attributes(const std::unordered_map<int, int>& attributes);

    static std::pair<VertexBuffer, int32_t> vbInitArrayVertexData(const std::vector<float>& vertices, const std::vector<float>& normals, const std::vector<float>& vertex_colors);
    
    /**
     * @brief buffer format pos(3float), normal(3float), color(3float), texcoord(2float)
     * 
     * @param buffer 
     * @return VertexBuffer 
     */
    static std::pair<VertexBuffer, int32_t> vbInitArrayVertexData(const std::vector<float>& buffer, bool instanced = false);
    static std::pair<VertexBuffer, int32_t> vbInitSphereArrayVertexData(const std::vector<float>& buffer, const std::vector<unsigned int>& indexBuffer, bool instanced = false);

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
    static std::pair<VertexBuffer, int32_t> vbInitVertexDataInstanced(const std::vector<float>& buffer, const Buffer& instance_buffer, const VertexLayout& layout);

    static std::pair<VertexBuffer, int32_t> vbInitArrayVertexSpec(const std::vector<float>& buffer, const VertexLayout& layout);
    static std::pair<VertexBuffer, int32_t> vbInitArrayVertexSpecIndexed(const std::vector<float>& buffer, const std::vector<unsigned int>& indexBuffer, const VertexLayout& layout);

    static std::pair<VertexBuffer, int32_t> vbInitDefault();

private:
    std::unordered_map<uint32_t, Buffer> buffers;
    std::unordered_map<uint32_t, VertexBufferAccessor> accessors;

    int indexed = -1;
};

}

#endif // EV2_MESH_H