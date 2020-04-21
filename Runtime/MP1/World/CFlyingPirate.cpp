#include "Runtime/MP1/World/CFlyingPirate.hpp"

#include "Runtime/Character/CPASAnimParmData.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/MP1/World/CSpacePirate.hpp"
#include "Runtime/Weapon/CEnergyProjectile.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"
#include "Runtime/World/CExplosion.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptCoverPoint.hpp"
#include "Runtime/World/CScriptWater.hpp"
#include "Runtime/World/CScriptWaypoint.hpp"
#include "Runtime/World/CTeamAiMgr.hpp"
#include "Runtime/World/CWorld.hpp"

#include <zeus/CColor.hpp>

namespace urde::MP1 {
namespace {
constexpr std::array<SBurst, 6> skBurstsFlying{{
    {4, {3, 4, 11, 12, -1, 0, 0, 0}, 0.1f, 0.05f},
    {20, {2, 3, 4, 5, -1, 0, 0, 0}, 0.1f, 0.05f},
    {20, {10, 11, 12, 13, -1, 0, 0, 0}, 0.1f, 0.05f},
    {25, {15, 16, 1, 2, -1, 0, 0, 0}, 0.1f, 0.05f},
    {25, {5, 6, 7, 8, -1, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
}};

constexpr std::array<SBurst, 6> skBurstsFlyingOutOfView{{
    {5, {3, 4, 8, 12, -1, 0, 0, 0}, 0.1f, 0.05f},
    {10, {2, 3, 4, 5, -1, 0, 0, 0}, 0.1f, 0.05f},
    {10, {10, 11, 12, 13, -1, 0, 0, 0}, 0.1f, 0.05f},
    {40, {15, 16, 1, 2, -1, 0, 0, 0}, 0.1f, 0.05f},
    {35, {5, 6, 7, 8, -1, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
}};

constexpr std::array<SBurst, 5> skBurstsLanded{{
    {30, {3, 4, 5, 11, 12, 4, -1, 0}, 0.1f, 0.05f},
    {20, {2, 3, 4, 5, 4, 3, -1, 0}, 0.1f, 0.05f},
    {20, {5, 4, 3, 13, 12, 11, -1, 0}, 0.1f, 0.05f},
    {30, {1, 2, 3, 4, 5, 6, -1, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
}};

constexpr std::array<SBurst, 5> skBurstsLandedOutOfView{{
    {10, {6, 5, 4, 14, 13, 12, -1, 0}, 0.1f, 0.05f},
    {20, {14, 13, 12, 11, 10, 9, -1, 0}, 0.1f, 0.05f},
    {20, {14, 15, 16, 11, 10, 9, -1, 0}, 0.1f, 0.05f},
    {50, {11, 10, 9, 8, 7, 6, -1, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
}};

constexpr std::array<const SBurst*, 5> skBursts{
    skBurstsFlying.data(),
    skBurstsFlyingOutOfView.data(),
    skBurstsLanded.data(),
    skBurstsLandedOutOfView.data(),
    nullptr,
};

constexpr std::array<std::string_view, 15> skParts{
    "Collar"sv, "Head_1"sv, "R_shoulder"sv, "R_elbow"sv, "R_wrist"sv, "L_shoulder"sv, "L_elbow"sv,     "L_wrist"sv,
    "R_hip"sv,  "R_knee"sv, "R_ankle"sv,    "L_hip"sv,   "L_knee"sv,  "L_ankle"sv,    "rocket_LCTR"sv,
};

constexpr std::array<float, 15> skRadii{
    0.45f, 0.52f, 0.35f, 0.1f, 0.15f, 0.35f, 0.1f, 0.15f, 0.15f, 0.15f, 0.15f, 0.15f, 0.15f, 0.15f, 0.35f,
};
} // namespace

CFlyingPirate::CFlyingPirateData::CFlyingPirateData(CInputStream& in, u32 propCount)
: x0_maxCoverDistance(in.readFloatBig())
, x4_hearingDistance(in.readFloatBig())
, x8_type(EFlyingPirateType(in.readUint32Big()))
, xc_gunProjectileInfo(in)
, x34_gunSfx(CSfxManager::TranslateSFXID(in.readUint32Big()))
, x38_altProjectileInfo1(in)
, x60_altProjectileInfo2(CAssetId(in), {})
, x88_knockBackDelay(in.readFloatBig())
, x8c_flyingHeight(in.readFloatBig())
, x90_particleGenDesc(g_SimplePool->GetObj({SBIG('PART'), CAssetId(in)}))
, x9c_dInfo(in)
, xb8_(in.readFloatBig())
, xbc_(in.readFloatBig())
, xc0_(in.readFloatBig())
, xc4_(in.readFloatBig())
, xc8_ragDollSfx1(CSfxManager::TranslateSFXID(in.readUint32Big()))
, xca_ragDollSfx2(CSfxManager::TranslateSFXID(in.readUint32Big()))
, xcc_coverCheckChance(in.readFloatBig())
, xd0_(in.readFloatBig())
, xd4_(in.readFloatBig())
, xd8_particleGen1(in)
, xdc_particleGen2(in)
, xe0_particleGen3(in)
, xe4_knockBackSfx(CSfxManager::TranslateSFXID(in.readUint32Big()))
, xe6_deathSfx(CSfxManager::TranslateSFXID(in.readUint32Big()))
, xe8_aggressionChance(in.readFloatBig())
, xec_(in.readFloatBig())
, xf0_projectileHomingDistance(propCount < 36 ? 0.f : in.readFloatBig()) {
  xc_gunProjectileInfo.Token().Lock();
  x38_altProjectileInfo1.Token().Lock();
  x60_altProjectileInfo2.Token().Lock();
}

CFlyingPirate::CFlyingPirateRagDoll::CFlyingPirateRagDoll(CStateManager& mgr, CFlyingPirate* actor, u16 w1, u16 w2)
: CRagDoll(-actor->GetGravityConstant(), 3.f, 8.f, 0)
, x6c_actor(actor)
, x88_sfx(w1)
, x9c_(w2)
, xa4_(actor->GetDestPos() - actor->GetTranslation()) {
  actor->RemoveMaterial(EMaterialTypes::Solid, EMaterialTypes::AIBlock, EMaterialTypes::GroundCollider, mgr);
  actor->HealthInfo(mgr)->SetHP(-1.f);
  SetNumParticles(skParts.size());
  SetNumLengthConstraints(45);
  SetNumJointConstraints(4);
  CModelData* modelData = actor->GetModelData();
  CAnimData* animData = modelData->GetAnimationData();
  const zeus::CVector3f& scale = modelData->GetScale();
  animData->BuildPose();
  const zeus::CVector3f& center = actor->GetBoundingBox().center();
  for (size_t i = 0; i < skParts.size(); ++i) {
    const CSegId& id = animData->GetLocatorSegId(skParts[i]);
    AddParticle(id, center, actor->GetTransform() * (scale * animData->GetPose().GetOffset(id)),
                skRadii[i] * scale.z());
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
  AddLengthConstraint(2, 8);
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
  AddMinLengthConstraint(13, 11, x14_lengthConstraints[17].GetLength());
  AddMinLengthConstraint(9, 2, 0.707f * x14_lengthConstraints[15].GetLength() + x14_lengthConstraints[10].GetLength());
  AddMinLengthConstraint(12, 5, 0.707f * x14_lengthConstraints[17].GetLength() + x14_lengthConstraints[13].GetLength());
  AddMinLengthConstraint(9, 11, x14_lengthConstraints[15].GetLength());
  AddMinLengthConstraint(12, 8, x14_lengthConstraints[17].GetLength());
  AddMinLengthConstraint(10, 0, x14_lengthConstraints[2].GetLength() + x14_lengthConstraints[15].GetLength());
  AddMinLengthConstraint(13, 0, x14_lengthConstraints[3].GetLength() + x14_lengthConstraints[17].GetLength());
  AddMinLengthConstraint(10, 13, x14_lengthConstraints[14].GetLength());
  AddMinLengthConstraint(9, 12, 0.5f * x14_lengthConstraints[14].GetLength());
  AddMinLengthConstraint(10, 12, 0.5f * x14_lengthConstraints[14].GetLength());
  AddMinLengthConstraint(13, 9, 0.5f * x14_lengthConstraints[14].GetLength());
  AddMinLengthConstraint(10, 13, 0.5f * x14_lengthConstraints[14].GetLength());
  AddJointConstraint(8, 2, 5, 8, 9, 10);
  AddJointConstraint(11, 2, 5, 11, 12, 13);
  AddJointConstraint(2, 11, 5, 2, 3, 4);
  AddJointConstraint(5, 2, 8, 5, 6, 7);
}

void CFlyingPirate::CFlyingPirateRagDoll::PreRender(const zeus::CVector3f& v, CModelData& mData) {
  if (!x68_25_over) {
    CAnimData* const animData = mData.GetAnimationData();
    const CCharLayoutInfo& layout = animData->GetCharLayoutInfo();
    CHierarchyPoseBuilder& poseBuilder = animData->PoseBuilder();
    for (const auto& id : layout.GetSegIdList().GetList()) {
      if (layout.GetRootNode()->GetBoneMap()[id].x10_children.size() > 1) {
        poseBuilder.GetTreeMap()[id].x4_rotation = zeus::CQuaternion();
      }
    }

    CHierarchyPoseBuilder::CTreeNode& skeletonRoot =
        poseBuilder.GetTreeMap()[animData->GetLocatorSegId("Skeleton_Root"sv)];
    const zeus::CVector3f& rHipPos = x4_particles[8].GetPosition();      // R_hip
    const zeus::CVector3f& lHipPos = x4_particles[11].GetPosition();     // L_hip
    const zeus::CVector3f& rShoulderPos = x4_particles[2].GetPosition(); // R_shoulder
    const zeus::CVector3f& lShoulderPos = x4_particles[5].GetPosition(); // L_shoulder
    const zeus::CVector3f& collarPos = x4_particles[0].GetPosition();    // Collar
    skeletonRoot.x14_offset = (0.5f * (rHipPos + lHipPos) - v) / mData.GetScale();

    const zeus::CVector3f& rootRight = rShoulderPos - lShoulderPos;
    const zeus::CVector3f& rootUp = (collarPos - (rHipPos + lHipPos) * 0.5f).normalized();
    const zeus::CVector3f& rootFore = rootUp.cross(rootRight).normalized();
    const zeus::CQuaternion& rootRot = zeus::CMatrix3f(rootFore.cross(rootUp), rootFore, rootUp);
    skeletonRoot.x4_rotation = rootRot;

    const CRagDollParticle& head = x4_particles[1]; // Head_1
    const zeus::CVector3f& headRestVec = layout.GetFromParentUnrotated(head.GetBone());
    poseBuilder.GetTreeMap()[head.GetBone()].x4_rotation = zeus::CQuaternion::shortestRotationArc(
        headRestVec, rootRot.inverse().transform(head.GetPosition() - collarPos));

    BoneAlign(poseBuilder, layout, 3, 4, rootRot * BoneAlign(poseBuilder, layout, 2, 3, rootRot));
    BoneAlign(poseBuilder, layout, 6, 7, rootRot * BoneAlign(poseBuilder, layout, 5, 6, rootRot));
    BoneAlign(poseBuilder, layout, 9, 10, rootRot * BoneAlign(poseBuilder, layout, 8, 9, rootRot));
    BoneAlign(poseBuilder, layout, 12, 13, rootRot * BoneAlign(poseBuilder, layout, 11, 12, rootRot));

    animData->MarkPoseDirty();
  }
}

void CFlyingPirate::CFlyingPirateRagDoll::Prime(CStateManager& mgr, const zeus::CTransform& xf, CModelData& mData) {
  if (x6c_actor->x6a1_30_spinToDeath) {
    xa0_ = CSfxManager::AddEmitter(x9c_, x6c_actor->GetTranslation(), zeus::skZero3f, true, true, 0x7f, kInvalidAreaId);
  }
  CRagDoll::Prime(mgr, xf, mData);
}

void CFlyingPirate::CFlyingPirateRagDoll::Update(CStateManager& mgr, float dt, float waterTop) {
  if (!x68_25_over) {
    if (x6c_actor->x6a1_30_spinToDeath) {
      x84_ -= dt;
      const zeus::CVector3f& v9c = (x6c_actor->x2e0_destPos - x4_particles[14].GetPosition()).normalized();
      x74_ = zeus::CVector3f::slerp(x74_, v9c, zeus::degToRad(360.f * dt));
      x70_ = 25.f;
      zeus::CVector3f mul = x70_ * x74_;
      if (x84_ <= 0.f) {
        x4_particles[14].Velocity() += 11.f * mul;
      } else {
        x4_particles[14].Velocity() += 25.f * mul;
      }
      zeus::CVector3f inv = -4 * mul;
      x4_particles[4].Velocity() += -mul;
      x4_particles[7].Velocity() += -mul;
      x4_particles[10].Velocity() += inv;
      x4_particles[10].Velocity() += inv;
      x4_particles[1].Velocity() += -mul;

      x80_ = std::min(1000.f * dt + x80_, 1000.f);

      zeus::CVector3f vc0 = ((x4_particles[5].Position() - x4_particles[2].Position())
                                 .cross(x4_particles[8].Position() - x4_particles[2].Position()) +
                             0.25f * (x4_particles[2].Position() - x4_particles[5].Position()))
                                .normalized() *
                            x80_;
      x4_particles[2].Velocity() += vc0;
      x4_particles[5].Velocity() += -vc0;

      x44_normalGravity = 0.f;
      CSfxManager::UpdateEmitter(xa0_, x6c_actor->GetTranslation(), x58_averageVel, 1.f);
    }

    // Collar-hips weighted center
    zeus::CVector3f oldTorsoCenter = x4_particles[8].GetPosition() * 0.25f + x4_particles[11].GetPosition() * 0.25f +
                                     x4_particles[0].GetPosition() * 0.5f;
    oldTorsoCenter.z() = std::min({x4_particles[0].GetPosition().z() - x4_particles[0].GetRadius(),
                                   x4_particles[8].GetPosition().z() - x4_particles[8].GetRadius(),
                                   x4_particles[11].GetPosition().z() - x4_particles[11].GetRadius()});

    CRagDoll::Update(mgr, dt, waterTop);

    // Collar-hips weighted center
    zeus::CVector3f newTorsoCenter = x4_particles[8].GetPosition() * 0.25f + x4_particles[11].GetPosition() * 0.25f +
                                     x4_particles[0].GetPosition() * 0.5f;
    newTorsoCenter.z() = std::min({x4_particles[0].GetPosition().z() - x4_particles[0].GetRadius(),
                                   x4_particles[8].GetPosition().z() - x4_particles[8].GetRadius(),
                                   x4_particles[11].GetPosition().z() - x4_particles[11].GetRadius()});
    x6c_actor->SetTransform({zeus::CMatrix3f(false), newTorsoCenter});
    x6c_actor->SetVelocityWR((newTorsoCenter - oldTorsoCenter) * (1.f / dt));

    if (x6c_actor->x6a1_30_spinToDeath) {
      if ((newTorsoCenter - x6c_actor->GetDestPos()).magSquared() > 0.f) {
        x6c_actor->x88c_ragDollTimer = 0.5f * dt;
      }
    }

    x8c_ -= dt;
    if (x54_impactVel > 2.f && x8c_ < 0.f) {
      if (xb0_24_ || (x6c_actor->GetTranslation() - x90_).magSquared() > 0.1f) {
        float vol = std::min(10.f * x54_impactVel, 127.f) / 127.f;
        CSfxManager::AddEmitter(x88_sfx, x6c_actor->GetTranslation(), zeus::skZero3f, vol, true, false, 0x7f,
                                kInvalidAreaId);
        x8c_ = 0.222f * mgr.GetActiveRandom()->Float() + 0.222f;
        xb0_24_ = false;
        x90_ = x6c_actor->GetTranslation();
      }
    }
  } else {
    x6c_actor->SetMomentumWR(zeus::skZero3f);
    x6c_actor->Stop();
  }
}

CFlyingPirate::CFlyingPirate(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                             CModelData&& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo,
                             CInputStream& in, u32 propCount)
: CPatterned(ECharacter::FlyingPirate, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Ground, EColliderType::One, EBodyType::NewFlyer, actParms, EKnockBackVariant::Medium)
, x568_data(in, propCount)
, x6a0_24_isFlyingPirate(x568_data.x8_type == EFlyingPirateType::FlyingPirate)
, x6a0_25_isAquaPirate(x568_data.x8_type == EFlyingPirateType::AquaPirate)
, x6a8_pathFindSearch(nullptr, x6a0_25_isAquaPirate ? 2 : 3, pInfo.GetHalfExtent(), pInfo.GetHeight(),
                      pInfo.GetPathfindingIndex())
, x7a0_boneTracking(*GetModelData()->GetAnimationData(), "Head_1"sv, zeus::degToRad(80.f), zeus::degToRad(180.f),
                    EBoneTrackingFlags::None)
, x7ec_burstFire(skBursts.data(), 0) {
  const CModelData* modelData = GetModelData();
  const CAnimData* animData = modelData->GetAnimationData();
  x798_headSegId = animData->GetLocatorSegId("Head_1"sv);
  x7e0_gunSegId = animData->GetLocatorSegId("L_gun_LCTR"sv);
  x864_missileSegments.push_back(animData->GetLocatorSegId("L_Missile_LCTR"sv));
  x864_missileSegments.push_back(animData->GetLocatorSegId("R_Missile_LCTR"sv));
  x850_height = modelData->GetScale().x() *
                GetAnimationDistance(CPASAnimParmData{3, CPASAnimParm::FromEnum(3), CPASAnimParm::FromEnum(1)});
  if (x568_data.xd8_particleGen1.IsValid() && x568_data.xdc_particleGen2.IsValid() &&
      x568_data.xe0_particleGen3.IsValid()) {
    x65c_particleGenDescs.push_back(g_SimplePool->GetObj({SBIG('PART'), x568_data.xd8_particleGen1}));
    x65c_particleGenDescs.push_back(g_SimplePool->GetObj({SBIG('PART'), x568_data.xdc_particleGen2}));
    x65c_particleGenDescs.push_back(g_SimplePool->GetObj({SBIG('PART'), x568_data.xe0_particleGen3}));
    for (const auto& desc : x65c_particleGenDescs) {
      x684_particleGens.push_back(std::make_unique<CElementGen>(desc));
    }
  }
  x460_knockBackController.SetLocomotionDuringElectrocution(true);
}

void CFlyingPirate::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (msg == EScriptObjectMessage::Alert) {
    if (GetActive()) {
      x400_24_hitByPlayerProjectile = true;
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
    x6a1_31_stopped = false;
    break;
  case EScriptObjectMessage::Stop:
    x6a1_31_stopped = true;
    break;
  case EScriptObjectMessage::OnFloor:
    x7ec_burstFire.SetBurstType(2);
    break;
  case EScriptObjectMessage::Falling:
    if (x450_bodyController->GetPercentageFrozen() == 0.f && !x400_28_pendingMassiveDeath && !x6a1_30_spinToDeath) {
      SetMomentumWR({0.f, 0.f, -GetGravityConstant() * xe8_mass});
    }
    x7ec_burstFire.SetBurstType(0);
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
        x6a0_27_canPatrol = true;
      } else if (conn.x0_state == EScriptObjectState::Attack && conn.x4_msg == EScriptObjectMessage::Action) {
        x85c_attackObjectId = mgr.GetIdForScript(conn.x8_objId);
      }
    }
    x6a8_pathFindSearch.SetArea(mgr.GetWorld()->GetAreaAlways(x4_areaId)->GetPostConstructed()->x10bc_pathArea);
    if (x30_24_active) {
      AddToTeam(mgr);
    }
    UpdateParticleEffects(mgr, 0.f, x6a0_24_isFlyingPirate);
    GetModelData()->GetAnimationData()->SetParticleEffectState("Eyes"sv, true, mgr);
    break;
  case EScriptObjectMessage::Jumped:
    if (CScriptCoverPoint* cover = GetCoverPoint(mgr, x6a4_currentCoverPoint)) {
      x328_25_verticalMovement = false;
      SetMomentumWR({0.f, 0.f, -xe8_mass * GetGravityConstant()});
      AddMaterial(EMaterialTypes::GroundCollider, mgr);
      SetDestPos(cover->GetTranslation());
      const zeus::CVector3f& dist = cover->GetTranslation() - GetTranslation();
      if (dist.z() < 0.f) {
        zeus::CVector3f velocity = GetVelocity();
        const float gravity = GetGravityConstant();
        float fVar2 = -(2.f * gravity) * dist.z() - (velocity.z() * velocity.z());
        float fVar3 = fVar2 != 0.f ? fVar2 * (1.f / std::sqrt(fVar2)) : 0.f;
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

void CFlyingPirate::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  for (const auto& gen : x684_particleGens) {
    if (frustum.aabbFrustumTest(GetBoundingBox())) {
      g_Renderer->AddParticleGen(*gen);
    }
  }
  CPatterned::AddToRenderer(frustum, mgr);
}

bool CFlyingPirate::AggressionCheck(CStateManager& mgr, float) { return x6a2_24_aggressive; }

bool CFlyingPirate::AnimOver(CStateManager& mgr, float arg) {
  if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Death) {
    return true;
  }
  return CPatterned::AnimOver(mgr, arg);
}

void CFlyingPirate::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case EStateMsg::Activate:
    x32c_animState = EAnimState::Ready;
    if (!x6a2_25_aggressionChecked) {
      float fVar1;
      if (3.f <= x858_) {
        fVar1 = x568_data.xe8_aggressionChance;
      } else {
        fVar1 = 2.f * x568_data.xe8_aggressionChance;
      }
      x6a2_24_aggressive = mgr.GetActiveRandom()->Range(0.f, 100.f) < fVar1;
      x6a2_25_aggressionChecked = true;
    }
    break;
  case EStateMsg::Update:
    TryCommand(mgr, pas::EAnimationState::ProjectileAttack, &CPatterned::TryProjectileAttack, 1);
    x450_bodyController->FaceDirection((mgr.GetPlayer().GetTranslation() - GetTranslation()).normalized(), arg);
    DeliverGetUp();
    UpdateCanSeePlayer(mgr);
    break;
  case EStateMsg::Deactivate:
    x32c_animState = EAnimState::NotReady;
    x6a2_24_aggressive = false;
    break;
  default:
    break;
  }
}

bool CFlyingPirate::Attacked(CStateManager& mgr, float arg) { return x854_ < (arg != 0.f ? arg : 0.5f); }

zeus::CVector3f CFlyingPirate::AvoidActors(CStateManager& mgr) {
  const zeus::CVector3f& origin = GetTranslation();
  const zeus::CAABox box(origin - 8.f, origin + 8.f);
  rstl::reserved_vector<TUniqueId, 1024> nearList;
  mgr.BuildNearList(nearList, box, CMaterialFilter::MakeInclude(EMaterialTypes::Character), this);

  zeus::CVector3f ret;
  for (const auto& id : nearList) {
    if (TCastToConstPtr<CActor> actor = mgr.GetObjectById(id)) {
      ret += x45c_steeringBehaviors.Separation(*this, actor->GetTranslation(), 10.f);
    }
  }
  const zeus::CVector3f& playerPos = mgr.GetPlayer().GetTranslation();
  return ret + x45c_steeringBehaviors.Separation(*this, {playerPos.x(), playerPos.y(), origin.z()}, 20.f);
}

void CFlyingPirate::Bounce(CStateManager& mgr, EStateMsg msg, float) {
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
    const zeus::CVector3f& dir = xf.origin + (3.f * xf.frontVector());
    CMaterialList matList(EMaterialTypes::Player, EMaterialTypes::ProjectilePassthrough);
    if (!LineOfSightTest(mgr, xf.origin, dir, matList) || !LineOfSightTest(mgr, dir, GetTargetPos(mgr), matList)) {
      x6a1_28_ = true;
      return false;
    }
  }
  return true;
}

void CFlyingPirate::CheckForProjectiles(CStateManager& mgr) {
  if (!x6a0_29_checkForProjectiles)
    return;

  const zeus::CVector3f& playerPos = mgr.GetPlayer().GetTranslation();
  const zeus::CAABox box(playerPos - 5.f, playerPos + 5.f);
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
  x6a0_29_checkForProjectiles = false;
}

bool CFlyingPirate::CoverCheck(CStateManager& mgr, float) {
  if (0.f < x888_)
    return false;
  x888_ = 10.f;
  return mgr.GetActiveRandom()->Range(0.f, 100.f) < x568_data.xcc_coverCheckChance;
}

bool CFlyingPirate::CoverFind(CStateManager& mgr, float) {
  float closestMag = x568_data.x0_maxCoverDistance * x568_data.x0_maxCoverDistance;
  CScriptCoverPoint* closest = nullptr;
  for (const auto& entity : *mgr.ObjectListById(EGameObjectList::PlatformAndDoor)) {
    if (TCastToPtr<CScriptCoverPoint> cover = entity) {
      if (cover->GetActive() && cover->ShouldLandHere() && !cover->GetInUse(x8_uid) &&
          cover->GetAreaIdAlways() == x4_areaId) {
        float mag = (GetTranslation() - cover->GetTranslation()).magSquared();
        if (mag < closestMag) {
          closest = cover;
          closestMag = mag;
        }
      }
    }
  }
  if (closest != nullptr) {
    ReleaseCoverPoint(mgr, x6a4_currentCoverPoint);
    SetCoverPoint(closest, x6a4_currentCoverPoint);
    x6a6_id2 = x6a4_currentCoverPoint;
    return true;
  }
  return false;
}

void CFlyingPirate::Deactivate(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x401_30_pendingDeath = true;
  }
}

void CFlyingPirate::Dead(CStateManager& mgr, EStateMsg msg, float arg) {
  CPatterned::Dead(mgr, msg, arg);
  if (msg == EStateMsg::Activate) {
    x7a0_boneTracking.SetActive(false);
    GetModelData()->GetAnimationData()->SetParticleEffectState("Eyes"sv, false, mgr);
    CTeamAiMgr::ResetTeamAiRole(CTeamAiMgr::EAttackType::Ranged, mgr, x890_teamAiMgr, x8_uid, true);
  }
}

void CFlyingPirate::Dodge(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    if ((x84c_dodgeDirection = GetDodgeDirection(mgr, x850_height)) == pas::EStepDirection::Invalid) {
      x84c_dodgeDirection =
          (mgr.GetActiveRandom()->Next() & 0x4000) == 0 ? pas::EStepDirection::Right : pas::EStepDirection::Left;
    }
    UpdateParticleEffects(mgr, 1.f, true);
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Step, &CPatterned::TryDodge, static_cast<int>(x84c_dodgeDirection));
    UpdateCanSeePlayer(mgr);
    x898_ = std::max(1.f, 2.f - x330_stateMachineState.GetTime());
    DeliverGetUp();
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    x6a1_28_ = false;
  }
}

void CFlyingPirate::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  if (type == EUserEventType::DeGenerate || type == EUserEventType::BecomeRagDoll) {
    if (!x89c_ragDoll && HealthInfo(mgr)->GetHP() <= 0.f) {
      x89c_ragDoll =
          std::make_unique<CFlyingPirateRagDoll>(mgr, this, x568_data.xc8_ragDollSfx1, x568_data.xca_ragDollSfx2);
    }
  } else if (type == EUserEventType::Projectile) {
    CProjectileInfo& pInfo =
        x6a1_26_isAttackingObject ? x568_data.x60_altProjectileInfo2 : x568_data.x38_altProjectileInfo1;
    if (pInfo.Token().IsLoaded() && mgr.CanCreateProjectile(x8_uid, EWeaponType::AI, 16)) {
      const zeus::CTransform& xf = GetLctrTransform(node.GetLocatorName());
      TUniqueId target = x6a1_26_isAttackingObject ? x85c_attackObjectId : mgr.GetPlayer().GetUniqueId();
      CEnergyProjectile* projectile =
          new CEnergyProjectile(true, pInfo.Token(), EWeaponType::AI, xf, EMaterialTypes::Character, pInfo.GetDamage(),
                                mgr.AllocateUniqueId(), x4_areaId, x8_uid, target, EProjectileAttrib::None, false,
                                zeus::skOne3f, std::nullopt, -1, false);
      mgr.AddObject(projectile);
      if (!x6a1_26_isAttackingObject) {
        projectile->SetCameraShake(
            CCameraShakeData::BuildPatternedExplodeShakeData(projectile->GetTranslation(), 0.3f, 0.2f, 50.f));
        if (x6a0_25_isAquaPirate) {
          projectile->SetMinHomingDistance(x568_data.xf0_projectileHomingDistance);
        }
      }
    }
  } else {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CFlyingPirate::Enraged(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg != EStateMsg::Update)
    return;

  x87c_ = (arg * arg * x568_data.xc4_) * zeus::skUp;
  x898_ = 1.f;
  x870_ += x87c_;
  x450_bodyController->GetCommandMgr().DeliverCmd(
      CBCLocomotionCmd(zeus::skUp, (GetTargetPos(mgr) - GetTranslation()).normalized(), 1.f));
}

void CFlyingPirate::Explode(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    RemoveMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, EMaterialTypes::GroundCollider, EMaterialTypes::Solid,
                   mgr);
    x150_momentum.zeroOut();
    if (!x400_27_fadeToDeath) {
      MassiveDeath(mgr);
    }
  } else if (msg == EStateMsg::Update) {
    if (x330_stateMachineState.GetTime() > 0.1f) {
      DeathDelete(mgr);
    }
  }
}

void CFlyingPirate::MassiveDeath(CStateManager& mgr) {
  CExplosion* explosion = new CExplosion(
      static_cast<const TLockedToken<CGenDescription>>(x568_data.x90_particleGenDesc), mgr.AllocateUniqueId(), true,
      {x4_areaId, CEntity::NullConnectionList}, "", GetTransform(), 0, zeus::CVector3f(1.5f), zeus::skWhite);
  mgr.AddObject(explosion);
  mgr.ApplyDamageToWorld(x8_uid, *this, GetTranslation(), x568_data.x9c_dInfo,
                         CMaterialFilter::MakeInclude({EMaterialTypes::Solid}));
  mgr.GetCameraManager()->AddCameraShaker(CCameraShakeData::BuildPatternedExplodeShakeData(0.5f, 0.3f), true);
  CPatterned::MassiveDeath(mgr);
}

void CFlyingPirate::FireProjectile(CStateManager& mgr, float dt) {
  bool projectileFired = false;
  const zeus::CTransform& xf = GetLctrTransform(x7e0_gunSegId);
  if (!x400_25_alive) {
    LaunchProjectile(xf, mgr, 8, EProjectileAttrib::None, false, std::nullopt, -1, false, zeus::skOne3f);
    projectileFired = true;
  } else {
    if (TCastToPtr<CActor> actor = mgr.ObjectById(x7e8_targetId)) {
      zeus::CVector3f origin = actor->GetTranslation();
      const CPlayer& player = mgr.GetPlayer();
      if (x7e8_targetId == player.GetUniqueId()) {
        origin = GetProjectileInfo()->PredictInterceptPos(xf.origin, player.GetAimPosition(mgr, 0.f), player, true, dt);
      }
      zeus::CVector3f dist = origin - xf.origin;
      float mag = dist.magnitude();
      float fVar13 = xf.frontVector().dot(dist * (1.f / mag));
      if (0.707f < fVar13 || (mag < 6.f && 0.5f < fVar13)) {
        if (LineOfSightTest(mgr, xf.origin, origin, {EMaterialTypes::Player, EMaterialTypes::ProjectilePassthrough})) {
          origin += GetTransform().rotate(x7ec_burstFire.GetDistanceCompensatedError(mag, 6.f));
          LaunchProjectile(zeus::lookAt(xf.origin, origin, zeus::skUp), mgr, 8, EProjectileAttrib::None, false,
                           std::nullopt, -1, false, zeus::skOne3f);
          projectileFired = true;
        }
      }
    }
  }
  if (projectileFired) {
    const std::pair<float, s32>& anim = x450_bodyController->GetPASDatabase().FindBestAnimation(
        CPASAnimParmData{24, CPASAnimParm::FromEnum(2)}, *mgr.GetActiveRandom(), -1);
    if (anim.first > 0.f) {
      GetModelData()->GetAnimationData()->AddAdditiveAnimation(anim.second, 1.f, false, true);
    }
    CSfxManager::AddEmitter(x568_data.x34_gunSfx, GetTranslation(), zeus::skZero3f, true, false, 0x7f, kInvalidAreaId);
  }
}

pas::EStepDirection CFlyingPirate::GetDodgeDirection(CStateManager& mgr, float arg) {
  float argSquared = arg * arg;
  bool canDodgeLeft = true;
  bool canDodgeRight = true;
  bool canDodgeUp = true;
  bool canDodgeDown = true;
  pas::EStepDirection direction = pas::EStepDirection::Invalid;
  for (const auto& entity : *mgr.ObjectListById(EGameObjectList::AiWaypoint)) {
    if (entity == this)
      continue;
    if (TCastToPtr<CPhysicsActor> actor = entity) {
      const zeus::CVector3f& dist = actor->GetTranslation() - GetTranslation();
      float distMagSquared = dist.magSquared();
      if (distMagSquared < argSquared) {
        float rightVecMag = GetTransform().rightVector().magSquared();
        if ((0.866f * distMagSquared) < rightVecMag || (0.f < rightVecMag && distMagSquared < 3.f)) {
          canDodgeRight = false;
        } else if (rightVecMag < 0.866f * -distMagSquared || (rightVecMag < 0.f && distMagSquared < 3.f)) {
          canDodgeLeft = false;
        }
        float upVecMag = GetTransform().upVector().magSquared();
        if ((0.866f * distMagSquared) < upVecMag || (0.f < upVecMag && distMagSquared < 3.f)) {
          canDodgeUp = false;
        } else if (upVecMag < 0.866f * -distMagSquared || (0.f < upVecMag && distMagSquared < 3.f)) {
          canDodgeDown = false;
        }
      }
    }
  }

  const zeus::CVector3f& center = GetBoundingBox().center();
  if (canDodgeRight) {
    canDodgeRight = LineOfSightTest(mgr, center, center + (arg * GetTransform().rightVector()), {});
  }
  if (canDodgeLeft) {
    canDodgeLeft = LineOfSightTest(mgr, center, center - (arg * GetTransform().rightVector()), {});
  }
  if (canDodgeUp) {
    canDodgeUp = LineOfSightTest(mgr, center, center + (arg * GetTransform().upVector()), {});
  }
  if (canDodgeDown) {
    canDodgeDown = LineOfSightTest(mgr, center, center - (arg * GetTransform().upVector()), {});
  }

  if ((canDodgeLeft || canDodgeRight) && (canDodgeUp || canDodgeDown)) {
    if ((mgr.GetActiveRandom()->Next() & 0x4000) == 0) {
      canDodgeUp = false;
      canDodgeDown = false;
    } else {
      canDodgeLeft = false;
      canDodgeRight = false;
    }
  }
  if (canDodgeLeft && canDodgeRight) {
    if ((mgr.GetActiveRandom()->Next() & 0x4000) == 0) {
      canDodgeRight = false;
    } else {
      canDodgeLeft = false;
    }
  }
  if (canDodgeUp && canDodgeDown) {
    const zeus::CVector3f& target = GetTargetPos(mgr);
    if (target.z() - (GetTranslation().z() + x568_data.x8c_flyingHeight) <= 0.f) {
      canDodgeUp = false;
    } else {
      canDodgeDown = false;
    }
  }

  if (canDodgeUp) {
    direction = pas::EStepDirection::Up;
  } else if (canDodgeDown) {
    direction = pas::EStepDirection::Down;
  } else if (canDodgeLeft) {
    direction = pas::EStepDirection::Left;
  } else if (canDodgeRight) {
    direction = pas::EStepDirection::Right;
  }
  return direction;
}

zeus::CVector3f CFlyingPirate::GetTargetPos(CStateManager& mgr) {
  const CPlayer& player = mgr.GetPlayer();
  const TUniqueId& playerUid = player.GetUniqueId();
  if (x7e8_targetId != playerUid) {
    if (TCastToPtr<CActor> actor = mgr.ObjectById(x7e8_targetId)) {
      if (actor->GetActive()) {
        return actor->GetTranslation();
      }
    }
    x7a0_boneTracking.SetTarget(playerUid);
    x7e8_targetId = playerUid;
  }
  return player.GetAimPosition(mgr, 0.f);
}

void CFlyingPirate::GetUp(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    CTeamAiMgr::ResetTeamAiRole(CTeamAiMgr::EAttackType::Ranged, mgr, x890_teamAiMgr, x8_uid, true);
  } else if (msg == EStateMsg::Update) {
    if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::LieOnGround) {
      // will always return Success?
      CPathFindSearch::EResult result = x6a8_pathFindSearch.Search(GetTranslation(), GetTranslation());
      if (result == CPathFindSearch::EResult::NoSourcePoint) {
        x401_30_pendingDeath = true;
      }
    }
    TryCommand(mgr, pas::EAnimationState::Getup, &CPatterned::TryGetUp, 0);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
  }
}

