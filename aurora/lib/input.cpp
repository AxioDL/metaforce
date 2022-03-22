#include "input.hpp"
#include "aurora/pad.hpp"

#include <SDL_haptic.h>

#include <absl/container/btree_map.h>
#include <absl/container/flat_hash_map.h>
#include <absl/strings/str_split.h>
#include <cmath>

namespace aurora::input {
static logvisor::Module Log("aurora::input");

struct GameController {
  SDL_GameController* m_controller = nullptr;
  bool m_isGameCube = false;
  Sint32 m_index = -1;
  bool m_hasRumble = false;
  constexpr bool operator==(const GameController&) const = default;
};
absl::flat_hash_map<Uint32, GameController> g_GameControllers;

GameController get_controller_for_player(u32 player) noexcept {
  for (const auto& [which, controller] : g_GameControllers) {
    if (player_index(which) == player) {
      return controller;
    }
  }

  return {};
}

Sint32 get_instance_for_player(u32 player) noexcept {
  for (const auto& [which, controller] : g_GameControllers) {
    if (player_index(which) == player) {
      return which;
    }
  }

  return {};
}

static std::optional<std::string> remap_controller_layout(std::string_view mapping) {
  std::string newMapping;
  newMapping.reserve(mapping.size());
  absl::btree_map<std::string_view, std::string_view> entries;
  for (size_t idx = 0; const auto value : absl::StrSplit(mapping, ',')) {
    if (idx < 2) {
      if (idx > 0) {
        newMapping.push_back(',');
      }
      newMapping.append(value);
    } else {
      const auto split = absl::StrSplit(value, absl::MaxSplits(':', 2));
      auto iter = split.begin();
      entries.emplace(*iter++, *iter);
    }
    idx++;
  }
  if (entries.contains("rightshoulder"sv) && !entries.contains("leftshoulder"sv)) {
    Log.report(logvisor::Info, FMT_STRING("Remapping GameCube controller layout"));
    entries.insert_or_assign("back"sv, entries["rightshoulder"sv]);
    // TODO trigger buttons may differ per platform
    entries.insert_or_assign("leftshoulder"sv, "b11"sv);
    entries.insert_or_assign("rightshoulder"sv, "b10"sv);
  } else if (entries.contains("leftshoulder"sv) && entries.contains("rightshoulder"sv) && entries.contains("back"sv)) {
    Log.report(logvisor::Info, FMT_STRING("Controller has standard layout"));
    auto a = entries["a"sv];
    entries.insert_or_assign("a"sv, entries["b"sv]);
    entries.insert_or_assign("b"sv, a);
    auto x = entries["x"sv];
    entries.insert_or_assign("x"sv, entries["y"sv]);
    entries.insert_or_assign("y"sv, x);
  } else {
    Log.report(logvisor::Error, FMT_STRING("Controller has unsupported layout: {}"), mapping);
    return {};
  }
  for (const auto [k, v] : entries) {
    newMapping.push_back(',');
    newMapping.append(k);
    newMapping.push_back(':');
    newMapping.append(v);
  }
  return newMapping;
}

Sint32 add_controller(Sint32 which) noexcept {
  auto* ctrl = SDL_GameControllerOpen(which);
  if (ctrl != nullptr) {
    {
      char* mapping = SDL_GameControllerMapping(ctrl);
      if (mapping != nullptr) {
        auto newMapping = remap_controller_layout(mapping);
        SDL_free(mapping);
        if (newMapping) {
          if (SDL_GameControllerAddMapping(newMapping->c_str()) == -1) {
            Log.report(logvisor::Error, FMT_STRING("Failed to update controller mapping: {}"), SDL_GetError());
          }
        }
      } else {
        Log.report(logvisor::Error, FMT_STRING("Failed to retrieve mapping for controller"));
      }
    }
    GameController controller;
    controller.m_controller = ctrl;
    controller.m_index = which;
    controller.m_isGameCube =
        SDL_GameControllerGetVendor(ctrl) == 0x057E && SDL_GameControllerGetProduct(ctrl) == 0x0337;
    controller.m_hasRumble = (SDL_GameControllerHasRumble(ctrl) != 0u);
    Sint32 instance = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(ctrl));
    g_GameControllers[instance] = controller;
    return instance;
  }

