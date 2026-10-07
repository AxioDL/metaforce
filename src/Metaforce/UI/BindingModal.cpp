#include "Metaforce/UI/BindingModal.hpp"

#include "Metaforce/Input.hpp"

namespace metaforce::ui {
using namespace aurora::input;
using namespace borealis::ui;
using PhysicalInput = aurora::binding::PhysicalInput;

namespace {
template < typename Mapping, typename Edit >
bool EditMapping(u16 target, Mapping* (*getMappings)(u32, u32*), u16 Mapping::* destination, Edit edit)
{
  auto* current = input::MappingFor(target, getMappings, destination);
  if (!current) {
    return false;
  }

  edit(*current);
  return true;
}

const char* BindingPrompt(BindingTarget::Kind kind) {
  if (input::KeyboardSelected()) {
    return "Press a key or mouse button, or press Escape to cancel.";
  }
  // TODO: Show the correct controller button name here instead of always Back
  if (kind == BindingTarget::Kind::Axis) {
    return "Move a stick or trigger, press a button, or press Back to cancel.";
  }
  return "Press a button, or press Back to cancel.";
}
} // namespace

BindingModal::BindingModal(BindingTarget target, const Rml::String& label)
: Modal({.title = "Bind " + label, .bodyText = BindingPrompt(target.kind)})
, mTarget(target)
, mDeviceSource(input::DeviceSource())
{
  const auto suppress = [](Rml::Event& event) { event.StopImmediatePropagation(); };
  listen(Rml::EventId::Keydown, suppress, true);
  listen(Rml::EventId::Click, suppress, true);
  listen(Rml::EventId::Mouseover, suppress, true);
}

BindingModal::~BindingModal() { StopCapture(); }

void BindingModal::show() {
  Modal::show();
  element()->Show(Rml::ModalFlag::Modal, Rml::FocusFlag::Document, Rml::ScrollFlag::None);

  if (!mCapturing) {
    CaptureNextInput();
  }
}

void BindingModal::hide(bool close) {
  element()->Show(Rml::ModalFlag::None, Rml::FocusFlag::None, Rml::ScrollFlag::None);
  Modal::hide(close);
  StopCapture();
}

bool BindingModal::focus() { return element()->Focus(); }

void BindingModal::update() {
  if (visible() && active() && mDeviceSource != input::DeviceSource()) {
    pop();
  }

  Modal::update();
}

bool BindingModal::handle_nav_command(Rml::Event&, NavCommand command) {
  if (command == NavCommand::Menu) {
    FinishBinding(false);
  }

  return true;
}

void BindingModal::CaptureNextInput() {
  mCapturing = true;
  aurora::binding::capture_next([this](const PhysicalInput& physical) {
    mCapturing = false;
    HandleCapturedInput(physical);
  });
}

void BindingModal::HandleCapturedInput(const PhysicalInput& physical) {
  if (!visible() || !active()) {
    return;
  }
  if (mDeviceSource != input::DeviceSource()) {
    pop();
    return;
  }

  const bool selectedController =
      mDeviceSource != 0 && gamepad_for_source(physical.source) == mDeviceSource;
  const bool handled = physical.control.match(
      [&](const PhysicalInput::Key& key) {
        if (key.scancode == SDL_SCANCODE_ESCAPE) {
          FinishBinding(false);
        } else if (mDeviceSource == 0) {
          FinishBinding(AssignKey(key.scancode));
        } else {
          return false;
        }
        return true;
      },
      [&](const PhysicalInput::GamepadButton& button) {
        if (button.button == SDL_GAMEPAD_BUTTON_BACK) {
          FinishBinding(false);
        } else if (selectedController) {
          FinishBinding(AssignButton(u32(button.button)));
        } else {
          return false;
        }
        return true;
      },
      [&](const PhysicalInput::MouseButton& mouse) {
        if (mDeviceSource == 0 && mouse.button >= SDL_BUTTON_LEFT &&
            mouse.button <= SDL_BUTTON_X2)
        {
          FinishBinding(AssignKey(-s32(mouse.button) - 1));
          return true;
        }
        return false;
      },
      [&](const PhysicalInput::GamepadAxis& axis) {
        const bool negative = axis.direction == PhysicalInput::GamepadAxis::Direction::Negative;
        if (selectedController && mTarget.kind == BindingTarget::Kind::Axis &&
            (!negative || axis.axis < SDL_GAMEPAD_AXIS_LEFT_TRIGGER))
        {
          FinishBinding(AssignAxis({s32(axis.axis), negative ? AXIS_SIGN_NEGATIVE : AXIS_SIGN_POSITIVE}));
          return true;
        }
        return false;
      });

  if (!handled) {
    CaptureNextInput();
  }
}

void BindingModal::StopCapture() {
  if (mCapturing) {
    aurora::binding::cancel_capture();
    mCapturing = false;
  }
}

bool BindingModal::AssignKey(s32 scancode) {
  const auto edit = [scancode](auto& mapping) {
    mapping.scancode = scancode == mapping.scancode ? PAD_KEY_INVALID : scancode;
  };

  if (mTarget.kind == BindingTarget::Kind::Axis) {
    return EditMapping(mTarget.id, PADGetKeyAxisBindings, &PADKeyAxisBinding::padAxis, edit);
  }
  return EditMapping(mTarget.id, PADGetKeyButtonBindings, &PADKeyButtonBinding::padButton, edit);
}

bool BindingModal::AssignButton(u32 button) {
  if (mTarget.kind == BindingTarget::Kind::Axis) {
    const auto edit = [button](PADAxisMapping& mapping) {
      const bool repeated = !input::HasAxis(mapping) && mapping.nativeButton == s32(button);
      mapping.nativeAxis = {-1, AXIS_SIGN_POSITIVE};
      mapping.nativeButton = button < SDL_GAMEPAD_BUTTON_COUNT && !repeated ? s32(button) : -1;
    };

    return EditMapping(mTarget.id, PADGetAxisMappings, &PADAxisMapping::padAxis, edit);
  }

  const auto edit = [button](PADButtonMapping& mapping) {
    mapping.nativeButton = button == mapping.nativeButton ? PAD_NATIVE_BUTTON_INVALID : button;
  };

  return EditMapping(mTarget.id, PADGetButtonMappings, &PADButtonMapping::padButton, edit);
}

bool BindingModal::AssignAxis(PADSignedNativeAxis axis) {
  const auto edit = [axis](PADAxisMapping& mapping) {
    const bool repeated =
        mapping.nativeAxis.nativeAxis == axis.nativeAxis && mapping.nativeAxis.sign == axis.sign;
    mapping.nativeAxis = repeated ? PADSignedNativeAxis{-1, AXIS_SIGN_POSITIVE} : axis;
    mapping.nativeButton = -1;
  };

  return EditMapping(mTarget.id, PADGetAxisMappings, &PADAxisMapping::padAxis, edit);
}

void BindingModal::FinishBinding(bool assigned) {
  if (assigned) {
    PADSerializeMappings();
  }
  pop();
  play_nav_sound(assigned ? NavSound::BindingChanged : NavSound::WindowClose);
}

} // namespace metaforce::ui
