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
#include <material.h>

namespace ev2 {

class VertexBuffer {
public:
    VertexBuffer() = default;

    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    VertexBuffer(VertexBuffer&& o) : buffers{std::move(o.buffers)} {
        std::swap(gl_vao, o.gl_vao);
    }
    VertexBuffer& operator=(VertexBuffer&& o) = delete;

    ~VertexBuffer() {
        if (gl_vao != -1)
            glDeleteVertexArrays(1, &gl_vao);
    }
    
    static VertexBuffer vbInitArrayVertexData(const std::vector<float>& vertices, const std::vector<float>& normals, const std::vector<float>& vertex_colors);
    
    /**
     * @brief buffer format pos(3float), normal(3float), color(3float), texcoord(2float)
     * 
     * @param buffer 
     * @return VertexBuffer 
     */
    static VertexBuffer vbInitArrayVertexData(const std::vector<float>& buffer);

    std::vector<Buffer> buffers;

private:
    GLuint gl_vao = -1;
};

struct Mesh {
    size_t start_index;
    size_t num_elements;
    size_t material_id;
};

class Model {
public:
    std::vector<Mesh> meshes;
    std::vector<Material> materials;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;

    VertexBuffer vb;
};

}

#endif // EV2_MESH_H