  return -1;
}

void remove_controller(Uint32 instance) noexcept {
  if (g_GameControllers.find(instance) != g_GameControllers.end()) {
    SDL_GameControllerClose(g_GameControllers[instance].m_controller);
    g_GameControllers.erase(instance);
  }
}

bool is_gamecube(Uint32 instance) noexcept {
  if (g_GameControllers.find(instance) != g_GameControllers.end()) {
    return g_GameControllers[instance].m_isGameCube;
  }
  return false;
}

int32_t player_index(Uint32 instance) noexcept {
  if (g_GameControllers.find(instance) != g_GameControllers.end()) {
    return SDL_GameControllerGetPlayerIndex(g_GameControllers[instance].m_controller);
  }
  return -1;
}

void set_player_index(Uint32 instance, Sint32 index) noexcept {
  if (g_GameControllers.find(instance) != g_GameControllers.end()) {
    SDL_GameControllerSetPlayerIndex(g_GameControllers[instance].m_controller, index);
  }
}

std::string controller_name(Uint32 instance) noexcept {
  if (g_GameControllers.find(instance) != g_GameControllers.end()) {
    const auto* name = SDL_GameControllerName(g_GameControllers[instance].m_controller);
    if (name != nullptr) {
      return {name};
    }
  }
  return {};
}

bool controller_has_rumble(Uint32 instance) noexcept {
  if (g_GameControllers.find(instance) != g_GameControllers.end()) {
    return g_GameControllers[instance].m_hasRumble;
  }

  return false;
}

