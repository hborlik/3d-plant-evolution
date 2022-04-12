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

#include <ssre.h>

#include <iostream>
#include <string>

#include <ssre_gl.h>
#include <glad/glad.h>

#include <glfw_context.h>

using namespace ssre;

ssre_exception::ssre_exception(std::string description) noexcept : description{std::move(description)} {

}

const char* ssre_exception::what() const noexcept {
    return description.data();
}

ssre_shader_error::ssre_shader_error(std::string shaderName, std::string errorString) noexcept : 
    ssre_exception{"Shader " + shaderName + " caused an error: " + errorString}{

}


void ssre::SSRE_init() {
    static GLFWContext _context{};
}

bool ssre::isGLError() {
    GLenum status = glGetError();
    return status != GL_NO_ERROR;
}