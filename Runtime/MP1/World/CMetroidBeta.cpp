#include "Runtime/MP1/World/CMetroidBeta.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/Collision/CCollisionActorManager.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Particle/CParticleSwoosh.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"
#include "Runtime/World/CGameArea.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CTeamAiMgr.hpp"
#include "Runtime/World/CWorld.hpp"
#include "Runtime/World/ScriptLoader.hpp"

namespace urde::MP1 {

CMetroidBetaData::CMetroidBetaData(CInputStream& in)
: x0_(in)
, x68_(in)
, xd0_(in.readFloatBig())
, xd4_(in.readFloatBig())
, xd8_(in.readFloatBig())
, xdc_(in.readFloatBig())
, xe0_(in.readFloatBig())
, xe4_(in.readFloatBig())
, xe8_(in.readFloatBig())
, xec_(in.readFloatBig())
, xf0_(in.readFloatBig())
, xf4_(in)
, xf8_(in)
, xfc_(in)
, x100_(in)
, x104_(in)
, x108_24_(in.readBool()) {}

CMetroidBeta::CMetroidBeta(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                           CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& aParms,
                           const CMetroidBetaData& metroidData)
: CPatterned(ECharacter::MetroidBeta, uid, name, EFlavorType::One, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::Flyer, aParms, EKnockBackVariant::Large)
, x56c_metroidBetaData(metroidData)
, x67c_pathFind(nullptr, 3, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x768_colPrim(GetBoundingBox(), GetMaterialList())
, x7b4_(GetModelData()->GetScale())
, x7c0_(GetModelData()->GetScale())
, x7cc_(GetModelData()->GetScale())
, x7e4_(g_SimplePool->GetObj({FOURCC('PART'), metroidData.xf4_}))
, x7f0_(g_SimplePool->GetObj({FOURCC('SWHC'), metroidData.xf8_}))
, x7fc_(g_SimplePool->GetObj({FOURCC('PART'), metroidData.xfc_}))
, x808_(g_SimplePool->GetObj({FOURCC('PART'), metroidData.x100_}))
, x814_(g_SimplePool->GetObj({FOURCC('PART'), metroidData.x104_}))
, x820_(std::make_unique<CElementGen>(x7e4_))
, x824_(std::make_unique<CParticleSwoosh>(x7f0_, 0))
, x828_(std::make_unique<CElementGen>(x7fc_))
, x82c_(std::make_unique<CElementGen>(x808_))
, x830_(std::make_unique<CElementGen>(x814_)) {
  x820_->SetParticleEmission(false);
  x828_->SetParticleEmission(false);
  x82c_->SetParticleEmission(false);
  x824_->DoElectricWarmup();
  const float scale = 0.75f * GetModelData()->GetScale().y();
  const zeus::CVector3f scaleVec(scale, scale, 2.f * scale);
  zeus::CAABox box = {-scaleVec, scaleVec};
  SetBoundingBox(box);
  x768_colPrim.SetBox(box);
}

void CMetroidBeta::Think(float dt, CStateManager& mgr) {
  if (CTeamAiMgr::GetTeamAiRole(mgr, x678_teamMgr, GetUniqueId())) {
    AddToTeam(mgr);
  }

  CPatterned::Think(dt, mgr);
  x764_collisionManager->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
  // sub801c1928(mgr);
  // sub801c0da4(dt, mgr);
  // sub801c21b4(dt, mgr);
}
void CMetroidBeta::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case EScriptObjectMessage::Registered: {
    x450_bodyController->Activate(mgr);
    CreateCollisionActorManager(mgr);
    //sub801c13d4();
    x760_ = GetModelData()->GetAnimationData()->GetLocatorSegId("L_Claw_1"sv);
    x761_ = GetModelData()->GetAnimationData()->GetLocatorSegId("R_Claw_1"sv);
    break;
  }
  case EScriptObjectMessage::Activate: {
    x764_collisionManager->SetActive(mgr, true);
    break;
  }
  case EScriptObjectMessage::Deactivate: {
    x764_collisionManager->SetActive(mgr, false);
    break;
  }
  case EScriptObjectMessage::Deleted: {
    x764_collisionManager->Destroy(mgr);
    RemoveFromTeam(mgr);
    break;
  }
  case EScriptObjectMessage::Damage:
  case EScriptObjectMessage::InvulnDamage: {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
      if (TCastToConstPtr<CGameProjectile> proj = mgr.GetObjectById(colAct->GetLastTouchedObject())) {
        if (proj->GetOwnerId() != mgr.GetPlayer().GetUniqueId())
          break;
        // sub801c14b4(proj->GetDamageInfo().GetDamage(), mgr);
        const CDamageInfo& dInfo = proj->GetDamageInfo();
        if (colAct->GetDamageVulnerability()->WeaponHits(dInfo.GetWeaponMode(), false)) {
          if (dInfo.GetWeaponMode().IsCharged() || dInfo.GetWeaponMode().IsComboed() ||
              dInfo.GetWeaponMode().GetType() == EWeaponType::Missile) {
            x840_31_ = true;
            x83c_ += 1.f;
          }
          KnockBack(proj->GetTranslation() - proj->GetPreviousPos(), mgr, dInfo, EKnockBackType::Radius, false,
                    dInfo.GetKnockBackPower());
        }
        if (x840_25_)
          x83c_ += 0.1f;
        x840_26_ = true;
      }
    } else if (TCastToConstPtr<CWeapon> weap = mgr.GetObjectById(uid)) {
      CDamageInfo info = weap->GetDamageInfo();
      info.SetRadius(0.f);
      mgr.ApplyDamage(uid, x790_, weap->GetOwnerId(), info,
                      CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
    }
    break;
  }
  case EScriptObjectMessage::Alert: {
    x840_26_ = true;
    break;
  }
  case EScriptObjectMessage::Touched: {
    if (TCastToConstPtr<CCollisionActor> colAct = mgr.GetObjectById(uid)) {
      if (HealthInfo(mgr)->GetHP() > 0.f && colAct->GetLastTouchedObject() == mgr.GetPlayer().GetUniqueId() &&
      x420_curDamageRemTime <= 0.f) {
        CDamageInfo dInfo = GetContactDamage();
        dInfo.SetDamage(0.5f * dInfo.GetDamage());
        if (x840_29_ && x840_30_)
          dInfo = GetContactDamage();

        mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), dInfo,
                        CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});

        x420_curDamageRemTime = x424_damageWaitTime;
        x840_30_ = false;
      }
    }
    break;
  }
  case EScriptObjectMessage::InitializedInArea: {
    if (x678_teamMgr == kInvalidUniqueId)
      x678_teamMgr = CTeamAiMgr::GetTeamAiMgr(*this, mgr);

    x67c_pathFind.SetArea(mgr.GetWorld()->GetAreaAlways(GetAreaId())->GetPostConstructed()->x10bc_pathArea);
    break;
  }
  case EScriptObjectMessage::SuspendedMove: {
    if (x764_collisionManager)
      x764_collisionManager->SetMovable(mgr, false);
    break;
  }
  default:
    break;
  }
}

