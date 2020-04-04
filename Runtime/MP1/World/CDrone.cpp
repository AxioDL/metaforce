#include "Runtime/MP1/World/CDrone.hpp"

#include "Runtime/Audio/CSfxManager.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Weapon/CWeapon.hpp"
#include "Runtime/World/CGameLight.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CTeamAiMgr.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {
CDrone::CDrone(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
               const zeus::CTransform& xf, float f1, CModelData&& mData, const CPatternedInfo& pInfo,
               const CActorParameters& actParms, EMovementType movement, EColliderType colliderType, EBodyType bodyType,
               const CDamageInfo& dInfo1, CAssetId aId1, const CDamageInfo& dInfo2, CAssetId aId2,
               std::vector<CVisorFlare::CFlareDef> flares, float f2, float f3, float f4, float f5, float f6, float f7,
               float f8, float f9, float f10, float f11, float f12, float f13, float f14, float f15, float f16,
               float f17, float f18, float f19, float f20, CAssetId crscId, float f21, float f22, float f23, float f24,
               s32 sId, bool b1)
: CPatterned(ECharacter::Drone, uid, name, flavor, info, xf, std::move(mData), pInfo, movement, colliderType, bodyType,
             actParms, EKnockBackVariant(flavor == EFlavorType::Zero))
, x568_(aId1)
, x56c_(g_SimplePool->GetObj({SBIG('CRSC'), crscId}))
, x57c_flares(std::move(flares))
, x590_(dInfo1)
, x5ac_(dInfo2)
, x5e4_(f23)
, x5ec_(f1)
, x5f0_(f2)
, x5f4_(f3)
, x5f8_(f4)
, x5fc_(f5)
, x600_(f11)
, x608_(f6)
, x60c_(f7)
, x610_(f8)
, x614_(f9)
, x618_(f10)
, x61c_(f12)
, x620_(f20)
, x63c_(f13)
, x640_(f14)
, x648_(f15)
, x64c_(f16)
, x650_(f17)
, x654_(f18)
, x658_(f19)
, x65c_(f21)
, x660_(f22)
, x664_(f24)
, x690_(zeus::CSphere({0.f, 0.f, 1.8f}, 1.1f), CActor::GetMaterialList())
, x6b0_pathFind(nullptr, 3 + int(b1), pInfo.GetPathfindingIndex(), 1.f, 2.4f)
, x7cc_(CSfxManager::TranslateSFXID(sId))
, x82c_(std::make_unique<CModelData>(CStaticRes{aId2, zeus::skOne3f}))
, x830_13_(0)
, x830_10_(0)
, x834_24_(false)
, x834_25_(false)
, x834_26_(false)
, x834_27_(false)
, x834_28_(false)
, x834_29_(false)
, x834_30_(false)
, x834_31_(false)
, x835_24_(false)
, x835_25_(b1)
, x835_26_(false) {
  UpdateTouchBounds(pInfo.GetHalfExtent());
  x460_knockBackController.SetEnableShock(true);
  x460_knockBackController.SetAvailableState(EKnockBackAnimationState::Hurled, false);
  x460_knockBackController.SetLocomotionDuringElectrocution(true);
  MakeThermalColdAndHot();
  CreateShadow(flavor != EFlavorType::One);
}

void CDrone::Accept(IVisitor& visitor) { visitor.Visit(this); }
void CDrone::Think(float dt, CStateManager& mgr) { CPatterned::Think(dt, mgr); }

