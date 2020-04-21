#pragma once

#include <vector>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Character/CModelData.hpp"
#include "Runtime/Weapon/CGunController.hpp"

#include <zeus/CVector3f.hpp>

namespace urde {

namespace SamusGun {
enum class EAnimationState { Wander, Fidget, Struck, FreeLook, ComboFire, Idle, BasePosition };
enum class EFidgetType { Invalid = -1, Minor, Major };
} // namespace SamusGun

class CGunMotion {
  CModelData x0_modelData;
  CGunController x4c_gunController;
  std::vector<CToken> xa8_anims;
  bool xb8_24_animPlaying : 1 = false;

  void LoadAnimations();

public:
  CGunMotion(CAssetId ancsId, const zeus::CVector3f& scale);
  const CModelData& GetModelData() const { return x0_modelData; }
  bool PlayPasAnim(SamusGun::EAnimationState state, CStateManager& mgr, float angle, bool bigStrike);
  void ReturnToDefault(CStateManager& mgr, bool setState);
  void BasePosition(bool bigStrikeReset);
  void EnterFidget(CStateManager& mgr, SamusGun::EFidgetType type, s32 parm2);
  void Update(float dt, CStateManager& mgr);
  void Draw(const CStateManager& mgr, const zeus::CTransform& xf) const;
  s32 GetFreeLookSetId() const { return x4c_gunController.GetFreeLookSetId(); }
  CGunController& GunController() { return x4c_gunController; }
  bool IsAnimPlaying() const { return xb8_24_animPlaying; }
};

} // namespace urde
