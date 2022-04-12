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
#ifndef SSRE_WINDOW_H
#define SSRE_WINDOW_H

#include <string>
#include <cstdint>

#include <ssre_gl.h>
#include <singleton.h>
#include <delegate.h>

namespace ssre {

class Renderer;
class Window : public util::Singleton<Window> {
public:

    bool w = false, a = false, s = false, d = false;
    bool comma = false, period = false, arrow_up = false, arrow_down = false;

    /**
     * @brief Construct a new Window object
     * 
     * @param width 
     * @param height 
     * @param name Window title
     * 
     */
    Window(uint32_t width, uint32_t height, std::string name);
    virtual ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    /**
     * @brief enter run loop, will execute until window close request
     * 
     */
    void Run();

    /**
     * @brief Set the Renderer
     * 
     * @param 
     */
    void SetRenderer(const std::shared_ptr<Renderer>& renderer) {this->renderer = renderer;}


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
    bool isMouseCursorVisible() const noexcept {return mouseCursorVisible;}

    /**
     * @brief Get the Mouse Velocity in pixles
     * 
     * @return glm::vec2 
     */
    glm::dvec2 getMouseVel() const noexcept {return mouseVelocity;}

protected:
    GLFWwindow* window_ptr;

    float deltaTime = 0;

    std::shared_ptr<Renderer> renderer;

    FramebufferResizeCallback framebufferResizeCallback;
    KeyCallback keyCallback;
    MouseCursorPositionCallback mouseCursorPositionCallback;

    bool mouseCursorVisible = true;
    glm::dvec2 mouseVelocity{};
    glm::dvec2 prevMousePosition{};
    glm::dvec2 mousePosition{};

    void updateMouseVel();

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

    static void mouse_cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

    /**
     * @brief glfw window resize callback
     */
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
};


} // ssre

#endif // SSRE_WINDOW_H