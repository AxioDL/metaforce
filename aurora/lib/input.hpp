#pragma once

#include <unordered_map>
#include "aurora/aurora.hpp"
#include "SDL_gamecontroller.h"
#include "SDL_keyboard.h"
#include "SDL_keycode.h"
#include "SDL_mouse.h"
namespace aurora::input {
Sint32 add_controller(Uint32 which);
void remove_controller(Uint32 which);
Sint32 player_index(Uint32 which);
void set_player_index(Uint32 which, Sint32 index);
std::string controller_name(Uint32 which);
bool is_gamecube(Uint32 which);
ControllerButton translate_controller_button(SDL_GameControllerButton button);
ControllerAxis translate_controller_axis(SDL_GameControllerAxis axis);
char translate_key(SDL_Keysym sym, SpecialKey& specialSym, ModifierKey& modifierSym);
ModifierKey translate_modifiers(Uint16 mods);
MouseButton translate_mouse_button(Uint8 button);
MouseButton translate_mouse_button_state(Uint8 state);
}