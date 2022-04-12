/**
 * @file skysphere.h
 * @author Hunter Borlik 
 * @brief 
 * @version 0.1
 * @date 2019-11-21
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#pragma once
#ifndef SSRE_SKYSPHERE_H
#define SSRE_SKYSPHERE_H

#include <renderer.h>

namespace ssre {

class Material;
class MaterialInfo;
class Program;
class Geometry;

class SkySphere : public Drawable {
public:
    SkySphere(std::shared_ptr<Program> prog, MaterialInfo m);

    void setModelMatrix(const glm::mat4& matrix) noexcept {mm = matrix;}

    void drawColor() override;
    void drawDepth() override {}

    const std::shared_ptr<Program>& getColorProgram() const noexcept override {return program;}
    const std::shared_ptr<Program>& getDepthProgram() const noexcept override {return depthProg;}

private:
    std::shared_ptr<Program> program;
    std::shared_ptr<Program> depthProg;
    std::unique_ptr<Material> skyMaterial;
    std::shared_ptr<Geometry> geometry;

    glm::mat4 mm{1.f};

    GLuint vaoid;
    GLuint nElements;
};

}

#endif // SSRE_SKYSPHERE_H