bool CFlyingPirate::HearPlayer(CStateManager& mgr, float) {
  const CPlayer& player = mgr.GetPlayer();
  const float hearingDist = x568_data.x4_hearingDistance * x568_data.x4_hearingDistance;
  return player.GetVelocity().magSquared() > 0.1f &&
         (player.GetTranslation() - GetTranslation()).magSquared() < hearingDist;
}

bool CFlyingPirate::HearShot(CStateManager& mgr, float) { return x6a0_26_hearShot; }

bool CFlyingPirate::InPosition(CStateManager& mgr, float) {
  CScriptCoverPoint* const cover = GetCoverPoint(mgr, x6a4_currentCoverPoint);
  if (cover == nullptr) {
    return true;
  }
  const zeus::CVector3f& dist = cover->GetTranslation() - GetTranslation();
  return dist.z() < 0.f && dist.magnitude() < 4.f;
}

bool CFlyingPirate::InRange(CStateManager& mgr, float) {
  const CPlayer& player = mgr.GetPlayer();
  const zeus::CVector3f& playerPos = player.GetTranslation();
  return std::abs(playerPos.z()) < x2fc_minAttackRange &&
         playerPos.magSquared() < x300_maxAttackRange * x300_maxAttackRange;
}

void CFlyingPirate::Jump(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Combat);
    x328_25_verticalMovement = true;
    x150_momentum.zeroOut();
    x888_ = 10.f;
    UpdateParticleEffects(mgr, 1.f, true);
    UpdateLandingSmoke(mgr, true);
    x6a2_24_aggressive = x568_data.xec_ > mgr.GetActiveRandom()->Range(0.f, 100.f);
  } else if (msg == EStateMsg::Deactivate) {
    UpdateParticleEffects(mgr, 0.5f, true);
    UpdateLandingSmoke(mgr, false);
    x6a2_24_aggressive = false;
  }
}

