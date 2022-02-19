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

std::string name(Uint32 which) {
  if (g_GameControllers.find(which) != g_GameControllers.end()) {
    auto* name = SDL_GameControllerName(g_GameControllers[which].m_controller);
    if (name) {
      return std::string(name);
    }
  }
  return {};
}

ControllerButton translate_button(SDL_GameControllerButton btn) {
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
  case SDL_CONTROLLER_BUTTON_LEFTSTICK:
    return ControllerButton::LeftStick;
  case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
    return ControllerButton::RightStick;
  case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
    return ControllerButton::LeftShoulder;
  case SDL_CONTROLLER_BUTTON_DPAD_UP:
    return ControllerButton::DPadUp;
  case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
    return ControllerButton::DPadDown;
  case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
    return ControllerButton::DPadRight;
  default:
    return ControllerButton::Other;
  }
}

ControllerAxis translate_axis(SDL_GameControllerAxis axis) {
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
} // namespace aurora::input