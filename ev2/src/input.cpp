#include <input.h>

namespace ev2::input {

struct KeyAndMods {
    uint8_t down = false;
    Modifier mods{};
};

struct InputState {
    bool input_enabled = true;

    KeyAndMods key_state[Key::Enum::Count] = {};
    glm::vec2 mouse_position{};
};

static InputState State;

void SetKeyState(Key::Enum key, Modifier mods, bool down) {
    State.key_state[key].down = down;
    State.key_state[key].mods = mods;
}

void SetModifiers(Key::Enum key, Modifier modifiers) {
    State.key_state[key].mods.mods |= modifiers.mods;
}

void ClearModifiers(Key::Enum key, Modifier modifiers) {
    State.key_state[key].mods.mods &= ~modifiers.mods;
}

void DisableInput() {
    State.input_enabled = false;
}

void EnableInput() {
    State.input_enabled = true;
}

void SetInputEnabled(bool enabled) {
    State.input_enabled = enabled;
}

void SetMousePosition(const glm::vec2& position) {
    State.mouse_position = position;
}

bool GetKeyDown(Key::Enum key, Modifier modifiers) {
    KeyAndMods& key_state = State.key_state[key];
    if (State.input_enabled && key_state.down) {
        uint8_t mask = 1;
        if (modifiers.mods != 0)
            mask = modifiers.mods & key_state.mods.mods;
        return (mask);
    }
    return false;
}

glm::vec2 GetMousePosition() {
    return State.mouse_position;
}

}