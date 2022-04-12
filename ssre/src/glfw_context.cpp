/**
 * @file glfw_context.cpp
 * @author Hunter Borlik 
 * @brief 
 * @version 0.1
 * @date 2019-09-09
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include <glfw_context.h>

#include <ssre_gl.h>
#include <ssre.h>

using namespace ssre;

GLFWContext::GLFWContext() {
    if(glfwInit() != GLFW_TRUE) {
        throw ssre_exception{"Cannot initialize GLFW"};
    }
}

GLFWContext::~GLFWContext() {
    glfwTerminate();
}