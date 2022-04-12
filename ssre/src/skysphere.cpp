/**
 * @file skysphere.cpp
 * @author Hunter Borlik 
 * @brief 
 * @version 0.1
 * @date 2019-11-26
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include <ssre.h>
#include <skysphere.h>
#include <shader.h>
#include <material.h>
#include <geometry.h>
#include <resource.h>

using namespace ssre;

SkySphere::SkySphere(std::shared_ptr<Program> prog, MaterialInfo m) : 
    program{std::move(prog)}, 
    skyMaterial{program->createMaterial(m)},
    geometry{Resource::StaticInst().loadObj("sphere.obj")} {

    SSRE_CHECK_THROW(program, "program must not be null!");

    glGenVertexArrays(1, &vaoid);
    GLint vertl = program->getInputInfo(mat_spec::VertexAttributeName).Location;
    GLint texl = program->getInputInfo(mat_spec::TextureAttributeName).Location;
    geometry->configVaoAttribPtrs(0, vaoid, vertl, -1, -1, texl);
    nElements = geometry->getDrawObjects()[0].numElements;
}

void SkySphere::drawColor() {

    GLint mloc = program->getUniformInfo(mat_spec::ModelMatrixUniformName).Location;
    if(mloc != -1) {
        GL_CHECKED_CALL(gl::glUniform(mm, mloc));
    }

    glCullFace(GL_FRONT);
    glBindVertexArray(vaoid);
    program->applyMaterial(skyMaterial);
    GL_CHECKED_CALL(glDrawElements(GL_TRIANGLES, nElements, GL_UNSIGNED_INT, (void*)0));
    glCullFace(GL_BACK);
}