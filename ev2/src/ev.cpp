/**
 * @file ssre.cpp
 * @author Hunter Borlik 
 * @brief definitions for SSRE library header
 * @version 0.1
 * @date 2019-09-18
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include <ev.h>

#include <iostream>
#include <string>

#include <ev_gl.h>
#include <glad/glad.h>

#include <glfw_context.h>

using namespace ev2;

engine_exception::engine_exception(std::string description) noexcept : description{std::move(description)} {

}

const char* engine_exception::what() const noexcept {
    return description.data();
}

shader_error::shader_error(std::string shaderName, std::string errorString) noexcept : 
    engine_exception{"Shader " + shaderName + " caused an error: " + errorString}{

}


void ev2::EV_init() {
    static internal::GLFWContext _context{};
}

bool ev2::isGLError() {
    GLenum status = glGetError();
    return status != GL_NO_ERROR;
}