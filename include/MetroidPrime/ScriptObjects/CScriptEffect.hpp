#ifndef _CSCRIPTEFFECT
#define _CSCRIPTEFFECT

#include "MetroidPrime/CActor.hpp"

class CLightParameters;
class CElectricDescription;
class CParticleElectric;
class CGenDescription;
class CElementGen;
class CScriptEffect : public CActor {
public:
  ~CScriptEffect() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void SetActive(bool active) override;
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  bool CanRenderUnsorted(const CStateManager& mgr) const override;
  void CalculateRenderBounds() override;
  CAABox GetSortingBounds(const CStateManager& mgr) const override;

  CScriptEffect(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                const CTransform4f& xf, const CVector3f& scale, CAssetId partId, CAssetId elscId,
                bool hotInThermal, bool noTimerUnlessAreaOccluded, bool rebuildSystemsOnActivate,
                bool active, bool useRateInverseCamDist, float rateInverseCamDist,
                float rateInverseCamDistRate, float duration, float durationResetWhileVisible,
                bool useRateCamDistRange, float rateCamDistRangeMin, float rateCamDistRangeMax,
                float rateCamDistRangeFarRate, bool combatVisorVisible, bool thermalVisorVisible,
                bool xrayVisorVisible, const CLightParameters& lParms, bool dieWhenSystemsDone);

  static void ResetParticleCounts();

private:
  bool AreBothSystemsDeleteable() const;
  static uint mNumParticlesUpdating;
  static uint mNumParticlesDrawing;

  TLockedToken< CElectricDescription > mElectricToken;
  rstl::single_ptr< CParticleElectric > mElectric;
  TLockedToken< CGenDescription > mParticleSystemToken;
  rstl::single_ptr< CElementGen > mParticleSystem;
  TUniqueId mLightId;
  CAssetId mPartId;
  bool mEnable : 1;
  bool mNoTimerUnlessAreaOccluded : 1;
  bool mRebuildSystemsOnActivate : 1;
  bool mUseRateInverseCamDist : 1;
  bool mCombatVisorVisible : 1;
  bool mThermalVisorVisible : 1;
  bool mXrayVisorVisible : 1;
  bool mAnyVisorVisible : 1;
  bool mUseRateCamDistRange : 1;
  bool mDieWhenSystemsDone : 1;
  bool mCanRender : 1;
  float mRateInverseCamDist;
  float mRateInverseCamDistSq;
  float mRateInverseCamDistRate;
  float mRateCamDistRangeMin;
  float mRateCamDistRangeMax;
  float mRateCamDistRangeFarRate;
  mutable float mRemTime;
  float mDuration;
  float mDurationResetWhileVisible;
  rstl::single_ptr< CActorLights > mActorLights;
  TUniqueId mTriggerId;
  float mDestroyDelayTimer;
};

CHECK_SIZEOF(CScriptEffect, (VERSION >= VERSION_GM8E_02 ? 0x158 : 0x148))

#endif // _CSCRIPTEFFECT
