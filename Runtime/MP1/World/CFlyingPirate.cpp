#include "Runtime/MP1/World/CFlyingPirate.hpp"

#include "Runtime/Character/CPASAnimParmData.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptCoverPoint.hpp"
#include "Runtime/World/CTeamAiMgr.hpp"
#include "Runtime/World/CWorld.hpp"

namespace urde::MP1 {
namespace {
constexpr std::array<const SBurst, 6> skBurst1{{
    {4, {3, 4, 11, 12, -1, 0, 0, 0}, 0.1f, 0.05f},
    {20, {2, 3, 4, 5, -1, 0, 0, 0}, 0.1f, 0.05f},
    {20, {10, 11, 12, 13, -1, 0, 0, 0}, 0.1f, 0.05f},
    {25, {15, 16, 1, 2, -1, 0, 0, 0}, 0.1f, 0.05f},
    {25, {5, 6, 7, 8, -1, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
}};

constexpr std::array<const SBurst, 6> skBurst2{{
    {5, {3, 4, 8, 12, -1, 0, 0, 0}, 0.1f, 0.05f},
    {10, {2, 3, 4, 5, -1, 0, 0, 0}, 0.1f, 0.05f},
    {10, {10, 11, 12, 13, -1, 0, 0, 0}, 0.1f, 0.05f},
    {40, {15, 16, 1, 2, -1, 0, 0, 0}, 0.1f, 0.05f},
    {35, {5, 6, 7, 8, -1, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
}};

constexpr std::array<const SBurst, 5> skBurst3{{
    {30, {3, 4, 5, 11, 12, 4, -1, 0}, 0.1f, 0.05f},
    {20, {2, 3, 4, 5, 4, 3, -1, 0}, 0.1f, 0.05f},
    {20, {5, 4, 3, 13, 12, 11, -1, 0}, 0.1f, 0.05f},
    {30, {1, 2, 3, 4, 5, 6, -1, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
}};

constexpr std::array<const SBurst, 5> skBurst4{{
    {10, {6, 5, 4, 14, 13, 12, -1, 0}, 0.1f, 0.05f},
    {20, {14, 13, 12, 11, 10, 9, -1, 0}, 0.1f, 0.05f},
    {20, {14, 15, 16, 11, 10, 9, -1, 0}, 0.1f, 0.05f},
    {50, {11, 10, 9, 8, 7, 6, -1, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
}};

constexpr std::array<const SBurst* const, 5> skBursts{
    skBurst1.data(), skBurst2.data(), skBurst3.data(), skBurst4.data(), nullptr,
};

constexpr std::array<const std::string_view, 14> skParts{
    "s_Head",  "s_R_shoulder", "s_R_elbow", "s_R_wrist", "s_L_shoulder", "s_L_elbow", "s_L_wrist",
    "s_R_hip", "s_R_knee",     "s_R_ankle", "s_L_hip",   "s_L_knee",     "s_L_ankle", "s_rocket_LCTR",
};
} // namespace

CFlyingPirate::CFlyingPirateData::CFlyingPirateData(CInputStream& in, u32 propCount)
: x0_(in.readFloatBig())
, x4_(in.readFloatBig())
, x8_(in.readInt32Big())
, xc_projInfo1(in)
, x34_sfx1(in.readUint16Big())
, x38_projInfo2(in)
, x60_projInfo3(in.readInt32Big(), {})
, x88_(in.readFloatBig())
, x8c_(in.readFloatBig())
, x90_particleGenDesc(g_SimplePool->GetObj({SBIG('PART'), in.readInt32Big()}))
, x9c_dInfo(in)
, xb8_(in.readFloatBig())
, xbc_(in.readFloatBig())
, xc0_(in.readFloatBig())
, xc4_(in.readFloatBig())
, xca_sfx3(in.readUint16Big())
, xcc_(in.readFloatBig())
, xd0_(in.readFloatBig())
, xd4_(in.readFloatBig())
, xd8_(in)
, xdc_(in)
, xe0_(in)
, xe4_sfx4(in.readUint16Big())
, xe6_sfx5(in.readUint16Big())
, xe8_(in.readFloatBig())
, xec_(in.readFloatBig())
, xf0_(propCount < 36 ? 0.f : in.readFloatBig()) {
  xc_projInfo1.Token().Lock();
  x38_projInfo2.Token().Lock();
  x60_projInfo3.Token().Lock();
}

CFlyingPirate::CFlyingPirateRagDoll::CFlyingPirateRagDoll(CStateManager& mgr, CFlyingPirate* actor, u16 w1, u16 w2)
: CRagDoll(-actor->GetGravityConstant(), 3.f, 8.f, 0)
, x88_(w1)
, x9c_(w2)
, xa4_(actor->GetDestPos() - actor->GetTranslation()) {
  actor->RemoveMaterial(EMaterialTypes::Solid, EMaterialTypes::AIBlock, EMaterialTypes::GroundCollider, mgr);
  actor->HealthInfo(mgr)->SetHP(-1.f);
  SetNumParticles(15);
  SetNumLengthConstraints(45);
  SetNumJointConstraints(4);
  CAnimData* animData = actor->GetModelData()->GetAnimationData();
  animData->BuildPose();
  const zeus::CVector3f& center = actor->GetBoundingBox().center();
  for (const auto& part : skParts) {
    const CSegId& id = animData->GetLocatorSegId(part);
    AddParticle(id, center, center * animData->GetPose().GetOffset(id), 0.45f * center.z());
  }
  SatisfyWorldConstraintsOnConstruction(mgr);
  AddLengthConstraint(0, 1);
  AddLengthConstraint(0, 2);
  AddLengthConstraint(0, 8);
  AddLengthConstraint(0, 11);
  AddLengthConstraint(0, 5);
  AddLengthConstraint(2, 3);
  AddLengthConstraint(3, 4);
  AddLengthConstraint(5, 6);
  AddLengthConstraint(6, 7);
  AddLengthConstraint(2, 5);
  AddLengthConstraint(2, 11);
  AddLengthConstraint(5, 8);
  AddLengthConstraint(5, 11);
  AddLengthConstraint(8, 11);
  AddLengthConstraint(8, 9);
  AddLengthConstraint(9, 10);
  AddLengthConstraint(11, 12);
  AddLengthConstraint(12, 13);
  AddLengthConstraint(14, 0);
  AddLengthConstraint(14, 2);
  AddLengthConstraint(14, 5);
  AddLengthConstraint(14, 8);
  AddLengthConstraint(14, 11);
  AddMinLengthConstraint(1, 8, x14_lengthConstraints[2].GetLength());
  AddMinLengthConstraint(1, 11, x14_lengthConstraints[3].GetLength());
  AddMinLengthConstraint(4, 2, x14_lengthConstraints[5].GetLength());
  AddMinLengthConstraint(7, 5, x14_lengthConstraints[7].GetLength());
  AddMinLengthConstraint(3, 5, 0.5f * x14_lengthConstraints[5].GetLength() + x14_lengthConstraints[9].GetLength());
  AddMinLengthConstraint(6, 2, 0.5f * x14_lengthConstraints[7].GetLength() + x14_lengthConstraints[9].GetLength());
  AddMinLengthConstraint(4, 5, 0.5f * x14_lengthConstraints[5].GetLength() + x14_lengthConstraints[9].GetLength());
  AddMinLengthConstraint(7, 2, 0.5f * x14_lengthConstraints[7].GetLength() + x14_lengthConstraints[9].GetLength());
  AddMinLengthConstraint(10, 8, x14_lengthConstraints[15].GetLength());
  AddMinLengthConstraint(11, 11, x14_lengthConstraints[17].GetLength());
  AddMinLengthConstraint(9, 2, 0.707f * x14_lengthConstraints[15].GetLength() + x14_lengthConstraints[10].GetLength());
  AddMinLengthConstraint(12, 5, 0.707f * x14_lengthConstraints[17].GetLength() + x14_lengthConstraints[13].GetLength());
  AddMinLengthConstraint(9, 11, x14_lengthConstraints[15].GetLength());
  AddMinLengthConstraint(12, 8, x14_lengthConstraints[17].GetLength());
  AddMinLengthConstraint(10, 0, x14_lengthConstraints[2].GetLength() + x14_lengthConstraints[15].GetLength());
  AddMinLengthConstraint(13, 0, x14_lengthConstraints[3].GetLength() + x14_lengthConstraints[17].GetLength());
  AddMinLengthConstraint(9, 12, 0.5f * x14_lengthConstraints[14].GetLength());
  AddMinLengthConstraint(10, 12, 0.5f * x14_lengthConstraints[14].GetLength());
  AddMinLengthConstraint(13, 9, 0.5f * x14_lengthConstraints[14].GetLength());
  AddMinLengthConstraint(10, 13, 0.5f * x14_lengthConstraints[14].GetLength());
  AddJointConstraint(8, 2, 5, 8, 9, 10);
  AddJointConstraint(11, 2, 5, 11, 12, 13);
  AddJointConstraint(2, 11, 5, 2, 3, 4);
  AddJointConstraint(5, 2, 8, 5, 6, 7);
}

CFlyingPirate::CFlyingPirate(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                             CModelData&& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo,
                             CInputStream& in, u32 propCount)
: CPatterned(ECharacter::FlyingPirate, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Ground, EColliderType::One, EBodyType::NewFlyer, actParms, EKnockBackVariant::Medium)
, x568_data(in, propCount)
, x6a8_pathFindSearch(nullptr, 0 /* TODO */, pInfo.GetHalfExtent(), pInfo.GetHeight(), pInfo.GetPathfindingIndex())
, x7a0_boneTracking(*GetModelData()->GetAnimationData(), "Head_1", zeus::degToRad(80.f), zeus::degToRad(180.f),
                    EBoneTrackingFlags::None)
, x7ec_burstFire(skBursts.data(), 0) {
  const CModelData* modelData = GetModelData();
  const CAnimData* animData = modelData->GetAnimationData();
  x798_ = animData->GetLocatorSegId("Head_1");
  x7e0_ = animData->GetLocatorSegId("L_gun_LCTR");
  x864_missileSegments.push_back(animData->GetLocatorSegId("L_Missile_LCTR"));
  x864_missileSegments.push_back(animData->GetLocatorSegId("R_Missile_LCTR"));
  x850_ = modelData->GetScale().x() * GetAnimationDistance({3, CPASAnimParm::FromEnum(3), CPASAnimParm::FromEnum(1)});
  if (x568_data.xd8_.IsValid() && x568_data.xdc_.IsValid() && x568_data.xe0_.IsValid()) {
    x65c_particleGenDescs.push_back(g_SimplePool->GetObj({SBIG('PART'), x568_data.xd8_}));
    x65c_particleGenDescs.push_back(g_SimplePool->GetObj({SBIG('PART'), x568_data.xdc_}));
    x65c_particleGenDescs.push_back(g_SimplePool->GetObj({SBIG('PART'), x568_data.xe0_}));
    for (const auto& desc : x65c_particleGenDescs) {
      x684_particleGens.push_back(std::make_unique<CElementGen>(desc));
    }
  }
  x460_knockBackController.SetLocomotionDuringElectrocution(true);
}

void CFlyingPirate::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (msg == EScriptObjectMessage::Alert) {
    if (GetActive()) {
      x400_24_hitByPlayerProjectile = true; // ?
    }
  } else if (msg == EScriptObjectMessage::Activate) {
    AddToTeam(mgr);
  } else if (msg == EScriptObjectMessage::Deleted) {
    RemoveFromTeam(mgr);
  }
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case EScriptObjectMessage::SetToZero:
    x6a2_28_ = true;
    break;
  case EScriptObjectMessage::Start:
    x6a1_31_ = false;
    break;
  case EScriptObjectMessage::Stop:
    x6a1_31_ = true;
    break;
  case EScriptObjectMessage::OnFloor:
    x7ec_burstFire.SetBurstType(2);
    break;
  case EScriptObjectMessage::Falling:
    if (x450_bodyController->GetPercentageFrozen() == 0.f && !x400_28_pendingMassiveDeath && !x6a1_30_) {
      SetMomentumWR({0.f, 0.f, -GetGravityConstant() * xe8_mass});
    }
    break;
  case EScriptObjectMessage::Registered:
    x86c_ = x568_data.xc0_ * mgr.GetActiveRandom()->Float() + x568_data.xbc_;
    break;
  case EScriptObjectMessage::InitializedInArea:
    for (const auto& conn : x20_conns) {
      if (conn.x0_state == EScriptObjectState::Retreat) {
        if (TCastToPtr<CScriptCoverPoint> cover = mgr.ObjectById(mgr.GetIdForScript(conn.x8_objId))) {
          cover->Reserve(x8_uid);
        }
      } else if (conn.x0_state == EScriptObjectState::Patrol && conn.x4_msg == EScriptObjectMessage::Follow) {
        x6a0_27_ = true;
      } else if (conn.x0_state == EScriptObjectState::Attack && conn.x4_msg == EScriptObjectMessage::Action) {
        x85c_ = mgr.GetIdForScript(conn.x8_objId);
      }
    }
    x6a8_pathFindSearch.SetArea(mgr.GetWorld()->GetAreaAlways(x4_areaId)->GetPostConstructed()->x10bc_pathArea);
    if (x30_24_active) {
      AddToTeam(mgr);
    }
    UpdateParticleEffects(mgr, 0.f, x6a0_24_);
    GetModelData()->GetAnimationData()->SetParticleEffectState("s_Eyes", true, mgr);
    break;
  case EScriptObjectMessage::Jumped:
    if (CScriptCoverPoint* cover = GetCoverPoint(mgr, x6a4_id1)) {
      x328_25_verticalMovement = false;
      SetMomentumWR({0.f, 0.f, -xe8_mass * GetGravityConstant()});
      AddMaterial(EMaterialTypes::GroundCollider, mgr);
      SetDestPos(cover->GetTranslation());
      const zeus::CVector3f& dist = cover->GetTranslation() - GetTranslation();
      if (dist.z() < 0.f) {
        zeus::CVector3f velocity = GetVelocity();
        const float gravity = GetGravityConstant();
        float fVar1 = (2.f * gravity) * dist.z() - (velocity.z() * velocity.z());
        float fVar2 = -fVar1;
        float fVar3 = 0.f;
        if (0.f != -fVar1) {
          fVar3 = fVar2 * (1.f / std::sqrt(fVar2));
        }
        float dVar9 = (-velocity.z() + fVar3) / gravity;
        if (0.f < dVar9) {
          zeus::CVector2f dist2f(dist.x(), dist.y());
          const zeus::CVector2f& normal = dist2f.normalized();
          const float mag = dist2f.magnitude();
          velocity.x() = (mag / dVar9) * normal.x();
          velocity.y() = (mag / dVar9) * normal.y();
          SetVelocityWR(velocity);

          x870_.zeroOut();
          x87c_.zeroOut();
          x898_ = 1.f;
        }
      }
    }
    break;
  default:
    break;
  }
}

void CFlyingPirate::AddToTeam(CStateManager& mgr) {
  if (x890_teamAiMgr == kInvalidUniqueId) {
    x890_teamAiMgr = CTeamAiMgr::GetTeamAiMgr(*this, mgr);
  }
  if (x890_teamAiMgr != kInvalidUniqueId) {
    if (TCastToPtr<CTeamAiMgr> team = mgr.ObjectById(x890_teamAiMgr)) {
      team->AssignTeamAiRole(*this, CTeamAiRole::ETeamAiRole::Melee, CTeamAiRole::ETeamAiRole::Ranged,
                             CTeamAiRole::ETeamAiRole::Invalid);
    }
  }
}

void CFlyingPirate::RemoveFromTeam(CStateManager& mgr) {
  if (x890_teamAiMgr != kInvalidUniqueId) {
    if (TCastToPtr<CTeamAiMgr> team = mgr.ObjectById(x890_teamAiMgr)) {
      if (team->IsPartOfTeam(GetUniqueId())) {
        team->RemoveTeamAiRole(GetUniqueId());
        x890_teamAiMgr = kInvalidUniqueId;
      }
    }
  }
}

void CFlyingPirate::AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr) const {
  for (const auto& gen : x684_particleGens) {
    if (frustum.aabbFrustumTest(GetBoundingBox())) {
      g_Renderer->AddParticleGen(*gen);
    }
  }
  CPatterned::AddToRenderer(frustum, mgr);
}

bool CFlyingPirate::AggressionCheck(CStateManager& mgr, float arg) { return x6a2_24_; }

bool CFlyingPirate::AnimOver(CStateManager& mgr, float arg) {
  if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Death) {
    return true;
  } else {
    return CPatterned::AnimOver(mgr, arg);
  }
}

void CFlyingPirate::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case EStateMsg::Activate:
    x32c_animState = EAnimState::Ready;
    if (!x6a2_25_) {
      float fVar1;
      if (3.f <= x858_) {
        fVar1 = x568_data.xe8_;
      } else {
        fVar1 = 2.f * x568_data.xe8_;
      }
      x6a2_24_ = mgr.GetActiveRandom()->Range(0.f, 100.f) < fVar1;
      x6a2_25_ = true;
    }
    break;
  case EStateMsg::Update:
    TryCommand(mgr, pas::EAnimationState::ProjectileAttack, &CPatterned::TryProjectileAttack, 1);
    x450_bodyController->FaceDirection((mgr.GetPlayer().GetTranslation() - GetTranslation()).normalized(), arg);
    DeliverGetUp();
    UpdateCantSeePlayer(mgr);
    break;
  case EStateMsg::Deactivate:
    x32c_animState = EAnimState::NotReady;
    x6a2_24_ = false;
    break;
  default:
    break;
  }
}

bool CFlyingPirate::Attacked(CStateManager& mgr, float arg) { return x854_ < (arg != 0.f ? arg : 0.5f); }

zeus::CVector3f CFlyingPirate::AvoidActors(CStateManager& mgr) {
  const zeus::CVector3f& origin = GetTranslation();
  zeus::CAABox box(origin - 8.f, origin + 8.f);
  rstl::reserved_vector<TUniqueId, 1024> nearList;
  mgr.BuildNearList(nearList, box, CMaterialFilter::MakeInclude(EMaterialTypes::CameraPassthrough), this);

  zeus::CVector3f ret;
  for (const auto& id : nearList) {
    if (TCastToConstPtr<CActor> actor = mgr.GetObjectById(id)) {
      ret += x45c_steeringBehaviors.Separation(*this, actor->GetTranslation(), 10.f);
    }
  }
  const zeus::CVector3f& playerPos = mgr.GetPlayer().GetTranslation();
  ret += x45c_steeringBehaviors.Separation(*this, {playerPos.x(), playerPos.y(), origin.z()}, 20.f);
  return ret;
}

void CFlyingPirate::Bounce(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    CTeamAiMgr::ResetTeamAiRole(CTeamAiMgr::EAttackType::Ranged, mgr, x8_uid, x890_teamAiMgr, true);
  } else if (msg == EStateMsg::Update) {
    switch (x450_bodyController->GetCurrentStateId()) {
    case pas::EAnimationState::Locomotion:
      x330_stateMachineState.SetCodeTrigger();
      break;
    case pas::EAnimationState::LieOnGround:
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCGetupCmd(pas::EGetupType::Zero));
      break;
    case pas::EAnimationState::Hurled:
      x450_bodyController->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::ExitState));
      x328_25_verticalMovement = true;
      break;
    default:
      break;
    }
  }
}

