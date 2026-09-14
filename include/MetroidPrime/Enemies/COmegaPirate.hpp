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
  bool IsElitePirate() const override { return false; }
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
    TCachedToken< CTexture > xe8_thermalSpot;
    float xf4_delay;
    float xf8_time;
    float xfc_size;
  };

  struct SOBBoxJointInfo {
    const char* x0_from;
    const char* x4_to;
    float x8_boundsX;
    float xc_boundsY;
    float x10_boundsZ;
  };
  static const char* const skpGrenadeLauncher2LCTR;
  static const ::SSphereJointInfo skSphereJointList[1];
  static const SOBBoxJointInfo skOBBJointList[11];
  void CreateFlash(CStateManager& mgr, float delay);
  void KillOmegaPirate(CStateManager& mgr);
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

  TUniqueId x990_launcherId2;
  ENormalFadeState x994_normalFadeState;
  float x998_normalFadeTime;
  float x99c_normalAlpha;
  bool x9a0_visible;
  bool x9a1_fadeIn;
  rstl::vector< rstl::pair< TUniqueId, rstl::vector< TUniqueId > > > x9a4_scriptWaypointPlatforms;
  bool x9b4_lostAllHp;
  rstl::vector< rstl::pair< TUniqueId, rstl::string > > x9b8_scriptEffects;
  EScaleState x9c8_scaleState;
  float x9cc_scaleTime;
  CVector3f x9d0_initialScale;
  rstl::vector< rstl::pair< TUniqueId, rstl::string > > x9dc_scriptPlatforms;
  bool x9ec_decrement;
  CSkinnedModel x9f0_skeletonModel;
  float xa2c_skeletonAlpha;
  ESkeletonFadeState xa30_skeletonFadeState;
  float xa34_skeletonStateTime;
  rstl::single_ptr< CCollisionActorManager > xa38_collisionActorMgr1;
  bool xa3c_hearPlayer;
  pas::ELocomotionType xa40_locomotionType;
  bool xa44_targetable;
  TUniqueId xa46_;
  TUniqueId xa48_;
  bool xa4a_heartVisible;
  CTransform4f xa4c_initialXf;
  EXRayFadeState xa7c_xrayAlphaState;
  float xa80_xrayAlpha;
  float xa84_xrayAlphaStateTime;
  bool xa88_xrayFadeInTrigger;
  float xa8c_xrayFadeOutTime;
  float xa90_xrayFadeInTime;
  float xa94_xrayFadeTriggerTime;
  float xa98_maxEnergy;
  rstl::single_ptr< CCollisionActorManager > xa9c_collisionActorMgr2;
  rstl::vector< rstl::pair< TUniqueId, rstl::string > > xaa0_scriptSounds;
  float xab0_;
  rstl::vector< uint > xab4_;
  int xac4_;
  int xac8_;
  uint xacc_;
  bool xad0_scaleUpTrigger;
  float xad4_cachedSpeed;
  bool xad8_cover;
  TUniqueId xada_lastWaypointId;
  uchar xadc_;
  uchar xadd_;
  uchar xade_armorPiecesDestroyed;
  bool xadf_launcher1FollowPlayer;
  bool xae0_launcher2FollowPlayer;
  CDamageVulnerability xae4_platformVuln;
  uint xb4c_armorPiecesHealed;
  float xb50_armorPieceHealTime;
  CColor xb54_platformColor;
  float xb58_healTime;
  float xb5c_hpLost;
  float xb60_hpLostInPhase;
  float xb64_stateTime;
  uint xb68_;
  bool xb6c_exit1Sent;
  bool xb6d_exit2Sent;
  bool xb6e_armorPieceActivated;
  TToken< CTexture > xb70_thermalSpot;
  bool xb78_codeTrigger;
  uchar xb79_bossPhaseActive;
  rstl::vector< uchar > xb7c_;
  float xb8c_avoidStaticCollisionTime;
};
CHECK_SIZEOF(COmegaPirate, (VERSION >= VERSION_GM8P_00 ? 0xba0 : 0xB90))

#endif // _COMEGAPIRATE