void CFlyingPirate::KnockBack(const zeus::CVector3f& pos, CStateManager& mgr, const CDamageInfo& info,
                              EKnockBackType type, bool inDeferred, float magnitude) {
  if (x400_25_alive) {
    x460_knockBackController.SetSeverity(x328_25_verticalMovement ? pas::ESeverity::Zero : pas::ESeverity::One);
  } else if (!IsOnGround()) {
    const float rand = mgr.GetActiveRandom()->Range(0.f, 100.f);
    if (x568_data.xb8_ <= rand) {
      UpdateParticleEffects(mgr, 0.f, false);
      SetMomentumWR({0.f, 0.f, -GetGravityConstant() * xe8_mass});
    } else {
      x6a1_30_spinToDeath = true;
      x150_momentum.zeroOut();
    }
    x460_knockBackController.SetAnimationStateRange(EKnockBackAnimationState::Hurled, EKnockBackAnimationState::Hurled);
    x328_25_verticalMovement = false;
    // const TUniqueId& waypointId = GetWaypointForState(mgr, EScriptObjectState::Retreat, EScriptObjectMessage::Next);
    // if (waypointId != kInvalidUniqueId) {
    //   casts and then does nothing?
    // }
    const zeus::CVector3f& homingPosition = mgr.GetPlayer().GetHomingPosition(mgr, 0.f);
    const zeus::CVector3f& homingDist = homingPosition - GetTranslation();
    zeus::CVector3f cross = homingDist.cross(zeus::skUp);
    if (zeus::close_enough(cross, zeus::skZero3f, 1.0E-4f)) {
      cross = homingDist.cross(zeus::skForward);
    }
    SetDestPos(homingPosition + (homingDist.y() * cross.normalized()));
    x7a0_boneTracking.SetActive(false);
  }
  CPatterned::KnockBack(pos, mgr, info, type, inDeferred, magnitude);
  if (x460_knockBackController.GetActiveParms().x0_animState == EKnockBackAnimationState::Hurled) {
    if (x400_25_alive) {
      if (!x450_bodyController->IsFrozen()) {
        x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), "GetUpNow"sv);
        x330_stateMachineState.SetDelay(x568_data.x88_knockBackDelay);
      }
      x6a1_28_ = false;
      x328_25_verticalMovement = false;
      CSfxManager::AddEmitter(x568_data.xe4_knockBackSfx, GetTranslation(), zeus::skZero3f, 1.f, true, false, 0x7f,
                              kInvalidAreaId);
    } else {
      CSfxManager::AddEmitter(x568_data.xe6_deathSfx, GetTranslation(), zeus::skZero3f, 1.f, true, false, 0x7f,
                              kInvalidAreaId);
      if (x400_27_fadeToDeath) {
        x6a1_30_spinToDeath = false;
        UpdateParticleEffects(mgr, 0.f, false);
        SetMomentumWR({0.f, 0.f, -GetGravityConstant() * xe8_mass});
      }
    }
  }
}