void CFlyingPirate::CalculateRenderBounds() {
  if (!x89c_ragDoll || !x89c_ragDoll->IsPrimed()) {
    CActor::CalculateRenderBounds();
  } else {
    const zeus::CAABox& bounds = x89c_ragDoll->CalculateRenderBounds();
    const zeus::CVector3f& scale = 0.25f * GetModelData()->GetScale();
    x9c_renderBounds = {bounds.min - scale, bounds.max + scale};
  }
}

bool CFlyingPirate::CanFireMissiles(CStateManager& mgr) {
  for (const auto& seg : x864_missileSegments) {
    const zeus::CTransform& xf = GetLctrTransform(seg);
    // TODO is this right?
    const zeus::CVector3f& pos = xf.origin + (3.f * zeus::CVector3f(xf.basis[0][1], xf.basis[0][2], xf.basis[0][3]));
    CMaterialList matList(EMaterialTypes::Player, EMaterialTypes::ProjectilePassthrough);
    if (!LineOfSightTest(mgr, xf.origin, pos, matList) || !LineOfSightTest(mgr, pos, GetTargetPos(mgr), matList)) {
      x6a1_28_ = true;
      return false;
    }
  }
  return true;
}

void CFlyingPirate::CheckForProjectiles(CStateManager& mgr) {
  if (!x6a0_29_)
    return;

  const zeus::CVector3f& playerPos = mgr.GetPlayer().GetTranslation();
  zeus::CAABox box(playerPos - 5.f, playerPos + 5.f);
  x6a0_30_ = false;

  rstl::reserved_vector<TUniqueId, 1024> nearList;
  mgr.BuildNearList(nearList, box, CMaterialFilter::MakeInclude(EMaterialTypes::Projectile), this);
  for (const auto& id : nearList) {
    if (TCastToConstPtr<CGameProjectile> proj = mgr.GetObjectById(id)) {
      zeus::CVector3f dist = GetBoundingBox().center() - proj->GetTranslation();
      if (dist.isMagnitudeSafe()) {
        if (GetTranslation().dot(dist) < 0.f) {
          dist.normalize();
          zeus::CVector3f nv = proj->GetTranslation() - proj->GetPreviousPos();
          if (!nv.isMagnitudeSafe()) {
            nv.normalize();
            if (0.939f < nv.dot(dist)) {
              x6a0_30_ = true;
            }
          }
        }
      } else {
        x6a0_30_ = true;
      }
      if (x6a0_30_)
        break;
    }
  }
  x6a0_29_ = false;
}

bool CFlyingPirate::CoverCheck(CStateManager& mgr, float arg) {
  if (0.f < x888_)
    return false;
  x888_ = 10.f;
  return mgr.GetActiveRandom()->Range(0.f, 100.f) < x568_data.xcc_;
}

} // namespace urde::MP1
