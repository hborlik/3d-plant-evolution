/**
 * @file glfw_context.cpp
 * @author Hunter Borlik 
 * @brief 
 * @version 0.1
 * @date 2019-09-09
 * 
 * 
 */

#include <glfw_context.h>

#include <ev_gl.h>
#include <ev.h>

using namespace ev2::internal;

GLFWContext::GLFWContext() {
    if(glfwInit() != GLFW_TRUE) {
        throw engine_exception{"Cannot initialize GLFW"};
    }
}

GLFWContext::~GLFWContext() {
    glfwTerminate();
}