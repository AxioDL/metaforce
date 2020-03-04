#include "Runtime/MP1/World/CChozoGhost.hpp"

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CRandom16.hpp"
#include "Runtime/Character/CPASAnimParmData.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path
namespace urde::MP1 {
CChozoGhost::CBehaveChance::CBehaveChance(CInputStream& in)
: x0_propertyCount(in.readUint32Big())
, x4_lurk(in.readFloatBig())
, x8_(in.readFloatBig())
, xc_attack(in.readFloatBig())
, x10_move(in.readFloatBig())
, x14_lurkTime(in.readFloatBig())
, x18_chargeAttack(x0_propertyCount <= 5 ? 0.5f : in.readFloatBig() * .01f)
, x1c_numBolts(x0_propertyCount <= 6 ? 2 : in.readUint32Big()) {
  float f2 = 1.f / (x10_move + xc_attack + x4_lurk + x8_);
  x4_lurk *= f2;
  x8_ *= f2;
  xc_attack *= f2;
  x10_move *= f2;
}

u32 CChozoGhost::CBehaveChance::GetBehave(EBehaveType type, CStateManager& mgr) const {
  float dVar5 = x4_lurk;
  float dVar4 = x8_;
  float dVar3 = xc_attack;
  if (type == EBehaveType::Zero) {
    float dVar2 = dVar5 / 3.f;
    dVar5 = 0.f;
    dVar4 += dVar2;
    dVar3 += dVar2;
  } else if (type == EBehaveType::One) {
    float dVar2 = dVar4 / 3.f;
    dVar4 = 0.f;
    dVar5 += dVar2;
    dVar3 += dVar2;
  } else if (type == EBehaveType::Two) {
    float dVar2 = dVar3 / 3.f;
    dVar3 = 0.f;
    dVar5 += dVar2;
    dVar4 += dVar2;
  } else if (type == EBehaveType::Three) {
    float dVar2 = x10_move / 3.f;
    dVar5 += dVar2;
    dVar4 += dVar2;
    dVar3 += dVar2;
  }

  float rnd = mgr.GetActiveRandom()->Float();
  if (dVar5 > rnd)
    return 0;
  else if (dVar4 > (rnd - dVar5))
    return 1;
  else if (dVar3 > (rnd - dVar5) - dVar4)
    return 2;
  return 3;
}

CChozoGhost::CChozoGhost(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                         CModelData&& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo,
                         float hearingRadius, float fadeOutDelay, float attackDelay, float freezeTime, CAssetId wpsc1,
                         const CDamageInfo& dInfo1, CAssetId wpsc2, const CDamageInfo& dInfo2,
                         const CBehaveChance& chance1, const CBehaveChance& chance2, const CBehaveChance& chance3,
                         u16 soundImpact, float f5, u16 sId2, u16 sId3, u32 w1, float f6, u32 w2, float hurlRecoverTime,
                         CAssetId projectileVisorEffect, s16 soundProjectileVisor, float f8, float f9, u32 nearChance,
                         u32 midChance)
: CPatterned(ECharacter::ChozoGhost, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::Zero, EBodyType::BiPedal, actParms, EKnockBackVariant::Medium)
, x568_hearingRadius(hearingRadius)
, x56c_fadeOutDelay(fadeOutDelay)
, x570_attackDelay(attackDelay)
, x574_freezeTime(freezeTime)
, x578_(wpsc1, dInfo1)
, x5a0_(wpsc2, dInfo2)
, x5c8_(chance1)
, x5e8_(chance2)
, x608_(chance3)
, x628_soundImpact(soundImpact)
, x62c_(f5)
, x630_(sId2)
, x632_(sId3)
, x634_(f6)
, x638_hurlRecoverTime(hurlRecoverTime)
, x63c_(w2)
, x650_sound_ProjectileVisor(soundProjectileVisor)
, x654_(f8)
, x658_(f9)
, x65c_nearChance(nearChance)
, x660_midChance(midChance)
, x664_24_onGround(w1)
, x664_25_(w1)
, x664_26_(false)
, x664_27_(false)
, x664_28_(false)
, x664_29_(false)
, x664_30_(false)
, x664_31_(false)
, x665_24_(true)
, x665_25_(false)
, x665_26_(false)
, x665_27_(false)
, x665_28_(false)
, x665_29_(false)
, x680_(x664_24_onGround ? 2 : 4)
, x68c_boneTracking(*GetModelData()->GetAnimationData(), "Head_1"sv, zeus::degToRad(80.f), zeus::degToRad(180.f),
                    EBoneTrackingFlags::None) {
  x578_.Token().Lock();
  x5a0_.Token().Lock();
  x668_ = GetModelData()->GetScale().z() *
          GetAnimationDistance(CPASAnimParmData(13, CPASAnimParm::FromEnum(3), CPASAnimParm::FromEnum(0)));
  x66c_ = GetModelData()->GetScale().z() *
          GetAnimationDistance(CPASAnimParmData(15, CPASAnimParm::FromEnum(1), CPASAnimParm::FromReal32(90.f)));
  x670_ = GetModelData()->GetScale().z() *
          GetAnimationDistance(CPASAnimParmData(7, CPASAnimParm::FromEnum(1), CPASAnimParm::FromEnum(2)));

  if (projectileVisorEffect.IsValid())
    x640_ = g_SimplePool->GetObj({SBIG('PART'), projectileVisorEffect});
  x460_knockBackController.SetEnableBurn(false);
  x460_knockBackController.SetEnableLaggedBurnDeath(false);
  x460_knockBackController.SetEnableShock(false);
  x460_knockBackController.SetEnableFreeze(false);
  CreateShadow(false);
  MakeThermalColdAndHot();
}
void CChozoGhost::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
}

