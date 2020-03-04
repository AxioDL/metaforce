#pragma once

#include <string_view>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/Character/CBoneTracking.hpp"
#include "Runtime/Weapon/CProjectileInfo.hpp"
#include "Runtime/World/CPatterned.hpp"

#include <zeus/CVector3f.hpp>

namespace urde::MP1 {
enum class EBehaveType {
  Zero,
  One,
  Two,
  Three
};

class CChozoGhost : public CPatterned {
public:
  class CBehaveChance {
    u32 x0_propertyCount;
    float x4_lurk;
    float x8_;
    float xc_attack;
    float x10_move;
    float x14_lurkTime;
    float x18_chargeAttack;
    u32 x1c_numBolts;

  public:
    CBehaveChance(CInputStream&);

    u32 GetBehave(EBehaveType type, CStateManager& mgr) const;
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
  s16 x630_;
  s16 x632_;
  float x634_;
  float x638_hurlRecoverTime;
  u32 x63c_;
  TLockedToken<CGenDescription> x640_;
  s16 x650_sound_ProjectileVisor;
  float x654_;
  float x658_;
  u32 x65c_nearChance;
  u32 x660_midChance;
  bool x664_24_onGround : 1;
  bool x664_25_ : 1;
  bool x664_26_ : 1;
  bool x664_27_ : 1;
  bool x664_28_ : 1;
  bool x664_29_ : 1;
  bool x664_30_ : 1;
  bool x664_31_ : 1;
  bool x665_24_ : 1;
  bool x665_25_ : 1;
  bool x665_26_ : 1;
  bool x665_27_ : 1;
  bool x665_28_ : 1;
  bool x665_29_ : 1;
  float x668_ = 0.f;
  float x66c_ = 0.f;
  float x670_ = 0.f;
  TUniqueId x674_ = kInvalidUniqueId;
  float x678_ = 0.f;
  u32 x67c_ = -1;
  u32 x680_ = 0;
  float x684_ = 1.f;
  CSteeringBehaviors x688_;
  CBoneTracking x68c_boneTracking;
  TUniqueId x6c4_ = kInvalidUniqueId;
  float x6c8_ = 0.f;
  zeus::CVector3f x6cc_;
  u32 x6d8_ = 1;
  u32 x6dc_;
  CTeamAiMgr
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
  void Render(const CStateManager& mgr) const override;
  void Touch(CActor& act, CStateManager& mgr) override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                                                         const CWeaponMode& mode, EProjectileAttrib attrib) const override;
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
  bool Leash(CStateManager& mgr, float arg) override;
  bool InRange(CStateManager& mgr, float arg) override;
  bool InPosition(CStateManager& mgr, float arg) override;
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