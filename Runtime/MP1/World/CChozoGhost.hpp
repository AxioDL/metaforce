#pragma once

#include <string_view>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/Character/CBoneTracking.hpp"
#include "Runtime/Weapon/CProjectileInfo.hpp"
#include "Runtime/World/CPatterned.hpp"

#include <zeus/CVector3f.hpp>

namespace urde::MP1 {
enum class EBehaveType { Lurk, Taunt, Attack, Move, None };

class CChozoGhost : public CPatterned {
public:
  class CBehaveChance {
    u32 x0_propertyCount;
    float x4_lurk;
    float x8_taunt;
    float xc_attack;
    float x10_move;
    float x14_lurkTime;
    float x18_chargeAttack;
    u32 x1c_numBolts;

  public:
    explicit CBehaveChance(CInputStream&);

    EBehaveType GetBehave(EBehaveType type, CStateManager& mgr) const;
    float GetLurk() const { return x4_lurk; }
    float GetTaunt() const { return x8_taunt; }
    float GetAttack() const { return xc_attack; }
    float GetMove() const { return x10_move; }
    float GetLurkTime() const { return x14_lurkTime; }
    float GetChargeAttack() const { return x18_chargeAttack; }
    u32 GetNumBolts() const { return x1c_numBolts; }
  };

private:
  float x568_hearingRadius;
  float x56c_fadeOutDelay;
  float x570_attackDelay;
  float x574_freezeTime;
  CProjectileInfo x578_;
  CProjectileInfo x5a0_;
  CBehaveChance x5c8_;
  CBehaveChance x5e8_;
  CBehaveChance x608_;
  s16 x628_soundImpact;
  float x62c_;
  s16 x630_sfxFadeIn;
  s16 x632_sfxFadeOut;
  float x634_;
  float x638_hurlRecoverTime;
  u32 x63c_;
  std::optional<TLockedToken<CGenDescription>> x640_projectileVisor;
  s16 x650_sound_ProjectileVisor;
  float x654_;
  float x658_;
  u32 x65c_nearChance;
  u32 x660_midChance;
  bool x664_24_behaviorEnabled : 1;
  bool x664_25_flinch : 1;
  bool x664_26_alert : 1 = false;
  bool x664_27_onGround : 1 = false;
  bool x664_28_ : 1 = false;
  bool x664_29_fadedIn : 1 = false;
  bool x664_30_fadedOut : 1 = false;
  bool x664_31_ : 1 = false;
  bool x665_24_ : 1 = true;
  bool x665_25_ : 1 = false;
  bool x665_26_shouldSwoosh : 1 = false;
  bool x665_27_playerInLeashRange : 1 = false;
  bool x665_28_inRange : 1 = false;
  bool x665_29_aggressive : 1 = false;
  float x668_ = 0.f;
  float x66c_ = 0.f;
  float x670_ = 0.f;
  TUniqueId x674_coverPoint = kInvalidUniqueId;
  float x678_floorLevel = 0.f;
  u32 x67c_attackType = -1;
  EBehaveType x680_behaveType = EBehaveType::Lurk;
  float x684_lurkDelay = 1.f;
  CSteeringBehaviors x688_;
  CBoneTracking x68c_boneTracking;
  TUniqueId x6c4_teamMgr = kInvalidUniqueId;
  float x6c8_spaceWarpTime = 0.f;
  zeus::CVector3f x6cc_spaceWarpPosition;
  u32 x6d8_ = 1;

  void AddToTeam(CStateManager& mgr);
  void RemoveFromTeam(CStateManager& mgr);
  void FloatToLevel(float f1, float dt);
  const CBehaveChance& ChooseBehaveChanceRange(CStateManager& mgr);
  bool IsVisibleEnough(const CStateManager& mgr) const { return GetModelAlphau8(mgr) > 31; }
  void FindSpaceWarpPosition(CStateManager& mgr, const zeus::CVector3f& dir);
  void FindBestAnchor(CStateManager& mgr);

public:
  DEFINE_PATTERNED(ChozoGhost)

  CChozoGhost(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
              const CActorParameters&, const CPatternedInfo&, float, float, float, float, CAssetId, const CDamageInfo&,
              CAssetId, const CDamageInfo&, const CChozoGhost::CBehaveChance&, const CChozoGhost::CBehaveChance&,
              const CBehaveChance&, u16, float, u16, u16, u32, float, u32, float, CAssetId, s16, float, float, u32,
              u32);

  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void Think(float dt, CStateManager&) override;
  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) override;
  void Render(CStateManager& mgr) override;
  void Touch(CActor& act, CStateManager& mgr) override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                                                         const CWeaponMode& mode,
                                                         EProjectileAttrib attrib) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) override;
  void KnockBack(const zeus::CVector3f& dir, CStateManager& mgr, const CDamageInfo& info, EKnockBackType type,
                 bool inDeferred, float magnitude) override;
  bool CanBeShot(const CStateManager& mgr, int w1) override;
  zeus::CVector3f GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                            const zeus::CVector3f& aimPos) const override {
    return x34_transform.origin;
  }
  void Dead(CStateManager& mgr, EStateMsg msg, float arg) override;
  void SelectTarget(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Run(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Deactivate(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Shuffle(CStateManager& mgr, EStateMsg msg, float arg) override;
  void InActive(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Taunt(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Hurled(CStateManager& mgr, EStateMsg msg, float arg) override;
  void WallDetach(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Growth(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Land(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Lurk(CStateManager& mgr, EStateMsg msg, float dt) override;
  bool Leash(CStateManager& mgr, float arg) override;
  bool InRange(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  bool AggressionCheck(CStateManager& mgr, float arg) override;
  bool ShouldTaunt(CStateManager& mgr, float arg) override;
  bool ShouldFlinch(CStateManager& mgr, float arg) override;
  bool ShouldMove(CStateManager& mgr, float arg) override;
  bool AIStage(CStateManager& mgr, float arg) override;
  u8 GetModelAlphau8(const CStateManager&) const override;
  bool IsOnGround() const override;
  float GetGravityConstant() const override { return 60.f; }
  CProjectileInfo* GetProjectileInfo() override;
};
} // namespace urde::MP1
