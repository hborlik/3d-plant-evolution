/**
 * @file application.h
 * @brief 
 * @date 2022-04-18
 * 
 */
#ifndef EV2_APPLICATION_H
#define EV2_APPLICATION_H

#include <input.h>

namespace ev2 {

class Application {
public:
    virtual ~Application() {}

    virtual void onKey(input::Key::Enum key, input::Modifier mods, bool down) = 0;
    virtual void onChar(uint32_t scancode) = 0;
    virtual void onScroll(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos) = 0;
    virtual void cursorPos(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos) = 0;
    virtual void onMouseButton(int32_t mouse_x, int32_t mouse_y, int32_t scroll_pos, input::MouseButton::Enum button, bool down) = 0;
    virtual void onWindowSizeChange(int32_t width, int32_t height) = 0;
    virtual void onDropFile(const std::string& path) = 0;
};

}

#endif // EV2_APPLICATION_H
