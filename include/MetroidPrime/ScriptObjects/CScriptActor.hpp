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

  bool IsPlayerActor() const { return mIsPlayerActor; }

protected:
  CHealthInfo mInitialHealth;
  CHealthInfo mCurrentHealth;
  CDamageVulnerability mDamageVulnerability;
  float mFadeInTime;
  float mFadeOutTime;
  int mShaderIdx;
  float mXrayAlpha;
  TUniqueId mTriggerId;
  bool mNoThermalHotZ : 1;
  bool mDead : 1;
  bool mAnimating : 1;
  bool mXrayAlphaEnabled : 1;
  bool mInXrayAlpha : 1;
  bool mProcessModelFlags : 1;
  bool mScaleAdvancementDelta : 1;
  bool mMaterialFlag54 : 1;
  bool mIsPlayerActor : 1;
};

#endif // _CSCRIPTACTOR
