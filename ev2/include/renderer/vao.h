/**
 * @file vao.h
 * @brief 
 * @date 2022-10-17
 * 
 * 
 */
#ifndef EV2_VAO_H
#define EV2_VAO_H

#include <map>

#include <renderer/ev_gl.h>
#include <renderer/model.h>

namespace ev2::renderer {

class VAOFactory {
public:
    VAOFactory() = delete;

    static GLuint make_vao(const std::map<VertexAttributeLabel, int>& binding_map, const VertexBuffer& vb);
};

}

#endif // EV2_VAO_H