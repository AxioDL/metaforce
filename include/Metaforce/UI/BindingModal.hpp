#pragma once

#include <aurora/binding.hpp>
#include <borealis/ui/modal.hpp>
#include <dolphin/pad.h>

#include <cstdint>

namespace metaforce::ui {

struct BindingTarget {
  enum class Kind { Button, Axis };
  Kind kind;
  u16 id;
};

class BindingModal : public borealis::ui::Modal {
public:
  BindingModal(BindingTarget target, const Rml::String& label);
  ~BindingModal() override;
  void show() override;
  void hide(bool close) override;
  bool focus() override;
  void update() override;

private:
  bool handle_nav_command(Rml::Event& event, borealis::ui::NavCommand command) override;

  void CaptureNextInput();
  void HandleCapturedInput(const aurora::binding::PhysicalInput& physical);
  void StopCapture();

  bool AssignKey(s32 scancode);
  bool AssignButton(u32 button);
  bool AssignAxis(PADSignedNativeAxis axis);
  void FinishBinding(bool assigned);

  BindingTarget mTarget;
  uint32_t mDeviceSource;
  bool mCapturing = false;
  aurora::input::LayerId mInputLayer = aurora::input::kInvalidLayerId;
};

} // namespace metaforce::ui
