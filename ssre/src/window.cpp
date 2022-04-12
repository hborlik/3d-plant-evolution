/**
 * @file window.cpp
 * @author Hunter Borlik
 * @version 0.1
 * @date 2019-09-04
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include <window.h>

#include <iostream>
#include <iomanip>

#include <ssre.h>
#include <renderer.h>

using namespace ssre;

namespace {

// glfw error callback
void glfw_error_callback(int error, const char *description)
{
	std::cerr << "GLFW ERROR: " << description << std::endl;
}

// gl error callback
__attribute__((__stdcall__))
void gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
        GLsizei length, const GLchar *message, const void *userParam) {
    std::string output_severity{};
    std::string output_source{};
    std::string output_type{};
    std::string output_message{message};

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        output_severity = "NOTIFICATION";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        output_severity = "LOW";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        output_severity = "MEDIUM";
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        output_severity = "HIGH";
        break;
    default:
        break;
    }

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        output_source = "API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        output_source = "WINDOW_SYSTEM";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        output_source = "THIRD_PARTY";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        output_source = "APPLICATION";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        output_source = "OTHER";
        break;
    default:
        break;
    }

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        output_type = "ERROR";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        output_type = "DEPRECATED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        output_type = "UNDEFINED_BEHAVIOR";
        break;
    //case GL_DEBUG_TYPE_PORTABILITIY:
    //    break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        output_type = "PERFORMANCE";
        break;
    case GL_DEBUG_TYPE_MARKER:
        output_type = "MARKER";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        output_type = "PUSH_GROUP";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        output_type = "POP_GROUP";
        break;
    case GL_DEBUG_TYPE_OTHER:
        output_type = "OTHER";
        break;
    default:
        break;
    }

    std::cout << "[" << std::setprecision(5) << std::fixed << glfwGetTime() << "] OpenGL[" << output_source << ":" << output_severity << ":" << output_type << "] " << output_message << std::endl;
}

}

Window::Window(uint32_t width, uint32_t height, std::string name) {
    glfwSetErrorCallback(glfw_error_callback);

    //request the highest possible version of OpenGL
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // for debugging in 4.3 and later
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    window_ptr = glfwCreateWindow(width, height, name.data(), nullptr, nullptr);
    if(!window_ptr) {
        std::cout << "GLFW create window failed" << std::endl;
        throw ssre_exception{"GLFW create window failed"};
    }

    glfwSetKeyCallback(window_ptr, key_callback);
    glfwSetCursorPosCallback(window_ptr, mouse_cursor_position_callback);
    glfwSetFramebufferSizeCallback(window_ptr, framebuffer_size_callback);

    // make the given window context the current glfw context. required for below
    glfwMakeContextCurrent(window_ptr);
    // load gl functions
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        throw ssre_exception{"Failed to initialize GLAD"};
    }

    if(glfwRawMouseMotionSupported())
        glfwSetInputMode(window_ptr, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    // prevent massive delta on first callback
    glfwGetCursorPos(window_ptr, &prevMousePosition.x, &prevMousePosition.y);
    mousePosition = prevMousePosition;

    // Set vsync interval
	glfwSwapInterval(1);

    glViewport(0, 0, width, height);

    // only for core version 4.3 and later
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(gl_debug_callback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, 25, "TEST DEBUG OUTPUT MESSAGE");
}

Window::~Window() {
if(window_ptr)
    glfwDestroyWindow(window_ptr);
}

static double getElapsedTime() {
    static float lastTime = glfwGetTime();
    double thisTime = glfwGetTime();
    double delta = thisTime - lastTime;
    lastTime = thisTime;
    return delta;
}

void Window::Run() {
    if(window_ptr) {
        while(!glfwWindowShouldClose(window_ptr)) {
            deltaTime = getElapsedTime();

            // render
            if(renderer)
                renderer->render(deltaTime);

            // display frame
            glfwSwapBuffers(window_ptr);
            // update inputs
            glfwPollEvents();
            // process inputs
            updateMouseVel();
        }
    }
}

void Window::setMouseCursorVisible(bool visible) {
    if(visible) {
        glfwSetInputMode(window_ptr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
        glfwSetInputMode(window_ptr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    mouseCursorVisible = visible;
}

void Window::updateMouseVel() {
    if(!mouseCursorVisible) {
        mouseVelocity = (mousePosition - prevMousePosition) * (double)deltaTime;
    } else {
        mouseVelocity = {};
    }
    prevMousePosition = mousePosition;
}


void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(Window::StaticInst().keyCallback)
        Window::StaticInst().keyCallback(key, scancode, action, mods);

    if(key == GLFW_KEY_W) {
        Window::StaticInst().w = action != GLFW_RELEASE;
    } else if(key == GLFW_KEY_A) {
        Window::StaticInst().a = action != GLFW_RELEASE;
    } else if(key == GLFW_KEY_S) {
        Window::StaticInst().s = action != GLFW_RELEASE;
    } else if(key == GLFW_KEY_D) {
        Window::StaticInst().d = action != GLFW_RELEASE;
    }

    if(key == GLFW_KEY_PERIOD) {
        Window::StaticInst().period = action != GLFW_RELEASE;
    } else if(key == GLFW_KEY_COMMA) {
        Window::StaticInst().comma = action != GLFW_RELEASE;
    } else if(key == GLFW_KEY_UP) {
        Window::StaticInst().arrow_up = action != GLFW_RELEASE;
    } else if(key == GLFW_KEY_DOWN) {
        Window::StaticInst().arrow_down = action != GLFW_RELEASE;
    }

    if(key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        Window::StaticInst().setMouseCursorVisible(!Window::StaticInst().mouseCursorVisible);
    }
}

void Window::mouse_cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    Window::StaticInst().mousePosition = glm::vec2{xpos, ypos};
    if(Window::StaticInst().mouseCursorPositionCallback)
        Window::StaticInst().mouseCursorPositionCallback(xpos, ypos);
}

void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    if(Window::StaticInst().framebufferResizeCallback)
        Window::StaticInst().framebufferResizeCallback(width, height);
}