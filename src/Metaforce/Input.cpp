#include "Metaforce/Input.hpp"

#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_scancode.h>

#include <algorithm>
#include <limits>

namespace metaforce::input {
namespace {
bool sInitialized = false;
uint32_t sDeviceSource = std::numeric_limits< uint32_t >::max();

constexpr PADDefaultKeyBindings kDefaultKeyBindings{
    .buttons =
        {
            {SDL_SCANCODE_SPACE, PAD_BUTTON_A},
            {SDL_SCANCODE_LSHIFT, PAD_BUTTON_B},
            {SDL_SCANCODE_F, PAD_BUTTON_X},
            {SDL_SCANCODE_R, PAD_BUTTON_Y},
            {SDL_SCANCODE_TAB, PAD_TRIGGER_Z},
            {SDL_SCANCODE_RETURN, PAD_BUTTON_START},
            {SDL_SCANCODE_UP, PAD_BUTTON_UP},
            {SDL_SCANCODE_DOWN, PAD_BUTTON_DOWN},
            {SDL_SCANCODE_LEFT, PAD_BUTTON_LEFT},
            {SDL_SCANCODE_RIGHT, PAD_BUTTON_RIGHT},
            {SDL_SCANCODE_Q, PAD_TRIGGER_L},
            {SDL_SCANCODE_E, PAD_TRIGGER_R},
        },
    .axes =
        {
            {SDL_SCANCODE_W, PAD_AXIS_LEFT_Y_POS, 1},
            {SDL_SCANCODE_S, PAD_AXIS_LEFT_Y_NEG, 1},
            {SDL_SCANCODE_A, PAD_AXIS_LEFT_X_NEG, 1},
            {SDL_SCANCODE_D, PAD_AXIS_LEFT_X_POS, 1},
            {SDL_SCANCODE_I, PAD_AXIS_RIGHT_Y_POS, 1},
            {SDL_SCANCODE_K, PAD_AXIS_RIGHT_Y_NEG, 1},
            {SDL_SCANCODE_J, PAD_AXIS_RIGHT_X_NEG, 1},
            {SDL_SCANCODE_L, PAD_AXIS_RIGHT_X_POS, 1},
            {SDL_SCANCODE_Q, PAD_AXIS_TRIGGER_L, 1},
            {SDL_SCANCODE_E, PAD_AXIS_TRIGGER_R, 1},
        },
};

} // namespace

bool Initialize() {
  if (!PADSetDefaultKeyBindings(0, &kDefaultKeyBindings) || !PADInit()) {
    return false;
  }
  sInitialized = true;
  Update();
  return true;
}

void Shutdown() {
  if (sInitialized) {
    PADControlMotor(0, PAD_MOTOR_STOP_HARD);
  }
  sDeviceSource = std::numeric_limits< uint32_t >::max();
  sInitialized = false;
}

void Update() {
  if (!sInitialized) {
    return;
  }
  const auto source = DeviceSource();
  if (source != sDeviceSource) {
    sDeviceSource = source;
    PADSetKeyboardActive(0, source == 0);
    if (source != 0) {
      u32 count = 0;
      (void)PADGetButtonMappings(0, &count);
    }
  }
  if (source != 0) {
    // I want trigger emulation to be implicit, so make sure this is on
    if (auto* zones = PADGetDeadZones(0); zones && !zones->emulateTriggers) {
      zones->emulateTriggers = true;
    }
  }
}

std::vector< InputDevice > Devices() {
  std::vector< InputDevice > devices{{0, "Mouse & Keyboard"}};
  for (u32 index = 0; index < PADCount(); ++index) {
    auto* gamepad = PADGetSDLGamepadForIndex(index);
    if (!gamepad) {
      continue;
    }
    const char* name = PADGetNameForControllerIndex(index);
    devices.push_back({SDL_GetGamepadID(gamepad), name ? name : "Unknown Controller"});
  }
  std::ranges::sort(devices.begin() + 1, devices.end(), {}, &InputDevice::source);
  return devices;
}

uint32_t DeviceSource() {
  if (auto* gamepad = SelectedGamepad()) {
    return SDL_GetGamepadID(gamepad);
  }
  return 0;
}

void SelectDevice(uint32_t source) {
  if (source != 0 && source == DeviceSource()) {
    Update();
    return;
  }
  int index = -1;
  if (source != 0) {
    for (u32 i = 0; i < PADCount(); ++i) {
      if (auto* gamepad = PADGetSDLGamepadForIndex(i);
          gamepad && SDL_GetGamepadID(gamepad) == source)
      {
        index = int(i);
        break;
      }
    }
    if (index < 0) {
      return;
    }
  }
  PADControlMotor(0, PAD_MOTOR_STOP_HARD);
  if (source == 0) {
    PADClearPort(0);
  } else {
    PADSetPortForIndex(u32(index), 0);
  }
  Update();
}

bool HasAxis(const PADAxisMapping& mapping) {
  return mapping.nativeAxis.nativeAxis >= 0 &&
         mapping.nativeAxis.nativeAxis < SDL_GAMEPAD_AXIS_COUNT;
}

bool KeyboardSelected() { return DeviceSource() == 0; }

SDL_Gamepad* SelectedGamepad() {
  const int index = PADGetIndexForPort(0);
  if (index < 0) {
    return nullptr;
  }
  return PADGetSDLGamepadForIndex(u32(index));
}

void ResetBindings() {
  if (KeyboardSelected()) {
    PADRestoreDefaultKeyBindings(0);
  } else {
    u32 count = 0;
    (void)PADGetButtonMappings(0, &count);
    PADRestoreDefaultMapping(0);
  }
  PADSerializeMappings();
}

} // namespace metaforce::input