void CDrone::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, sender, mgr);

  switch (msg) {
  case EScriptObjectMessage::Activate: {
    SetLightEnabled(mgr, true);
    AddToTeam(mgr);
    break;
  }
  case EScriptObjectMessage::Deactivate:
  case EScriptObjectMessage::Deleted: {
    for (TUniqueId& unkId : x7d4_) {
      if (unkId != kInvalidUniqueId) {
        mgr.FreeScriptObject(unkId);
        unkId = kInvalidUniqueId;
      }
    }
    RemoveFromTeam(mgr);
    mgr.GetPlayerState()->GetStaticInterference().RemoveSource(GetUniqueId());
    if (x578_lightId != kInvalidUniqueId) {
      mgr.FreeScriptObject(x578_lightId);
    }
    if (x57a_ != kInvalidUniqueId) {
      mgr.FreeScriptObject(x57a_);
    }

    if (x7d0_) {
      CSfxManager::RemoveEmitter(x7d0_);
      x7d0_.reset();
    }
    break;
  }
  case EScriptObjectMessage::Alert:
    x834_29_ = true;
    break;
  case EScriptObjectMessage::OnFloor:
    if (!x835_26_ && x834_24_) {
      x835_26_ = true;
      MassiveFrozenDeath(mgr);
    }
    break;
  case EScriptObjectMessage::Registered:
    x450_bodyController->Activate(mgr);
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
    x450_bodyController->BodyStateInfo().SetMaximumPitch(0.f);
    x5cc_ = 0.f;
    x460_knockBackController.SetEnableFreeze(false);
    AddMaterial(EMaterialTypes::AIJoint, mgr);
    x578_lightId = mgr.AllocateUniqueId();
    mgr.AddObject(new CGameLight(x578_lightId, GetAreaIdAlways(), GetActive(), "LaserLight"sv, {}, GetUniqueId(),
                                 CLight::BuildPoint(zeus::skZero3f, zeus::skRed), 0, 0, 0.f));
    break;
  case EScriptObjectMessage::InitializedInArea: {
    x6b0_pathFind.SetArea(mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
    if (x688_teamMgr == kInvalidUniqueId) {
      x688_teamMgr = CTeamAiMgr::GetTeamAiMgr(*this, mgr);
      if (GetActive()) {
        AddToTeam(mgr);
      }
    }

    x604_ = HealthInfo(mgr)->GetHP();
    x55c_moveScale = 1.f / GetModelData()->GetScale();
    if (x835_25_)
      SetSoundEventPitchBend(0);
    break;
  }
  default:
    break;
  }
}

void CDrone::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  CPatterned::PreRender(mgr, frustum);
  if (x3fc_flavor == EFlavorType::One) {
    if (HasModelData() && GetModelData()->HasAnimData()) {
      if (GetModelAlphau8(mgr) == 0)
        GetModelData()->GetAnimationData()->BuildPose();
    }
  }
}

void CDrone::AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr) const {
  CPatterned::AddToRenderer(frustum, mgr);
}

void CDrone::Render(const CStateManager& mgr) const {
  bool isOne = x3fc_flavor == EFlavorType::One;
  if (!isOne || GetModelAlphau8(mgr) != 0) {
    if (isOne && mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay) {
      CGraphics::SetFog(ERglFogMode::PerspLin, 0.f, 75.f, zeus::skBlack);
      GetModelData()->GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnFirst();
      mgr.SetupFogForArea3XRange(GetAreaIdAlways());
    }
    CPatterned::Render(mgr);
    if (isOne && mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay) {
      CGraphics::SetFog(ERglFogMode::PerspLin, 0.f, 75.f, zeus::skBlack);
      GetModelData()->GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnLast();
      mgr.SetupFogForArea3XRange(GetAreaIdAlways());
    }

    if (isOne && zeus::close_enough(x5dc_, 0)) {
      x82c_->Render(mgr, GetLctrTransform("Shield_LCTR"sv), GetActorLights(),
                    CModelFlags{8, 0, 3, zeus::CColor::lerp({1.f, 0.f, 0.f, 1.f}, zeus::skWhite, x5e8_)});
    }
  }
}
bool CDrone::CanRenderUnsorted(const CStateManager& mgr) const {
  if (zeus::close_enough(x5dc_, 0.f))
    return false;
  return CPatterned::CanRenderUnsorted(mgr);
}

