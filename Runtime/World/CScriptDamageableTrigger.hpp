#pragma once

#include <string_view>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/World/CActor.hpp"
#include "Runtime/World/CDamageVulnerability.hpp"
#include "Runtime/World/CFluidPlaneDoor.hpp"
#include "Runtime/World/CHealthInfo.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CFrustum.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CVisorParameters;

class CScriptDamageableTrigger : public CActor {
private:
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
  bool x300_24_notOccluded : 1 = false;
  bool x300_25_alphaOut : 1 = false;
  bool x300_26_outOfFrustum : 1 = false;
  bool x300_27_invulnerable : 1 = false;
  bool x300_28_canOrbit : 1;

  void SetLinkedObjectAlpha(float a, CStateManager& mgr);
  float GetPuddleAlphaScale() const;

public:
  enum class ECanOrbit {
    NoOrbit,
    Orbit,
  };

  CScriptDamageableTrigger(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                           const zeus::CVector3f& position, const zeus::CVector3f& extent, const CHealthInfo& hInfo,
                           const CDamageVulnerability& dVuln, u32 faceFlag, CAssetId patternTex1, CAssetId patternTex2,
                           CAssetId colorTex, ECanOrbit canOrbit, bool active, const CVisorParameters& vParams);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                         const CWeaponMode&, EProjectileAttrib) const override;
  void Render(CStateManager& mgr) override;
  void AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) override;
  const CDamageVulnerability* GetDamageVulnerability() const override { return &x174_dVuln; }
  CHealthInfo* HealthInfo(CStateManager&) override { return &x16c_hInfo; }
  void Think(float, CStateManager&) override;
  std::optional<zeus::CAABox> GetTouchBounds() const override;
};
} // namespace urde
