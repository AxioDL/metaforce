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
    float GetLurk() const { return x4_lurk; }
    float GetTaunt() const { return x8_taunt; }
    float GetAttack() const { return xc_attack; }
    float GetMove() const { return x10_move; }
    float GetLurkTime() const { return x14_lurkTime; }
    float GetChargeAttack() const { return x18_chargeAttack; }
    uint GetNumBolts() const { return x1c_numBolts; }

  private:
    int x0_propertyCount;
    float x4_lurk;
    float x8_taunt;
    float xc_attack;
    float x10_move;
    float x14_lurkTime;
    float x18_chargeAttack;
    uint x1c_numBolts;
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

  float x568_hearingRadius;
  float x56c_fadeOutDelay;
  float x570_attackDelay;
  float x574_freezeTime;
  CProjectileInfo x578_projectileInfo1;
  CProjectileInfo x5a0_projectileInfo2;
  CBehaveChance x5c8_behaveChance1;
  CBehaveChance x5e8_behaveChance2;
  CBehaveChance x608_behaveChance3;
  ushort x628_soundImpact;
  float x62c_;
  ushort x630_sfxFadeIn;
  ushort x632_sfxFadeOut;
  float x634_;
  float x638_hurlRecoverTime;
  int x63c_;
  rstl::optional_object< TLockedToken< CGenDescription > > x640_projectileVisor;
  ushort x650_soundProjectileVisor;
  float x654_;
  float x658_;
  int x65c_nearChance;
  int x660_midChance;
  bool x664_24_behaviorEnabled : 1;
  bool x664_25_flinch : 1;
  bool x664_26_alert : 1;
  bool x664_27_onGround : 1;
  bool x664_28_ : 1;
  bool x664_29_fadedIn : 1;
  bool x664_30_fadedOut : 1;
  bool x664_31_ : 1;
  bool x665_24_ : 1;
  bool x665_25_ : 1;
  bool x665_26_shouldSwoosh : 1;
  bool x665_27_playerInLeashRange : 1;
  bool x665_28_inRange : 1;
  bool x665_29_aggressive : 1;
  float x668_;
  float x66c_;
  float x670_;
  TUniqueId x674_coverPoint;
  float x678_floorLevel;
  int x67c_attackType;
  EBehaveType x680_behaveType;
  float x684_lurkDelay;
  CSteeringBehaviors x688_steeringBehaviors;
  CBoneTracking x68c_boneTracking;
  TUniqueId x6c4_teamMgr;
  float x6c8_spaceWarpTime;
  CVector3f x6cc_spaceWarpPosition;
  int x6d8_;

  static const rstl::string skSpeedSwooshName;
};
CHECK_SIZEOF(CChozoGhost, 0x6E0)

#endif // _CCHOZOGHOST
