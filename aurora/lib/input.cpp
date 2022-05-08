#include "input.hpp"
#include "aurora/pad.hpp"
#include "Runtime/ConsoleVariables/FileStoreManager.hpp"

#include "magic_enum.hpp"

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
  PADDeadZones m_deadZones;
  u16 m_vid = 0;
  u16 m_pid = 0;
  std::array<PADButtonMapping, 12> m_mapping;
  bool m_mappingLoaded = false;
  constexpr bool operator==(const GameController& other) const {
    return m_controller == other.m_controller && m_index == other.m_index;
  }
};
absl::flat_hash_map<Uint32, GameController> g_GameControllers;

GameController* get_controller_for_player(u32 player) noexcept {
  for (auto& [which, controller] : g_GameControllers) {
    if (player_index(which) == player) {
      return &controller;
    }
  }

  /* If we don't have a controller assigned to this port use the first unassigned controller */
  if (!g_GameControllers.empty()) {
    int32_t availIndex = -1;
    for (const auto& controller : g_GameControllers) {
      if (player_index(controller.second.m_index) == -1) {
        availIndex = controller.second.m_index;
        break;
      }
    }
    if (availIndex != -1) {
      set_player_index(g_GameControllers.begin()->second.m_index, player);
      return get_controller_for_player(player);
    }
  }
  return nullptr;
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
    controller.m_vid = SDL_GameControllerGetVendor(ctrl);
    controller.m_pid = SDL_GameControllerGetProduct(ctrl);
    controller.m_isGameCube = controller.m_vid == 0x057E && controller.m_pid == 0x0337;
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

uint32_t controller_count() noexcept { return g_GameControllers.size(); }

} // namespace aurora::input

