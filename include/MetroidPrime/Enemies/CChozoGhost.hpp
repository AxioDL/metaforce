#ifndef _CCHOZOGHOST
#define _CCHOZOGHOST

#include "types.h"

#include "MetroidPrime/CBoneTracking.hpp"
#include "MetroidPrime/CSteeringBehaviors.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

#include "Kyoto/TToken.hpp"

#include "rstl/optional_object.hpp"

class CGenDescription;

class CChozoGhost : public CPatterned {
public:
  enum EBehaveType {
    kBT_Lurk,
    kBT_Taunt,
    kBT_Attack,
    kBT_Move,
    kBT_None,
  };

  class CBehaveChance {
  public:
    explicit CBehaveChance(CInputStream& in);

    EBehaveType GetBehave(EBehaveType type, CStateManager& mgr) const;
    float GetLurk() const { return mLurk; }
    float GetTaunt() const { return mTaunt; }
    float GetAttack() const { return mAttack; }
    float GetMove() const { return mMove; }
    float GetLurkTime() const { return mLurkTime; }
    float GetChargeAttack() const { return mChargeAttack; }
    uint GetNumBolts() const { return mNumBolts; }

  private:
    int mPropertyCount;
    float mLurk;
    float mTaunt;
    float mAttack;
    float mMove;
    float mLurkTime;
    float mChargeAttack;
    uint mNumBolts;
  };

  CChozoGhost(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
              const CTransform4f& xf, const CModelData& mData, const CActorParameters& actParms,
              const CPatternedInfo& pInfo, float hearingRadius, float fadeOutDelay,
              float attackDelay, float freezeTime, CAssetId wpsc1, const CDamageInfo& dInfo1,
              CAssetId wpsc2, const CDamageInfo& dInfo2, const CBehaveChance& chance1,
              const CBehaveChance& chance2, const CBehaveChance& chance3, ushort soundImpact,
              float f1, ushort sfxFadeIn, ushort sfxFadeOut, uint w1, float f2, uint w2,
              float hurlRecoverTime, CAssetId projectileVisor, ushort soundProjectileVisor,
              float f3, float f4, uint nearChance, uint midChance);

  // CEntity
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;
  void Render(const CStateManager& mgr) const override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  void Touch(CActor& act, CStateManager& mgr) override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                         const CWeaponMode&, int) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CPhysicsActor

  // CAi
  CVector3f GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                      const CVector3f& aimPos) const override;

  // CPatterned
  void KnockBack(const CVector3f&, CStateManager&, const CDamageInfo& info, float magnitude,
                 bool direct, const bool inDeferred) override;
  bool CanBeShot(const CStateManager& mgr, int w1) override;
  uchar GetModelAlphau8(const CStateManager& mgr) const override;
  bool IsOnGround() const override;
  float GetGravityConstant() const override { return 60.f; }
  CProjectileInfo* ProjectileInfo() override;

  // State functions
  void Dead(CStateManager& mgr, EStateMsg msg, float arg) override;
  void SelectTarget(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Run(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Deactivate(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Shuffle(CStateManager& mgr, EStateMsg msg, float arg) override;
  void InActive(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Taunt(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Hurled(CStateManager& mgr, EStateMsg msg, float arg) override;
  void WallDetach(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Growth(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Land(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Lurk(CStateManager& mgr, EStateMsg msg, float arg) override;

  // Transition functions
  bool Leash(CStateManager& mgr, float arg) override;
  bool InRange(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  bool AggressionCheck(CStateManager& mgr, float arg) override;
  bool ShouldTaunt(CStateManager& mgr, float arg) override;
  bool ShouldFlinch(CStateManager& mgr, float arg) override;
  bool ShouldMove(CStateManager& mgr, float arg) override;
  bool AIStage(CStateManager& mgr, float arg) override;

private:
  void AddToTeam(CStateManager& mgr);
  void RemoveFromTeam(CStateManager& mgr);
  void FloatToLevel(float f1, float dt);
  const CBehaveChance& ChooseBehaveChanceRange(CStateManager& mgr) const;
  bool IsVisibleEnough(const CStateManager& mgr) const; // { return GetModelAlphau8(mgr) > 31; }
  void SetWarpPosition(CStateManager& mgr, const CVector3f& dir);
  void FindBestAnchor(CStateManager& mgr);

  float mHearingRadius;
  float mFadeOutDelay;
  float mAttackDelay;
  float mFreezeTime;
  CProjectileInfo mProjectileInfo1;
  CProjectileInfo mProjectileInfo2;
  CBehaveChance mBehaveChance1;
  CBehaveChance mBehaveChance2;
  CBehaveChance mBehaveChance3;
  ushort mSoundImpact;
  float x62c_;
  ushort mSfxFadeIn;
  ushort mSfxFadeOut;
  float x634_;
  float mHurlRecoverTime;
  int x63c_;
  rstl::optional_object< TLockedToken< CGenDescription > > mProjectileVisor;
  ushort mSoundProjectileVisor;
  float x654_;
  float x658_;
  int mNearChance;
  int mMidChance;
  bool mBehaviorEnabled : 1;
  bool mFlinch : 1;
  bool mAlert : 1;
  bool mOnGround : 1;
  bool x664_28_ : 1;
  bool mFadedIn : 1;
  bool mFadedOut : 1;
  bool x664_31_ : 1;
  bool x665_24_ : 1;
  bool x665_25_ : 1;
  bool mShouldSwoosh : 1;
  bool mPlayerInLeashRange : 1;
  bool mInRange : 1;
  bool mAggressive : 1;
  float x668_;
  float x66c_;
  float x670_;
  TUniqueId mCoverPoint;
  float mFloorLevel;
  int mAttackType;
  EBehaveType mBehaveType;
  float mLurkDelay;
  CSteeringBehaviors mSteeringBehaviors;
  CBoneTracking mBoneTracking;
  TUniqueId mTeamMgr;
  float mSpaceWarpTime;
  CVector3f mSpaceWarpPosition;
  int x6d8_;

  static const rstl::string skSpeedSwooshName;
};
CHECK_SIZEOF(CChozoGhost, (VERSION >= VERSION_GM8E_02 ? 0x6f0 : 0x6E0))

#endif // _CCHOZOGHOST
