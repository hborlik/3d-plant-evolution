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
    Buffer vb;

private:
    GLuint gl_vbo;
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

    VertexBuffer buffer;

    void draw();
};

}

#endif // EV2_MESH_H