void CFlyingPirate::Land(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    UpdateLandingSmoke(mgr, true);
    UpdateParticleEffects(mgr, 1.f, true);
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Jump, &CPatterned::TryJump, 0);
    if (x32c_animState == EAnimState::Repeat) {
      x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    }
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    UpdateLandingSmoke(mgr, false);
    UpdateParticleEffects(mgr, 0.f, false);
  }
}

bool CFlyingPirate::Landed(CStateManager& mgr, float) {
  return x450_bodyController->GetCurrentStateId() == pas::EAnimationState::LieOnGround;
}

bool CFlyingPirate::LineOfSight(CStateManager& mgr, float) { return !x6a0_31_canSeePlayer; }

bool CFlyingPirate::LineOfSightTest(CStateManager& mgr, const zeus::CVector3f& start, const zeus::CVector3f& end,
                                    CMaterialList exclude) {
  return mgr.RayCollideWorld(start, end, CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, exclude), this);
}

bool CFlyingPirate::Listen(const zeus::CVector3f& pos, EListenNoiseType type) {
  bool ret = false;
  if (x400_25_alive) {
    float x4Squared = x568_data.x4_hearingDistance * x568_data.x4_hearingDistance;
    const zeus::CVector3f& dist = pos - GetTranslation();
    if (dist.magSquared() < x4Squared && (x3c0_detectionHeightRange == 0.f || (dist.z() * dist.z() < x4Squared))) {
      ret = true;
      x6a0_26_hearShot = true;
    }
    if (type == EListenNoiseType::PlayerFire) {
      x6a0_29_checkForProjectiles = true;
    }
  }
  return ret;
}

