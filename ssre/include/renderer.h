/**
 * @file renderer.h
 * @author Hunter Borlik 
 * @brief Renderer
 * @version 0.1
 * @date 2019-10-20
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#pragma once
#ifndef SSRE_RENDERER_H
#define SSRE_RENDERER_H

#include <memory>
#include <queue>
#include <list>
#include <unordered_map>
#include <map>

#include <ssre_gl.h>
#include <viewport.h>

namespace ssre {

class Program;
class Buffer;
class Texture3D;

/**
 * @brief Render Info. Has information about currently active program.
 * 
 */
class RenderInfo {
public:
    glm::mat4 viewMatrix{1.f};
};

class Drawable {
public:
    virtual void drawColor() = 0;
    virtual void drawDepth() = 0;
    virtual const std::shared_ptr<Program>& getColorProgram() const noexcept = 0;
    virtual const std::shared_ptr<Program>& getDepthProgram() const noexcept = 0;
};

class Scene;

class Renderer {
public:
    static constexpr uint32_t DepthMapWidth = 512, DepthMapHeight = 512;
    static constexpr float DepthMapNear = 0.1f, DepthMapFar = 25.f;

    Renderer();
    ~Renderer();

    const glm::mat4& getViewMatrix() const noexcept {return viewMatrix;}
    void setViewMatrix(const glm::mat4& view) noexcept {viewMatrix = view;}

    const glm::mat4& getProjectionMatrix() const noexcept {return projectionMatrix;}
    void setProjectionMatrix(const glm::mat4& proj) noexcept {projectionMatrix = proj;}

    void setViewPortSize(uint32_t w, uint32_t h) noexcept {width = w; height = h;}

    void setScene(const std::shared_ptr<Scene>& s) {scene = s;}

    void render(float delta);

private:
    double lastDrawTime = 0.;

    std::unique_ptr<RenderInfo> renderInfo;

    glm::mat4 viewMatrix{1.f};
    glm::mat4 projectionMatrix{1.f};

    uint32_t width, height;

    std::shared_ptr<Buffer> globalUBO;
    uint32_t nextUBOLocation = 1;

    // scene to render
    std::shared_ptr<Scene> scene;

    std::vector<GLuint> lightDepthFbs;
    std::vector<std::unique_ptr<Texture3D>> depthTexs;
    std::vector<glm::mat4> lightMatrices;
};

}

#endif // SSRE_RENDERER_H