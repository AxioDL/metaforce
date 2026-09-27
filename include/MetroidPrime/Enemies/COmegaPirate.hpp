#ifndef _COMEGAPIRATE
#define _COMEGAPIRATE

#include "types.h"

#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/TToken.hpp"

#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/Collision/CJointCollisionDescription.hpp"
#include "MetroidPrime/Enemies/CElitePirate.hpp"

#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CCollisionActorManager;
class CTexture;

class COmegaPirate : public CElitePirate {
public:
  COmegaPirate(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
               const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
               const CActorParameters& actParms, const CElitePirateData& data, int skeletonModelId,
               int skeletonSkinRulesId, int skeletonLayoutInfoId);
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;
  void Render(const CStateManager&) const override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Run(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void JumpBack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void DoubleSnap(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Shuffle(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Skid(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Suck(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Explode(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Retreat(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Cover(CStateManager& mgr, EStateMsg msg, float dt) override;
  void WallHang(CStateManager& mgr, EStateMsg msg, float dt) override;
  void WallDetach(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Enraged(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Growth(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Faint(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Dizzy(CStateManager& mgr, EStateMsg msg, float dt) override;
  bool Landed(CStateManager& mgr, float arg) override;
  bool HearPlayer(CStateManager& mgr, float arg) override;
  bool CoverBlown(CStateManager& mgr, float arg) override;
  bool AggressionCheck(CStateManager& mgr, float arg) override;
  bool ShouldFire(CStateManager& mgr, float arg) override;
  bool ShouldMove(CStateManager& mgr, float arg) override;
  bool ShotAt(CStateManager& mgr, float arg) override;
  bool CodeTrigger(CStateManager& mgr, float arg) override;
  bool ShouldCallForBackup(CStateManager& mgr, float arg) override;
  bool IsUsingBaseCollisionActors() const override { return false; }
  void SetupHealthInfo(CStateManager& mgr) override;
  void ActivateGrenadeLauncher(CStateManager& mgr, bool val) override;
  CShockWaveInfo GetShockWaveInfo() const override;


private:
  class CFlash : public CActor {
  public:
    DECLARE_TYPES_MATCH_OR_ACCEPT;
    void Think(float dt, CStateManager& mgr) override;
    void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;
    void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
    void Render(const CStateManager& mgr) const override;
    CFlash(TUniqueId uid, const CEntityInfo& info, const CVector3f& pos,
           const TToken< CTexture >& thermalSpot, float delay);

  private:
    TCachedToken< CTexture > mThermalSpot;
    float mDelay;
    float mTime;
    float mSize;
  };

  struct SOBBoxJointInfo {
    const char* mFrom;
    const char* mTo;
    float mBoundsX;
    float mBoundsY;
    float mBoundsZ;
  };
  static const char* const skpGrenadeLauncher2LCTR;
  static const ::SSphereJointInfo skSphereJointList[1];
  static const SOBBoxJointInfo skOBBJointList[11];
  void CreateFlash(CStateManager& mgr, float delay);
  void KillOmegaPirate(CStateManager& mgr);
  bool IsElitePirate() const override;
  void InitializeOmegaPirateCollisionManagers(CStateManager& mgr);
  void UpdateVeinsModel(CStateManager& mgr, float dt);
  void UpdateTeleportEffect(CStateManager& mgr, float dt);
  void TeleportToFurthestPlatform(CStateManager& mgr);
  static CVector3f FindGround(const CVector3f& pos, CStateManager& mgr);
  void UpdateInvisibility(CStateManager& mgr, float dt);
  void UpdateVeinsModelGlowEffect(CStateManager& mgr, float dt);
  void SetOmegaPirateCrystalCollisionMaterialProperties(
      rstl::single_ptr< CCollisionActorManager >& actors, CStateManager& mgr) const;
  void
  SetOmegaPirateOBBCollisionMaterialProperties(rstl::single_ptr< CCollisionActorManager >& actors,
                                               CStateManager& mgr) const;
  void SpawnNextQueuedTrooperPirate(CStateManager& mgr, float dt);
  void QueueTrooperPiratesOfActiveType(uint count, CStateManager& mgr);
  void QueueTrooperPiratesOfOneRandomColor(uint count, CStateManager& mgr);
  uint GetNumActiveTrooperPirates() const;
  uint GetNumTypesOfActiveAndQueuedTrooperPirates() const;
  void AddOmegaPirateSphereCollisionList(const ::SSphereJointInfo* joints, int count,
                                         rstl::vector< CJointCollisionDescription >& list) const;
  void AddOBBCollisionList(const SOBBoxJointInfo* joints, int count,
                           rstl::vector< CJointCollisionDescription >& list) const;

  enum ENormalFadeState {
    kNFS_Zero,
    kNFS_One,
    kNFS_Two,
    kNFS_Three,
  };

  enum EScaleState {
    kSS_None,
    kSS_ScaleDownX,
    kSS_ScaleDownY,
    kSS_ScaleDownZ,
    kSS_WaitForTrigger,
    kSS_ScaleUpX,
    kSS_ScaleUpY,
    kSS_ScaleUpZ,
  };

  enum ESkeletonFadeState {
    kSFS_None,
    kSFS_FadeOut,
    kSFS_Flash,
    kSFS_FadeIn,
  };

  enum EXRayFadeState {
    kXFS_None,
    kXFS_FadeIn,
    kXFS_WaitForTrigger,
    kXFS_FadeOut,
  };

  TUniqueId mLauncherId2;
  ENormalFadeState mNormalFadeState;
  float mNormalFadeTime;
  float mNormalAlpha;
  bool mVisible;
  bool mFadeIn;
  rstl::vector< rstl::pair< TUniqueId, rstl::vector< TUniqueId > > > mScriptWaypointPlatforms;
  bool mLostAllHp;
  rstl::vector< rstl::pair< TUniqueId, rstl::string > > mScriptEffects;
  EScaleState mScaleState;
  float mScaleTime;
  CVector3f mInitialScale;
  rstl::vector< rstl::pair< TUniqueId, rstl::string > > mScriptPlatforms;
  bool mDecrement;
  CSkinnedModel mSkeletonModel;
  float mSkeletonAlpha;
  ESkeletonFadeState mSkeletonFadeState;
  float mSkeletonStateTime;
  rstl::single_ptr< CCollisionActorManager > mCollisionActorMgr1;
  bool mHearPlayer;
  pas::ELocomotionType mLocomotionType;
  bool mTargetable;
  TUniqueId xa46_;
  TUniqueId xa48_;
  bool mHeartVisible;
  CTransform4f mInitialXf;
  EXRayFadeState mXrayAlphaState;
  float mXrayAlpha;
  float mXrayAlphaStateTime;
  bool mXrayFadeInTrigger;
  float mXrayFadeOutTime;
  float mXrayFadeInTime;
  float mXrayFadeTriggerTime;
  float mMaxEnergy;
  rstl::single_ptr< CCollisionActorManager > mCollisionActorMgr2;
  rstl::vector< rstl::pair< TUniqueId, rstl::string > > mScriptSounds;
  float xab0_;
  rstl::vector< uint > xab4_;
  int xac4_;
  int xac8_;
  uint xacc_;
  bool mScaleUpTrigger;
  float mCachedSpeed;
  bool mCover;
  TUniqueId mLastWaypointId;
  uchar xadc_;
  uchar xadd_;
  uchar mArmorPiecesDestroyed;
  bool mLauncher1FollowPlayer;
  bool mLauncher2FollowPlayer;
  CDamageVulnerability mPlatformVuln;
  uint mArmorPiecesHealed;
  float mArmorPieceHealTime;
  CColor mPlatformColor;
  float mHealTime;
  float mHpLost;
  float mHpLostInPhase;
  float mStateTime;
  uint xb68_;
  bool mExit1Sent;
  bool mExit2Sent;
  bool mArmorPieceActivated;
  TToken< CTexture > mThermalSpot;
  bool mCodeTrigger;
  uchar mBossPhaseActive;
  rstl::vector< uchar > xb7c_;
  float mAvoidStaticCollisionTime;
};
CHECK_SIZEOF(COmegaPirate, (VERSION >= VERSION_GM8E_02 ? 0xba0 : 0xB90))

#endif // _COMEGAPIRATE