void CMetroidBeta::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  CPatterned::AddToRenderer(frustum, mgr);
}

void CMetroidBeta::Render(CStateManager& mgr) { CPatterned::Render(mgr); }

const CDamageVulnerability* CMetroidBeta::GetDamageVulnerability() const { return CAi::GetDamageVulnerability(); }

const CDamageVulnerability* CMetroidBeta::GetDamageVulnerability(const zeus::CVector3f& vec1,
                                                                 const zeus::CVector3f& vec2,
                                                                 const CDamageInfo& dInfo) const {
  return CActor::GetDamageVulnerability(vec1, vec2, dInfo);
}

void CMetroidBeta::Touch(CActor& act, CStateManager& mgr) { CPatterned::Touch(act, mgr); }

zeus::CVector3f CMetroidBeta::GetAimPosition(const CStateManager& mgr, float dt) const {
  return CPatterned::GetAimPosition(mgr, dt);
}

void CMetroidBeta::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType eType, float dt) {
  CPatterned::DoUserAnimEvent(mgr, node, eType, dt);
}

const CCollisionPrimitive* CMetroidBeta::GetCollisionPrimitive() const {
  return CPhysicsActor::GetCollisionPrimitive();
}

void CMetroidBeta::CollidedWith(TUniqueId collidee, const CCollisionInfoList& info, CStateManager& mgr) {
  CPatterned::CollidedWith(collidee, info, mgr);
}