const CDamageVulnerability* CDrone::GetDamageVulnerability(const zeus::CVector3f&, const zeus::CVector3f& dir,
                                                           const CDamageInfo&) const {
  if (x3fc_flavor == EFlavorType::One && HitShield(-dir)) {
    return &CDamageVulnerability::ImmuneVulnerabilty();
  }
  return CAi::GetDamageVulnerability();
}
void CDrone::Touch(CActor& act, CStateManager& mgr) {
  CPatterned::Touch(act, mgr);
  if (TCastToPtr<CWeapon> weapon = act) {
    if (IsAlive()) {
      x834_24_ = weapon->GetType() == EWeaponType::Wave;
      if (HitShield(weapon->GetTranslation() - GetTranslation())) {
        x5e8_ = 1.f;
      }
    }
  }
}

EWeaponCollisionResponseTypes CDrone::GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f& dir,
                                                               const CWeaponMode&, EProjectileAttrib) const {
  if (x3fc_flavor == EFlavorType::One && HitShield(-dir)) {
    return EWeaponCollisionResponseTypes::Unknown86;
  }
  return EWeaponCollisionResponseTypes::Unknown36;
}

void CDrone::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}
const CCollisionPrimitive* CDrone::GetCollisionPrimitive() const { return CPhysicsActor::GetCollisionPrimitive(); }
void CDrone::Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) {
  CPatterned::Death(mgr, direction, state);
}
void CDrone::KnockBack(const zeus::CVector3f& backVec, CStateManager& mgr, const CDamageInfo& info, EKnockBackType type,
                       bool inDeferred, float magnitude) {
  CPatterned::KnockBack(backVec, mgr, info, type, inDeferred, magnitude);
}
void CDrone::Patrol(CStateManager& mgr, EStateMsg msg, float dt) { CPatterned::Patrol(mgr, msg, dt); }
void CDrone::PathFind(CStateManager& mgr, EStateMsg msg, float dt) { CPatterned::PathFind(mgr, msg, dt); }
void CDrone::TargetPlayer(CStateManager& mgr, EStateMsg msg, float dt) { CPatterned::TargetPlayer(mgr, msg, dt); }
void CDrone::TargetCover(CStateManager& mgr, EStateMsg msg, float dt) { CAi::TargetCover(mgr, msg, dt); }
void CDrone::Deactivate(CStateManager& mgr, EStateMsg msg, float dt) { CAi::Deactivate(mgr, msg, dt); }
void CDrone::Attack(CStateManager& mgr, EStateMsg msg, float dt) {}
void CDrone::Active(CStateManager& mgr, EStateMsg msg, float dt) { CAi::Active(mgr, msg, dt); }
void CDrone::Flee(CStateManager& mgr, EStateMsg msg, float dt) { CAi::Flee(mgr, msg, dt); }
void CDrone::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float dt) { CAi::ProjectileAttack(mgr, msg, dt); }
void CDrone::TelegraphAttack(CStateManager& mgr, EStateMsg msg, float dt) { CAi::TelegraphAttack(mgr, msg, dt); }
void CDrone::Dodge(CStateManager& mgr, EStateMsg msg, float dt) { CAi::Dodge(mgr, msg, dt); }
void CDrone::Retreat(CStateManager& mgr, EStateMsg msg, float dt) { CAi::Retreat(mgr, msg, dt); }
void CDrone::Cover(CStateManager& mgr, EStateMsg msg, float dt) { CAi::Cover(mgr, msg, dt); }
void CDrone::SpecialAttack(CStateManager& mgr, EStateMsg msg, float dt) { CAi::SpecialAttack(mgr, msg, dt); }
void CDrone::PathFindEx(CStateManager& mgr, EStateMsg msg, float dt) { CAi::PathFindEx(mgr, msg, dt); }
bool CDrone::Leash(CStateManager& mgr, float arg) { return CPatterned::Leash(mgr, arg); }
bool CDrone::InRange(CStateManager& mgr, float arg) { return CPatterned::InRange(mgr, arg); }
bool CDrone::SpotPlayer(CStateManager& mgr, float arg) { return CPatterned::SpotPlayer(mgr, arg); }
bool CDrone::AnimOver(CStateManager& mgr, float arg) { return CPatterned::AnimOver(mgr, arg); }
bool CDrone::ShouldAttack(CStateManager& mgr, float arg) { return CAi::ShouldAttack(mgr, arg); }
bool CDrone::HearShot(CStateManager& mgr, float arg) { return CAi::HearShot(mgr, arg); }
bool CDrone::CoverCheck(CStateManager& mgr, float arg) { return CAi::CoverCheck(mgr, arg); }
bool CDrone::LineOfSight(CStateManager& mgr, float arg) { return CAi::LineOfSight(mgr, arg); }
bool CDrone::ShouldMove(CStateManager& mgr, float arg) { return CAi::ShouldMove(mgr, arg); }
bool CDrone::CodeTrigger(CStateManager& mgr, float arg) { return CPatterned::CodeTrigger(mgr, arg); }
void CDrone::Burn(float duration, float damage) { CPatterned::Burn(duration, damage); }
CPathFindSearch* CDrone::GetSearchPath() { return CPatterned::GetSearchPath(); }