void controller_rumble(uint32_t instance, uint16_t low_freq_intensity, uint16_t high_freq_intensity,
                       uint16_t duration_ms) noexcept {

  if (g_GameControllers.find(instance) != g_GameControllers.end()) {
    SDL_GameControllerRumble(g_GameControllers[instance].m_controller, low_freq_intensity, high_freq_intensity,
                             duration_ms);
  }
}
ControllerButton translate_controller_button(SDL_GameControllerButton btn) noexcept {
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

ControllerAxis translate_controller_axis(SDL_GameControllerAxis axis) noexcept {
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

char translate_key(SDL_Keysym sym, SpecialKey& specialSym, ModifierKey& modifierSym) noexcept {
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

ModifierKey translate_modifiers(Uint16 mods) noexcept {
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

MouseButton translate_mouse_button(Uint8 button) noexcept {
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

MouseButton translate_mouse_button_state(Uint8 state) noexcept {
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

void PADSetSpec(s32 spec) {}
void PADInit() {}

static const std::array<std::pair<SDL_GameControllerButton, PAD::BUTTON>, 12> mMapping{{
    {SDL_CONTROLLER_BUTTON_A, PAD::BUTTON_A},
    {SDL_CONTROLLER_BUTTON_B, PAD::BUTTON_B},
    {SDL_CONTROLLER_BUTTON_X, PAD::BUTTON_X},
    {SDL_CONTROLLER_BUTTON_Y, PAD::BUTTON_Y},
    {SDL_CONTROLLER_BUTTON_START, PAD::BUTTON_START},
    {SDL_CONTROLLER_BUTTON_BACK, PAD::TRIGGER_Z},
    {SDL_CONTROLLER_BUTTON_LEFTSHOULDER, PAD::TRIGGER_L},
    {SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, PAD::TRIGGER_R},
    {SDL_CONTROLLER_BUTTON_DPAD_UP, PAD::BUTTON_UP},
    {SDL_CONTROLLER_BUTTON_DPAD_DOWN, PAD::BUTTON_DOWN},
    {SDL_CONTROLLER_BUTTON_DPAD_LEFT, PAD::BUTTON_LEFT},
    {SDL_CONTROLLER_BUTTON_DPAD_RIGHT, PAD::BUTTON_RIGHT},
}};

u32 PADRead(PAD::Status* status) {
  u32 rumbleSupport = 0;
  for (u32 i = 0; i < 4; ++i) {
    memset(&status[i], 0, sizeof(PAD::Status));
    auto controller = aurora::input::get_controller_for_player(i);
    if (controller == aurora::input::GameController{}) {
      status[i].xa_err = PAD::ERR_NO_CONTROLLER;
      continue;
    }
    status[i].xa_err = PAD::ERR_NONE;
    std::for_each(mMapping.begin(), mMapping.end(), [&controller, &i, &status](const auto& pair) {
      if (SDL_GameControllerGetButton(controller.m_controller, pair.first)) {
        status[i].x0_buttons |= pair.second;
      }
    });

    Sint16 x = SDL_GameControllerGetAxis(controller.m_controller, SDL_CONTROLLER_AXIS_LEFTX);
    Sint16 y = SDL_GameControllerGetAxis(controller.m_controller, SDL_CONTROLLER_AXIS_LEFTY);
    if (std::abs(x) > 8000) {
      x /= 256;
    } else {
      x = 0;
    }

    if (std::abs(y) > 8000) {
      y = (-(y + 1u)) / 256u;
    } else {
      y = 0;
    }

    status[i].x2_stickX = static_cast<s8>(x);
    status[i].x3_stickY = static_cast<s8>(y);

    x = SDL_GameControllerGetAxis(controller.m_controller, SDL_CONTROLLER_AXIS_RIGHTX);
    y = SDL_GameControllerGetAxis(controller.m_controller, SDL_CONTROLLER_AXIS_RIGHTY);
    if (std::abs(x) > 8000) {
      x /= 256;
    } else {
      x = 0;
    }

    if (std::abs(y) > 8000) {
      y = (-(y + 1u)) / 256u;
    } else {
      y = 0;
    }

    status[i].x4_substickX = static_cast<s8>(x);
    status[i].x5_substickY = static_cast<s8>(y);

    x = SDL_GameControllerGetAxis(controller.m_controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
    y = SDL_GameControllerGetAxis(controller.m_controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
    x /= 128;
    y /= 128;

    status[i].x6_triggerL = static_cast<s8>(x);
    status[i].x7_triggerR = static_cast<s8>(y);

    if (controller.m_hasRumble) {
      rumbleSupport |= PAD::CHAN0_BIT >> i;
    }
  }
  return rumbleSupport;
}

void PADControlAllMotors(const u32* commands) {
  for (u32 i = 0; i < 4; ++i) {
    auto controller = aurora::input::get_controller_for_player(i);
    auto instance = aurora::input::get_instance_for_player(i);
    if (controller == aurora::input::GameController{}) {
      continue;
    }

    if (controller.m_isGameCube) {
      if (commands[i] == PAD::MOTOR_STOP) {
        aurora::input::controller_rumble(instance, 0, 1, 0);
      } else if (commands[i] == PAD::MOTOR_RUMBLE) {
        aurora::input::controller_rumble(instance, 1, 1, 0);
      } else if (commands[i] == PAD::MOTOR_STOP_HARD) {
        aurora::input::controller_rumble(instance, 0, 0, 0);
      }
    } else {
      // TODO: Figure out sane values for generic controllers
    }
  }
}

u32 SIProbe(s32 chan) {
  const auto controller = aurora::input::get_controller_for_player(chan);
  if (controller == aurora::input::GameController{}) {
    return SI::ERROR_NO_RESPONSE;
  }

  if (controller.m_isGameCube) {
    auto level = SDL_JoystickCurrentPowerLevel(SDL_GameControllerGetJoystick(controller.m_controller));
    if (level == SDL_JOYSTICK_POWER_UNKNOWN) {
      return SI::GC_WAVEBIRD;
    }
  }

  return SI::GC_CONTROLLER;
}

struct PADCLampRegion {
  u8 minTrigger;
  u8 maxTrigger;
  s8 minStick;
  s8 maxStick;
  s8 xyStick;
  s8 minSubstick;
  s8 maxSubstick;
  s8 xySubstick;
  s8 radStick;
  s8 radSubstick;
};

static constexpr PADCLampRegion ClampRegion{
    // Triggers
    30,
    180,

    // Left stick
    15,
    72,
    40,

    // Right stick
    15,
    59,
    31,

    // Stick radii
    56,
    44,
};

void ClampTrigger(u8* trigger, u8 min, u8 max) {
  if (*trigger <= min) {
    *trigger = 0;
  } else {
    if (*trigger > max) {
      *trigger = max;
    }
    *trigger -= min;
  }
}

void ClampCircle(s8* px, s8* py, s8 radius, s8 min) {
  int x = *px;
  int y = *py;

  if (-min < x && x < min) {
    x = 0;
  } else if (0 < x) {
    x -= min;
  } else {
    x += min;
  }

  if (-min < y && y < min) {
    y = 0;
  } else if (0 < y) {
    y -= min;
  } else {
    y += min;
  }

  int squared = x * x + y * y;
  if (radius * radius < squared) {
    s32 length = static_cast<s32>(std::sqrt(squared));
    x = (x * radius) / length;
    y = (y * radius) / length;
  }

  *px = static_cast<s8>(x);
  *py = static_cast<s8>(y);
}

void ClampStick(s8* px, s8* py, s8 max, s8 xy, s8 min) {
  s32 x = *px;
  s32 y = *py;

  s32 signX = 0;
  if (0 <= x) {
    signX = 1;
  } else {
    signX = -1;
    x = -x;
  }

  s8 signY = 0;
  if (0 <= y) {
    signY = 1;
  } else {
    signY = -1;
    y = -y;
  }

  if (x <= min) {
    x = 0;
  } else {
    x -= min;
  }
  if (y <= min) {
    y = 0;
  } else {
    y -= min;
  }

  if (x == 0 && y == 0) {
    *px = *py = 0;
    return;
  }

  if (xy * y <= xy * x) {
    s32 d = xy * x + (max - xy) * y;
    if (xy * max < d) {
      x = (xy * max * x / d);
      y = (xy * max * y / d);
    }
  } else {
    s32 d = xy * y + (max - xy) * x;
    if (xy * max < d) {
      x = (xy * max * x / d);
      y = (xy * max * y / d);
    }
  }

  *px = (signX * x);
  *py = (signY * y);
}

void PADClamp(PAD::Status* status) {
  for (u32 i = 0; i < 4; ++i) {
    if (status[i].xa_err != PAD::ERR_NONE) {
      continue;
    }

    ClampStick(&status[i].x2_stickX, &status[i].x3_stickY, ClampRegion.maxStick, ClampRegion.xyStick,
               ClampRegion.minStick);
    ClampStick(&status[i].x4_substickX, &status[i].x5_substickY, ClampRegion.maxSubstick, ClampRegion.xySubstick,
               ClampRegion.minSubstick);
    ClampTrigger(&status[i].x6_triggerL, ClampRegion.minTrigger, ClampRegion.maxTrigger);
    ClampTrigger(&status[i].x7_triggerR, ClampRegion.minTrigger, ClampRegion.maxTrigger);
  }
}

void PADClampCircle(PAD::Status* status) {
  for (u32 i = 0; i < 4; ++i) {
    if (status[i].xa_err != PAD::ERR_NONE) {
      continue;
    }

    ClampCircle(&status[i].x2_stickX, &status[i].x3_stickY, ClampRegion.radStick, ClampRegion.minStick);
    ClampCircle(&status[i].x4_substickX, &status[i].x5_substickY, ClampRegion.radSubstick, ClampRegion.minSubstick);
    ClampTrigger(&status[i].x6_triggerL, ClampRegion.minTrigger, ClampRegion.maxTrigger);
    ClampTrigger(&status[i].x7_triggerR, ClampRegion.minTrigger, ClampRegion.maxTrigger);
  }
}