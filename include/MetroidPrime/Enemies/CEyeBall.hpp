#ifndef _CEYEBALL
#define _CEYEBALL

#include "MetroidPrime/CBoneTracking.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

class CEyeBall : public CPatterned {
public:
  CEyeBall(const TUniqueId uid, const rstl::string& name, const EFlavorType flavor,
           const CEntityInfo& info, const CTransform4f& xf, const CModelData& mData,
           const CPatternedInfo& pInfo, const float attackDelay, const float attackStartTime,
           const CAssetId wpscId, const CDamageInfo& dInfo, const CAssetId beamContactFxId,
           const CAssetId beamPulseFxId, const CAssetId beamTextureId,
           const CAssetId beamGlowTextureId, const uint anim0, const uint anim1, const uint anim2,
           const uint anim3, const uint beamSfx, bool attackDisabled,
           const CActorParameters& actParams);

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType event,
                       float dt) override;

  void InActive(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Active(CStateManager& mgr, EStateMsg msg, float arg) override;
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;
  void Think(float dt, CStateManager& mgr) override;
  void Render(const CStateManager& mgr) const override;
  void Flinch(CStateManager&, EStateMsg, float) override;
  void Cover(CStateManager& mgr, EStateMsg msg, float arg) override;
  bool ShouldFire(CStateManager&, float) override;
  bool ShouldAttack(CStateManager&, float) override;

  void Touch(CActor&, CStateManager&) override;
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;

  // void PreRenderBoneTracking(CStateManager& mgr, const CVector3f& scale,
  //                            const CBodyController& ctrl) {
  //   mBoneTracking.PreRender(mgr, *AnimationData(), GetTransform(), scale, ctrl);
  // }

private:
  void CreateBeam(CStateManager& mgr);
  void TryFlinch(CStateManager& mgr, int arg);
  void TurnLaserOff(CStateManager& mgr);
  void TurnLaserOn(CStateManager& mgr, const CTransform4f& xf);
  void UpdateCycleAnimation(float dt);

  float GetAttackDelay() const { return mAttackDelay; }
  float GetAttackStartTime() const { return mAttackStartTime; }

  float mAttackDelay;
  float mAttackStartTime;
  CBoneTracking mBoneTracking;
  CVector3f mTargetPosition;
  CProjectileInfo mProjectileInfo;
  CAssetId mBeamContactFxId;
  CAssetId mBeamPulseFxId;
  CAssetId mBeamTextureId;
  CAssetId mBeamGlowTextureId;
  TUniqueId mProjectileId;
  int mCurrentAnim;
  int mAnimIndices[4];
  ushort mBeamSfxId;
  CSfxHandle mBeamSfx;
  bool mCanAttack : 1;
  bool mPlayerInRange : 1;
  bool mAlert : 1;
  bool mAttackDisabled : 1;
  bool mFiringBeam : 1;

  static const char* skEyeLocator;
#define skMaxRadianAngle CMath::Deg2Rad(45.f)
};

#endif // _CEYEBALL
