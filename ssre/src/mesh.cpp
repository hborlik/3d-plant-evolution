/**
 * @file mesh.cpp
 * @author Hunter Borlik 
 * @brief 
 * @version 0.1
 * @date 2019-11-12
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include <mesh.h>

#include <renderer.h>
#include <material.h>
#include <geometry.h>
#include <shader.h>

using namespace ssre;

Mesh::Mesh(std::string name, std::shared_ptr<Geometry> geom, std::shared_ptr<Program> prog, std::shared_ptr<Program> depthProg) : 
    Node{std::move(name)}, 
    geometry{std::move(geom)}, 
    color_program{std::move(prog)}, 
    depth_program{std::move(depthProg)} 
    {
    SSRE_CHECK_THROW(color_program, "Mesh cannot be created, program invalid");
    SSRE_CHECK_THROW(geometry, "Mesh cannot be created, geometry invalid");
    rebuildMaterials();
}

Mesh::~Mesh() {
    for(auto& obj : objects) {
        glDeleteVertexArrays(1, &obj.color_vao_id);
        glDeleteVertexArrays(1, &obj.depth_vao_id);
    }
}

void Mesh::drawColor() {
    if(color_program->getModifiedCount() != lastColorModifiedCount) {
        rebuildVAOs();
        // save vao info
        lastColorModifiedCount = color_program->getModifiedCount();
    }

    // upload mesh specific uniform
    GLint mloc = color_program->getUniformInfo(mat_spec::ModelMatrixUniformName).Location;
    if(mloc != -1) {
        GL_CHECKED_CALL(gl::glUniform(getModelMatrix(), mloc));
    }

    mloc = color_program->getUniformInfo(mat_spec::NormalMatrixUniformName).Location;
    if(mloc != -1) {
        // normal transform
        GL_CHECKED_CALL(gl::glUniform(glm::mat3{glm::transpose(glm::inverse(getModelMatrix()))}, mloc));
    }

    int32_t lastMaterialID = materials.size();
    for(auto& dro : objects) {
        glBindVertexArray(dro.color_vao_id);
        // check if material parameters need to be updated
        if(dro.mat_id != lastMaterialID) {
            color_program->applyMaterial(materials[dro.mat_id]);// upload all uniform data
            lastMaterialID = dro.mat_id;
        }
        
        GL_CHECKED_CALL(glDrawElements(GL_TRIANGLES, dro.elementCount, GL_UNSIGNED_INT, (void*)0));
    }
    glBindVertexArray(0);
}

void Mesh::drawDepth() {
    if(depth_program->getModifiedCount() != lastDepthModifiedCount) {
        rebuildVAOs();
        // save vao info
        lastDepthModifiedCount = depth_program->getModifiedCount();
    }

    // upload mesh specific uniform
    GLint mloc = color_program->getUniformInfo(mat_spec::ModelMatrixUniformName).Location;
    if(mloc != -1) {
        GL_CHECKED_CALL(gl::glUniform(getModelMatrix(), mloc));
    }

    mloc = color_program->getUniformInfo(mat_spec::NormalMatrixUniformName).Location;
    if(mloc != -1) {
        // normal transform
        GL_CHECKED_CALL(gl::glUniform(glm::mat3{glm::transpose(glm::inverse(getModelMatrix()))}, mloc));
    }

    for(auto& dro : objects) {
        glBindVertexArray(dro.color_vao_id);
        
        GL_CHECKED_CALL(glDrawElements(GL_TRIANGLES, dro.elementCount, GL_UNSIGNED_INT, (void*)0));
    }
    glBindVertexArray(0);
}

void Mesh::setMaterial(uint32_t shape, uint32_t mat_id) {
    if(shape < objects.size()) {
        objects[shape].mat_id = mat_id;
    }
}

void Mesh::setGeometry(const std::shared_ptr<Geometry>& geom) {
    if(geom) {
        geometry = geom;
        rebuildMaterials();
        forceVAORebuildOnNextDraw();
    }
}

void Mesh::rebuildVAOs() {
    for(uint32_t i = objects.size(); i < geometry->nObjects(); i++) {
        GLuint vao0, vao1;
        glGenVertexArrays(1, &vao0);
        glGenVertexArrays(1, &vao1);
        objects.push_back(VAODrawObject{vao0, vao1, 0, 0});
    }

    const auto& geomObj = geometry->getDrawObjects();
    for(size_t i = 0; i < objects.size(); i++) {
        auto& obj = objects[i];
        const GLint color_vert_loc = color_program->getVertexInputLocation();
        const GLint color_texc_loc = color_program->getTexChoordInputLocation();
        const GLint color_tan_loc = color_program->getTangentInputLocation();
        const GLint color_bitan_loc = color_program->getBitangentInputLocation();

        const GLint depth_vert_loc = depth_program->getVertexInputLocation();
        const GLint depth_texc_loc = depth_program->getTexChoordInputLocation();
        const GLint depth_tan_loc = depth_program->getTangentInputLocation();
        const GLint depth_bitan_loc = depth_program->getBitangentInputLocation();

        obj.mat_id = geomObj[i].material_id;
        obj.elementCount = geomObj[i].numElements;

        // bind element array and config vertex inputs
        geometry->configVaoAttribPtrs(i, obj.color_vao_id, color_vert_loc, color_tan_loc, color_bitan_loc, color_texc_loc);
        geometry->configVaoAttribPtrs(i, obj.depth_vao_id, depth_vert_loc, depth_tan_loc, depth_bitan_loc, depth_texc_loc);
    }
    glBindVertexArray(0);
}

void Mesh::rebuildMaterials() {
    materials.clear();
    const std::vector<MaterialInfo>& mats = geometry->getMaterials();
    for(uint32_t i = 0; i < mats.size(); i++) {
        // get a new material for each geometry material config
        std::unique_ptr<Material> newMat = color_program->createMaterial(mats[i]);
        materials.push_back(std::move(newMat));
    }
}