/**
 * @file renderer.cpp
 * @author Hunter Borlik 
 * @brief 
 * @version 0.1
 * @date 2019-10-21
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include <renderer.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <geometry.h>
#include <shader.h>
#include <scene.h>
#include <camera.h>
#include <skysphere.h>
#include <window.h>
#include <texture.h>

using namespace ssre;
//////////////////////////////////////////////////////////////////////////

Renderer::Renderer() : 
    renderInfo{std::make_unique<RenderInfo>()},
    globalUBO{std::make_unique<Buffer>(gl::BindingTarget::UNIFORM, gl::Usage::DYNAMIC_DRAW)} {

    // create depth cubemaps
    for(int i = 0; i < mat_spec::GUBMaxNumLights; i++) {
        depthTexs.push_back(std::make_unique<Texture3D>("depthTex"));
    }

    // config depth textures
    for(auto& depthTex : depthTexs) {
        depthTex->setFilterMode(gl::TextureParamFilter::TEXTURE_MIN_FILTER, gl::TextureFilterMode::NEAREST);
        depthTex->setFilterMode(gl::TextureParamFilter::TEXTURE_MAG_FILTER, gl::TextureFilterMode::NEAREST);
        depthTex->setWrapMode(gl::TextureParamWrap::TEXTURE_WRAP_S, gl::TextureWrapMode::CLAMP_TO_EDGE);
        depthTex->setWrapMode(gl::TextureParamWrap::TEXTURE_WRAP_T, gl::TextureWrapMode::CLAMP_TO_EDGE);
        depthTex->setWrapMode(gl::TextureParamWrap::TEXTURE_WRAP_R, gl::TextureWrapMode::CLAMP_TO_EDGE);

        // allocate depth cube map
        for(uint32_t i = 0; i < 6; i++)
            depthTex->setData(nullptr, gl::PixelFormat::DEPTH_COMPONENT, gl::PixelType::FLOAT, gl::TextureInternalFormat::DEPTH_COMPONENT16, DepthMapWidth, DepthMapHeight, i);
    }

    // create depth frame buffers
    lightDepthFbs.resize(mat_spec::GUBMaxNumLights);
    glGenFramebuffers(10, lightDepthFbs.data());
    for(int i = 0; i < mat_spec::GUBMaxNumLights; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, lightDepthFbs[i]);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexs[i]->getHandle(), 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        SSRE_CHECK_THROW(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Light depth framebuffer is not complete!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // enable z buffer testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    globalUBO->Allocate(mat_spec::GUBSize);
    //glBindBufferBase(GL_UNIFORM_BUFFER, mat_spec::GUBBindingLocation, globalUBO->Handle());
    glBindBufferRange(GL_UNIFORM_BUFFER, mat_spec::GUBBindingLocation, globalUBO->Handle(), 0, mat_spec::GUBSize);
}

Renderer::~Renderer() {
    glDeleteFramebuffers(mat_spec::GUBMaxNumLights, lightDepthFbs.data());
}

void Renderer::render(float delta) {
    uint32_t programInUse = 0;

    // update step
    scene->update(delta);

    // prepare scene
    scene->pre_render();

    glm::vec3 camPos{};
    if(Camera* c = scene->getCamera().get()) {
        viewMatrix = c->getViewMatrix();
        camPos = c->getPosition();
    }

    globalUBO->SubData(viewMatrix, mat_spec::GUBViewMatOffset);
    globalUBO->SubData(projectionMatrix, mat_spec::GUBProjectionMatOffset);
    globalUBO->SubData(camPos, mat_spec::GUBCameraPosOffset);

    // get lights
    std::vector<glm::vec3> lpositions;
    std::vector<glm::vec3> lcolors;
    for(auto& node : scene->getNodes()) {
        if(PointLight* p = dynamic_cast<PointLight*>(node.second.get())) {
            lpositions.push_back({p->getModelMatrix() * glm::vec4{0, 0, 0, 1}});
            lcolors.push_back(p->getRadiantIntensity());
        }
    }
    size_t nLights = std::min<size_t>(lpositions.size(), mat_spec::GUBMaxNumLights);
    lpositions.resize(mat_spec::GUBMaxNumLights);
    lcolors.resize(mat_spec::GUBMaxNumLights);

    // depth pass
    glViewport(0, 0, DepthMapWidth, DepthMapHeight);
    // enable hardware depth biasing
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.1f, 4.0f);
    for(uint32_t i = 0; i < nLights; i++) {

        glBindFramebuffer(GL_FRAMEBUFFER, lightDepthFbs[i]);
        glClear(GL_DEPTH_BUFFER_BIT);
        glm::vec3 lpos = lpositions[i];

        // create light space matrices
        lightMatrices.clear();
        glm::mat4 lproj = glm::perspective(0.5f*glm::pi<float>(), (float)(DepthMapWidth)/DepthMapHeight, DepthMapNear, DepthMapFar);
        lightMatrices.push_back(lproj*glm::lookAt(lpos, lpos + glm::vec3{1, 0, 0}, {0, -1, 0}));
        lightMatrices.push_back(lproj*glm::lookAt(lpos, lpos + glm::vec3{-1, 0, 0}, {0, -1, 0}));
        lightMatrices.push_back(lproj*glm::lookAt(lpos, lpos + glm::vec3{0, 1, 0}, {0, 0, 1}));
        lightMatrices.push_back(lproj*glm::lookAt(lpos, lpos + glm::vec3{0, -1, 0}, {0, 0, -1}));
        lightMatrices.push_back(lproj*glm::lookAt(lpos, lpos + glm::vec3{0, 0, 1}, {0, -1, 0}));
        lightMatrices.push_back(lproj*glm::lookAt(lpos, lpos + glm::vec3{0, 0, -1}, {0, -1, 0}));

        programInUse = 0;
        for(auto& drawable : scene->getNodes()) {
            if(Drawable* d = dynamic_cast<Drawable*>(drawable.second.get())) {
                if(d->getDepthProgram()) {
                    if(programInUse != d->getDepthProgram()->program_id) {
                        programInUse = d->getDepthProgram()->program_id;
                        d->getDepthProgram()->setShaderParameter("SM[0]", lightMatrices);
                        d->getDepthProgram()->setShaderParameter("lightFarPlane", (float)DepthMapFar);
                        d->getDepthProgram()->setShaderParameter("lightNearPlane", (float)DepthMapNear);
                        d->getDepthProgram()->use();
                        // uniform vec3 lightPos;
                        // uniform float far_plane;
                        glUniform3fv(d->getDepthProgram()->getUniformInfo("lightPos").Location, 1, &lpos[0]);
                    }
                    d->drawDepth();
                }
            }
        }
    }
    glDisable(GL_POLYGON_OFFSET_FILL);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

   
    // update point light positions
    globalUBO->SubData(lpositions, mat_spec::GUBLightPositionsArrayOffset, mat_spec::GUBLightPositionsArrayStride);
    globalUBO->SubData(lcolors, mat_spec::GUBLightColorsOffset, mat_spec::GUBLightColorsArrayStride);
    globalUBO->SubData<GLuint>(nLights, mat_spec::GUBNumLightsOffset);

    static float SamplerDiskRadius = 0.1;
    if(Window::StaticInst().arrow_up) {
        SamplerDiskRadius += 0.05 * delta;
        std::cout << "SamplerDiskRadius: " << SamplerDiskRadius << std::endl;
    }
    if(Window::StaticInst().arrow_down) {
        SamplerDiskRadius -= 0.05 * delta;
        std::cout << "SamplerDiskRadius: " << SamplerDiskRadius << std::endl;
    }

    // color pass
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    programInUse = 0;
    for(auto& drawable : scene->getNodes()) {
        if(Drawable* d = dynamic_cast<Drawable*>(drawable.second.get())) {
            if(programInUse != d->getColorProgram()->program_id) {
                programInUse = d->getColorProgram()->program_id;
                d->getColorProgram()->setShaderParameter("lightFarPlane", (float)DepthMapFar);
                d->getColorProgram()->setShaderParameter("lightNearPlane", (float)DepthMapNear);
                d->getColorProgram()->use();

                glUniform1f(d->getColorProgram()->getUniformInfo("SamplingRadius").Location, SamplerDiskRadius);

                for(uint32_t light = 0; light < nLights; light++) {
                    glUniform1i(d->getColorProgram()->getUniformInfo("lightDepthTex[0]").Location + light, 10 + light);
                    glActiveTexture(gl::TextureUnit[10 + light]);
                    depthTexs[light]->bind();
                }
            }
            d->drawColor();
        }
    }

    if(scene->getSkySphere()) {
        scene->getSkySphere()->getColorProgram()->use();
        scene->getSkySphere()->drawColor();
    }
}