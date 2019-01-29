#pragma once

#include "CPhysicsActor.hpp"
#include "CHealthInfo.hpp"
#include "CDamageVulnerability.hpp"

namespace urde {

class CScriptActor : public CPhysicsActor {
protected:
  CHealthInfo x258_initialHealth;
  CHealthInfo x260_currentHealth;
  CDamageVulnerability x268_damageVulnerability;
  float x2d0_fadeInTime;
  float x2d4_fadeOutTime;
  s32 x2d8_shaderIdx;
  float x2dc_xrayAlpha;
  TUniqueId x2e0_triggerId = kInvalidUniqueId;
  bool x2e2_24_noThermalHotZ : 1;
  bool x2e2_25_dead : 1;
  bool x2e2_26_animating : 1;
  bool x2e2_27_xrayAlphaEnabled : 1;
  bool x2e2_28_inXrayAlpha : 1;
  bool x2e2_29_processModelFlags : 1;
  bool x2e2_30_scaleAdvancementDelta : 1;
  bool x2e2_31_materialFlag54 : 1;
  bool x2e3_24_isPlayerActor : 1;

public:
  CScriptActor(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
               CModelData&& mData, const zeus::CAABox& aabb, float mass, float zMomentum, const CMaterialList& matList,
               const CHealthInfo& hInfo, const CDamageVulnerability& dVuln, const CActorParameters& actParms,
               bool looping, bool active, s32 shaderIdx, float xrayAlpha, bool noThermalHotZ, bool castsShadow,
               bool scaleAdvancementDelta, bool materialFlag54);
  void Accept(IVisitor& visitor);
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
  void Think(float, CStateManager&);
  void PreRender(CStateManager&, const zeus::CFrustum&);
  zeus::CAABox GetSortingBounds(const CStateManager&) const;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                         const CWeaponMode&, EProjectileAttrib) const;
  rstl::optional<zeus::CAABox> GetTouchBounds() const;
  void Touch(CActor&, CStateManager&);
  const CDamageVulnerability* GetDamageVulnerability() const { return &x268_damageVulnerability; }
  CHealthInfo* HealthInfo(CStateManager&) { return &x260_currentHealth; }
  bool IsPlayerActor() const { return x2e3_24_isPlayerActor; }
};
}; // namespace urde
