/**
 * @file window.h
 * @author Hunter Borlik
 * @brief Window manager. Manages a single window resource.
 * @version 0.1
 * @date 2019-09-04
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef EV2_WINDOW_H
#define EV2_WINDOW_H

#include <string>
#include <cstdint>

#include <ev.h>
#include <ev_gl.h>
#include <delegate.h>
#include <singleton.h>

namespace ev2::window {

void init(const Args& args);

class Window {
public:

    /**
     * @brief Construct a new Window object
     * 
     * @param width 
     * @param height 
     * @param name Window title
     * 
     */
    Window(std::string name, uint32_t width, uint32_t height);
    virtual ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    /**
     * @brief enter run loop, will execute until window close request
     * 
     */
    void run();


    using FramebufferResizeCallback = util::delegate<void(uint32_t, uint32_t)>;
    using KeyCallback = util::delegate<void(int, int, int, int)>;
    using MouseCursorPositionCallback = util::delegate<void(double, double)>;

    /**
     * @brief Set the Framebuffer Resize Callback
     * 
     * @param callback 
     */
    void SetFramebufferResizeCallback(const FramebufferResizeCallback& callback) {framebufferResizeCallback = callback;}

    void setMouseCursorPositionCallback(const MouseCursorPositionCallback& callback) {mouseCursorPositionCallback = callback;}

    void setMouseCursorVisible(bool visible);

protected:
    GLFWwindow* window_ptr;

    float deltaTime = 0;

    FramebufferResizeCallback framebufferResizeCallback;
    KeyCallback keyCallback;
    MouseCursorPositionCallback mouseCursorPositionCallback;


    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

    static void mouse_cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

    /**
     * @brief glfw window resize callback
     */
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
};


} // ev2

#endif // EV2_WINDOW_H