#pragma once

#include <array>

#include "Runtime/Input/CKeyboardMouseController.hpp"
#include "Runtime/Input/CControllerGamepadData.hpp"
#include "Runtime/RetroTypes.hpp"

namespace metaforce {

struct SAuroraControllerState {
  u32 m_which = -1;
  bool m_isGamecube = false;
  bool m_hasRumble = false;
  std::array<int16_t, size_t(aurora::ControllerAxis::MAX)> m_axes{};
  std::bitset<size_t(aurora::ControllerButton::MAX)> m_btns{};

  SAuroraControllerState() = default;
  SAuroraControllerState(uint32_t which, bool isGamecube, bool hasRumble)
  : m_which(which), m_isGamecube(isGamecube), m_hasRumble(hasRumble) {}
  void clamp();
};

struct CFinalInput {
  float x0_dt = 0.0f;
  u32 x4_controllerIdx = 0;
  float x8_anaLeftX = 0.0f;
  float xc_anaLeftY = 0.0f;
  float x10_anaRightX = 0.0f;
  float x14_anaRightY = 0.0f;
  float x18_anaLeftTrigger = 0.0f;
  float x1c_anaRightTrigger = 0.0f;

  bool x20_enableAnaLeftXP  = false;
  bool x21_enableAnaLeftYP  = false;
  bool x22_enableAnaRightXP = false;
  bool x23_enableAnaRightYP = false;

  float x24_anaLeftTriggerP = 0.f;
  float x28_anaRightTriggerP = 0.f;

  bool x2c_b24_A : 1 = false;
  bool x2c_b25_B : 1 = false;
  bool x2c_b26_X : 1 = false;
  bool x2c_b27_Y : 1 = false;
  bool x2c_b28_Z : 1 = false;
  bool x2c_b29_L : 1 = false;
  bool x2c_b30_R : 1 = false;
  bool x2c_b31_DPUp : 1 = false;
  bool x2d_b24_DPRight : 1 = false;
  bool x2d_b25_DPDown : 1 = false;
  bool x2d_b26_DPLeft : 1 = false;
  bool x2d_b27_Start : 1 = false;

  bool x2d_b28_PA : 1 = false;
  bool x2d_b29_PB : 1 = false;
  bool x2d_b30_PX : 1 = false;
  bool x2d_b31_PY : 1 = false;
  bool x2e_b24_PZ : 1 = false;
  bool x2e_b25_PL : 1 = false;
  bool x2e_b26_PR : 1 = false;
  bool x2e_b27_PDPUp : 1 = false;
  bool x2e_b28_PDPRight : 1 = false;
  bool x2e_b29_PDPDown : 1 = false;
  bool x2e_b30_PDPLeft : 1 = false;
  bool x2e_b31_PStart : 1 = false;

  std::optional<CKeyboardMouseControllerData> m_kbm;

  std::array<bool, 256> m_PCharKeys{};
  std::array<bool, size_t(aurora::SpecialKey::MAX)> m_PSpecialKeys{};
  std::array<bool, 6> m_PMouseButtons{};


  CFinalInput();
  CFinalInput(int cIdx, float dt, const CControllerGamepadData& data, float leftDiv, float rightDiv);
  CFinalInput(int cIdx, float dt, const CKeyboardMouseControllerData& data, const CFinalInput& prevInput);

  //CFinalInput& operator|=(const CFinalInput& other);

  bool operator==(const CFinalInput& other) const { return memcmp(this, &other, sizeof(CFinalInput)) == 0; }
  bool operator!=(const CFinalInput& other) const { return !operator==(other); }

  float DeltaTime() const { return x0_dt; }
  u32 ControllerIdx() const { return x4_controllerIdx; }

