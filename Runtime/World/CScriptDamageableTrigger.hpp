#pragma once

#include "CActor.hpp"
#include "CFluidPlaneDoor.hpp"
#include "CHealthInfo.hpp"
#include "CDamageVulnerability.hpp"

namespace urde {
class CVisorParameters;
class CScriptDamageableTrigger : public CActor {
public:
  enum class ECanOrbit {
    NoOrbit,
    Orbit,
  };

  zeus::CFrustum xe8_frustum;
  zeus::CAABox x14c_bounds;
  CHealthInfo x164_origHInfo;
  CHealthInfo x16c_hInfo;
  CDamageVulnerability x174_dVuln;
  u32 x1dc_faceFlag;
  float x1e0_alpha = 1.f;
  zeus::CTransform x1e4_faceDir;
  zeus::CTransform x214_faceDirInv;
  zeus::CVector3f x244_faceTranslate;
  float x250_alphaTimer = 0.f;
  CFluidPlaneDoor x254_fluidPlane;
  union {
    struct {
      bool x300_24_notOccluded : 1;
      bool x300_25_alphaOut : 1;
      bool x300_26_outOfFrustum : 1;
      bool x300_27_invulnerable : 1;
      bool x300_28_canOrbit : 1;
    };
    u32 _dummy = 0;
  };
  void SetLinkedObjectAlpha(float a, CStateManager& mgr);
  float GetPuddleAlphaScale() const;

public:
  CScriptDamageableTrigger(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                           const zeus::CVector3f& position, const zeus::CVector3f& extent, const CHealthInfo& hInfo,
                           const CDamageVulnerability& dVuln, u32 faceFlag, CAssetId patternTex1, CAssetId patternTex2,
                           CAssetId colorTex, ECanOrbit canOrbit, bool active, const CVisorParameters& vParams);

  void Accept(IVisitor& visitor);
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
  EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                         const CWeaponMode&, EProjectileAttrib) const;
  void Render(const CStateManager& mgr) const;
  void AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr) const;
  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum);
  const CDamageVulnerability* GetDamageVulnerability() const { return &x174_dVuln; }
  CHealthInfo* HealthInfo(CStateManager&) { return &x16c_hInfo; }
  void Think(float, CStateManager&);
  rstl::optional<zeus::CAABox> GetTouchBounds() const;
};
} // namespace urde