void CFlyingPirate::Lurk(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    ReleaseCoverPoint(mgr, x6a4_currentCoverPoint);
    x6a0_31_canSeePlayer = true;
    x7d8_ = 0.f;
    x7dc_ = 0;
    CTeamAiMgr::ResetTeamAiRole(CTeamAiMgr::EAttackType::Ranged, mgr, x890_teamAiMgr, x8_uid, true);
    x330_stateMachineState.SetDelay(3.f);
    UpdateParticleEffects(mgr, 0.f, true);
    x6a2_25_aggressionChecked = false;
  } else if (msg == EStateMsg::Update) {
    UpdateCanSeePlayer(mgr);
    if (x32c_animState != EAnimState::NotReady) {
      TryCommand(mgr, pas::EAnimationState::Turn, &CPatterned::TryTurn, 0);
    }
    if (x32c_animState != EAnimState::Repeat) {
      x2e0_destPos = GetTargetPos(mgr);
      zeus::CVector3f dist = x2e0_destPos - GetTranslation();
      dist.z() = 0.f;
      if (GetTransform().frontVector().dot(dist.normalized()) < 0.8f) {
        x32c_animState = EAnimState::Ready;
      }
    }
  } else if (msg == EStateMsg::Deactivate) {
    x6a1_25_ = false;
    x6a1_28_ = false;
    x32c_animState = EAnimState::NotReady;
  }
}

void CFlyingPirate::PathFind(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    zeus::CVector3f target = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
    if (!x6a1_29_isMoving) {
      if (const CScriptCoverPoint* const cover = GetCoverPoint(mgr, x6a4_currentCoverPoint)) {
        target = cover->GetTranslation();
      }
    } else {
      target = x2e0_destPos;
    }
    CPathFindSearch* search = GetSearchPath();
    CPathFindSearch::EResult result = search->Search(GetTranslation(), target);
    if (result != CPathFindSearch::EResult::Success &&
        (result == CPathFindSearch::EResult::NoDestPoint || result == CPathFindSearch::EResult::NoPath)) {
      result = search->FindClosestReachablePoint(GetTranslation(), target);
      if (result == CPathFindSearch::EResult::Success) {
        search->Search(GetTranslation(), target);
      }
    }
    UpdateParticleEffects(mgr, 0.5f, true);
  } else if (msg == EStateMsg::Update) {
    zeus::CVector3f move = zeus::skZero3f;
    CPathFindSearch* search = GetSearchPath();
    if (search->GetResult() == CPathFindSearch::EResult::Success &&
        search->GetCurrentWaypoint() < search->GetWaypoints().size() - 1) {
      zeus::CVector3f out = GetTranslation();
      const zeus::CVector3f& front = out + GetTransform().frontVector();
      search->GetSplinePointWithLookahead(out, front, 3.f);
      if (search->SegmentOver(out)) {
        search->SetCurrentWaypoint(search->GetCurrentWaypoint() + 1);
      }
      move = front - GetTranslation();
      if (move.canBeNormalized()) {
        move.normalize();
      }
    }
    move += 3.f * AvoidActors(mgr);
    if (move.canBeNormalized()) {
      move.normalize();
    }

    float fVar1 = 1.f;
    if (x858_ < 2.f) {
      fVar1 = 4.f;
    }
    float fVar2 = 1.5f * fVar1;
    fVar1 = arg * (arg * (fVar1 * x568_data.xc4_));
    x87c_ = fVar1 * move;
    x898_ = fVar2;
    x870_ += x87c_;

    const zeus::CVector3f& face = (GetTargetPos(mgr) - GetTranslation()).normalized();
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(move, face, 1.f));
    UpdateCanSeePlayer(mgr);
  } else if (msg == EStateMsg::Deactivate) {
    x6a1_29_isMoving = false;
  }
}

