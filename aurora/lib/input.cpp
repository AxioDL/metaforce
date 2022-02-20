#include "input.hpp"

namespace aurora::input {
struct GameController {
  SDL_GameController* m_controller;
  bool m_isGameCube = false;
  Sint32 m_index;
};
std::unordered_map<Uint32, GameController> g_GameControllers;

Sint32 add_controller(Uint32 which) {
  auto* ctrl = SDL_GameControllerOpen(which);
  if (ctrl != nullptr) {
    GameController controller;
    controller.m_controller = ctrl;
    controller.m_index = which;
    controller.m_isGameCube =
        SDL_GameControllerGetVendor(ctrl) == 0x057E && SDL_GameControllerGetProduct(ctrl) == 0x0337;

    Sint32 instance = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(ctrl));
    g_GameControllers[instance] = controller;
    return instance;
  }

  return -1;
}

void remove_controller(Uint32 which) {
  if (g_GameControllers.find(which) != g_GameControllers.end()) {
    SDL_GameControllerClose(g_GameControllers[which].m_controller);
    g_GameControllers.erase(which);
  }
}

bool is_gamecube(Uint32 which) {
  if (g_GameControllers.find(which) != g_GameControllers.end()) {
    return g_GameControllers[which].m_isGameCube;
  }
  return false;
}

int32_t player_index(Uint32 which) {
  if (g_GameControllers.find(which) != g_GameControllers.end()) {
    return SDL_GameControllerGetPlayerIndex(g_GameControllers[which].m_controller);
  }
  return -1;
}

void set_player_index(Uint32 which, Sint32 index) {
  if (g_GameControllers.find(which) != g_GameControllers.end()) {
    SDL_GameControllerSetPlayerIndex(g_GameControllers[which].m_controller, index);
  }
}

std::string controller_name(Uint32 which) {
  if (g_GameControllers.find(which) != g_GameControllers.end()) {
    auto* name = SDL_GameControllerName(g_GameControllers[which].m_controller);
    if (name) {
      return std::string(name);
    }
  }
  return {};
}

ControllerButton translate_controller_button(SDL_GameControllerButton btn) {
  switch (btn) {
  case SDL_CONTROLLER_BUTTON_A:
    return ControllerButton::A;
  case SDL_CONTROLLER_BUTTON_B:
    return ControllerButton::B;
  case SDL_CONTROLLER_BUTTON_X:
    return ControllerButton::X;
  case SDL_CONTROLLER_BUTTON_Y:
    return ControllerButton::Y;
  case SDL_CONTROLLER_BUTTON_BACK:
    return ControllerButton::Back;
  case SDL_CONTROLLER_BUTTON_GUIDE:
    return ControllerButton::Guide;
  case SDL_CONTROLLER_BUTTON_START:
    return ControllerButton::Start;
  case SDL_CONTROLLER_BUTTON_LEFTSTICK:
    return ControllerButton::LeftStick;
  case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
    return ControllerButton::RightStick;
  case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
    return ControllerButton::LeftShoulder;
  case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
    return ControllerButton::RightShoulder;
  case SDL_CONTROLLER_BUTTON_DPAD_UP:
    return ControllerButton::DPadUp;
  case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
    return ControllerButton::DPadDown;
  case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
    return ControllerButton::DPadRight;
  case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
    return ControllerButton::DPadLeft;
  default:
    return ControllerButton::Other;
  }
}

ControllerAxis translate_controller_axis(SDL_GameControllerAxis axis) {
  switch (axis) {
  case SDL_CONTROLLER_AXIS_LEFTX:
    return ControllerAxis::LeftX;
  case SDL_CONTROLLER_AXIS_LEFTY:
    return ControllerAxis::LeftY;
  case SDL_CONTROLLER_AXIS_RIGHTX:
    return ControllerAxis::RightX;
  case SDL_CONTROLLER_AXIS_RIGHTY:
    return ControllerAxis::RightY;
  case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
    return ControllerAxis::TriggerLeft;
  case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
    return ControllerAxis::TriggerRight;
  default:
    return ControllerAxis::MAX;
  }
}

