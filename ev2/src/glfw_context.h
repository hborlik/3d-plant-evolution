/**
 * @file glfw_context.h
 * @author Hunter Borlik 
 * @brief GLFW context class to control initialization and cleanup of glfw.
 * @version 0.1
 * @date 2019-09-09
 * 
 * 
 */
#pragma once

#include <memory>

namespace ev2::internal {

class GLFWContext {
public:
    GLFWContext();
    ~GLFWContext();
};

}