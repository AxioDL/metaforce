#pragma once

#include "aurora/aurora.hpp"
#include "SDL_gamecontroller.h"
#include "SDL_keyboard.h"
#include "SDL_keycode.h"
#include "SDL_mouse.h"

namespace aurora::input {
Sint32 get_instance_for_player(u32 player) noexcept;
Sint32 add_controller(Sint32 which) noexcept;
void remove_controller(Uint32 instance) noexcept;
Sint32 player_index(Uint32 instance) noexcept;
void set_player_index(Uint32 instance, Sint32 index) noexcept;
std::string controller_name(Uint32 instance) noexcept;
bool is_gamecube(Uint32 instance) noexcept;
bool controller_has_rumble(Uint32 instance) noexcept;
void controller_rumble(uint32_t instance, uint16_t low_freq_intensity, uint16_t high_freq_intensity,
                       uint16_t duration_ms) noexcept;
ControllerButton translate_controller_button(SDL_GameControllerButton button) noexcept;
ControllerAxis translate_controller_axis(SDL_GameControllerAxis axis) noexcept;
char translate_key(SDL_Keysym sym, SpecialKey& specialSym, ModifierKey& modifierSym) noexcept;
ModifierKey translate_modifiers(Uint16 mods) noexcept;
MouseButton translate_mouse_button(Uint8 button) noexcept;
MouseButton translate_mouse_button_state(Uint8 state) noexcept;
} // namespace aurora::input
