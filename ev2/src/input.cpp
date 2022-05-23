#include <input.h>

namespace ev2::input {

struct InputState {
    bool input_enabled = true;

    uint8_t key_state[Key::Enum::Count] = {};
    Modifier modifiers;
};

static InputState State;

void SetKeyState(Key::Enum key, bool down) {
    State.key_state[key] = down;
}

void SetModifiers(Modifier modifiers) {
    State.modifiers.mods |= modifiers.mods;
}

void ClearModifiers(Modifier modifiers) {
    State.modifiers.mods &= ~modifiers.mods;
}

void DisableInput() {
    State.input_enabled = false;
}

void EnableInput() {
    State.input_enabled = true;
}

bool GetKeyDown(Key key, Modifier modifiers) {
    if (State.input_enabled) {

    }
    return false;
}

}