#include <input.h>

namespace ev2::input {

struct KeyAndMods {
    uint8_t down = false;
    Modifier mods{};
};

struct InputState {
    bool input_enabled = true;

    KeyAndMods key_state[Key::Enum::Count] = {};
    Modifier modifiers;
};

static InputState State;

void SetKeyState(Key::Enum key, Modifier mods, bool down) {
    State.key_state[key].down = down;
    State.key_state[key].mods = mods;
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

void SetInputEnabled(bool enabled) {
    State.input_enabled = enabled;
}

bool GetKeyDown(Key::Enum key, Modifier modifiers) {
    KeyAndMods& key_state = State.key_state[key];
    return State.input_enabled && key_state.down && (modifiers.mods & key_state.mods.mods);
}

}