static const std::array<PADButtonMapping, 12> mDefaultButtons{{
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

void PADSetSpec(s32 spec) {}
void PADInit() {}

void __PADLoadMapping(aurora::input::GameController* controller) {
  s32 playerIndex = SDL_GameControllerGetPlayerIndex(controller->m_controller);
  if (playerIndex == -1) {
    return;
  }

  std::string basePath = std::string(metaforce::FileStoreManager::instance()->getStoreRoot());
  if (!controller->m_mappingLoaded) {
    controller->m_mapping = mDefaultButtons;
  }

  controller->m_mappingLoaded = true;

  FILE* file =
      fopen(fmt::format(FMT_STRING("{}/{}_{:04X}_{:04X}.controller"), basePath,
                        aurora::input::controller_name(controller->m_index), controller->m_vid, controller->m_pid)
                .c_str(),
            "rbe");
  if (file == nullptr) {
    return;
  }

  u32 magic = 0;
  fread(&magic, 1, sizeof(u32), file);
  if (magic != SBIG('CTRL')) {
    fmt::print(FMT_STRING("Invalid controller mapping magic!\n"));
    return;
  }

  u32 version = 0;
  fread(&version, 1, sizeof(u32), file);
  if (version != 1) {
    fmt::print(FMT_STRING("Invalid controller mapping version!\n"));
    return;
  }

  bool isGameCube = false;
  fread(&isGameCube, 1, 1, file);
  fseek(file, (ftell(file) + 31) & ~31, SEEK_SET);
  u32 dataStart = ftell(file);
  if (isGameCube) {
    fseek(file, dataStart + ((sizeof(PADDeadZones) + sizeof(PADButtonMapping)) * playerIndex), SEEK_SET);
  }

  fread(&controller->m_deadZones, 1, sizeof(PADDeadZones), file);
  fread(&controller->m_mapping, 1, sizeof(PADButtonMapping), file);
  fclose(file);
}

bool gBlockPAD = false;
u32 PADRead(PAD::Status* status) {
  if (gBlockPAD) {
    return 0;
  }

  u32 rumbleSupport = 0;
  for (u32 i = 0; i < 4; ++i) {
    memset(&status[i], 0, sizeof(PAD::Status));
    auto controller = aurora::input::get_controller_for_player(i);
    if (controller == nullptr) {
      status[i].xa_err = PAD::ERR_NO_CONTROLLER;
      continue;
    }

    if (!controller->m_mappingLoaded) {
      __PADLoadMapping(controller);
#ifndef NDEBUG
      PADSerializeMappings();
#endif
    }
    status[i].xa_err = PAD::ERR_NONE;
    std::for_each(controller->m_mapping.begin(), controller->m_mapping.end(),
                  [&controller, &i, &status](const auto& mapping) {
                    if (SDL_GameControllerGetButton(controller->m_controller,
                                                    static_cast<SDL_GameControllerButton>(mapping.nativeButton))) {
                      status[i].x0_buttons |= mapping.padButton;
                    }
                  });

    Sint16 x = SDL_GameControllerGetAxis(controller->m_controller, SDL_CONTROLLER_AXIS_LEFTX);
    Sint16 y = SDL_GameControllerGetAxis(controller->m_controller, SDL_CONTROLLER_AXIS_LEFTY);
    if (controller->m_deadZones.useDeadzones) {
      if (std::abs(x) > controller->m_deadZones.stickDeadZone) {
        x /= 256;
      } else {
        x = 0;
      }
      if (std::abs(y) > controller->m_deadZones.stickDeadZone) {
        y = (-(y + 1u)) / 256u;
      } else {
        y = 0;
      }
    } else {
      x /= 256;
      y = (-(y + 1u)) / 256u;
    }

    status[i].x2_stickX = static_cast<s8>(x);
    status[i].x3_stickY = static_cast<s8>(y);

    x = SDL_GameControllerGetAxis(controller->m_controller, SDL_CONTROLLER_AXIS_RIGHTX);
    y = SDL_GameControllerGetAxis(controller->m_controller, SDL_CONTROLLER_AXIS_RIGHTY);
    if (controller->m_deadZones.useDeadzones) {
      if (std::abs(x) > controller->m_deadZones.substickDeadZone) {
        x /= 256;
      } else {
        x = 0;
      }

      if (std::abs(y) > controller->m_deadZones.substickDeadZone) {
        y = (-(y + 1u)) / 256u;
      } else {
        y = 0;
      }
    } else {
      x /= 256;
      y = (-(y + 1u)) / 256u;
    }

    status[i].x4_substickX = static_cast<s8>(x);
    status[i].x5_substickY = static_cast<s8>(y);

    x = SDL_GameControllerGetAxis(controller->m_controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
    y = SDL_GameControllerGetAxis(controller->m_controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
    if (/*!controller->m_isGameCube && */ controller->m_deadZones.emulateTriggers) {
      if (x > controller->m_deadZones.leftTriggerActivationZone) {
        status[i].x0_buttons |= PAD::TRIGGER_L;
      }
      if (y > controller->m_deadZones.rightTriggerActivationZone) {
        status[i].x0_buttons |= PAD::TRIGGER_R;
      }
    }
    x /= 128;
    y /= 128;

    status[i].x6_triggerL = static_cast<s8>(x);
    status[i].x7_triggerR = static_cast<s8>(y);

    if (controller->m_hasRumble) {
      rumbleSupport |= PAD::CHAN0_BIT >> i;
    }
  }
  return rumbleSupport;
}

void PADControlAllMotors(const u32* commands) {
  for (u32 i = 0; i < 4; ++i) {
    auto controller = aurora::input::get_controller_for_player(i);
    auto instance = aurora::input::get_instance_for_player(i);
    if (controller == nullptr) {
      continue;
    }

    if (controller->m_isGameCube) {
      if (commands[i] == PAD::MOTOR_STOP) {
        aurora::input::controller_rumble(instance, 0, 1, 0);
      } else if (commands[i] == PAD::MOTOR_RUMBLE) {
        aurora::input::controller_rumble(instance, 1, 1, 0);
      } else if (commands[i] == PAD::MOTOR_STOP_HARD) {
        aurora::input::controller_rumble(instance, 0, 0, 0);
      }
    } else {
      if (commands[i] == PAD::MOTOR_STOP) {
        aurora::input::controller_rumble(instance, 0, 0, 1);
      } else if (commands[i] == PAD::MOTOR_RUMBLE) {
        aurora::input::controller_rumble(instance, 32767, 32767, 0);
      } else if (commands[i] == PAD::MOTOR_STOP_HARD) {
        aurora::input::controller_rumble(instance, 0, 0, 0);
      }
    }
  }
}

u32 SIProbe(s32 chan) {
  auto *const controller = aurora::input::get_controller_for_player(chan);
  if (controller == nullptr) {
    return SI::ERROR_NO_RESPONSE;
  }

  if (controller->m_isGameCube) {
    auto level = SDL_JoystickCurrentPowerLevel(SDL_GameControllerGetJoystick(controller->m_controller));
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

s32 PADGetCount() { return aurora::input::g_GameControllers.size(); }

aurora::input::GameController* __PADGetController(s32 idx) {
  auto iter = aurora::input::g_GameControllers.begin();
  s32 i = 0;
  for (; aurora::input::g_GameControllers.begin() != aurora::input::g_GameControllers.end() && i < idx; ++iter, ++i) {}
  if (iter == aurora::input::g_GameControllers.end()) {
    return nullptr;
  }

  return &iter->second;
}

void PADGetVidPid(u32 idx, u32* vid, u32* pid) {
  *vid = 0;
  *pid = 0;
  if (idx < 0 || idx >= aurora::input::g_GameControllers.size()) {
    return;
  }
  auto* controller = __PADGetController(idx);
  if (controller == nullptr) {
    return;
  }

  *vid = controller->m_vid;
  *pid = controller->m_pid;
}

const char* PADGetName(u32 idx) {
  auto* controller = __PADGetController(idx);
  if (controller == nullptr) {
    return nullptr;
  }

  return SDL_GameControllerName(controller->m_controller);
}

void PADSetButtonMapping(u32 port, PADButtonMapping mapping) {
  auto* controller = aurora::input::get_controller_for_player(port);
  if (controller == nullptr) {
    return;
  }

  auto* iter = std::find_if(controller->m_mapping.begin(), controller->m_mapping.end(),
                            [mapping](const auto& pair) { return mapping.padButton == pair.padButton; });
  if (iter == controller->m_mapping.end()) {
    return;
  }

  *iter = mapping;
}

void PADSetAllButtonMappings(u32 port, PADButtonMapping buttons[12]) {
  for (u32 i = 0; i < 12; ++i) {
    PADSetButtonMapping(port, buttons[i]);
  }
}

PADButtonMapping* PADGetButtonMappings(u32 port, u32* buttonCount) {
  auto* controller = aurora::input::get_controller_for_player(port);
  if (controller == nullptr) {
    *buttonCount = 0;
    return nullptr;
  }

  *buttonCount = controller->m_mapping.size();
  return controller->m_mapping.data();
}

void __PADWriteDeadZones(FILE* file, aurora::input::GameController& controller) {
  fwrite(&controller.m_deadZones, 1, sizeof(PADDeadZones), file);
}

void PADSerializeMappings() {
  std::string basePath = std::string(metaforce::FileStoreManager::instance()->getStoreRoot());

  bool wroteGameCubeAlready = false;
  for (auto& controller : aurora::input::g_GameControllers) {
    if (!controller.second.m_mappingLoaded) {
      __PADLoadMapping(&controller.second);
    }
    FILE* file = fopen(fmt::format(FMT_STRING("{}/{}_{:04X}_{:04X}.controller"), basePath,
                                   aurora::input::controller_name(controller.second.m_index), controller.second.m_vid,
                                   controller.second.m_pid)
                           .c_str(),
                       "wbe");
    if (file == nullptr) {
      return;
    }

    uint32_t magic = SBIG('CTRL');
    uint32_t version = 1;
    fwrite(&magic, 1, sizeof(magic), file);
    fwrite(&version, 1, sizeof(magic), file);
    fwrite(&controller.second.m_isGameCube, 1, 1, file);
    fseek(file, (ftell(file) + 31) & ~31, SEEK_SET);
    int32_t dataStart = ftell(file);
    if (!controller.second.m_isGameCube) {
      __PADWriteDeadZones(file, controller.second);
      fwrite(controller.second.m_mapping.data(), 1, sizeof(PADButtonMapping) * controller.second.m_mapping.size(),
             file);
    } else {
      if (!wroteGameCubeAlready) {
        for (u32 i = 0; i < 4; ++i) {
          /* Just use the current controller's configs for this */
          __PADWriteDeadZones(file, controller.second);
          fwrite(mDefaultButtons.data(), 1, sizeof(PADButtonMapping) * mDefaultButtons.size(), file);
        }
        fflush(file);
        wroteGameCubeAlready = true;
      }
      uint32_t port = aurora::input::player_index(controller.second.m_index);
      fseek(file, dataStart + ((sizeof(PADDeadZones) + sizeof(PADButtonMapping)) * port), SEEK_SET);
      __PADWriteDeadZones(file, controller.second);
      fwrite(controller.second.m_mapping.data(), 1, sizeof(PADButtonMapping) * controller.second.m_mapping.size(),
             file);
    }
    fclose(file);
  }
}

PADDeadZones* PADGetDeadZones(u32 port) {
  auto* controller = aurora::input::get_controller_for_player(port);
  if (controller == nullptr) {
    return nullptr;
  }
  return &controller->m_deadZones;
}

static constexpr std::array<std::pair<PAD::BUTTON, std::string_view>, 12> skButtonNames = {{
    {PAD::BUTTON_LEFT, "Left"sv},
    {PAD::BUTTON_RIGHT, "Right"sv},
    {PAD::BUTTON_DOWN, "Down"sv},
    {PAD::BUTTON_UP, "Up"sv},
    {PAD::TRIGGER_Z, "Z"sv},
    {PAD::TRIGGER_R, "R"sv},
    {PAD::TRIGGER_L, "L"sv},
    {PAD::BUTTON_A, "A"sv},
    {PAD::BUTTON_B, "B"sv},
    {PAD::BUTTON_X, "X"sv},
    {PAD::BUTTON_Y, "Y"sv},
    {PAD::BUTTON_START, "Start"sv},
}};

const char* PADGetButtonName(PAD::BUTTON button) {
  auto it = std::find_if(skButtonNames.begin(), skButtonNames.end(),
                         [&button](const auto& pair) { return button == pair.first; });

  if (it != skButtonNames.end()) {
    return it->second.data();
  }

  return nullptr;
}

const char* PADGetNativeButtonName(u32 button) {
  return SDL_GameControllerGetStringForButton(static_cast<SDL_GameControllerButton>(button));
}

s32 PADGetNativeButtonPressed(u32 port) {
  auto* controller = aurora::input::get_controller_for_player(port);
  if (controller == nullptr) {
    return -1;
  }

  for (u32 i = 0; i < SDL_CONTROLLER_BUTTON_MAX; ++i) {
    if (SDL_GameControllerGetButton(controller->m_controller, static_cast<SDL_GameControllerButton>(i)) != 0u) {
      return i;
    }
  }
  return -1;
}

void PADRestoreDefaultMapping(u32 port) {
  auto* controller = aurora::input::get_controller_for_player(port);
  if (controller == nullptr) {
    return;
  }
  controller->m_mapping = mDefaultButtons;
}

void PADBlockInput(bool block) {
  gBlockPAD = block;
}