void CFlyingPirate::Patrol(CStateManager& mgr, EStateMsg msg, float arg) {
  if (!x6a0_27_canPatrol)
    return;

  CPatterned::Patrol(mgr, msg, arg);
  if (msg == EStateMsg::Activate) {
    x450_bodyController->GetCommandMgr().SetSteeringBlendMode(ESteeringBlendMode::FullSpeed);
    x8a0_patrolTarget = x2dc_destObj;
    x8a4_ = 0.f;
  } else if (msg == EStateMsg::Update) {
    if (x8a0_patrolTarget != x2dc_destObj) {
      x8a0_patrolTarget = x2dc_destObj;
      x8a4_ = 0.f;
    }
    if (x2d8_patrolState == EPatrolState::Patrol) {
      float f78 = x3b0_moveSpeed * x568_data.xc4_;
      x8a4_ = std::min(f78, arg * f78 + x8a4_);
      x87c_ = (arg * x8a4_ * arg) * (x2e0_destPos - GetTranslation()).normalized();
      x898_ = 1.5f * x3b0_moveSpeed;
      x870_ += x87c_;
    }
    if (x30c_behaviourOrient == EBehaviourOrient::Constant) {
      x450_bodyController->GetCommandMgr().DeliverTargetVector(GetTargetPos(mgr) - GetTranslation());
    }
    UpdateCanSeePlayer(mgr);
  } else if (msg == EStateMsg::Deactivate) {
    x450_bodyController->GetCommandMgr().SetSteeringBlendMode(ESteeringBlendMode::Normal);
  }
}

bool CFlyingPirate::PatternOver(CStateManager& mgr, float) { return x2dc_destObj == kInvalidUniqueId; }

void CFlyingPirate::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  CModelData* modelData = GetModelData();
  if (x89c_ragDoll && x89c_ragDoll->IsPrimed()) {
    x89c_ragDoll->PreRender(GetTranslation(), *modelData);
  }
  CPatterned::PreRender(mgr, frustum);
  x7a0_boneTracking.PreRender(mgr, *modelData->GetAnimationData(), GetTransform(), modelData->GetScale(),
                              *x450_bodyController);
}

void CFlyingPirate::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x6a1_26_isAttackingObject = true;
    x32c_animState = EAnimState::Ready;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::ProjectileAttack, &CPatterned::TryProjectileAttack, 0);
    DeliverGetUp();
  } else if (msg == EStateMsg::Deactivate) {
    x6a1_26_isAttackingObject = false;
    x32c_animState = EAnimState::NotReady;
  }
}

void CFlyingPirate::Retreat(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    const zeus::CVector3f& origin = GetTranslation();
    const zeus::CVector3f& playerOrigin = mgr.GetPlayer().GetTranslation();
    const zeus::CVector3f& dist = (playerOrigin - origin).normalized();
    zeus::CVector3f target{
        origin.x() - x2fc_minAttackRange * dist.x(),
        origin.y() - x2fc_minAttackRange * dist.y(),
        playerOrigin.z() + x568_data.x8c_flyingHeight,
    };
    CPathFindSearch* const search = GetSearchPath();
    if (search->OnPath(target) == CPathFindSearch::EResult::NoSourcePoint) {
      search->FindClosestReachablePoint(origin, target);
      target.z() += x568_data.x8c_flyingHeight;
      if ((playerOrigin - target).magSquared() < 0.25f * x2fc_minAttackRange * x2fc_minAttackRange) {
        target.z() -= x568_data.x8c_flyingHeight; // can just subtract instead of recreating dist/target
        if (search->OnPath(target) == CPathFindSearch::EResult::NoSourcePoint) {
          search->FindClosestReachablePoint(origin, target);
          target.z() += x568_data.x8c_flyingHeight;
        }
      }
    }
    search->Search(origin, target);
    UpdateParticleEffects(mgr, 0.5f, true);
  } else if (msg == EStateMsg::Update) {
    zeus::CVector3f move = zeus::skZero3f;
    CPathFindSearch* const search = GetSearchPath();
    if (search->GetCurrentWaypoint() < search->GetWaypoints().size() - 1) {
      const zeus::CVector3f& origin = GetTranslation();
      zeus::CVector3f out = origin + GetTransform().frontVector();
      search->GetSplinePointWithLookahead(out, origin, 3.f);
      if (search->SegmentOver(out)) {
        search->SetCurrentWaypoint(search->GetCurrentWaypoint() + 1);
      }
      move = out - origin;
      if (move.canBeNormalized()) {
        move.normalize();
      }
    }
    move += 3.f * AvoidActors(mgr);
    if (move.canBeNormalized()) {
      move.normalize();
    }

    float fVar1 = 1.f;
    if (x858_ < 2.f) {
      fVar1 = 4.f;
    }
    float fVar2 = 1.5f * fVar1;
    fVar1 = arg * (arg * (fVar1 * x568_data.xc4_));
    x87c_ = fVar1 * move;
    x898_ = fVar2;
    x870_ += x87c_;

    const zeus::CVector3f& face = (GetTargetPos(mgr) - GetTranslation()).normalized();
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(move, face, 1.f));
    UpdateCanSeePlayer(mgr);
  }
}

bool CFlyingPirate::ShotAt(CStateManager& mgr, float arg) { return x858_ < (arg != 0.f ? arg : 0.5f); }

bool CFlyingPirate::ShouldAttack(CStateManager& mgr, float) {
  CTeamAiRole* const role = CTeamAiMgr::GetTeamAiRole(mgr, x890_teamAiMgr, x8_uid);
  const CPlayer& player = mgr.GetPlayer();
  if ((role == nullptr || role->GetTeamAiRole() == CTeamAiRole::ETeamAiRole::Ranged) &&
      x7e8_targetId == player.GetUniqueId() && (x86c_ <= 0.f || x854_ < 1.f) && CanFireMissiles(mgr)) {
    const zeus::CVector3f& dist = player.GetTranslation() - GetTranslation();
    if (dist.z() * dist.z() < dist.y() * dist.y() + dist.x() * dist.x()) {
      if (x890_teamAiMgr != kInvalidUniqueId) {
        if (!CTeamAiMgr::AddAttacker(CTeamAiMgr::EAttackType::Ranged, mgr, x890_teamAiMgr, x8_uid)) {
          return false;
        }
      }
      x86c_ = x568_data.xc0_ * mgr.GetActiveRandom()->Float() + x568_data.xbc_;
      return true;
    }
  }
  return false;
}

bool CFlyingPirate::ShouldDodge(CStateManager& mgr, float) {
  if (x6a1_28_ || x6a1_25_)
    return false;
  return 0.f < (GetTargetPos(mgr) - GetTranslation()).dot(GetTransform().frontVector()) &&
         (x854_ < 0.33f || x858_ < 0.33f) && x7d8_ < 0.5f;
}

bool CFlyingPirate::ShouldMove(CStateManager& mgr, float) {
  const CPlayer& player = mgr.GetPlayer();
  const zeus::CVector3f& origin = GetTranslation();
  const zeus::CVector3f& playerOrigin = player.GetTranslation();
  const zeus::CVector3f& dist = origin - playerOrigin;

  CRandom16* activeRandom = mgr.GetActiveRandom();
  float rand = activeRandom->Float();
  if (0.5f <= rand) {
    rand = activeRandom->Range(15.f, 25.f);
  } else {
    rand = activeRandom->Range(-25.f, -15.f);
  }

  const zeus::CVector3f& cross = dist.cross(zeus::skUp).normalized();
  SetDestPos({
      origin.x() + (rand * cross.x()),
      origin.y() + (rand * cross.y()),
      playerOrigin.z() + x568_data.x8c_flyingHeight,
  });
  x6a1_29_isMoving = true;
  return true;
}

bool CFlyingPirate::ShouldRetreat(CStateManager& mgr, float) {
  if (!x6a2_28_)
    return false;

  TUniqueId id = GetWaypointForState(mgr, EScriptObjectState::Patrol, EScriptObjectMessage::Follow);
  TCastToPtr<CScriptWaypoint> waypoint = mgr.ObjectById(id);
  if (!waypoint) {
    id = GetWaypointForState(mgr, EScriptObjectState::Retreat, EScriptObjectMessage::Follow);
    waypoint = mgr.ObjectById(id);
  }
  if (waypoint) {
    x6a2_28_ = false;
    x2dc_destObj = id;
    SetDestPos(waypoint->GetTranslation());
    x2ec_reflectedDestPos = GetTranslation();
    x328_24_inPosition = false;
    x6a1_29_isMoving = true;
    x6a0_26_hearShot = false;
    x6a0_28_ = false;
    x400_24_hitByPlayerProjectile = false;
    return true;
  }
  return false;
}

