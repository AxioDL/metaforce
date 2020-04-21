#pragma once

#include "Runtime/Character/CModelData.hpp"
#include "Runtime/Weapon/CGSComboFire.hpp"
#include "Runtime/Weapon/CGSFidget.hpp"
#include "Runtime/Weapon/CGSFreeLook.hpp"

namespace urde {
enum class EGunState { Inactive, Default, FreeLook, ComboFire, Idle, Fidget, Strike, BigStrike };

class CGunController {
  CModelData& x0_modelData;
  CGSFreeLook x4_freeLook;
  CGSComboFire x1c_comboFire;
  CGSFidget x30_fidget;
  EGunState x50_gunState = EGunState::Inactive;
  s32 x54_curAnimId = -1;
  bool x58_24_animDone : 1 = true;
  bool x58_25_enteredComboFire : 1 = false;

public:
  explicit CGunController(CModelData& modelData) : x0_modelData(modelData) {}

  void UnLoadFidget() { x30_fidget.UnLoadAnim(); }
  void LoadFidgetAnimAsync(CStateManager& mgr, s32 type, s32 gunId, s32 animSet);
  void EnterFidget(CStateManager& mgr, s32 type, s32 gunId, s32 animSet);
  bool IsFidgetLoaded() const { return x30_fidget.IsAnimLoaded(); }
  s32 GetFreeLookSetId() const { return x4_freeLook.GetSetId(); }
  bool IsComboOver() const { return x1c_comboFire.IsComboOver(); }
  void EnterFreeLook(CStateManager& mgr, s32 gunId, s32 setId);
  void EnterComboFire(CStateManager& mgr, s32 gunId);
  void EnterStruck(CStateManager& mgr, float angle, bool bigStrike, bool b2);
  void EnterIdle(CStateManager& mgr);
  bool Update(float dt, CStateManager& mgr);
  void ReturnToDefault(CStateManager& mgr, float dt, bool setState);
  void ReturnToBasePosition(CStateManager&, float);
  void Reset();
  s32 GetCurAnimId() const { return x54_curAnimId; }
};
} // namespace urde
