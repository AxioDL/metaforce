#pragma once

#include <unordered_map>
#include "aurora/aurora.hpp"
#include "SDL_gamecontroller.h"

namespace aurora::input {
Sint32 add_controller(Uint32 which);
void remove_controller(Uint32 which);
Sint32 player_index(Uint32 which);
void set_player_index(Uint32 which, Sint32 index);
std::string name(Uint32 which);
bool is_gamecube(Uint32 which);
ControllerButton translate_button(SDL_GameControllerButton button);
ControllerAxis translate_axis(SDL_GameControllerAxis axis);
}