void CChozoGhost::Think(float dt, CStateManager& mgr) { CPatterned::Think(dt, mgr); }

void CChozoGhost::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) { CPatterned::PreRender(mgr, frustum); }

void CChozoGhost::Render(const CStateManager& mgr) const { CPatterned::Render(mgr); }

void CChozoGhost::Touch(CActor& act, CStateManager& mgr) { CPatterned::Touch(act, mgr); }

EWeaponCollisionResponseTypes CChozoGhost::GetCollisionResponseType(const zeus::CVector3f& pos,
                                                                    const zeus::CVector3f& dir, const CWeaponMode& mode,
                                                                    EProjectileAttrib attrib) const {
  return CAi::GetCollisionResponseType(pos, dir, mode, attrib);
}

void CChozoGhost::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

void CChozoGhost::KnockBack(const zeus::CVector3f& dir, CStateManager& mgr, const CDamageInfo& info,
                            EKnockBackType type, bool inDeferred, float magnitude) {
  CPatterned::KnockBack(dir, mgr, info, type, inDeferred, magnitude);
}

bool CChozoGhost::CanBeShot(const CStateManager& mgr, int w1) { return CAi::CanBeShot(mgr, w1); }

void CChozoGhost::Dead(CStateManager& mgr, EStateMsg msg, float arg) { CPatterned::Dead(mgr, msg, arg); }

void CChozoGhost::SelectTarget(CStateManager& mgr, EStateMsg msg, float arg) { CAi::SelectTarget(mgr, msg, arg); }

void CChozoGhost::Run(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Run(mgr, msg, arg); }

void CChozoGhost::Generate(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Generate(mgr, msg, arg); }

void CChozoGhost::Deactivate(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Deactivate(mgr, msg, arg); }

void CChozoGhost::Attack(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Attack(mgr, msg, arg); }

void CChozoGhost::Shuffle(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Shuffle(mgr, msg, arg); }

void CChozoGhost::InActive(CStateManager& mgr, EStateMsg msg, float arg) { CAi::InActive(mgr, msg, arg); }

void CChozoGhost::Taunt(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Taunt(mgr, msg, arg); }

void CChozoGhost::Hurled(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Hurled(mgr, msg, arg); }

void CChozoGhost::WallDetach(CStateManager& mgr, EStateMsg msg, float arg) { CAi::WallDetach(mgr, msg, arg); }

void CChozoGhost::Growth(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Growth(mgr, msg, arg); }

void CChozoGhost::Land(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Land(mgr, msg, arg); }

bool CChozoGhost::Leash(CStateManager& mgr, float arg) { return CPatterned::Leash(mgr, arg); }

bool CChozoGhost::InRange(CStateManager& mgr, float arg) { return CPatterned::InRange(mgr, arg); }

bool CChozoGhost::InPosition(CStateManager& mgr, float arg) { return CPatterned::InPosition(mgr, arg); }

bool CChozoGhost::AggressionCheck(CStateManager& mgr, float arg) { return CAi::AggressionCheck(mgr, arg); }

bool CChozoGhost::ShouldTaunt(CStateManager& mgr, float arg) { return CAi::ShouldTaunt(mgr, arg); }

bool CChozoGhost::ShouldFlinch(CStateManager& mgr, float arg) { return CAi::ShouldFlinch(mgr, arg); }

bool CChozoGhost::ShouldMove(CStateManager& mgr, float arg) { return CAi::ShouldMove(mgr, arg); }

bool CChozoGhost::AIStage(CStateManager& mgr, float arg) { return CAi::AIStage(mgr, arg); }

u8 CChozoGhost::GetModelAlphau8(const CStateManager& mgr) const {
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::EPlayerVisor::XRay || !IsAlive())
    return u8(x42c_color.a() * 255);

  return 255;
}

bool CChozoGhost::IsOnGround() const { return x664_24_onGround; }

CProjectileInfo* CChozoGhost::GetProjectileInfo() { return CPatterned::GetProjectileInfo(); }

} // namespace urde::MP1