zeus::CVector3f CMetroidBeta::GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                                        const zeus::CVector3f& aimPos) const {
  return CAi::GetOrigin(mgr, role, aimPos);
}

void CMetroidBeta::Patrol(CStateManager& mgr, EStateMsg msg, float arg) { CPatterned::Patrol(mgr, msg, arg); }

void CMetroidBeta::PathFind(CStateManager& mgr, EStateMsg msg, float arg) { CPatterned::PathFind(mgr, msg, arg); }

void CMetroidBeta::SelectTarget(CStateManager& mgr, EStateMsg msg, float arg) { CAi::SelectTarget(mgr, msg, arg); }

void CMetroidBeta::TargetPatrol(CStateManager& mgr, EStateMsg msg, float arg) {
  CPatterned::TargetPatrol(mgr, msg, arg);
}

void CMetroidBeta::Generate(CStateManager& mgr, EStateMsg msg, float arg) {
  CAi::Generate(mgr, msg, arg);
}

void CMetroidBeta::Attack(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Attack(mgr, msg, arg); }

void CMetroidBeta::TurnAround(CStateManager& mgr, EStateMsg msg, float arg) { CAi::TurnAround(mgr, msg, arg); }

void CMetroidBeta::TelegraphAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  CAi::TelegraphAttack(mgr, msg, arg);
}

void CMetroidBeta::WallHang(CStateManager& mgr, EStateMsg msg, float arg) { CAi::WallHang(mgr, msg, arg); }

void CMetroidBeta::SpecialAttack(CStateManager& mgr, EStateMsg msg, float arg) { CAi::SpecialAttack(mgr, msg, arg); }

bool CMetroidBeta::InAttackPosition(CStateManager& mgr, float arg) { return CAi::InAttackPosition(mgr, arg); }

bool CMetroidBeta::Attacked(CStateManager& mgr, float arg) { return CPatterned::Attacked(mgr, arg); }

bool CMetroidBeta::PathShagged(CStateManager& mgr, float arg) { return CPatterned::PathShagged(mgr, arg); }

bool CMetroidBeta::InDetectionRange(CStateManager& mgr, float arg) { return CPatterned::InDetectionRange(mgr, arg); }

bool CMetroidBeta::AnimOver(CStateManager& mgr, float arg) { return CPatterned::AnimOver(mgr, arg); }

bool CMetroidBeta::ShouldAttack(CStateManager& mgr, float arg) { return CAi::ShouldAttack(mgr, arg); }

bool CMetroidBeta::InPosition(CStateManager& mgr, float arg) { return CPatterned::InPosition(mgr, arg); }

bool CMetroidBeta::ShouldTurn(CStateManager& mgr, float arg) { return CAi::ShouldTurn(mgr, arg); }

bool CMetroidBeta::AttackOver(CStateManager& mgr, float arg) { return CAi::AttackOver(mgr, arg); }

bool CMetroidBeta::ShotAt(CStateManager& mgr, float arg) { return CAi::ShotAt(mgr, arg); }

bool CMetroidBeta::ShouldWallHang(CStateManager& mgr, float arg) { return CAi::ShouldWallHang(mgr, arg); }