bool CFlyingPirate::ShouldSpecialAttack(CStateManager& mgr, float) {
  if (x3fc_flavor != EFlavorType::One || x85c_attackObjectId == kInvalidUniqueId || x860_ > 0.f)
    return false;

  x860_ = 15.f * mgr.GetActiveRandom()->Float() + 15.f;
  if (!mgr.GetPlayer().CheckOrbitDisableSourceList()) {
    if (TCastToPtr<CActor> actor = mgr.ObjectById(x85c_attackObjectId)) {
      if (x890_teamAiMgr != kInvalidUniqueId) {
        if (!CTeamAiMgr::AddAttacker(CTeamAiMgr::EAttackType::Ranged, mgr, x890_teamAiMgr, x8_uid)) {
          return false;
        }
      }
      SetDestPos(actor->GetTranslation() + (15.f * zeus::skDown));
      x6a1_29_isMoving = true;
      return true;
    }
  }
  return false;
}

bool CFlyingPirate::SpotPlayer(CStateManager& mgr, float) {
  const zeus::CVector3f& dir = mgr.GetPlayer().GetAimPosition(mgr, 0.f) - GetGunEyePos();
  return dir.magnitude() * x3c4_detectionAngle < dir.dot(GetTransform().frontVector());
}

bool CFlyingPirate::Stuck(CStateManager& mgr, float arg) {
  if (x330_stateMachineState.GetTime() < 0.5f)
    return false;
  return CPatterned::Stuck(mgr, arg) || GetSearchPath()->GetResult() != CPathFindSearch::EResult::Success;
}

void CFlyingPirate::UpdateLandingSmoke(CStateManager& mgr, bool active) {
  if (active) {
    if (x684_particleGens.size()) {
      const zeus::CVector3f& origin = GetTranslation();
      float particleLevel = origin.z() - 5.f;
      CScriptCoverPoint* const cover = GetCoverPoint(mgr, x6a4_currentCoverPoint);
      if (cover != nullptr) {
        particleLevel = cover->GetTranslation().z() - 1.f;
      }
      const CRayCastResult& result = mgr.RayStaticIntersection(origin, zeus::skDown, origin.z() - particleLevel,
                                                               CMaterialFilter::MakeInclude({EMaterialTypes::Solid}));
      int idx = 1;
      if (result.IsValid()) {
        const CMaterialList& list = result.GetMaterial();
        if (!list.HasMaterial(EMaterialTypes::Ice) && !list.HasMaterial(EMaterialTypes::Snow)) {
          if (list.HasMaterial(EMaterialTypes::Dirt) || list.HasMaterial(EMaterialTypes::MudSlow) ||
              list.HasMaterial(EMaterialTypes::Sand)) {
            idx = 0;
          }
        } else {
          idx = 2;
        }
        particleLevel = origin.z() - result.GetT();
      }
      x684_particleGens[idx]->SetParticleEmission(true);
      x684_particleGens[idx]->SetTranslation({origin.x(), origin.y(), particleLevel});
    }
    GetModelData()->GetAnimationData()->SetParticleEffectState("LandingSmoke"sv, true, mgr);
  } else {
    for (const auto& gen : x684_particleGens) {
      gen->SetParticleEmission(false);
    }
    GetModelData()->GetAnimationData()->SetParticleEffectState("LandingSmoke"sv, false, mgr);
  }
}

void CFlyingPirate::UpdateParticleEffects(CStateManager& mgr, float intensity, bool active) {
  CAnimData* const animData = GetModelData()->GetAnimationData();
  std::string_view name = x6a0_25_isAquaPirate ? "ScubaGear"sv : "JetPack"sv;
  if (active != x6a2_26_jetpackActive) {
    animData->SetParticleEffectState(name, active, mgr);
    if (x6a0_25_isAquaPirate) {
      animData->SetParticleEffectState("ScubaBubbles"sv, active, mgr);
    }
    x6a2_26_jetpackActive = active;
  }
  if (active) {
    animData->SetParticleCEXTValue(name, 0, 0.75f * intensity + 2.25f);
    animData->SetParticleCEXTValue(name, 1, -0.13f * intensity + -0.1f);
  }
  if (!x6a0_25_isAquaPirate) {
    const bool sparksActive = active && intensity > 0.8f;
    if (sparksActive != x6a2_27_sparksActive) {
      animData->SetParticleEffectState("Sparks"sv, sparksActive, mgr);
      x6a2_27_sparksActive = sparksActive;
    }
  }
}

void CFlyingPirate::DeliverGetUp() {
  if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::LieOnGround) {
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCGetupCmd(pas::EGetupType::Zero));
  }
}

void CFlyingPirate::UpdateCanSeePlayer(CStateManager& mgr) {
  if (x7dc_ % 7 == 0) {
    bool bVar4 = true;
    const zeus::CVector3f& start = GetGunEyePos() - GetTransform().rightVector();
    const zeus::CVector3f& end = GetAimPosition(mgr, 0.f);
    const CMaterialList matList(EMaterialTypes::Player, EMaterialTypes::ProjectilePassthrough);
    if (LineOfSightTest(mgr, start, end, matList)) {
      bVar4 = !LineOfSightTest(mgr, start, end, matList);
    }
    x6a0_31_canSeePlayer = bVar4;
  }
  x7dc_++;
}

void CFlyingPirate::TargetPatrol(CStateManager& mgr, EStateMsg msg, float arg) {
  CPatterned::Patrol(mgr, msg, arg);

  if (msg == EStateMsg::Activate) {
    x450_bodyController->GetCommandMgr().SetSteeringBlendMode(ESteeringBlendMode::Normal);
    x2dc_destObj = GetWaypointForState(mgr, EScriptObjectState::Attack, EScriptObjectMessage::Follow);
    if (x2dc_destObj != kInvalidUniqueId) {
      if (TCastToPtr<CScriptWaypoint> waypoint = mgr.ObjectById(x2dc_destObj)) {
        x30c_behaviourOrient = EBehaviourOrient(waypoint->GetBehaviourOrient());
        x3b0_moveSpeed = waypoint->GetSpeed();
      }
    }
    x8a0_patrolTarget = x2dc_destObj;
    x8a4_ = 0.f;
  } else if (msg == EStateMsg::Update) {
    if (x2dc_destObj != x8a0_patrolTarget) {
      x8a0_patrolTarget = x2dc_destObj;
      x8a4_ = 0.f;
    }
    if (x2d8_patrolState == EPatrolState::Patrol) {
      float f80 = x3b0_moveSpeed * x568_data.xc4_;
      x8a4_ = std::min(arg * f80 + x8a4_, f80);
      x87c_ = (arg * x8a4_ * arg) * (x2e0_destPos - GetTranslation()).normalized();
      x898_ = 1.5f * x3b0_moveSpeed;
      x870_ += x87c_;
    }
    if (x30c_behaviourOrient == EBehaviourOrient::Constant) {
      x450_bodyController->GetCommandMgr().DeliverTargetVector(GetTargetPos(mgr) - GetTranslation());
    }
    UpdateCanSeePlayer(mgr);
  }
}

void CFlyingPirate::Taunt(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x6a0_28_ = true;
    x7a0_boneTracking.SetActive(true);
    const TUniqueId& playerUid = mgr.GetPlayer().GetUniqueId();
    x7a0_boneTracking.SetTarget(playerUid);
    bool foundPirate = false;
    for (const auto& obj : *mgr.ObjectListById(EGameObjectList::AiWaypoint)) {
      if (const CSpacePirate* const pirate = CPatterned::CastTo<CSpacePirate>(obj)) {
        if (pirate->GetEnableAim() && pirate->IsAlive() && pirate->GetAreaIdAlways() == x4_areaId &&
            (pirate->GetTranslation() - GetTranslation()).magSquared() <
                x568_data.x4_hearingDistance * x568_data.x4_hearingDistance) {
          foundPirate = true;
        }
      }
    }
    x79c_ = foundPirate ? 0 : 1;
    if (x7e8_targetId == kInvalidUniqueId) {
      x7e8_targetId = playerUid;
    }
  } else if (msg == EStateMsg::Deactivate) {
    if (x79c_ == 0) {
      mgr.InformListeners(GetTranslation(), EListenNoiseType::PlayerFire);
    }
  }
}

void CFlyingPirate::TurnAround(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x2e0_destPos = GetTargetPos(mgr);
    zeus::CVector3f dist = x2e0_destPos - GetTranslation();
    dist.z() = 0.f;
    if (GetTransform().frontVector().dot(dist.normalized()) < 0.8f) {
      x32c_animState = EAnimState::Ready;
    }
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Turn, &CPatterned::TryTurn, 0);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
  }
}

void CFlyingPirate::Walk(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    UpdateParticleEffects(mgr, 0.f, false);
  } else if (msg == EStateMsg::Update) {
    if (x32c_animState != EAnimState::NotReady) {
      TryCommand(mgr, pas::EAnimationState::Turn, &CPatterned::TryTurn, 0);
    }
    if (x32c_animState != EAnimState::Repeat) {
      x2e0_destPos = GetTargetPos(mgr);
      zeus::CVector3f dist = x2e0_destPos - GetTranslation();
      dist.z() = 0.f;
      if (GetTransform().frontVector().dot(dist.normalized()) < 0.8f) {
        x32c_animState = EAnimState::Ready;
      }
    }
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Combat);
    x328_25_verticalMovement = true;
    x150_momentum.zeroOut();
  }
}

