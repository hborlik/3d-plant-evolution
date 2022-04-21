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
        indexed = o.indexed;
    }
    VertexBuffer& operator=(VertexBuffer&& o) = delete;

    ~VertexBuffer() {
        if (gl_vao != -1)
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

    std::vector<Buffer> buffers;

    int getIndexed() {return indexed;}
private:
    GLuint gl_vao = -1;
    int indexed = -1;
};

struct Mesh {
    size_t start_index;
    size_t num_elements;
    size_t material_id;
};

class Model {
public:
    Model(std::vector<Mesh> meshes, std::vector<Material> materials, glm::vec3 bmin, glm::vec3 bmax, VertexBuffer&& vb) : 
        meshes{std::move(meshes)}, materials{std::move(materials)}, bmin{bmin}, bmax{bmax}, vb{std::move(vb)} {}
    
    std::vector<Mesh>       meshes;
    std::vector<Material>   materials;

    glm::vec3 bmin, bmax;

    VertexBuffer vb;

    void draw();
};

struct VisualInstance {
    glm::mat4   transform;
    Model*      model;
};

}

#endif // EV2_MESH_H