void CDrone::BuildNearList(EMaterialTypes includeMat, EMaterialTypes excludeMat,
                           rstl::reserved_vector<TUniqueId, 1024>& listOut, float radius, CStateManager& mgr) {
  const zeus::CVector3f pos = GetTranslation();
  mgr.BuildNearList(listOut, zeus::CAABox(pos - radius, pos + radius),
                    CMaterialFilter::MakeIncludeExclude({includeMat}, {excludeMat}), nullptr);
}
void CDrone::SetLightEnabled(CStateManager& mgr, bool activate) {
  mgr.SendScriptMsgAlways(x578_lightId, GetUniqueId(),
                          activate ? EScriptObjectMessage::Activate : EScriptObjectMessage::Deactivate);
}
void CDrone::UpdateVisorFlare(CStateManager& mgr) {}

void CDrone::UpdateTouchBounds(float radius) {
  const zeus::CTransform xf = GetLctrTransform("Skeleton_Root"sv);
  const zeus::CVector3f diff = xf.origin - GetTranslation();
  SetBoundingBox(zeus::CAABox{diff - radius, diff + radius});
  x6b0_pathFind.SetRadius(0.25f + radius);
}

bool CDrone::HitShield(const zeus::CVector3f& dir) const {
  if (x3fc_flavor == EFlavorType::One && zeus::close_enough(x5dc_, 0.f)) {
    return GetLctrTransform("Shield_LCTR"sv).basis[1].dot(dir.normalized()) > 0.85f;
  }

  return false;
}
void CDrone::AddToTeam(CStateManager& mgr) const {
  if (x688_teamMgr == kInvalidUniqueId) {
    return;
  }

  if (TCastToPtr<CTeamAiMgr> teamMgr = mgr.ObjectById(x688_teamMgr)) {
    teamMgr->AssignTeamAiRole(*this, CTeamAiRole::ETeamAiRole::Ranged, CTeamAiRole::ETeamAiRole::Melee,
                              CTeamAiRole::ETeamAiRole::Invalid);
  }
}
void CDrone::RemoveFromTeam(CStateManager& mgr) const {
  if (TCastToPtr<CTeamAiMgr> teamMgr = mgr.ObjectById(x688_teamMgr)) {
    if (teamMgr->IsPartOfTeam(GetUniqueId())) {
      teamMgr->RemoveTeamAiRole(GetUniqueId());
    }
  }
}

} // namespace urde::MP1