  bool PStart() const { return x2e_b31_PStart; }
  bool PR() const { return x2e_b26_PR; }
  bool PL() const { return x2e_b25_PL; }
  bool PZ() const { return x2e_b24_PZ; }
  bool PY() const { return x2d_b31_PY; }
  bool PX() const { return x2d_b30_PX; }
  bool PB() const { return x2d_b29_PB; }
  bool PA() const { return x2d_b28_PA; }
  bool PDPRight() const { return x2e_b28_PDPRight; }
  bool PDPLeft() const { return x2e_b30_PDPLeft; }
  bool PDPDown() const { return x2e_b29_PDPDown; }
  bool PDPUp() const { return x2e_b27_PDPUp; }
  bool PRTrigger() const { return x28_anaRightTriggerP > 0.05f; }
  bool PLTrigger() const { return x24_anaLeftTriggerP > 0.05f; }
  bool PRARight() const { return x10_anaRightX > 0.7f && x22_enableAnaRightXP; }
  bool PRALeft() const { return x10_anaRightX < -0.7f && x22_enableAnaRightXP; }
  bool PRADown() const { return x14_anaRightY < -0.7f && x23_enableAnaRightYP; }
  bool PRAUp() const { return x14_anaRightY > 0.7f && x23_enableAnaRightYP; }
  bool PLARight() const { return x8_anaLeftX > 0.7f && x20_enableAnaLeftXP; }
  bool PLALeft() const { return x8_anaLeftX < -0.7f && x20_enableAnaLeftXP; }
  bool PLADown() const { return xc_anaLeftY < -0.7f && x21_enableAnaLeftYP; }
  bool PLAUp() const { return xc_anaLeftY > 0.7f && x21_enableAnaLeftYP; }
  bool DStart() const { return x2d_b27_Start; }
  bool DR() const { return x2c_b30_R; }
  bool DL() const { return x2c_b29_L; }
  bool DZ() const { return x2c_b28_Z; }
  bool DY() const { return x2c_b27_Y; }
  bool DX() const { return x2c_b26_X; }
  bool DB() const { return x2c_b25_B; }
  bool DA() const { return x2c_b24_A; }
  bool DDPRight() const { return x2d_b24_DPRight; }
  bool DDPLeft() const { return x2d_b26_DPLeft; }
  bool DDPDown() const { return x2d_b25_DPDown; }
  bool DDPUp() const { return x2c_b31_DPUp; }
  bool DRTrigger() const { return x1c_anaRightTrigger > 0.05f; }
  bool DLTrigger() const { return x18_anaLeftTrigger > 0.05f; }
  bool DRARight() const { return x10_anaRightX > 0.7f; }
  bool DRALeft() const { return x10_anaRightX < -0.7f; }
  bool DRADown() const { return x14_anaRightY < -0.7f; }
  bool DRAUp() const { return x14_anaRightY > 0.7f; }
  bool DLARight() const { return x8_anaLeftX > 0.7f; }
  bool DLALeft() const { return x8_anaLeftX < -0.7f; }
  bool DLADown() const { return xc_anaLeftY < -0.7f; }
  bool DLAUp() const { return xc_anaLeftY > 0.7f; }
  float AStart() const { return x2d_b27_Start ? 1.f : 0.f; }
  float AR() const { return x2c_b30_R ? 1.f : 0.f; }
  float AL() const { return x2c_b29_L ? 1.f : 0.f; }
  float AZ() const { return x2c_b28_Z ? 1.f : 0.f; }
  float AY() const { return x2c_b27_Y ? 1.f : 0.f; }
  float AX() const { return x2c_b26_X ? 1.f : 0.f; }
  float AB() const { return x2c_b25_B ? 1.f : 0.f; }
  float AA() const { return x2c_b24_A ? 1.f : 0.f; }
  float ADPRight() const { return x2d_b24_DPRight ? 1.f : 0.f; }
  float ADPLeft() const { return x2d_b26_DPLeft ? 1.f : 0.f; }
  float ADPDown() const { return x2d_b25_DPDown ? 1.f : 0.f; }
  float ADPUp() const { return x2c_b31_DPUp ? 1.f : 0.f; }
  float ARTrigger() const { return x1c_anaRightTrigger; }
  float ALTrigger() const { return x18_anaLeftTrigger; }
  float ARARight() const { return x10_anaRightX > 0.f ? x10_anaRightX : 0.f; }
  float ARALeft() const { return x10_anaRightX < 0.f ? -x10_anaRightX : 0.f; }
  float ARADown() const { return x14_anaRightY < 0.f ? -x14_anaRightY : 0.f; }
  float ARAUp() const { return x14_anaRightY > 0.f ? x14_anaRightY : 0.f; }
  float ALARight() const { return x8_anaLeftX > 0.f ? x8_anaLeftX : 0.f; }
  float ALALeft() const { return x8_anaLeftX < 0.f ? -x8_anaLeftX : 0.f; }
  float ALADown() const { return xc_anaLeftY < 0.f ? -xc_anaLeftY : 0.f; }
  float ALAUp() const { return xc_anaLeftY > 0.f ? xc_anaLeftY : 0.f; }

  float ALeftX() const { return x8_anaLeftX; }
  float ALeftY() const { return xc_anaLeftY; }
  float ARightX() const { return x10_anaRightX; }
  float ARightY() const { return x14_anaRightY; }
  float ALeftTrigger() const { return x18_anaLeftTrigger; }
  float ARightTrigger() const { return x1c_anaRightTrigger; }

  CFinalInput ScaleAnalogueSticks(float leftDiv, float rightDiv) const;
  void InitializeAnalog(float leftDiv, float rightDiv);

  bool PKey(char k) const { return m_kbm && m_PCharKeys[size_t(k)]; }
  bool PSpecialKey(aurora::SpecialKey k) const { return m_kbm && m_PSpecialKeys[size_t(k)]; }
  bool PMouseButton(EMouseButton k) const { return m_kbm && m_PMouseButtons[size_t(k)]; }
  bool DKey(char k) const { return m_kbm && m_kbm->m_charKeys[size_t(k)]; }
  bool DSpecialKey(aurora::SpecialKey k) const { return m_kbm && m_kbm->m_specialKeys[size_t(k)]; }
  bool DMouseButton(EMouseButton k) const { return m_kbm && m_kbm->m_mouseButtons[size_t(k)]; }
  float AKey(char k) const { return DKey(k) ? 1.f : 0.f; }
  float ASpecialKey(aurora::SpecialKey k) const { return DSpecialKey(k) ? 1.f : 0.f; }
  float AMouseButton(EMouseButton k) const { return DMouseButton(k) ? 1.f : 0.f; }

  const std::optional<CKeyboardMouseControllerData>& GetKBM() const { return m_kbm; }
};

} // namespace metaforce
