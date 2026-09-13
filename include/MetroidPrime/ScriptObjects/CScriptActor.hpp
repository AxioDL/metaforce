#ifndef _CSCRIPTACTOR
#define _CSCRIPTACTOR

#include "types.h"

#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"

class CActorParameters;

class CScriptActor : public CPhysicsActor {
public:
  CScriptActor(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
               const CTransform4f& xf, const CModelData& mData, const CAABox& aabb,
               const CMaterialList& matList, float mass, float zMomentum, const CHealthInfo& hInfo,
               const CDamageVulnerability& dVuln, const CActorParameters& actParms, bool looping,
               bool active, uint shaderIdx, float xrayAlpha, const bool noThermalHotZ,
               const bool castsShadow, const bool scaleAdvancementDelta,
               const bool materialFlag54);

  // CEntity
  ~CScriptActor() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void PreRender(CStateManager&, const CFrustumPlanes&) override;
  CHealthInfo* HealthInfo(CStateManager&) override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor&, CStateManager&) override;

  // CPhysicsActor
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                         const CWeaponMode&,
                                                         int) const override;
  CAABox GetSortingBounds(const CStateManager&) const override;

  bool IsPlayerActor() const { return x2e3_24_isPlayerActor; }

protected:
  CHealthInfo x258_initialHealth;
  CHealthInfo x260_currentHealth;
  CDamageVulnerability x268_damageVulnerability;
  float x2d0_fadeInTime;
  float x2d4_fadeOutTime;
  int x2d8_shaderIdx;
  float x2dc_xrayAlpha;
  TUniqueId x2e0_triggerId;
  bool x2e2_24_noThermalHotZ : 1;
  bool x2e2_25_dead : 1;
  bool x2e2_26_animating : 1;
  bool x2e2_27_xrayAlphaEnabled : 1;
  bool x2e2_28_inXrayAlpha : 1;
  bool x2e2_29_processModelFlags : 1;
  bool x2e2_30_scaleAdvancementDelta : 1;
  bool x2e2_31_materialFlag54 : 1;
  bool x2e3_24_isPlayerActor : 1;
};

#endif // _CSCRIPTACTOR
