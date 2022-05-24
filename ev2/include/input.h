/**
 * @file input.h
 * @brief 
 * @date 2022-04-15
 * 
 */
#ifndef EV2_INPUT_H
#define EV2_INPUT_H

#include <stdint.h>

#include <glm/glm.hpp>

namespace ev2::input {

struct MouseButton {
    enum Enum
    {
        Empty = 0,
        Left,
        Middle,
        Right,

        Count
    };
};

struct Modifier {
    enum Enum
    {
        Empty = 0,
        LeftAlt = 0x01,
        RightAlt = 0x02,
        LeftCtrl = 0x04,
        RightCtrl = 0x08,
        LeftShift = 0x10,
        RightShift = 0x20,
        LeftMeta = 0x40,
        RightMeta = 0x80
    };
    uint8_t mods = Empty;
};

struct Key {
    enum Enum
    {
        Empty = 0,
        Esc,
        Return,
        Tab,
        Space,
        Backspace,
        Up,
        Down,
        Left,
        Right,
        Insert,
        Delete,
        Home,
        End,
        PageUp,
        PageDown,
        Print,
        Plus,
        Minus,
        LeftBracket,
        RightBracket,
        Semicolon,
        Quote,
        Comma,
        Period,
        Slash,
        Backslash,
        Tilde,
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        NumPad0,
        NumPad1,
        NumPad2,
        NumPad3,
        NumPad4,
        NumPad5,
        NumPad6,
        NumPad7,
        NumPad8,
        NumPad9,
        Key0,
        Key1,
        Key2,
        Key3,
        Key4,
        Key5,
        Key6,
        Key7,
        Key8,
        Key9,
        KeyA,
        KeyB,
        KeyC,
        KeyD,
        KeyE,
        KeyF,
        KeyG,
        KeyH,
        KeyI,
        KeyJ,
        KeyK,
        KeyL,
        KeyM,
        KeyN,
        KeyO,
        KeyP,
        KeyQ,
        KeyR,
        KeyS,
        KeyT,
        KeyU,
        KeyV,
        KeyW,
        KeyX,
        KeyY,
        KeyZ,

        GamepadA,
        GamepadB,
        GamepadX,
        GamepadY,
        GamepadThumbL,
        GamepadThumbR,
        GamepadShoulderL,
        GamepadShoulderR,
        GamepadUp,
        GamepadDown,
        GamepadLeft,
        GamepadRight,
        GamepadBack,
        GamepadStart,
        GamepadGuide,

        Count
    };

    Key() = delete;
};

void SetKeyState(Key::Enum key, Modifier mods, bool down);
void SetModifiers(Key::Enum key, Modifier modifiers);
void ClearModifiers(Key::Enum key, Modifier modifiers);
void DisableInput();
void EnableInput();
void SetInputEnabled(bool enabled);
void SetMousePosition(const glm::vec2& position);

bool GetKeyDown(Key::Enum key, Modifier modifiers = {});
glm::vec2 GetMousePosition();

}

#endif // EV2_INPUT_H