bool CMetroidBeta::StartAttack(CStateManager& mgr, float arg) { return CAi::StartAttack(mgr, arg); }

bool CMetroidBeta::BreakAttack(CStateManager& mgr, float arg) { return CAi::BreakAttack(mgr, arg); }

bool CMetroidBeta::ShouldSpecialAttack(CStateManager& mgr, float arg) { return CAi::ShouldSpecialAttack(mgr, arg); }

void CMetroidBeta::RenderHitGunEffect() const {}

void CMetroidBeta::RenderHitBallEffect() const {}

static SSphereJointInfo skPelvisInfo[1] {
    {"Pelvis", 1.5f},
};

void CMetroidBeta::CreateCollisionActorManager(CStateManager& mgr) {
  std::vector<CJointCollisionDescription> joints;
  AddSphereJoints(skPelvisInfo, 1, joints);

  x764_collisionManager =
      std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), joints, false);
  x764_collisionManager->SetActive(mgr, GetActive());

  for (u32 i = 0; i < x764_collisionManager->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = x764_collisionManager->GetCollisionDescFromIndex(i);
    if (TCastToPtr<CCollisionActor>(mgr.ObjectById(desc.GetCollisionActorId()))) {
      if (desc.GetName() == "Pelvis"sv)
        x790_ = desc.GetCollisionActorId();
    }
  }

  SetCollisionActorHealthAndVulnerability(mgr);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      {EMaterialTypes::Solid, EMaterialTypes::Wall, EMaterialTypes::Floor, EMaterialTypes::Ceiling},
      {EMaterialTypes::CollisionActor, EMaterialTypes::Player, EMaterialTypes::Character}));
  AddMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
  x764_collisionManager->AddMaterial(mgr, CMaterialList(EMaterialTypes::AIJoint, EMaterialTypes::CameraPassthrough));
}

void CMetroidBeta::AddSphereJoints(SSphereJointInfo* sphereJoints, s32 count,
    std::vector<CJointCollisionDescription>& joints) {

  for (s32 i = 0; i < count; ++i) {
    const auto& sphereJoint = sphereJoints[i];
    const CSegId id = GetModelData()->GetAnimationData()->GetLocatorSegId(sphereJoint.name);

    if (id.IsInvalid()) {
      continue;
    }

    joints.push_back(CJointCollisionDescription::SphereCollision(id, sphereJoint.radius, sphereJoint.name, 1000.0f));
  }
}

void CMetroidBeta::SetCollisionActorHealthAndVulnerability(CStateManager& mgr) {
  CHealthInfo* hInfo = HealthInfo(mgr);
  if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x790_)) {
    *colAct->HealthInfo(mgr) = *hInfo;
    colAct->SetDamageVulnerability(*GetDamageVulnerability());
  }
}

void CMetroidBeta::RemoveFromTeam(CStateManager& mgr) {
  if (x678_teamMgr == kInvalidUniqueId)
    return;

  if (TCastToPtr<CTeamAiMgr> teamMgr = mgr.ObjectById(x678_teamMgr)) {
    if (teamMgr->IsPartOfTeam(GetUniqueId()))
      teamMgr->RemoveTeamAiRole(GetUniqueId());
  }
}

void CMetroidBeta::AddToTeam(CStateManager& mgr) {
  if (x678_teamMgr == kInvalidUniqueId)
    return;

  if (TCastToPtr<CTeamAiMgr> teamMgr = mgr.ObjectById(x678_teamMgr)) {
    if (!teamMgr->IsPartOfTeam(GetUniqueId()))
      teamMgr->AssignTeamAiRole(*this, CTeamAiRole::ETeamAiRole::Ranged, CTeamAiRole::ETeamAiRole::Invalid,
                                CTeamAiRole::ETeamAiRole::Invalid);
  }
}
} // namespace urde::MP1
