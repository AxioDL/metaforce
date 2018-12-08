#pragma once

#include "World/CPatterned.hpp"
#include "Character/CBoneTracking.hpp"
#include "Weapon/CProjectileInfo.hpp"

namespace urde::MP1 {
class CEyeball : public CPatterned {
  static constexpr std::string_view skEyeLocator = "Laser_LCTR"sv;
  float x568_attackDelay;
  float x56c_maxAttackDelay;
  CBoneTracking x570_boneTracking;
  zeus::CVector3f x5a8_targetPosition;
  CProjectileInfo x5b4_projectileInfo;
  CAssetId x5dc_;
  CAssetId x5e0_;
  CAssetId x5e4_;
  CAssetId x5e8_;
  TUniqueId x5ec_projectileId = kInvalidUniqueId;
  u32 x5f0_currentAnim = 0;
  s32 x5f4_animIdxs[4];
  u16 x604_beamSfxId;
  CSfxHandle x608_beamSfx = 0;
  bool x60c_24_canAttack : 1;
  bool x60c_25_playerInRange : 1;
  bool x60c_26_alert : 1;
  bool x60c_27_attackDisabled : 1;
  bool x60c_28_firingBeam : 1;

  void CreateBeam(CStateManager&);
  void FireBeam(CStateManager&, const zeus::CTransform&);
  void TryFlinch(CStateManager&, int);
  void UpdateAnimation();
  void ResetBeamState(CStateManager&);

public:
  DEFINE_PATTERNED(EyeBall)

  CEyeball(TUniqueId, std::string_view, EFlavorType, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
           const CPatternedInfo&, float, float, CAssetId, const CDamageInfo&, CAssetId, CAssetId, CAssetId, CAssetId,
           u32, u32, u32, u32, u32, bool, const CActorParameters&);

  void Accept(IVisitor& visitor);
  void PreRender(CStateManager&, const zeus::CFrustum&);
  void Touch(CActor&, CStateManager&){};
  void Death(CStateManager&, const zeus::CVector3f&, EScriptObjectState);

  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr);
  void DoUserAnimEvent(CStateManager&, const CInt32POINode&, EUserEventType, float);
  void Think(float, CStateManager&);
  void Flinch(CStateManager&, EStateMsg, float);
  void Active(CStateManager&, EStateMsg, float);
  void InActive(CStateManager&, EStateMsg, float);

  void Cover(CStateManager&, EStateMsg, float);

  bool ShouldAttack(CStateManager&, float) { return x60c_26_alert; }
  bool ShouldFire(CStateManager&, float) { return !x60c_27_attackDisabled; }
};
} // namespace urde::MP1