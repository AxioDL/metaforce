#ifndef _CNEWINTROBOSS
#define _CNEWINTROBOSS

#include "MetroidPrime/Enemies/CPatterned.hpp"

#include "Kyoto/Animation/CharacterCommon.hpp"
#include "MetroidPrime/CBoneTracking.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

class CCollisionActorManager;
class CPatternedInfo;

class CNewIntroBoss : public CPatterned {
public:
  CNewIntroBoss(TUniqueId, const rstl::string&, const CEntityInfo& info, const CTransform4f& xf,
                const CModelData& mData, const CPatternedInfo& pInfo,
                const CActorParameters& actParms, float minTurnAngle, CAssetId projectile,
                const CDamageInfo& dInfo, CAssetId beamContactFxId, CAssetId beamPulseFxId,
                CAssetId beamTextureId, CAssetId beamGlowTextureId);

  // CEntity
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void OnScanStateChange(EScanState, CStateManager&) override;
  CAABox GetSortingBounds(const CStateManager&) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CAi
  void Patrol(CStateManager&, EStateMsg, float) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Attack(CStateManager&, EStateMsg, float) override;
  bool InAttackPosition(CStateManager&, float) override;
  bool AnimOver(CStateManager&, float) override;
  bool ShouldAttack(CStateManager&, float) override;
  bool ShouldTurn(CStateManager&, float) override;
  bool AIStage(CStateManager&, float) override;

  // CPatterned
  CProjectileInfo* ProjectileInfo() override;

  pas::ELocomotionType GetLocoForHealth(const CStateManager& mgr) const;
  pas::EGenerateType GetGenerateForHealth(const CStateManager& mgr) const;
  float GetNextAttackTime(CStateManager& mgr) const;
  CVector3f PlayerPos(const CStateManager& mgr) const;
  void DeleteBeam(CStateManager& mgr);
  void StopRumble(CStateManager& mgr);

  float GetInitialHP() const { return mInitialHp; }

private:
  pas::ELocomotionType mLocomotion;
  int mStateProg;
  float mMinTurnAngle;
  CBoneTracking mBoneTracking;
  CProjectileInfo mProjectileInfo;
  TUniqueId mStage1Projectile;
  TUniqueId mStage2Projectile;
  TUniqueId mStage3Projectile;
  rstl::string mDamageLocator;
  rstl::single_ptr< CCollisionActorManager > mCollisionManager;
  CAssetId mBeamContactFxId;
  CAssetId mBeamPulseFxId;
  CAssetId mBeamTextureId;
  CAssetId mBeamGlowTextureId;
  TUniqueId mHeadActor;
  TUniqueId mPelvisActor;
  CVector3f mPredictedPlayerPos;
  CVector3f mLookPos;
  CVector3f mStartPlayerPos;
  float mFiringTime;
  CVector3f mTargetPos;
  float x638_;
  float mAttackTime;
  float mInitialHp;
  CTransform4f mInitialXf;
  short mRumbleVoice;
  TUniqueId mCurProjectile;
  bool x678_;
};

CHECK_SIZEOF(CNewIntroBoss, (VERSION >= VERSION_GM8E_02 ? 0x690 : 0x680))

#endif // _CNEWINTROBOSS