char translate_key(SDL_Keysym sym, SpecialKey& specialSym, ModifierKey& modifierSym) {
  specialSym = SpecialKey::None;
  modifierSym = ModifierKey::None;
  if (sym.sym >= SDLK_F1 && sym.sym <= SDLK_F12) {
    specialSym = SpecialKey(int(SpecialKey::F1) + sym.sym - SDLK_F1);
  } else if (sym.sym == SDLK_ESCAPE) {
    specialSym = SpecialKey::Esc;
  } else if (sym.sym == SDLK_RETURN) {
    specialSym = SpecialKey::Enter;
  } else if (sym.sym == SDLK_KP_ENTER) {
    specialSym = SpecialKey::KpEnter;
  } else if (sym.sym == SDLK_BACKSPACE) {
    specialSym = SpecialKey::Backspace;
  } else if (sym.sym == SDLK_INSERT) {
    specialSym = SpecialKey::Insert;
  } else if (sym.sym == SDLK_DELETE) {
    specialSym = SpecialKey::Delete;
  } else if (sym.sym == SDLK_HOME) {
    specialSym = SpecialKey::Home;
  } else if (sym.sym == SDLK_END) {
    specialSym = SpecialKey::End;
  } else if (sym.sym == SDLK_PAGEUP) {
    specialSym = SpecialKey::PgUp;
  } else if (sym.sym == SDLK_PAGEDOWN) {
    specialSym = SpecialKey::PgDown;
  } else if (sym.sym == SDLK_LEFT) {
    specialSym = SpecialKey::Left;
  } else if (sym.sym == SDLK_RIGHT) {
    specialSym = SpecialKey::Right;
  } else if (sym.sym == SDLK_UP) {
    specialSym = SpecialKey::Up;
  } else if (sym.sym == SDLK_DOWN) {
    specialSym = SpecialKey::Down;
  } else if (sym.sym == SDLK_TAB) {
    specialSym = SpecialKey::Tab;
  } else if (sym.sym == SDLK_LSHIFT) {
    modifierSym = ModifierKey::LeftShift;
  } else if (sym.sym == SDLK_RSHIFT) {
    modifierSym = ModifierKey::RightShift;
  } else if (sym.sym == SDLK_LCTRL) {
    modifierSym = ModifierKey::LeftControl;
  } else if (sym.sym == SDLK_RCTRL) {
    modifierSym = ModifierKey::RightControl;
  } else if (sym.sym == SDLK_LALT) {
    modifierSym = ModifierKey::LeftAlt;
  } else if (sym.sym == SDLK_RALT) {
    modifierSym = ModifierKey::RightAlt;
  } else if (sym.sym >= ' ' && sym.sym <= 'z') {
    return static_cast<char>(sym.sym);
  }
  return 0;
}

ModifierKey translate_modifiers(Uint16 mods) {
  ModifierKey ret = ModifierKey::None;
  if ((mods & SDLK_LSHIFT) != 0) {
    ret |= ModifierKey::LeftShift;
  }
  if ((mods & SDLK_RSHIFT) != 0) {
    ret |= ModifierKey::RightShift;
  }
  if ((mods & SDLK_LCTRL) != 0) {
    ret |= ModifierKey::LeftControl;
  }
  if ((mods & SDLK_RCTRL) != 0) {
    ret |= ModifierKey::RightControl;
  }
  if ((mods & SDLK_LALT) != 0) {
    ret |= ModifierKey::LeftAlt;
  }
  if ((mods & SDLK_RALT) != 0) {
    ret |= ModifierKey::RightAlt;
  }

  return ret;
}

MouseButton translate_mouse_button(Uint8 button) {
  if (button == 1) {
    return MouseButton::Primary;
  }
  if (button == 2) {
    return MouseButton::Middle;
  }
  if (button == 3) {
    return MouseButton::Secondary;
  }
  if (button == 4) {
    return MouseButton::Aux1;
  }
  if (button == 5) {
    return MouseButton::Aux2;
  }

  return MouseButton::None;
}

MouseButton translate_mouse_button_state(Uint8 state) {
  auto ret = MouseButton::None;
  if ((state & 0x01) != 0) {
    ret |= MouseButton::Primary;
  }
  if ((state & 0x02) != 0) {
    ret |= MouseButton::Middle;
  }
  if ((state & 0x04) != 0) {
    ret |= MouseButton::Secondary;
  }
  if ((state & 0x08) != 0) {
    ret |= MouseButton::Aux1;
  }
  if ((state & 0x10) != 0) {
    ret |= MouseButton::Aux2;
  }

  return ret;
}

} // namespace aurora::input