#pragma once

#include <array>
#include <string_view>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Character/CBoneTracking.hpp"
#include "Runtime/Weapon/CProjectileInfo.hpp"
#include "Runtime/World/CPatterned.hpp"

#include <zeus/CVector3f.hpp>

namespace urde::MP1 {
class CEyeball : public CPatterned {
  float x568_attackDelay;
  float x56c_attackStartTime;
  CBoneTracking x570_boneTracking;
  zeus::CVector3f x5a8_targetPosition;
  CProjectileInfo x5b4_projectileInfo;
  CAssetId x5dc_beamContactFxId;
  CAssetId x5e0_beamPulseFxId;
  CAssetId x5e4_beamTextureId;
  CAssetId x5e8_beamGlowTextureId;
  TUniqueId x5ec_projectileId = kInvalidUniqueId;
  u32 x5f0_currentAnim = 0;
  std::array<s32, 4> x5f4_animIdxs;
  u16 x604_beamSfxId;
  CSfxHandle x608_beamSfx;
  bool x60c_24_canAttack : 1 = false;
  bool x60c_25_playerInRange : 1 = false;
  bool x60c_26_alert : 1 = false;
  bool x60c_27_attackDisabled : 1;
  bool x60c_28_firingBeam : 1 = false;

  void CreateBeam(CStateManager&);
  void FireBeam(CStateManager&, const zeus::CTransform&);
  void TryFlinch(CStateManager&, int);
  void UpdateAnimation();
  void ResetBeamState(CStateManager&);

public:
  DEFINE_PATTERNED(EyeBall)

  CEyeball(TUniqueId uid, std::string_view name, CPatterned::EFlavorType flavor, const CEntityInfo& info,
           const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo, float attackDelay,
           float attackStartTime, CAssetId wpscId, const CDamageInfo& dInfo, CAssetId beamContactFxId,
           CAssetId beamPulseFxId, CAssetId beamTextureId, CAssetId beamGlowTextureId, u32 anim0, u32 anim1, u32 anim2,
           u32 anim3, u32 beamSfx, bool attackDisabled, const CActorParameters& actParms);

  void Accept(IVisitor& visitor) override;
  void PreRender(CStateManager&, const zeus::CFrustum&) override;
  void Touch(CActor&, CStateManager&) override {}
  void Death(CStateManager&, const zeus::CVector3f&, EScriptObjectState) override;

  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void DoUserAnimEvent(CStateManager&, const CInt32POINode&, EUserEventType, float) override;
  void Think(float, CStateManager&) override;
  void Flinch(CStateManager&, EStateMsg, float) override;
  void Active(CStateManager&, EStateMsg, float) override;
  void InActive(CStateManager&, EStateMsg, float) override;

  void Cover(CStateManager&, EStateMsg, float) override;

  bool ShouldAttack(CStateManager&, float) override { return x60c_26_alert; }
  bool ShouldFire(CStateManager&, float) override { return !x60c_27_attackDisabled; }
};
} // namespace urde::MP1