void CFlyingPirate::Think(float dt, CStateManager& mgr) {
  if (!x30_24_active)
    return;

  if (!x450_bodyController->GetActive()) {
    x450_bodyController->Activate(mgr);
    if (x6a0_24_isFlyingPirate) {
      x450_bodyController->SetLocomotionType(pas::ELocomotionType::Combat);
      x328_25_verticalMovement = true;
    }
  }

  bool inCineCam = mgr.GetCameraManager()->IsInCinematicCamera();
  if (inCineCam && !x6a1_24_prevInCineCam) {
    RemoveMaterial(EMaterialTypes::AIBlock, mgr);
    CMaterialFilter filter = GetMaterialFilter();
    filter.IncludeList().Remove(EMaterialTypes::AIBlock);
    SetMaterialFilter(filter);
  } else if (!inCineCam && x6a1_24_prevInCineCam) {
    AddMaterial(EMaterialTypes::AIBlock, mgr);
    CMaterialFilter filter = GetMaterialFilter();
    filter.IncludeList().Add(EMaterialTypes::AIBlock);
    SetMaterialFilter(filter);
  }
  x6a1_24_prevInCineCam = inCineCam;

  for (const auto& gen : x684_particleGens) {
    gen->Update(dt);
  }

  x78c_ = std::max(0.f, x78c_ - dt);
  if (x400_25_alive) {
    if (x6a0_30_) {
      x858_ = 0.f;
      x6a0_30_ = false;
    } else {
      x858_ += dt;
    }
    if (x400_24_hitByPlayerProjectile) {
      x854_ = 0.f;
      x400_24_hitByPlayerProjectile = false;
    } else {
      x854_ += dt;
    }
    if (!x6a0_25_isAquaPirate && xc4_fluidId != kInvalidUniqueId) {
      if (TCastToPtr<CScriptWater> water = mgr.ObjectById(xc4_fluidId)) {
        const zeus::CAABox& box = water->GetTriggerBoundsWR();
        if (2.f + GetTranslation().z() < box.max.z()) {
          x401_30_pendingDeath = true;
        }
      }
    }
  }

  if (x450_bodyController->GetPercentageFrozen() == 0.f) {
    x86c_ = std::max(0.f, x86c_ - dt);
    x860_ = std::max(0.f, x860_ - dt);
    x888_ = std::max(0.f, x888_ - dt);
    if (x6a0_31_canSeePlayer) {
      x7d8_ += dt;
    } else {
      x7d8_ = 0.f;
    }
    if (x400_25_alive) {
      CheckForProjectiles(mgr);
    }
    if (!x6a0_25_isAquaPirate &&
        (!x400_25_alive || !(!x450_bodyController->GetBodyStateInfo().GetCurrentState()->CanShoot() || !x6a0_28_ ||
                             x450_bodyController->GetCurrentStateId() == pas::EAnimationState::ProjectileAttack ||
                             x6a1_31_stopped || x450_bodyController->IsElectrocuting()))) {
      if (x7ec_burstFire.GetBurstType() != -1) {
        x7e4_ -= dt;
        if (x7e4_ < 0.f) {
          s32 newType = x7ec_burstFire.GetBurstType() & ~1;
          if (!PlayerSpot(mgr, 0.f)) {
            newType += 1;
          }
          x7ec_burstFire.SetBurstType(newType);
          x7ec_burstFire.Start(mgr);
          if (x400_25_alive) {
            x7e4_ = x308_attackTimeVariation * mgr.GetActiveRandom()->Float() + x304_averageAttackTime;
            const zeus::CVector3f& dist =
                (GetBoundingBox().center() - mgr.GetPlayer().GetAimPosition(mgr, 0.f)).normalized();
            if (dist.magSquared() < 0.9f) {
              for (const auto& obj : *mgr.ObjectListById(EGameObjectList::AiWaypoint)) {
                if (const CSpacePirate* pirate = CPatterned::CastTo<const CSpacePirate>(obj)) {
                  if (pirate->GetEnableAim() && pirate->GetAreaIdAlways() == x4_areaId) {
                    x7e4_ += 0.2f;
                  }
                }
              }
            }
          } else {
            x7e4_ = 22050.f;
          }
        }

        x7ec_burstFire.Update(mgr, dt);
        if (x7ec_burstFire.ShouldFire()) {
          FireProjectile(mgr, dt);
          if (0.f < x7ec_burstFire.GetTimeToNextShot()) {
            x7ec_burstFire.SetTimeToNextShot(x568_data.xd4_ * (mgr.GetActiveRandom()->Float() - 0.5f) + x568_data.xd0_);
          }
        }
      }
    }
  }

  if (x89c_ragDoll && x89c_ragDoll->IsPrimed()) {
    UpdateAlphaDelta(dt, mgr);
    UpdateDamageColor(dt);
  } else {
    if (!x400_25_alive || x450_bodyController->IsFrozen() || x450_bodyController->IsElectrocuting() || !x6a0_28_ ||
        x89c_ragDoll || x6a0_25_isAquaPirate) {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::AdditiveIdle));
    } else {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCAdditiveAimCmd());
      x450_bodyController->GetCommandMgr().DeliverAdditiveTargetVector(
          GetTransform().transposeRotate(GetTargetPos(mgr) - GetTranslation()));
    }
    if (0.f < x870_.magSquared()) {
      float mag = x870_.magnitude();
      float fVar5 = 0.2f;
      if (0.f == x87c_.magSquared()) {
        fVar5 = 0.2f * 3.f;
      }
      float mul = -(dt * mag * fVar5 * mag - mag);
      x870_ = mul * (1.f / mag) * x870_;
    }
    pas::EAnimationState state = x450_bodyController->GetCurrentStateId();
    if (!x400_25_alive || state == pas::EAnimationState::LoopReaction || state == pas::EAnimationState::Hurled ||
        state == pas::EAnimationState::LieOnGround || state == pas::EAnimationState::Getup) {
      x870_.zeroOut();
      x87c_.zeroOut();
    } else {
      ApplyImpulseWR(xe8_mass * x870_, {});
    }

    if (const auto& handle = GetSfxHandle()) {
      x898_ = std::clamp(x898_, 1.f, 1.999f);
      x894_pitchBend += std::clamp(x898_ - x894_pitchBend, -dt, dt);
      CSfxManager::PitchBend(handle, x894_pitchBend);
    }

    x87c_.zeroOut();
    x898_ = 1.f;
    CPatterned::Think(dt, mgr);

    zeus::CVector3f vf8 = x87c_;
    if (vf8.canBeNormalized()) {
      vf8.normalize();
    }
    zeus::CVector3f v1d0 = std::min(0.333f * x87c_.magnitude(), 0.333f) * vf8;
    const zeus::CVector3f& v104 = (zeus::skUp + v1d0).normalized();
    const zeus::CVector3f& v110 = GetTransform().upVector();
    float f26c = std::abs(zeus::CVector3f::getAngleDiff(v110, v104));
    if (f26c > 0.f) {
      float f1f4 = std::min(f26c, 30.f * zeus::degToRad(dt)); // ?
      float f200 = f26c - f1f4;
      zeus::CVector3f v1dc = (f1f4 * v104 + (f200 * v110)).normalized();
      zeus::CVector3f v128 = GetTransform().frontVector().cross(v1dc);
      zeus::CVector3f v20c = v1dc.cross(v128).normalized();
      zeus::CVector3f v128_2 = v20c.cross(v1dc);
      SetTransform({v128_2, v20c, v1dc, GetTranslation()});
    }

    if (!x450_bodyController->IsFrozen()) {
      x7a0_boneTracking.Update(dt);
    }
  }

  if (x89c_ragDoll) {
    if (x89c_ragDoll->IsPrimed()) {
      float waterTop = -FLT_MAX;
      if (xc4_fluidId != kInvalidUniqueId) {
        if (TCastToPtr<CScriptWater> water = mgr.ObjectById(xc4_fluidId)) {
          waterTop = water->GetTriggerBoundsWR().max.z();
        }
      }
      x89c_ragDoll->Update(mgr, dt * CalcDyingThinkRate(), waterTop);
      x64_modelData->AdvanceParticles(GetTransform(), dt, mgr);
    } else {
      // SetMuted(true); ??
      SetMuted(false);
      x89c_ragDoll->Prime(mgr, GetTransform(), *x64_modelData);
      SetTransform(zeus::CTransform::Translate(GetTranslation()));
      x450_bodyController->SetPlaybackRate(0.f);
    }

    if (x89c_ragDoll->IsOver() && !x400_27_fadeToDeath) {
      x400_27_fadeToDeath = true;
      x3e8_alphaDelta = -1.f / 3.f;
      SetVelocityWR(zeus::skZero3f);
      x150_momentum.zeroOut();
      x870_.zeroOut();
    }

    bool wasGtZero = x88c_ragDollTimer > 0.f;
    x88c_ragDollTimer -= dt;
    if (x88c_ragDollTimer < 2.f) {
      if (x89c_ragDoll->GetImpactCount() > 2) {
        x88c_ragDollTimer = std::min(0.1f, x88c_ragDollTimer);
      }
      if (wasGtZero && x88c_ragDollTimer <= 0.f) {
        x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), "Explode");
      }
    }
  }
}

} // namespace urde::MP1
