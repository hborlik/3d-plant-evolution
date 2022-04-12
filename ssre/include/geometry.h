/**
 * @file geometry.h
 * @author Hunter Borlik 
 * @brief 
 * @version 0.1
 * @date 2019-10-05
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#pragma once
#ifndef SSRE_GEOMETRY_H
#define SSRE_GEOMETRY_H

#include <memory>
#include <list>

#include <glm/glm.hpp>

#include <ssre.h>
#include <buffer.h>
#include <material.h>

namespace ssre {

/**
 * @brief 
 * 
 */
struct DrawObject {
    std::unique_ptr<Buffer> element_buffer{
        std::make_unique<Buffer>(gl::BindingTarget::ELEMENT_ARRAY, gl::Usage::STATIC_DRAW)
    };
    size_t numElements;
    size_t material_id;
};

constexpr size_t GeomSizeAndStride = 11;
constexpr size_t GeomSizeAndStrideBytes = GeomSizeAndStride * sizeof(GLfloat);
constexpr size_t GeomVertexOffsetBytes = 0;
constexpr size_t GeomTangentOffsetBytes = 3 * sizeof(GLfloat);
constexpr size_t GeomBitangentOffset = 6 * sizeof(GLfloat);
constexpr size_t GeomTexCoordOffsetBytes = 9 * sizeof(GLfloat);

/**
 * @brief Geometry data
 * layout:
 *      index
 *        |
 *        |
 *        v
 *      +---+---+---+---+---+---+---+---+---+---+---+---+
 *      | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |10 | ...
 *      |vx |vy |vz |tx |ty |tz |bx |by |bz |tx |ty | ...
 *      +---+---+---+---+---+---+---+---+---+---+---+---+
 *      
 *      vertex data:
 *          offset: 0 * sizeof(GLfloat)
 *          size:   3
 *          stride: 11 * sizeof(GLfloat)
 * 
 *      tangent data:
 *          offset: 3 * sizeof(GLfloat)
 *          size:   3
 *          stride: 11 * sizeof(GLfloat)
 * 
 *      bitangent data:
 *          offset: 6 * sizeof(GLfloat)
 *          size:   3
 *          stride: 11 * sizeof(GLfloat)
 * 
 *      tex coord:
 *          offset: 9 * sizeof(GLfloat)
 *          size:   2
 *          stride: 11 * sizeof(GLfloat)
 *          
 */
class Geometry {
public:
    Geometry(std::vector<DrawObject> dobjs, const std::vector<GLfloat>& data);
    Geometry(float uvextent);

    void setMaterials(const std::vector<MaterialInfo>& mat) noexcept {materials = mat;}
    const std::vector<MaterialInfo>& getMaterials() const noexcept {return materials;}

    const std::unique_ptr<Buffer>& getBuffer() const noexcept {return buffer;}

    bool setMaterialId(uint32_t shape, int32_t id) {
        if(shape < drawObjects.size() && id < materials.size()) {
            drawObjects[shape].material_id = id;
            return true;
        }
        return false;
    }
    size_t getMaterialId(uint32_t shape) const {
        return (shape < drawObjects.size() ? drawObjects[shape].material_id : 0);
    }

    /**
     * @brief Add a new material
     * 
     * @param mat 
     * @return uint32_t id of added material, can be used to set shape material id
     */
    size_t addMaterial(const MaterialInfo& mat) {
        size_t id = materials.size();
        materials.push_back(mat);
        return id;
    }

    const MaterialInfo& getMaterial(uint32_t shape) const {
        return materials[getMaterialId(shape)];
    }

    size_t nObjects() const noexcept {return drawObjects.size();}
    const std::vector<DrawObject>& getDrawObjects() const noexcept {return drawObjects;}

    /**
     * @brief configure a vao to point to buffers with geom data
     * 
     * @param vao_id 
     * @param vert_loc 
     * @param tan_loc 
     * @param bitan_loc 
     * @param texc_loc 
     */
    void configVaoAttribPtrs(uint32_t shape, GLuint vao_id, GLint vert_loc, GLint tan_loc, GLint bitan_loc, GLint texc_loc);

protected:
    /**
     * @brief Buffers for mesh data. All index buffers count from start of vertex data buffers. 
     * vertex_buffer, normal_buffer, and tex_buffer shared between all
     */
    std::unique_ptr<Buffer> buffer;

    std::vector<DrawObject> drawObjects;

    // materials indexed by material id
    std::vector<MaterialInfo> materials;
};

}

#endif // SSRE_GEOMETRY_H