#include "Metaforce/UI/ControllerConfigWindow.hpp"

#include "Metaforce/Input.hpp"
#include "Metaforce/UI/BindingModal.hpp"

#include <borealis/ui/bool_button.hpp>
#include <borealis/ui/modal.hpp>
#include <borealis/ui/number_button.hpp>
#include <borealis/ui/pane.hpp>
#include <borealis/ui/ui.hpp>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_keyboard.h>

#include <array>
#include <cmath>
#include <memory>

namespace metaforce::ui {
using namespace borealis::ui;
namespace {

// TODO: All this friendly naming should probably be moved into Aurora for all ports to benefit
constexpr std::array kDefaultGamepadButtonNames{
    "South",          "East",           "West",          "North",          "Back",
    "Guide",          "Start",          "Left Stick",    "Right Stick",    "Left Shoulder",
    "Right Shoulder", "D-Pad Up",       "D-Pad Down",    "D-Pad Left",     "D-Pad Right",
    "Misc Button 1",  "Right Paddle 1", "Left Paddle 1", "Right Paddle 2", "Left Paddle 2",
    "Touchpad",       "Misc Button 2",  "Misc Button 3", "Misc Button 4",  "Misc Button 5",
    "Misc Button 6",
};

SDL_GamepadType GamepadType(SDL_Gamepad* gamepad) {
  if (gamepad) {
    return SDL_GetGamepadType(gamepad);
  }
  return SDL_GAMEPAD_TYPE_UNKNOWN;
}

const char* GamepadButtonName(SDL_Gamepad* gamepad, u32 nativeButton) {
  if (nativeButton >= SDL_GAMEPAD_BUTTON_COUNT) {
    return "Unbound";
  }

  const auto button = SDL_GamepadButton(nativeButton);
  if (gamepad) {
    switch (SDL_GetGamepadButtonLabel(gamepad, button)) {
    case SDL_GAMEPAD_BUTTON_LABEL_A:
      return "A";
    case SDL_GAMEPAD_BUTTON_LABEL_B:
      return "B";
    case SDL_GAMEPAD_BUTTON_LABEL_X:
      return "X";
    case SDL_GAMEPAD_BUTTON_LABEL_Y:
      return "Y";
    case SDL_GAMEPAD_BUTTON_LABEL_CROSS:
      return "Cross";
    case SDL_GAMEPAD_BUTTON_LABEL_CIRCLE:
      return "Circle";
    case SDL_GAMEPAD_BUTTON_LABEL_SQUARE:
      return "Square";
    case SDL_GAMEPAD_BUTTON_LABEL_TRIANGLE:
      return "Triangle";
    default:
      break;
    }
  }

  const auto type = GamepadType(gamepad);
  switch (button) {
  case SDL_GAMEPAD_BUTTON_LEFT_STICK:
  case SDL_GAMEPAD_BUTTON_RIGHT_STICK: {
    const bool left = button == SDL_GAMEPAD_BUTTON_LEFT_STICK;
    switch (type) {
    case SDL_GAMEPAD_TYPE_PS3:
    case SDL_GAMEPAD_TYPE_PS4:
    case SDL_GAMEPAD_TYPE_PS5:
      return left ? "L3" : "R3";
    case SDL_GAMEPAD_TYPE_GAMECUBE:
      return left ? "Control Stick" : "C Stick";
    default:
      break;
    }
    break;
  }
  case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
  case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: {
    const bool left = button == SDL_GAMEPAD_BUTTON_LEFT_SHOULDER;
    switch (type) {
    case SDL_GAMEPAD_TYPE_PS3:
    case SDL_GAMEPAD_TYPE_PS4:
    case SDL_GAMEPAD_TYPE_PS5:
      return left ? "L1" : "R1";
    case SDL_GAMEPAD_TYPE_XBOX360:
    case SDL_GAMEPAD_TYPE_XBOXONE:
      return left ? "LB" : "RB";
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO:
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_LEFT:
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT:
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR:
      return left ? "L" : "R";
    case SDL_GAMEPAD_TYPE_GAMECUBE:
      if (!left) {
        return "Z";
      }
      break;
    default:
      break;
    }
    break;
  }
  case SDL_GAMEPAD_BUTTON_BACK:
  case SDL_GAMEPAD_BUTTON_START: {
    const bool start = button == SDL_GAMEPAD_BUTTON_START;
    switch (type) {
    case SDL_GAMEPAD_TYPE_PS3:
      return start ? "Start" : "Select";
    case SDL_GAMEPAD_TYPE_PS4:
      return start ? "Options" : "Share";
    case SDL_GAMEPAD_TYPE_PS5:
      return start ? "Options" : "Create";
    case SDL_GAMEPAD_TYPE_XBOX360:
      return start ? "Start" : "Back";
    case SDL_GAMEPAD_TYPE_XBOXONE:
      return start ? "Menu" : "View";
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO:
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_LEFT:
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT:
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR:
      return start ? "Plus" : "Minus";
    case SDL_GAMEPAD_TYPE_GAMECUBE:
      if (start) {
        return "Start/Pause";
      }
      break;
    default:
      break;
    }
    break;
  }
  case SDL_GAMEPAD_BUTTON_MISC3:
  case SDL_GAMEPAD_BUTTON_MISC4:
    if (type == SDL_GAMEPAD_TYPE_GAMECUBE) {
      return button == SDL_GAMEPAD_BUTTON_MISC3 ? "L" : "R";
    }
    break;
  default:
    break;
  }

  return kDefaultGamepadButtonNames[button];
}

const char* GamepadAxisName(SDL_Gamepad* gamepad, SDL_GamepadAxis axis) {
  const auto type = GamepadType(gamepad);
  const bool gamecube = type == SDL_GAMEPAD_TYPE_GAMECUBE;
  switch (axis) {
  case SDL_GAMEPAD_AXIS_LEFTX:
    return gamecube ? "Control Stick X" : "Left Stick X";
  case SDL_GAMEPAD_AXIS_LEFTY:
    return gamecube ? "Control Stick Y" : "Left Stick Y";
  case SDL_GAMEPAD_AXIS_RIGHTX:
    return gamecube ? "C Stick X" : "Right Stick X";
  case SDL_GAMEPAD_AXIS_RIGHTY:
    return gamecube ? "C Stick Y" : "Right Stick Y";
  case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
  case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER: {
    const bool left = axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER;
    switch (type) {
    case SDL_GAMEPAD_TYPE_PS3:
    case SDL_GAMEPAD_TYPE_PS4:
    case SDL_GAMEPAD_TYPE_PS5:
      return left ? "L2" : "R2";
    case SDL_GAMEPAD_TYPE_XBOX360:
    case SDL_GAMEPAD_TYPE_XBOXONE:
      return left ? "LT" : "RT";
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO:
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_LEFT:
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT:
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR:
      return left ? "ZL" : "ZR";
    case SDL_GAMEPAD_TYPE_GAMECUBE:
      return left ? "L" : "R";
    default:
      return left ? "Left Trigger" : "Right Trigger";
    }
  }
  default:
    return "Unknown Control";
  }
}

const char* KeyboardInputName(s32 scancode) {
  switch (scancode) {
  case PAD_KEY_INVALID:
    return "Unbound";
  case PAD_KEY_MOUSE_LEFT:
    return "Mouse Left";
  case PAD_KEY_MOUSE_MIDDLE:
    return "Mouse Middle";
  case PAD_KEY_MOUSE_RIGHT:
    return "Mouse Right";
  case PAD_KEY_MOUSE_X1:
    return "Mouse X1";
  case PAD_KEY_MOUSE_X2:
    return "Mouse X2";
  default:
    return SDL_GetScancodeName(SDL_Scancode(scancode));
  }
}

PADDeadZones* DeadZones() {
  u32 count = 0;
  (void)PADGetButtonMappings(0, &count);
  return PADGetDeadZones(0);
}

int RawToPercent(int value) { return int(std::lround(value * 100.f / 32767.f)); }

void AddTriggerThreshold(Pane& pane, const char* label, PADButton button, u16 PADDeadZones::* field) {
  pane.add_child< NumberButton >(NumberButton::Props{
    .key = label,
    .getValue = [field] {
      if (const auto* zones = DeadZones()) {
        return RawToPercent(zones->*field + 1);
      }
      return 100;
    },
    .setValue = [field](int value) {
      if (auto* zones = DeadZones()) {
        zones->*field = u16(std::floor(value * 32767.f / 100.f) - 1);
        PADSerializeMappings();
      }
    },
    .isDisabled = [button] {
      if (input::KeyboardSelected()) {
        return true;
      }
      const auto* mapping =
          input::MappingFor(button, PADGetButtonMappings, &PADButtonMapping::padButton);
      return !mapping || mapping->nativeButton < SDL_GAMEPAD_BUTTON_COUNT;
    },
    .isModified = [field] {
      const auto* zones = DeadZones();
      return zones && RawToPercent(zones->*field + 1) != RawToPercent(31150 + 1);
    },
    .min = 1, .max = 100, .step = 5, .suffix = "%",
  });
}

} // namespace

ControllerConfigWindow::ControllerConfigWindow() : Window({.tabBar = false}) {
  listen(
      Rml::EventId::Focus,
      [this](Rml::Event& event) {
        if (!visible() || !active()) {
          event.StopImmediatePropagation();
        }
      },
      true);

  set_content([this](Rml::Element* content) {
    auto& leftPane = add_child< Pane >(content, Pane::Type::Controlled);
    auto& rightPane = add_child< Pane >(content, Pane::Type::Uncontrolled);

    // TODO: This band-aid kinda sucks, think of something better or just enable the tabBar
    rightPane.root()->SetProperty("margin-top", "var(--toolbar-height)");

    const auto addPage = [this, &leftPane, &rightPane](Page page, const char* title) {
      leftPane.register_control(leftPane.add_group_button({
                                    .text = title,
                                }),
                                rightPane, [this, page](Pane& pane) { RenderPage(pane, page); });
    };

    addPage(Page::Buttons, "Buttons");
    addPage(Page::Triggers, "Triggers");
    addPage(Page::Sticks, "Sticks");

    // TODO: Section header styling only applies margins for the second occurrence onward, which
    // means that panes that don't immediately start with a section look broken. Fix this.
    leftPane.add_section("Options");

    leftPane.register_control(
      leftPane.add_child< BoolButton >(BoolButton::Props{
        .key = "Enable Deadzones",
        .getValue = [] {
          const auto* zones = DeadZones();
          return zones && zones->useDeadzones;
        },
        .setValue = [](bool enabled) {
          if (auto* zones = DeadZones()) {
            zones->useDeadzones = enabled;
            PADSerializeMappings();
          }
        },
        .isDisabled = [] { return DeadZones() == nullptr; },
      }),
      rightPane, [](Pane& pane) {
        pane.add_text("Apply configured deadzones to the Control Stick and C Stick.");
      }
    );

    leftPane.register_control(
      leftPane.add_button({.text = "Reset Bindings"}).on_pressed([this] {
        const auto source = input::DeviceSource();
        const auto deviceChanged = [source] { return source != input::DeviceSource(); };
        push(std::make_unique< Modal >(Modal::Props{
          .title = "Reset Bindings?",
          .bodyText = "This will reset all bindings to this controller's defaults, are you sure you want to proceed?",
          .actions = {
            {
              .label = "Reset",
              .onPressed = [deviceChanged](Modal& modal) {
                 if (!deviceChanged()) {
                   input::ResetBindings();
                 }
                 modal.pop();
              },
              .isDisabled = deviceChanged
            },
            {
              .label = "Cancel",
              .onPressed = [](Modal& modal) { modal.pop(); }
            }
          },
          .variant = "danger",
          .icon = "warning",
        }));
      }),
      rightPane, [](Pane& pane) {
        pane.add_text("Reset all bindings for this device.");
      }
    );

    RenderPage(rightPane, Page::Buttons);
  });
}

void ControllerConfigWindow::RenderPage(Pane& pane, Page page) {
  switch (page) {
  case Page::Buttons:
    pane.add_section("Buttons");
    for (PADButton button : {PAD_BUTTON_A, PAD_BUTTON_B, PAD_BUTTON_X, PAD_BUTTON_Y, PAD_TRIGGER_Z,
                             PAD_BUTTON_START}) {
      AddBinding(pane, {BindingTarget::Kind::Button, button});
    }
    pane.add_section("D-Pad");
    for (PADButton button : {PAD_BUTTON_UP, PAD_BUTTON_DOWN, PAD_BUTTON_LEFT, PAD_BUTTON_RIGHT}) {
      AddBinding(pane, {BindingTarget::Kind::Button, button});
    }
    break;
  case Page::Triggers:
    pane.add_section("Analog");
    AddBinding(pane, {BindingTarget::Kind::Axis, PAD_AXIS_TRIGGER_L});
    AddBinding(pane, {BindingTarget::Kind::Axis, PAD_AXIS_TRIGGER_R});
    pane.add_section("Digital");
    AddBinding(pane, {BindingTarget::Kind::Button, PAD_TRIGGER_L});
    AddBinding(pane, {BindingTarget::Kind::Button, PAD_TRIGGER_R});
    pane.add_text("Unbound digital(s) enable trigger emulation, simulating a digital press at your configured threshold.");
    pane.add_section("Emulated Trigger Thresholds");
    AddTriggerThreshold(pane, "L Threshold", PAD_TRIGGER_L, &PADDeadZones::leftTriggerActivationZone);
    AddTriggerThreshold(pane, "R Threshold", PAD_TRIGGER_R, &PADDeadZones::rightTriggerActivationZone);
    break;
  case Page::Sticks: {
    const auto addStick = [this, &pane](const char* label, std::array< PADAxis, 4 > axes,
                                        u16 PADDeadZones::* field)
    {
      pane.add_section(label);
      for (PADAxis axis : axes) {
        AddBinding(pane, {BindingTarget::Kind::Axis, axis});
      }
      pane.add_child< NumberButton >(NumberButton::Props{
        .key = "Deadzone",
        .getValue =
            [field] {
              const auto* zones = DeadZones();
              return zones ? RawToPercent(zones->*field) : 0;
            },
        .setValue =
            [field](int value) {
              if (auto* zones = DeadZones()) {
                zones->useDeadzones = true;
                zones->*field = u16(value * 32767 / 100);
                PADSerializeMappings();
              }
            },
        .isDisabled =
            [] {
              const auto* zones = DeadZones();
              return !zones || !zones->useDeadzones;
            },
        .isModified =
            [field] {
              const auto* zones = DeadZones();
              return zones && (!zones->useDeadzones ||
                               RawToPercent(zones->*field) != RawToPercent(8000));
            },
        .min = 0,
        .max = 90,
        .step = 5,
        .suffix = "%",
      });
    };
    addStick("Control Stick",
             {PAD_AXIS_LEFT_Y_POS, PAD_AXIS_LEFT_Y_NEG, PAD_AXIS_LEFT_X_NEG, PAD_AXIS_LEFT_X_POS},
             &PADDeadZones::stickDeadZone);
    addStick("C Stick",
             {PAD_AXIS_RIGHT_Y_POS, PAD_AXIS_RIGHT_Y_NEG, PAD_AXIS_RIGHT_X_NEG, PAD_AXIS_RIGHT_X_POS},
             &PADDeadZones::substickDeadZone);
    break;
  }
  }
}

void ControllerConfigWindow::AddBinding(Pane& pane, BindingTarget target) {
  Rml::String label;
  if (target.kind != BindingTarget::Kind::Axis) {
    label = PADGetButtonName(target.id);
    const bool isDPad = target.id == PAD_BUTTON_UP || target.id == PAD_BUTTON_DOWN ||
                        target.id == PAD_BUTTON_LEFT || target.id == PAD_BUTTON_RIGHT;
    if (isDPad) {
      label = "D-Pad " + label;
    }
  } else if (target.id >= PAD_AXIS_TRIGGER_L) {
    label = PADGetAxisName(target.id);
  } else {
    const bool isControlStick = target.id < PAD_AXIS_RIGHT_X_POS;
    label = isControlStick ? "Control Stick " : "C Stick ";
    label += PADGetAxisDirectionLabel(target.id);
  }

  auto& button = pane.add_select_button({
      .key = label,
      .getValue = [target] { return BindingLabel(target); },
  });
  button.on_pressed([this, target, label] {
    mWasVisible = visible();
    push_document(std::make_unique< BindingModal >(target, label));
  });
}

Rml::String ControllerConfigWindow::BindingLabel(BindingTarget target) {
  const bool isKeyboard = input::KeyboardSelected();
  const bool isAxis = target.kind == BindingTarget::Kind::Axis;

  if (isKeyboard && isAxis) {
    const auto* axisBinding =
        input::MappingFor(target.id, PADGetKeyAxisBindings, &PADKeyAxisBinding::padAxis);
    return axisBinding ? KeyboardInputName(axisBinding->scancode) : "Unbound";
  }

  if (isKeyboard) {
    const auto* buttonBinding =
        input::MappingFor(target.id, PADGetKeyButtonBindings, &PADKeyButtonBinding::padButton);
    return buttonBinding ? KeyboardInputName(buttonBinding->scancode) : "Unbound";
  }

  auto* gamepad = input::SelectedGamepad();
  if (isAxis) {
    const auto* axisMapping =
        input::MappingFor(target.id, PADGetAxisMappings, &PADAxisMapping::padAxis);
    if (!axisMapping) {
      return "Unbound";
    }
    if (!input::HasAxis(*axisMapping)) {
      return GamepadButtonName(gamepad, u32(axisMapping->nativeButton));
    }
    const auto& axis = axisMapping->nativeAxis;
    Rml::String name = GamepadAxisName(gamepad, SDL_GamepadAxis(axis.nativeAxis));
    if (axis.nativeAxis < SDL_GAMEPAD_AXIS_LEFT_TRIGGER) {
      name += axis.sign == AXIS_SIGN_NEGATIVE ? "-" : "+";
    }
    return name;
  }

  const auto* buttonMapping =
      input::MappingFor(target.id, PADGetButtonMappings, &PADButtonMapping::padButton);
  return buttonMapping ? GamepadButtonName(gamepad, buttonMapping->nativeButton) : "Unbound";
}

} // namespace metaforce::ui
