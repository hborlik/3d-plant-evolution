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

namespace ev2 {

class Mesh {
public:
    void init();
private:
    std::unique_ptr<VertexBuffer> vb;
};

}

#endif // EV2_MESH_H