/**
 * @file mesh.h
 * @author Hunter Borlik 
 * @brief 
 * @version 0.1
 * @date 2019-11-12
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#pragma once
#ifndef SSRE_MESH_H
#define SSRE_MESH_H

#include <string>
#include <memory>
#include <vector>

#include <ssre_gl.h>
#include <scene.h>
#include <renderer.h>

namespace ssre {

struct RenderInfo;
class Material;
class Geometry;
class Program;

/**
 * @brief Geometry data pointer and materials to draw it
 */
class Mesh : public Drawable, public Node {
public:
    Mesh(std::string name, std::shared_ptr<Geometry> geom, std::shared_ptr<Program> colorProg, std::shared_ptr<Program> depthProg);
    virtual ~Mesh();

    Mesh& operator=(const Mesh&) = delete;

    void drawColor() override;

    void drawDepth() override;

    const std::shared_ptr<Program>& getColorProgram() const noexcept override {return color_program;}
    const std::shared_ptr<Program>& getDepthProgram() const noexcept override {return depth_program;}

    std::size_t getNumObjects() const noexcept { return objects.size(); }

    /**
     * @brief Set the Material to use for shape
     * 
     * @param shape 
     * @param mat_id 
     */
    void setMaterial(uint32_t shape, uint32_t mat_id);

    void setGeometry(const std::shared_ptr<Geometry>& geom);
    
    void sortDrawOrder();

protected:

    /**
     * @brief Get a new material set from active program
     * 
     */
    void rebuildMaterials();

    /**
     * @brief Update the VAO buffer bindings for each DrawObject.
     */
    void rebuildVAOs();

    void forceVAORebuildOnNextDraw() noexcept {
        lastColorProgramId = std::numeric_limits<uint32_t>::max();
        lastColorModifiedCount = std::numeric_limits<uint32_t>::max();
        lastDepthProgramId = std::numeric_limits<uint32_t>::max();
        lastDepthModifiedCount = std::numeric_limits<uint32_t>::max();
    }
    // track state of program used to build the vaos
    uint32_t lastColorProgramId = std::numeric_limits<uint32_t>::max();
    uint32_t lastColorModifiedCount = std::numeric_limits<uint32_t>::max();
    uint32_t lastDepthProgramId = std::numeric_limits<uint32_t>::max();
    uint32_t lastDepthModifiedCount = std::numeric_limits<uint32_t>::max();

    std::shared_ptr<Geometry> geometry;

    std::vector<std::unique_ptr<Material>> materials;

    // color pass program
    std::shared_ptr<Program> color_program;
    // depth pass program
    std::shared_ptr<Program> depth_program;

    struct VAODrawObject {
        GLuint color_vao_id;
        GLuint depth_vao_id;
        uint32_t elementCount;
        size_t mat_id;
    };
    using VAODrawObjectList = std::vector<VAODrawObject>;

    /**
     * @brief Object list in draw order.
     */
    VAODrawObjectList objects;

    /**
     * @brief Get the Draw Object List
     * 
     * @return const VAODrawObjectList& 
     */
    const VAODrawObjectList& getDrawObjectList() const noexcept { return objects; }
};

}

#endif // SSRE_MESH_H