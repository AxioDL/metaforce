#include "Runtime/MP1/World/CSpacePirate.hpp"

#include <array>

#include "Runtime/CTimeProvider.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Character/CCharLayoutInfo.hpp"
#include "Runtime/Character/CPASAnimParmData.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/MP1/World/CMetroid.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptAiJumpPoint.hpp"
#include "Runtime/World/CScriptCoverPoint.hpp"
#include "Runtime/World/CScriptTargetingPoint.hpp"
#include "Runtime/World/CScriptWater.hpp"
#include "Runtime/World/CScriptWaypoint.hpp"
#include "Runtime/World/CTeamAiMgr.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {
namespace {
constexpr std::array skParts{
    "Collar"sv,  "Neck_1"sv, "R_shoulder"sv, "R_elbow"sv, "R_wrist"sv, "L_shoulder"sv, "L_elbow"sv,
    "L_wrist"sv, "R_hip"sv,  "R_knee"sv,     "R_ankle"sv, "L_hip"sv,   "L_knee"sv,     "L_ankle"sv,
};

constexpr std::array skRadii{
    0.45f, 0.52f, 0.35f, 0.1f, 0.15f, 0.35f, 0.1f, 0.15f, 0.15f, 0.15f, 0.15f, 0.15f, 0.15f, 0.15f,
};

constexpr std::array<SBurst, 6> skBurstsQuick{{
    {20, {3, 4, 5, -1, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {20, {2, 3, 4, -1, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {20, {6, 5, 4, -1, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {20, {1, 2, 3, -1, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {20, {7, 6, 5, -1, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000f, 0.000000f},
}};

constexpr std::array<SBurst, 7> skBurstsStandard{{
    {15, {5, 3, 2, 1, -1, 0, 0, 0}, 0.100000f, 0.050000f},
    {20, {1, 2, 3, 4, -1, 0, 0, 0}, 0.100000f, 0.050000f},
    {20, {7, 6, 5, 4, -1, 0, 0, 0}, 0.100000f, 0.050000f},
    {15, {3, 4, 5, 6, -1, 0, 0, 0}, 0.100000f, 0.050000f},
    {15, {6, 5, 4, 3, -1, 0, 0, 0}, 0.100000f, 0.050000f},
    {15, {2, 3, 4, 5, -1, 0, 0, 0}, 0.100000f, 0.050000f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000f, 0.000000f},
}};

constexpr std::array<SBurst, 5> skBurstsFrenzied{{
    {40, {1, 2, 3, 4, 5, 6, -1, 0}, 0.100000f, 0.050000f},
    {40, {7, 6, 5, 4, 3, 2, -1, 0}, 0.100000f, 0.050000f},
    {10, {2, 3, 4, 5, 4, 3, -1, 0}, 0.100000f, 0.050000f},
    {10, {6, 5, 4, 3, 4, 5, -1, 0}, 0.100000f, 0.050000f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000f, 0.000000f},
}};

constexpr std::array<SBurst, 4> skBurstsJumping{{
    {20, {16, 4, -1, 0, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {40, {5, 7, -1, 0, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {40, {1, 10, -1, 0, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000f, 0.000000f},
}};

constexpr std::array<SBurst, 6> skBurstsInjured{{
    {15, {16, 1, 3, -1, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {20, {3, 4, 6, -1, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {25, {7, 5, 4, -1, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {25, {2, 6, 4, -1, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {15, {7, 5, 3, -1, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000f, 0.000000f},
}};

constexpr std::array<SBurst, 4> skBurstsSeated{{
    {35, {7, 13, -1, 0, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {35, {9, 1, -1, 0, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {30, {16, 12, -1, 0, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000f, 0.000000f},
}};

constexpr std::array<SBurst, 6> skBurstsQuickOOV{{
    {10, {16, 15, 13, -1, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {20, {13, 12, 10, -1, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {30, {9, 11, 12, -1, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {30, {14, 10, 12, -1, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {10, {9, 11, 13, -1, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000f, 0.000000f},
}};

constexpr std::array<SBurst, 7> skBurstsStandardOOV{{
    {26, {16, 8, 11, 14, -1, 0, 0, 0}, 0.100000f, 0.050000f},
    {26, {16, 13, 11, 12, -1, 0, 0, 0}, 0.100000f, 0.050000f},
    {16, {9, 11, 13, 10, -1, 0, 0, 0}, 0.100000f, 0.050000f},
    {16, {14, 13, 12, 11, -1, 0, 0, 0}, 0.100000f, 0.050000f},
    {8, {10, 11, 12, 13, -1, 0, 0, 0}, 0.100000f, 0.050000f},
    {8, {6, 8, 11, 13, -1, 0, 0, 0}, 0.100000f, 0.050000f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000f, 0.000000f},
}};

constexpr std::array<SBurst, 5> skBurstsFrenziedOOV{{
    {40, {1, 16, 14, 12, 10, 11, -1, 0}, 0.100000f, 0.050000f},
    {40, {9, 11, 12, 13, 11, 7, -1, 0}, 0.100000f, 0.050000f},
    {10, {8, 10, 11, 12, 13, 12, -1, 0}, 0.100000f, 0.050000f},
    {10, {15, 13, 12, 10, 12, 9, -1, 0}, 0.100000f, 0.050000f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000f, 0.000000f},
}};

constexpr std::array<SBurst, 4> skBurstsJumpingOOV{{
    {40, {7, 13, -1, 0, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {40, {9, 1, -1, 0, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {20, {16, 12, -1, 0, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000f, 0.000000f},
}};

constexpr std::array<SBurst, 6> skBurstsInjuredOOV{{
    {30, {9, 11, 13, -1, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {10, {13, 12, 10, -1, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {15, {9, 11, 12, -1, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {15, {14, 10, 12, -1, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {30, {16, 15, 13, -1, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000f, 0.000000f},
}};

constexpr std::array<SBurst, 4> skBurstsSeatedOOV{{
    {35, {7, 13, -1, 0, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {35, {9, 1, -1, 0, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {30, {16, 12, -1, 0, 0, 0, 0, 0}, 0.100000f, 0.050000f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000f, 0.000000f},
}};

constexpr std::array<const SBurst*, 13> skBursts{
    skBurstsQuick.data(),
    skBurstsStandard.data(),
    skBurstsFrenzied.data(),
    skBurstsJumping.data(),
    skBurstsInjured.data(),
    skBurstsSeated.data(),
    skBurstsQuickOOV.data(),
    skBurstsStandardOOV.data(),
    skBurstsFrenziedOOV.data(),
    skBurstsJumpingOOV.data(),
    skBurstsInjuredOOV.data(),
    skBurstsSeatedOOV.data(),
    nullptr,
};
} // Anonymous namespace

CSpacePirate::CSpacePirateData::CSpacePirateData(urde::CInputStream& in, u32 propCount)
: x0_AggressionCheck(in.readFloatBig())
, x4_CoverCheck(in.readFloatBig())
, x8_SearchRadius(in.readFloatBig())
, xc_FallBackCheck(in.readFloatBig())
, x10_FallBackRadius(in.readFloatBig())
, x14_HearingRadius(in.readFloatBig())
, x18_flags(in.readUint32Big())
, x1c_(in.readBool())
, x20_Projectile(in)
, x48_Sound_Projectile(CSfxManager::TranslateSFXID(in.readUint32Big()))
, x4c_BladeDamage(in)
, x68_KneelAttackChance(in.readFloatBig())
, x6c_KneelAttackShot(in)
, x94_DodgeCheck(in.readFloatBig())
, x98_Sound_Impact(CSfxManager::TranslateSFXID(in.readUint32Big()))
, x9c_averageNextShotTime(in.readFloatBig())
, xa0_nextShotTimeVariation(in.readFloatBig())
, xa4_Sound_Alert(CSfxManager::TranslateSFXID(in.readUint32Big()))
, xa8_GunTrackDelay(in.readFloatBig())
, xac_firstBurstCount(in.readUint32Big())
, xb0_CloakOpacity(in.readFloatBig())
, xb4_MaxCloakOpacity(in.readFloatBig())
, xb8_dodgeDelayTimeMin(in.readFloatBig())
, xbc_dodgeDelayTimeMax(in.readFloatBig())
, xc0_Sound_Hurled(CSfxManager::TranslateSFXID(in.readUint32Big()))
, xc2_Sound_Death(CSfxManager::TranslateSFXID(in.readUint32Big()))
, xc4_(propCount > 35 ? in.readFloatBig() : 0.2f)
, xc8_AvoidDistance(propCount > 36 ? in.readFloatBig() : 8.f) {}

CPirateRagDoll::CPirateRagDoll(CStateManager& mgr, CSpacePirate* sp, u16 thudSfx, u32 flags)
: CRagDoll(-sp->GetGravityConstant(), -3.f, 8.f, flags), x6c_spacePirate(sp), x70_thudSfx(thudSfx) {
  x6c_spacePirate->RemoveMaterial(EMaterialTypes::Solid, EMaterialTypes::AIBlock, EMaterialTypes::GroundCollider, mgr);
  x6c_spacePirate->HealthInfo(mgr)->SetHP(-1.f);
  SetNumParticles(14);
  SetNumLengthConstraints(47);
  SetNumJointConstraints(4);
  zeus::CVector3f scale = x6c_spacePirate->GetModelData()->GetScale();
  CAnimData* aData = x6c_spacePirate->GetModelData()->GetAnimationData();
  aData->BuildPose();
  zeus::CVector3f center = x6c_spacePirate->GetBoundingBox().center();
  for (size_t i = 0; i < skParts.size(); ++i) {
    const CSegId id = aData->GetLocatorSegId(skParts[i]);
    AddParticle(id, center, x6c_spacePirate->GetTransform() * (aData->GetPose().GetOffset(id) * scale),
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
  AddMinLengthConstraint(1, 8, x14_lengthConstraints[2].GetLength());
  AddMinLengthConstraint(1, 11, x14_lengthConstraints[3].GetLength());
  AddMinLengthConstraint(1, 2, x14_lengthConstraints[1].GetLength() * 0.9f);
  AddMinLengthConstraint(1, 5, x14_lengthConstraints[4].GetLength() * 0.9f);
  AddMinLengthConstraint(1, 4, x14_lengthConstraints[0].GetLength() * 2.5f);
  AddMinLengthConstraint(1, 7, x14_lengthConstraints[0].GetLength() * 2.5f);
  AddMinLengthConstraint(4, 2, x14_lengthConstraints[5].GetLength());
  AddMinLengthConstraint(7, 5, x14_lengthConstraints[7].GetLength());
  AddMinLengthConstraint(3, 5, x14_lengthConstraints[5].GetLength() * 0.5f + x14_lengthConstraints[9].GetLength());
  AddMinLengthConstraint(6, 2, x14_lengthConstraints[7].GetLength() * 0.5f + x14_lengthConstraints[9].GetLength());
  AddMinLengthConstraint(4, 5, x14_lengthConstraints[5].GetLength() * 0.5f + x14_lengthConstraints[9].GetLength());
  AddMinLengthConstraint(7, 2, x14_lengthConstraints[7].GetLength() * 0.5f + x14_lengthConstraints[9].GetLength());
  AddMinLengthConstraint(4, 7, x14_lengthConstraints[9].GetLength());
  AddMinLengthConstraint(4, 8, x14_lengthConstraints[14].GetLength());
  AddMinLengthConstraint(7, 11, x14_lengthConstraints[14].GetLength());
  AddMinLengthConstraint(10, 8, x14_lengthConstraints[15].GetLength());
  AddMinLengthConstraint(13, 11, x14_lengthConstraints[17].GetLength());
  AddMinLengthConstraint(9, 2, x14_lengthConstraints[15].GetLength() * 0.707f + x14_lengthConstraints[10].GetLength());
  AddMinLengthConstraint(12, 5, x14_lengthConstraints[17].GetLength() * 0.707f + x14_lengthConstraints[13].GetLength());
  AddMinLengthConstraint(9, 11, x14_lengthConstraints[15].GetLength());
  AddMinLengthConstraint(12, 8, x14_lengthConstraints[17].GetLength());
  AddMinLengthConstraint(10, 0, x14_lengthConstraints[2].GetLength() + x14_lengthConstraints[15].GetLength());
  AddMinLengthConstraint(13, 0, x14_lengthConstraints[3].GetLength() + x14_lengthConstraints[17].GetLength());
  AddMinLengthConstraint(10, 13, x14_lengthConstraints[14].GetLength());
  AddMinLengthConstraint(9, 12, x14_lengthConstraints[14].GetLength());
  AddMinLengthConstraint(10, 12, x14_lengthConstraints[14].GetLength());
  AddMinLengthConstraint(13, 9, x14_lengthConstraints[14].GetLength());
  AddMaxLengthConstraint(10, 13, x14_lengthConstraints[14].GetLength() * 5.f);
  AddJointConstraint(8, 2, 5, 8, 9, 10);    // R_hip, R_shoulder, L_shoulder, R_hip, R_knee, R_ankle
  AddJointConstraint(11, 2, 5, 11, 12, 13); // L_hip, R_shoulder, L_shoulder, L_hip, L_knee, L_ankle
  AddJointConstraint(2, 11, 5, 2, 3, 4);    // R_shoulder, L_hip, L_shoulder, R_shoulder, R_elbow, R_wrist
  AddJointConstraint(5, 2, 8, 5, 6, 7);     // L_shoulder, R_shoulder, R_hip, L_shoulder, L_elbow, R_wrist
  for (const auto& conn : x6c_spacePirate->GetConnectionList()) {
    if (conn.x0_state == EScriptObjectState::Modify && conn.x4_msg == EScriptObjectMessage::Follow) {
      TUniqueId wpId = mgr.GetIdForScript(conn.x8_objId);
      if (TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(wpId)) {
        x90_waypoints.push_back(wpId);
        x9c_wpParticleIdxs.push_back(wp->GetAnimation());
        if (x90_waypoints.size() == 4)
          break;
      }
    }
  }
}

void CPirateRagDoll::PreRender(const zeus::CVector3f& v, CModelData& mData) {
  if (!x68_25_over || x68_27_continueSmallMovements) {
    CAnimData* aData = mData.GetAnimationData();
    for (CSegId id : aData->GetCharLayoutInfo().GetSegIdList().GetList())
      if (aData->GetCharLayoutInfo().GetRootNode()->GetBoneMap()[id].x10_children.size() > 1)
        aData->PoseBuilder().GetTreeMap()[id].x4_rotation = zeus::CQuaternion();
    CSegId rootId = aData->GetLocatorSegId("Skeleton_Root"sv);
    // R_hip, L_hip
    aData->PoseBuilder().GetTreeMap()[rootId].x14_offset =
        (0.5f * (x4_particles[8].GetPosition() + x4_particles[11].GetPosition()) - v) / mData.GetScale();
    // R_shoulder, L_shoulder
    zeus::CVector3f rootRight = x4_particles[2].GetPosition() - x4_particles[5].GetPosition();
    // Collar, R_hip, L_hip
    zeus::CVector3f rootUp =
        (x4_particles[0].GetPosition() - (x4_particles[8].GetPosition() + x4_particles[11].GetPosition()) * 0.5f)
            .normalized();
    zeus::CVector3f rootFore = rootUp.cross(rootRight).normalized();
    zeus::CQuaternion rootRot(zeus::CMatrix3f(rootFore.cross(rootUp), rootFore, rootUp));
    aData->PoseBuilder().GetTreeMap()[rootId].x4_rotation = rootRot;
    if (x6c_spacePirate->x7b4_attachedActor == kInvalidUniqueId) {
      zeus::CVector3f neckRestVec = aData->GetCharLayoutInfo().GetFromParentUnrotated(x4_particles[1].GetBone());
      aData->PoseBuilder().GetTreeMap()[x4_particles[1].GetBone()].x4_rotation = zeus::CQuaternion::shortestRotationArc(
          neckRestVec, rootRot.inverse().transform(x4_particles[1].GetPosition() - x4_particles[0].GetPosition()));
    }
    BoneAlign(aData->PoseBuilder(), aData->GetCharLayoutInfo(), 3, 4,
              rootRot * BoneAlign(aData->PoseBuilder(), aData->GetCharLayoutInfo(), 2, 3, rootRot));
    BoneAlign(aData->PoseBuilder(), aData->GetCharLayoutInfo(), 6, 7,
              rootRot * BoneAlign(aData->PoseBuilder(), aData->GetCharLayoutInfo(), 5, 6, rootRot));
    BoneAlign(aData->PoseBuilder(), aData->GetCharLayoutInfo(), 9, 10,
              rootRot * BoneAlign(aData->PoseBuilder(), aData->GetCharLayoutInfo(), 8, 9, rootRot));
    BoneAlign(aData->PoseBuilder(), aData->GetCharLayoutInfo(), 12, 13,
              rootRot * BoneAlign(aData->PoseBuilder(), aData->GetCharLayoutInfo(), 11, 12, rootRot));
    zeus::CQuaternion q;
    q.rotateX(zeus::degToRad(-70.f));
    aData->PoseBuilder().GetTreeMap()[x4_particles[10].GetBone()].x4_rotation = q; // R_ankle
    aData->PoseBuilder().GetTreeMap()[x4_particles[13].GetBone()].x4_rotation = q; // L_ankle
    aData->MarkPoseDirty();
  }
}

void CPirateRagDoll::Update(CStateManager& mgr, float dt, float waterTop) {
  if (!x68_25_over || x68_27_continueSmallMovements) {
    if (x6c_spacePirate->x7b4_attachedActor != kInvalidUniqueId) {
      // Shoulder height delta
      float f2 = x4_particles[2].GetPosition().z() - x4_particles[5].GetPosition().z();
      if (f2 * f2 > 0.0625f) {
        zeus::CVector3f vec(0.f, 0.f, ((f2 > 0.f) ? f2 - 0.25f : f2 + 0.25f) * 0.1f);
        x4_particles[2].Position() -= vec;
        x4_particles[5].Position() += vec;
      }
      // Collar-hips height delta
      f2 = x4_particles[0].GetPosition().z() -
           (x4_particles[8].GetPosition().z() + x4_particles[11].GetPosition().z()) * 0.5f;
      if (f2 * f2 > 0.0625f) {
        zeus::CVector3f vec(0.f, 0.f, ((f2 > 0.f) ? f2 - 0.25f : f2 + 0.25f) * 0.1f);
        x4_particles[0].Position() -= vec;
        x4_particles[8].Position() += vec;
        x4_particles[11].Position() += vec;
      }
    }
    // Collar-hips weighted center
    zeus::CVector3f oldTorsoCenter = x4_particles[8].GetPosition() * 0.25f + x4_particles[11].GetPosition() * 0.25f +
                                     x4_particles[0].GetPosition() * 0.5f;
    oldTorsoCenter.z() = std::min(x4_particles[0].GetPosition().z() - x4_particles[0].GetRadius(),
                                  std::min(x4_particles[8].GetPosition().z() - x4_particles[8].GetRadius(),
                                           x4_particles[11].GetPosition().z() - x4_particles[11].GetRadius()));
    if (oldTorsoCenter.z() < 0.5f + waterTop)
      x84_torsoImpulse = x84_torsoImpulse * 1000.f;
    zeus::CVector3f accDelta = x84_torsoImpulse * 0.333f * (1.f / x6c_spacePirate->GetMass());
    x4_particles[11].Velocity() += accDelta;
    x4_particles[8].Velocity() += accDelta;
    x4_particles[0].Velocity() += accDelta;
    x84_torsoImpulse = zeus::skZero3f;
    CRagDoll::Update(mgr, dt, waterTop);
    auto particleIdxIt = x9c_wpParticleIdxs.begin();
    for (TUniqueId id : x90_waypoints) {
      if (const CScriptWaypoint* wp = static_cast<const CScriptWaypoint*>(mgr.GetObjectById(id)))
        if (wp->GetActive())
          x4_particles[*particleIdxIt].Position() = wp->GetTranslation();
      ++particleIdxIt;
    }
    // Collar-hips weighted center
    zeus::CVector3f newTorsoCenter = x4_particles[8].GetPosition() * 0.25f + x4_particles[11].GetPosition() * 0.25f +
                                     x4_particles[0].GetPosition() * 0.5f;
    newTorsoCenter.z() = std::min(x4_particles[0].GetPosition().z() - x4_particles[0].GetRadius(),
                                  std::min(x4_particles[8].GetPosition().z() - x4_particles[8].GetRadius(),
                                           x4_particles[11].GetPosition().z() - x4_particles[11].GetRadius()));
    x6c_spacePirate->SetTransform({});
    x6c_spacePirate->SetTranslation(newTorsoCenter);
    x6c_spacePirate->SetVelocityWR((newTorsoCenter - oldTorsoCenter) * (1.f / dt));
    x74_sfxTimer -= dt;
    if (x54_impactVel > 2.5f && x74_sfxTimer < 0.f &&
        (xb0_24_initSfx || (x6c_spacePirate->GetTranslation() - x78_lastSFXPos).magSquared() > 0.1f)) {
      CSfxManager::AddEmitter(x70_thudSfx, x6c_spacePirate->GetTranslation(), zeus::skZero3f,
                              std::min(25.f * x54_impactVel, 127.f) / 127.f, true, false, 0x7f, kInvalidAreaId);
      x74_sfxTimer = mgr.GetActiveRandom()->Float() * 0.222f + 0.222f;
      xb0_24_initSfx = false;
      x78_lastSFXPos = x6c_spacePirate->GetTranslation();
    }
  } else {
    x6c_spacePirate->SetMomentumWR(zeus::skZero3f);
    x6c_spacePirate->Stop();
  }
}

void CPirateRagDoll::Prime(CStateManager& mgr, const zeus::CTransform& xf, CModelData& mData) {
  const auto& aabb = x6c_spacePirate->GetBaseBoundingBox();
  zeus::CVector3f newMax = aabb.max;
  newMax.z() = (aabb.max.z() - aabb.min.z()) * 0.5f + aabb.min.z();
  x6c_spacePirate->SetBoundingBox({aabb.min, newMax});
  CRagDoll::Prime(mgr, xf, mData);
}

std::list<TUniqueId> CSpacePirate::mChargePlayerList;

CSpacePirate::CSpacePirate(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                           CModelData&& mData, const CActorParameters& aParams, const CPatternedInfo& pInfo,
                           CInputStream& in, u32 propCount)
: CPatterned(ECharacter::SpacePirate, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Ground, EColliderType::One, EBodyType::BiPedal, aParams, EKnockBackVariant::Medium)
, x568_pirateData(in, propCount)
, x660_pathFindSearch(nullptr, 0x1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x750_initialHP(pInfo.GetHealthInfo().GetHP())
, x764_boneTracking(*x64_modelData->GetAnimationData(), "Head_1"sv, zeus::degToRad(70.f), zeus::degToRad(180.f),
                    EBoneTrackingFlags::None)
, x7c4_burstFire(skBursts.data(), x568_pirateData.xac_firstBurstCount)
, x8b8_minCloakAlpha(x568_pirateData.xb0_CloakOpacity)
, x8bc_maxCloakAlpha(x568_pirateData.xb4_MaxCloakOpacity)
, x8c0_dodgeDelayTimer(x568_pirateData.xb8_dodgeDelayTimeMin)
, x8c4_aimDelayTimer(x568_pirateData.xa8_GunTrackDelay) {
  x634_24_pendingAmbush = bool(x568_pirateData.x18_flags & 0x1);
  x634_25_ceilingAmbush = bool(x568_pirateData.x18_flags & 0x2);
  x634_26_nonAggressive = bool(x568_pirateData.x18_flags & 0x4);
  x634_27_melee = bool(x568_pirateData.x18_flags & 0x8);
  x634_28_noShuffleCloseCheck = bool(x568_pirateData.x18_flags & 0x10);
  x634_29_onlyAttackInRange = bool(x568_pirateData.x18_flags & 0x20);
  x634_30_ = bool(x568_pirateData.x18_flags & 0x40);
  x634_31_noKnockbackImpulseReset = bool(x568_pirateData.x18_flags & 0x80);
  x635_24_noMeleeAttack = bool(x568_pirateData.x18_flags & 0x200);
  x635_25_breakAttack = bool(x568_pirateData.x18_flags & 0x400);
  x635_26_seated = bool(x568_pirateData.x18_flags & 0x1000);
  x635_27_shadowPirate = bool(x568_pirateData.x18_flags & 0x2000);
  x635_28_alertBeforeCloak = bool(x568_pirateData.x18_flags & 0x4000);
  x635_29_noBreakDodge = bool(x568_pirateData.x18_flags & 0x8000);
  x635_30_floatingCorpse = bool(x568_pirateData.x18_flags & 0x10000);
  x635_31_ragdollNoAiCollision = bool(x568_pirateData.x18_flags & 0x20000);
  x636_24_trooper = bool(x568_pirateData.x18_flags & 0x40000);

  x758_headSeg = x64_modelData->GetAnimationData()->GetLocatorSegId("Head_1"sv);
  x7b6_gunSeg = x64_modelData->GetAnimationData()->GetLocatorSegId("R_gun_LCTR"sv);
  x7b7_elbowSeg = x64_modelData->GetAnimationData()->GetLocatorSegId("R_elbow"sv);
  x7b8_wristSeg = x64_modelData->GetAnimationData()->GetLocatorSegId("R_wrist"sv);
  x7b9_swooshSeg = x64_modelData->GetAnimationData()->GetLocatorSegId("Swoosh_LCTR"sv);

  if (!x634_29_onlyAttackInRange) {
    x7a4_intoJumpDist =
        GetAnimationDistance(CPASAnimParmData(13, CPASAnimParm::FromEnum(0), CPASAnimParm::FromEnum(0)));
    x848_dodgeDist = GetAnimationDistance(CPASAnimParmData(3, CPASAnimParm::FromEnum(3), CPASAnimParm::FromEnum(1)));
    x84c_breakDodgeDist =
        GetAnimationDistance(CPASAnimParmData(3, CPASAnimParm::FromEnum(3), CPASAnimParm::FromEnum(2)));
  } else {
    x450_bodyController->BodyStateInfo().SetLocoAnimChangeAtEndOfAnimOnly(true);
  }

  const auto& baseAABB = GetBaseBoundingBox();
  x7a8_eyeHeight = (baseAABB.max.z() - baseAABB.min.z()) * 0.6f;

  if (x90_actorLights)
    x90_actorLights->SetAmbienceGenerated(false);

  x460_knockBackController.sub80233d40(3, 3.f, FLT_MAX);
  x460_knockBackController.SetLocomotionDuringElectrocution(true);

  if (x634_29_onlyAttackInRange)
    x460_knockBackController.SetKnockBackVariant(EKnockBackVariant::Small);
  else if (x636_24_trooper && x260_damageVulnerability.WeaponHurts(CWeaponMode(EWeaponType::Plasma), false))
    x460_knockBackController.SetKnockBackVariant(EKnockBackVariant::Large);

  if (!x450_bodyController->HasBodyState(pas::EAnimationState::AdditiveAim))
    x634_27_melee = true;

  if (x636_24_trooper) {
    if (x260_damageVulnerability.WeaponHurts(CWeaponMode(EWeaponType::Plasma), false))
      x8cc_trooperColor = zeus::CColor(0.996f, 0.f, 0.157f, 1.f);
    else if (x260_damageVulnerability.WeaponHurts(CWeaponMode(EWeaponType::Ice), false))
      x8cc_trooperColor = zeus::skWhite;
    else if (x260_damageVulnerability.WeaponHurts(CWeaponMode(EWeaponType::Power), false))
      x8cc_trooperColor = zeus::CColor(0.992f, 0.937f, 0.337f, 1.f);
    else if (x260_damageVulnerability.WeaponHurts(CWeaponMode(EWeaponType::Wave), false))
      x8cc_trooperColor = zeus::CColor(0.776f, 0.054f, 1.f, 1.f);
  }

  x568_pirateData.x20_Projectile.Token().Lock();
  x568_pirateData.x6c_KneelAttackShot.Token().Lock();
}

void CSpacePirate::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CSpacePirate::UpdateCloak(float dt, CStateManager& mgr) {
  if (x635_27_shadowPirate) {
    if (x400_25_alive) {
      if (x8a8_cloakDelayTimer > 0.f) {
        x8a8_cloakDelayTimer -= dt;
        if (x8a8_cloakDelayTimer <= 0.f)
          x3e8_alphaDelta = -0.4f;
      }
    } else {
      x8b8_minCloakAlpha = 0.f;
      x8bc_maxCloakAlpha = 1.f;
    }

    if (x8ac_electricParticleTimer > 0.f) {
      x8ac_electricParticleTimer -= dt;
      if (x8ac_electricParticleTimer <= 0.f && !x450_bodyController->IsElectrocuting())
        mgr.GetActorModelParticles()->StopElectric(*this);
    }

    if (x450_bodyController->IsFrozen())
      x3e8_alphaDelta = 2.f;

    if (x3e8_alphaDelta < 0.f && x42c_color.a() < x8b8_minCloakAlpha) {
      x42c_color.a() = x8b8_minCloakAlpha;
      x3e8_alphaDelta = 0.f;
      RemoveMaterial(EMaterialTypes::Target, mgr);
    }

    if (x3e8_alphaDelta > 0.f && x42c_color.a() > x8bc_maxCloakAlpha) {
      x42c_color.a() = x8bc_maxCloakAlpha;
      AddMaterial(EMaterialTypes::Target, mgr);
    }

    x8b0_cloakStepTime -= dt;
    if (x8b0_cloakStepTime < 0.f) {
      x8b0_cloakStepTime = (1.f - mgr.GetActiveRandom()->Float()) * 0.08f;
      if (x3e8_alphaDelta < 0.f) {
        x8b4_shadowPirateAlpha = x42c_color.a();
        if (x400_25_alive)
          x8b4_shadowPirateAlpha -= (x42c_color.a() - x8b8_minCloakAlpha) * x8b0_cloakStepTime;
      } else if (x3e8_alphaDelta > 0.f) {
        x8b4_shadowPirateAlpha = x42c_color.a() + x8b0_cloakStepTime * (x8bc_maxCloakAlpha - x42c_color.a());
      } else {
        x8b4_shadowPirateAlpha = x42c_color.a();
      }
    }
  }
}

bool CSpacePirate::ShouldFrenzy(CStateManager& mgr) {
  bool reset = false;
  if (x638_24_pendingFrenzyChance) {
    x638_24_pendingFrenzyChance = false;
    if (mgr.GetActiveRandom()->Next() % 100 < 25)
      reset = true;
  }

  if (!mChargePlayerList.empty())
    reset = true;

  if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed)
    reset = true;

  if (HealthInfo(mgr)->GetHP() < 0.3f * x750_initialHP && mgr.GetActiveRandom()->Next() % 100 < 60 &&
      x854_lowHealthFrenzyTimer < 0.5f)
    reset = true;

  if (reset)
    x63c_frenzyFrames = mgr.GetActiveRandom()->Range(2, 4);
  x63c_frenzyFrames -= 1;
  return x63c_frenzyFrames >= 0;
}

void CSpacePirate::SquadReset(CStateManager& mgr) {
  CTeamAiMgr::ResetTeamAiRole(!x634_27_melee ? CTeamAiMgr::EAttackType::Ranged : CTeamAiMgr::EAttackType::Melee, mgr,
                              x8c8_teamAiMgrId, GetUniqueId(), true);
}

void CSpacePirate::SquadAdd(CStateManager& mgr) {
  if (x8c8_teamAiMgrId == kInvalidUniqueId)
    x8c8_teamAiMgrId = CTeamAiMgr::GetTeamAiMgr(*this, mgr);
  if (x8c8_teamAiMgrId != kInvalidUniqueId) {
    if (TCastToPtr<CTeamAiMgr> aimgr = mgr.ObjectById(x8c8_teamAiMgrId)) {
      aimgr->AssignTeamAiRole(*this, x634_27_melee ? CTeamAiRole::ETeamAiRole::Melee : CTeamAiRole::ETeamAiRole::Ranged,
                              CTeamAiRole::ETeamAiRole::Unknown, CTeamAiRole::ETeamAiRole::Invalid);
    }
  }
}

void CSpacePirate::SquadRemove(CStateManager& mgr) {
  if (x8c8_teamAiMgrId != kInvalidUniqueId) {
    if (TCastToPtr<CTeamAiMgr> aimgr = mgr.ObjectById(x8c8_teamAiMgrId)) {
      if (aimgr->IsPartOfTeam(GetUniqueId())) {
        aimgr->RemoveTeamAiRole(GetUniqueId());
        x8c8_teamAiMgrId = kInvalidUniqueId;
      }
    }
  }
}

bool CSpacePirate::CheckTargetable(const CStateManager& mgr) const { return GetModelAlphau8(mgr) > 127; }

bool CSpacePirate::FireProjectile(float dt, CStateManager& mgr) {
  bool ret = false;
  zeus::CTransform gunXf = GetLctrTransform(x7b6_gunSeg);
  if (!x400_25_alive) {
    LaunchProjectile(gunXf, mgr, 6, EProjectileAttrib::None, false, {}, 0xffff, false, zeus::skOne3f);
    ret = true;
  } else {
    if (TCastToConstPtr<CActor> act = mgr.GetObjectById(x7c0_targetId)) {
      zeus::CVector3f pos = act->GetTranslation();
      if (mgr.GetPlayer().GetUniqueId() == x7c0_targetId)
        pos = GetProjectileInfo()->PredictInterceptPos(gunXf.origin, mgr.GetPlayer().GetAimPosition(mgr, 0.f),
                                                       mgr.GetPlayer(), true, dt);
      zeus::CVector3f gunToPos = pos - gunXf.origin;
      float mag = gunToPos.magnitude();
      gunToPos = gunToPos / mag;
      float dot =
          (GetLctrTransform(x7b8_wristSeg).origin - GetLctrTransform(x7b7_elbowSeg).origin).normalized().dot(gunToPos);
      if ((dot > 0.707f || (mag < 6.f && dot > 0.5f)) &&
          LineOfSightTest(mgr, gunXf.origin, pos, {EMaterialTypes::Player, EMaterialTypes::ProjectilePassthrough})) {
        pos += GetTransform().rotate(x7c4_burstFire.GetDistanceCompensatedError(mag, 6.f));
        LaunchProjectile(zeus::lookAt(gunXf.origin, pos), mgr, 6, EProjectileAttrib::None, false, {}, 0xffff, false,
                         zeus::skOne3f);
        ret = true;
      }
    }
  }
  if (ret) {
    const auto bestAnim = x450_bodyController->GetPASDatabase().FindBestAnimation(
        CPASAnimParmData{24, CPASAnimParm::FromEnum(2)}, *mgr.GetActiveRandom(), -1);
    if (bestAnim.first > 0.f) {
      x64_modelData->GetAnimationData()->AddAdditiveAnimation(bestAnim.second, 1.f, false, true);
    }
    CSfxManager::AddEmitter(x568_pirateData.x48_Sound_Projectile, GetTranslation(), zeus::skZero3f, true, false, 0x7f,
                            kInvalidAreaId);
  }
  return ret;
}

void CSpacePirate::UpdateAttacks(float dt, CStateManager& mgr) {
  bool reset = true;
  if ((!x400_25_alive ||
       (x450_bodyController->GetBodyStateInfo().GetCurrentState()->CanShoot() && x637_25_enableAim && !x634_27_melee &&
        !x634_25_ceilingAmbush && !x639_26_started && !x450_bodyController->IsElectrocuting())) &&
      x7c4_burstFire.GetBurstType() != -1) {
    if (x400_25_alive) {
      if (!x634_29_onlyAttackInRange ||
          (mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared() < x3c8_leashRadius * x3c8_leashRadius) {
        reset = false;
        x7bc_attackRemTime -= dt;
        if (x7bc_attackRemTime < 0.f) {
          const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x8c8_teamAiMgrId, GetUniqueId());
          if (!role || role->GetTeamAiRole() == CTeamAiRole::ETeamAiRole::Ranged) {
            if (x8c8_teamAiMgrId == kInvalidUniqueId ||
                CTeamAiMgr::AddAttacker(CTeamAiMgr::EAttackType::Ranged, mgr, x8c8_teamAiMgrId, GetUniqueId())) {
              if (ShouldFrenzy(mgr))
                x7c4_burstFire.SetBurstType(2);
              if (x635_26_seated)
                x7c4_burstFire.SetBurstType(5);
              if (!PlayerSpot(mgr, 0.f) && x7c4_burstFire.GetBurstType() < 6)
                x7c4_burstFire.SetBurstType(x7c4_burstFire.GetBurstType() + 6);

              x7c4_burstFire.Start(mgr);
              x7bc_attackRemTime = mgr.GetActiveRandom()->Float() * x308_attackTimeVariation + x304_averageAttackTime;
              if ((GetGunEyePos() - mgr.GetPlayer().GetAimPosition(mgr, 0.f))
                      .normalized()
                      .dot(mgr.GetPlayer().GetTransform().basis[1]) < 0.9f) {
                for (CEntity* ent : mgr.GetListeningAiObjectList()) {
                  if (CSpacePirate* otherPirate = CPatterned::CastTo<CSpacePirate>(ent)) {
                    if (otherPirate != this && otherPirate->x637_25_enableAim &&
                        otherPirate->GetAreaIdAlways() == GetAreaIdAlways())
                      x7bc_attackRemTime += 0.2f;
                  }
                }
              }
            }
          }
        }
      }
    }

    x7c4_burstFire.Update(mgr, dt);

    if (x7c4_burstFire.ShouldFire()) {
      if (mgr.GetPlayer().IsSidewaysDashing() && mgr.GetActiveRandom()->Float() < 0.5f)
        x7c4_burstFire.SetAvoidAccuracy(true);
      FireProjectile(dt, mgr);
      x7c4_burstFire.SetAvoidAccuracy(false);
      float nextShotTime = x568_pirateData.xa0_nextShotTimeVariation * (mgr.GetActiveRandom()->Float() - 0.5f) +
                           x568_pirateData.x9c_averageNextShotTime;
      if (x7c4_burstFire.GetTimeToNextShot() > 0.f)
        x7c4_burstFire.SetTimeToNextShot(nextShotTime);
    } else if (!x7c4_burstFire.IsBurstSet()) {
      reset = true;
    }
  }

  if (reset)
    SquadReset(mgr);

  xe7_31_targetable = CheckTargetable(mgr);
}

zeus::CVector3f CSpacePirate::GetTargetPos(const CStateManager& mgr) {
  if (x7c0_targetId != mgr.GetPlayer().GetUniqueId()) {
    if (TCastToConstPtr<CActor> act = mgr.GetObjectById(x7c0_targetId))
      if (act->GetActive())
        return act->GetTranslation();
    x764_boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
    x7c0_targetId = mgr.GetPlayer().GetUniqueId();
  }
  return mgr.GetPlayer().GetTranslation();
}

void CSpacePirate::UpdateAimBodyState(float dt, const CStateManager& mgr) {
  if (x400_25_alive && x637_25_enableAim && !x637_29_inWallHang && !x450_bodyController->IsFrozen() && !x634_27_melee &&
      !x85c_ragDoll && (!x635_26_seated || x639_28_satUp) && x31c_faceVec.z() <= 0.f) {
    x8c4_aimDelayTimer = std::max(0.f, x8c4_aimDelayTimer - dt);
    if (x8c4_aimDelayTimer == 0.f) {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCAdditiveAimCmd());
      x450_bodyController->GetCommandMgr().DeliverAdditiveTargetVector(
          x34_transform.transposeRotate(GetTargetPos(mgr) - GetTranslation()));
    }
  } else if (x637_25_enableAim && !x634_27_melee) {
    x450_bodyController->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::AdditiveIdle));
  }
}

void CSpacePirate::SetCinematicCollision(CStateManager& mgr) {
  RemoveMaterial(EMaterialTypes::AIBlock, mgr);
  CMaterialFilter filter = GetMaterialFilter();
  filter.IncludeList().Remove(EMaterialTypes::AIBlock);
  SetMaterialFilter(filter);
}

void CSpacePirate::SetNonCinematicCollision(CStateManager& mgr) {
  AddMaterial(EMaterialTypes::AIBlock, mgr);
  CMaterialFilter filter = GetMaterialFilter();
  filter.IncludeList().Add(EMaterialTypes::AIBlock);
  SetMaterialFilter(filter);
}

void CSpacePirate::CheckForProjectiles(CStateManager& mgr) {
  if (x637_26_hearPlayerFire) {
    zeus::CVector3f aimPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
    zeus::CAABox aabb(aimPos - 5.f, aimPos + 5.f);
    rstl::reserved_vector<TUniqueId, 1024> nearList;
    mgr.BuildNearList(nearList, aabb, CMaterialFilter::MakeInclude({EMaterialTypes::Projectile}), nullptr);
    for (TUniqueId id : nearList) {
      if (TCastToConstPtr<CGameProjectile> proj = mgr.GetObjectById(id)) {
        zeus::CVector3f delta = GetBoundingBox().center() - proj->GetTranslation();
        if (delta.isMagnitudeSafe()) {
          if (x34_transform.basis[1].dot(delta) < 0.f) {
            delta.normalize();
            zeus::CVector3f projDelta = proj->GetTranslation() - proj->GetPreviousPos();
            if (projDelta.isMagnitudeSafe()) {
              projDelta.normalize();
              if (projDelta.dot(delta) > 0.939f)
                x637_27_inProjectilePath = true;
            }
          }
        } else {
          x637_27_inProjectilePath = true;
        }
        if (x637_27_inProjectilePath)
          break;
      }
    }
    x637_26_hearPlayerFire = false;
  }
}

void CSpacePirate::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  if (!x450_bodyController->GetActive())
    x450_bodyController->Activate(mgr);

  bool inCineCam = mgr.GetCameraManager()->IsInCinematicCamera();
  if (inCineCam && !x637_31_prevInCineCam)
    SetCinematicCollision(mgr);
  else if (!inCineCam && x637_31_prevInCineCam && !x635_31_ragdollNoAiCollision)
    SetNonCinematicCollision(mgr);
  x637_31_prevInCineCam = inCineCam;

  float steeringSpeed = x748_steeringDelayTimer == 0.f ? x644_steeringSpeed : 0.f;
  x450_bodyController->GetCommandMgr().SetSteeringSpeedRange(steeringSpeed, steeringSpeed);

  x744_unkTimer = std::max(x744_unkTimer - dt, 0.f);

  if (x400_25_alive) {
    x850_timeSinceHitByPlayer += dt;
    x854_lowHealthFrenzyTimer += dt;
    if (x637_27_inProjectilePath) {
      x854_lowHealthFrenzyTimer = 0.f;
      x637_27_inProjectilePath = false;
    }
    if (x400_24_hitByPlayerProjectile) {
      x850_timeSinceHitByPlayer = 0.f;
      x400_24_hitByPlayerProjectile = false;
    }
  }

  UpdateCloak(dt, mgr);

  if (!x450_bodyController->IsFrozen()) {
    if (x400_25_alive) {
      x748_steeringDelayTimer = std::max(x748_steeringDelayTimer - dt, 0.f);
      if (x637_28_noPlayerLos)
        x7ac_timeNoPlayerLos += dt;
      else
        x7ac_timeNoPlayerLos = 0.f;
      x838_strafeDelayTimer = std::max(x838_strafeDelayTimer - dt, 0.f);
      x8c0_dodgeDelayTimer = std::max(x8c0_dodgeDelayTimer - dt, 0.f);
      CheckForProjectiles(mgr);
    }
    UpdateAttacks(dt, mgr);
    UpdateAimBodyState(dt, mgr);
    x860_ikChain.Update(dt);
  }

  if (x634_24_pendingAmbush) {
    x634_24_pendingAmbush = false;
    if (x634_25_ceilingAmbush)
      x450_bodyController->SetLocomotionType(pas::ELocomotionType::Internal6);
    else
      x450_bodyController->SetLocomotionType(pas::ELocomotionType::Crouch);
    x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), "Ambushing"sv);
  }

  if (!x85c_ragDoll || !x85c_ragDoll->IsPrimed()) {
    CPatterned::Think(dt, mgr);
    if (!x450_bodyController->IsFrozen())
      x764_boneTracking.Update(dt);
  } else {
    UpdateAlphaDelta(dt, mgr);
    UpdateDamageColor(dt);
    if (CSfxHandle hnd = GetSfxHandle())
      CSfxManager::UpdateEmitter(hnd, GetTranslation(), zeus::skZero3f, 1.f);
  }
  if (x85c_ragDoll) {
    if (!x85c_ragDoll->IsPrimed()) {
      x85c_ragDoll->Prime(mgr, GetTransform(), *x64_modelData);
      zeus::CVector3f trans = GetTranslation();
      SetTransform({});
      SetTranslation(trans);
      x450_bodyController->SetPlaybackRate(0.f);
    } else {
      float waterTop = -FLT_MAX;
      if (xc4_fluidId != kInvalidUniqueId)
        if (TCastToConstPtr<CScriptWater> water = mgr.GetObjectById(xc4_fluidId))
          if (water->GetActive())
            waterTop = water->GetTriggerBoundsWR().max.z();
      x85c_ragDoll->Update(mgr, dt * CalcDyingThinkRate(), waterTop);
      x64_modelData->AdvanceParticles(x34_transform, dt, mgr);
    }
    if (x85c_ragDoll->IsOver() && !x85c_ragDoll->WillContinueSmallMovements() && !x400_27_fadeToDeath) {
      /* Ragdoll has finished animating */
      x400_27_fadeToDeath = true;
      AddMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
      x3e8_alphaDelta = -0.333333f;
      x638_30_ragdollOver = true;
      SetMomentumWR(zeus::skZero3f);
      CPhysicsActor::Stop();
    }
  }
  if (x858_ragdollDelayTimer > 0.f) {
    x858_ragdollDelayTimer -= dt;
    if (x858_ragdollDelayTimer <= 0.f) {
      if (!x85c_ragDoll) {
        x85c_ragDoll =
            std::make_unique<CPirateRagDoll>(mgr, this, x568_pirateData.x98_Sound_Impact,
                                             (x635_30_floatingCorpse ? 3 : 0) | (x635_31_ragdollNoAiCollision ? 4 : 0));
        RemoveMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
      }
      x858_ragdollDelayTimer = 0.f;
    }
  }
}

void CSpacePirate::SetEyeParticleActive(CStateManager& mgr, bool active) {
  if (!x636_24_trooper) {
    if (!x634_29_onlyAttackInRange || x635_26_seated) {
      if (!x635_27_shadowPirate)
        x64_modelData->GetAnimationData()->SetParticleEffectState("TwoEyes"sv, active, mgr);
    } else {
      x64_modelData->GetAnimationData()->SetParticleEffectState("OneEye"sv, active, mgr);
    }
  }
}

void CSpacePirate::SetVelocityForJump() {
  if (!x637_30_jumpVelSet) {
    zeus::CVector3f delta = x828_patrolDestPos - GetTranslation();
    float grav = GetGravityConstant();
    float jumpZ = x824_jumpHeight + std::max(float(x828_patrolDestPos.z()), float(GetTranslation().z()));
    float zVel = std::sqrt((jumpZ - GetTranslation().z()) * (2.f * grav));
    float dt = zVel / grav;
    dt += std::sqrt((jumpZ - x828_patrolDestPos.z()) * 2.f / grav);
    zeus::CVector3f vel = (1.f / dt) * delta;
    vel.z() = zVel;
    CPhysicsActor::SetVelocityWR(vel);
    x637_30_jumpVelSet = true;
  }
}

void CSpacePirate::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  if (x637_29_inWallHang || x634_25_ceilingAmbush) {
    switch (msg) {
    case EScriptObjectMessage::Falling:
      if (!x637_29_inWallHang || x450_bodyController->GetCurrentStateId() != pas::EAnimationState::WallHang ||
          x450_bodyController->GetBodyStateInfo().GetCurrentState()->ApplyGravity()) {
        if (x634_25_ceilingAmbush) {
          if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Locomotion ||
              (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Jump &&
               !x450_bodyController->GetBodyStateInfo().GetCurrentState()->IsMoving())) {
            CPhysicsActor::Stop();
            SetMomentumWR(zeus::skZero3f);
            return;
          }
        }
      }
      break;
    case EScriptObjectMessage::OnFloor:
      x850_timeSinceHitByPlayer = FLT_MAX;
      break;
    default:
      break;
    }
  }
  switch (msg) {
  case EScriptObjectMessage::Alert:
  case EScriptObjectMessage::Activate:
    if (GetActive()) {

      if (x634_29_onlyAttackInRange)
        x638_31_mayStartAttack = true;
      else
        x400_24_hitByPlayerProjectile = true;

      SquadAdd(mgr);
    } else if (x634_25_ceilingAmbush) {
      RemoveMaterial(EMaterialTypes::GroundCollider, mgr);
      x328_27_onGround = false;
    }
    break;
  default:
    break;
  }
  CPatterned::AcceptScriptMsg(msg, sender, mgr);
  switch (msg) {
  case EScriptObjectMessage::InitializedInArea:
    for (const auto& conn : GetConnectionList()) {
      if (conn.x0_state == EScriptObjectState::Retreat && conn.x4_msg == EScriptObjectMessage::Next) {
        TUniqueId id = mgr.GetIdForScript(conn.x8_objId);
        if (TCastToPtr<CScriptCoverPoint> cp = mgr.ObjectById(id))
          cp->Reserve(GetUniqueId());
      } else if (conn.x0_state == EScriptObjectState::Patrol && conn.x4_msg == EScriptObjectMessage::Follow) {
        x637_24_enablePatrol = true;
      }
    }
    x660_pathFindSearch.SetArea(mgr.GetWorld()->GetAreaAlways(x4_areaId)->GetPostConstructed()->x10bc_pathArea);
    if (x635_30_floatingCorpse) {
      x858_ragdollDelayTimer = 0.01f;
      RemoveMaterial(EMaterialTypes::Character, mgr);
      x400_25_alive = false;
      HealthInfo(mgr)->SetHP(-1.f);
    } else {
      SetEyeParticleActive(mgr, true);
    }
    break;
  case EScriptObjectMessage::Decrement:
    if (x85c_ragDoll) {
      x85c_ragDoll->SetNoOverTimer(false);
      x85c_ragDoll->SetContinueSmallMovements(false);
    }
    break;
  case EScriptObjectMessage::Registered: {
    if (x634_25_ceilingAmbush) {
      x634_24_pendingAmbush = true;
      if (x635_27_shadowPirate) {
        x42c_color.a() = x568_pirateData.xb0_CloakOpacity;
        x3e8_alphaDelta = -1.f;
      }
    }
    x75c_ = mgr.GetActiveRandom()->Next() % 6;
    CMaterialFilter filter = GetMaterialFilter();
    filter.IncludeList().Remove(EMaterialTypes::AIPassthrough);
    filter.ExcludeList().Add(EMaterialTypes::AIPassthrough);
    SetMaterialFilter(filter);
    break;
  }
  case EScriptObjectMessage::SetToZero:
    if (x30_24_active)
      x636_29_enableRetreat = true;
    break;
  case EScriptObjectMessage::Falling:
    if (!x450_bodyController->IsFrozen()) {
      float zMom = GetGravityConstant() * xe8_mass;
      if (x634_25_ceilingAmbush)
        zMom *= 3.f;
      SetMomentumWR({0.f, 0.f, -zMom});
    }
    if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Step)
      SetVelocityWR({0.f, 0.f, x138_velocity.z()});
    x7c4_burstFire.SetBurstType(3);
    break;
  case EScriptObjectMessage::Jumped:
    CPatterned::AcceptScriptMsg(msg, sender, mgr);
    SetMomentumWR({0.f, 0.f, -GetGravityConstant() * xe8_mass});
    SetVelocityForJump();
    break;
  case EScriptObjectMessage::OnFloor:
    if (!x634_29_onlyAttackInRange)
      x7c4_burstFire.SetBurstType(1);
    else
      x7c4_burstFire.SetBurstType(4);
    x637_30_jumpVelSet = false;
    if (x635_27_shadowPirate && x138_velocity.z() < -1.f) {
      x3e8_alphaDelta = 1.f;
      x8a8_cloakDelayTimer += -0.05f * x138_velocity.z();
      x8a8_cloakDelayTimer = zeus::clamp(0.f, x8a8_cloakDelayTimer, 1.f);
      x8bc_maxCloakAlpha = 0.5f;
      if (x400_25_alive) {
        mgr.GetActorModelParticles()->LoadAndStartElectric(*this);
        x8ac_electricParticleTimer = 1.f + x8a8_cloakDelayTimer;
      }
    }
    break;
  case EScriptObjectMessage::Action:
    if (TCastToPtr<CScriptTargetingPoint> tp = mgr.ObjectById(sender)) {
      if (tp->GetActive()) {
        x764_boneTracking.SetTarget(sender);
        x7c0_targetId = sender;
        x400_24_hitByPlayerProjectile = true;
      } else {
        x764_boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
        x7c0_targetId = mgr.GetPlayer().GetUniqueId();
      }
      x7bc_attackRemTime = 0.f;
    }
    break;
  case EScriptObjectMessage::Deactivate:
  case EScriptObjectMessage::Deleted:
    SquadRemove(mgr);
    mChargePlayerList.remove(GetUniqueId());
    break;
  case EScriptObjectMessage::Start:
    x639_26_started = false;
    break;
  case EScriptObjectMessage::Stop:
    x639_26_started = true;
    break;
  default:
    break;
  }
}

void CSpacePirate::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  if (x85c_ragDoll && x85c_ragDoll->IsPrimed())
    x85c_ragDoll->PreRender(GetTranslation(), *x64_modelData);
  CPatterned::PreRender(mgr, frustum);
  if (!x85c_ragDoll || !x85c_ragDoll->IsPrimed()) {
    x764_boneTracking.PreRender(mgr, *x64_modelData->GetAnimationData(), x34_transform, x64_modelData->GetScale(),
                                *x450_bodyController);
    x860_ikChain.PreRender(*x64_modelData->GetAnimationData(), x34_transform, x64_modelData->GetScale());
  }
}

void CSpacePirate::Render(CStateManager& mgr) {
  float time = x400_25_alive ? CGraphics::GetSecondsMod900() : 0.f;
  CTimeProvider prov(time);
  g_Renderer->SetGXRegister1Color(x8cc_trooperColor);
  CPatterned::Render(mgr);
}

void CSpacePirate::CalculateRenderBounds() {
  if (x85c_ragDoll && x85c_ragDoll->IsPrimed()) {
    zeus::CVector3f margin = x64_modelData->GetScale() * 0.2f;
    zeus::CAABox ragdollBounds = x85c_ragDoll->CalculateRenderBounds();
    x9c_renderBounds = zeus::CAABox(ragdollBounds.min - margin, ragdollBounds.max + margin);
  } else {
    CActor::CalculateRenderBounds();
  }
}

void CSpacePirate::Touch(CActor& other, CStateManager& mgr) {
  CPatterned::Touch(other, mgr);
  if (x85c_ragDoll && x85c_ragDoll->IsPrimed()) {
    if (TCastToPtr<CScriptTrigger> trig = other) {
      if (trig->GetActive() && True(trig->GetTriggerFlags() & ETriggerFlags::DetectAI) &&
          trig->GetForceMagnitude() > 0.f) {
        x85c_ragDoll->TorsoImpulse() += trig->GetForceVector();
      }
    }
  }
}

zeus::CAABox CSpacePirate::GetSortingBounds(const CStateManager& mgr) const {
  zeus::CAABox aabb = x64_modelData->GetBounds(x34_transform);
  zeus::CVector3f radius = aabb.extents() * 0.5f;
  zeus::CVector3f center = aabb.center();
  return zeus::CAABox(center - radius, center + radius);
}

void CSpacePirate::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case EUserEventType::BeginAction:
    RemoveMaterial(EMaterialTypes::Solid, mgr);
    x638_30_ragdollOver = true;
    handled = true;
    break;
  case EUserEventType::EndAction:
    x639_30_closeMelee = false;
    handled = true;
    break;
  case EUserEventType::Projectile:
  case EUserEventType::BecomeRagDoll:
    if (x634_29_onlyAttackInRange || HealthInfo(mgr)->GetHP() <= 0.f)
      x858_ragdollDelayTimer = mgr.GetActiveRandom()->Float() * 0.05f + 0.001f;
    handled = true;
    break;
  case EUserEventType::IkLock:
    if (!x860_ikChain.GetActive()) {
      CSegId lctrId = x64_modelData->GetAnimationData()->GetLocatorSegId(node.GetLocatorName());
      if (lctrId != 3) {
        zeus::CTransform xf = GetLctrTransform(lctrId);
        x860_ikChain.Activate(*x64_modelData->GetAnimationData(), lctrId, xf);
        x639_28_satUp = true;
      }
    }
    handled = true;
    break;
  case EUserEventType::IkRelease:
    x860_ikChain.Deactivate();
    handled = true;
    break;
  case EUserEventType::ScreenShake:
    SendScriptMsgs(EScriptObjectState::Play, mgr, EScriptObjectMessage::None);
    handled = true;
    break;
  case EUserEventType::FadeOut:
    x3e8_alphaDelta = -0.8f;
    mgr.GetActorModelParticles()->LoadAndStartElectric(*this);
    x8ac_electricParticleTimer = 1.f;
    handled = true;
    break;
  default:
    break;
  }
  if (!handled)
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

void CSpacePirate::Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) {
  if (x400_25_alive) {
    CPatterned::Death(mgr, direction, state);
    if (x7b4_attachedActor != kInvalidUniqueId) {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCKnockDownCmd(GetTransform().basis[1], pas::ESeverity::Two));
    }
  }
}

void CSpacePirate::KnockBack(const zeus::CVector3f& backVec, CStateManager& mgr, const CDamageInfo& info,
                             EKnockBackType type, bool inDeferred, float magnitude) {
  if (!x634_25_ceilingAmbush || !x400_25_alive || inDeferred) {
    x460_knockBackController.SetAutoResetImpulse(!x634_31_noKnockbackImpulseReset);
    x460_knockBackController.SetAvailableState(EKnockBackAnimationState::KnockBack, IsOnGround());
    x460_knockBackController.SetEnableFreeze(!((x636_24_trooper || x635_27_shadowPirate) &&
                                               !info.GetWeaponMode().IsCharged() && !info.GetWeaponMode().IsComboed()));
    CPatterned::KnockBack(backVec, mgr, info, type, inDeferred, magnitude);
    if (x635_27_shadowPirate) {
      if (x400_25_alive) {
        if (magnitude >= 4.f && !x450_bodyController->IsFrozen()) {
          x3e8_alphaDelta = 1.f;
          x8a8_cloakDelayTimer += 0.1f * magnitude;
          x8a8_cloakDelayTimer = zeus::clamp(0.f, x8a8_cloakDelayTimer, 1.f);
          x8bc_maxCloakAlpha = 0.5f;
          mgr.GetActorModelParticles()->LoadAndStartElectric(*this);
          x8ac_electricParticleTimer = x8a8_cloakDelayTimer + 1.f;
        }
      } else {
        x8bc_maxCloakAlpha = x3e8_alphaDelta = 1.f;
        x8b8_minCloakAlpha = 0.f;
        mgr.GetActorModelParticles()->LoadAndStartElectric(*this);
        x8ac_electricParticleTimer = 2.f;
      }
    }
    if (x635_30_floatingCorpse && x85c_ragDoll)
      x85c_ragDoll->TorsoImpulse() += (20.f * magnitude) * backVec;
    if (x400_25_alive) {
      if (x460_knockBackController.GetActiveParms().x0_animState == EKnockBackAnimationState::Hurled) {
        x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), "GetUpNow"sv);
        CSfxManager::AddEmitter(x568_pirateData.xc0_Sound_Hurled, GetTranslation(), zeus::skZero3f, 1.f, true, false,
                                0x7f, kInvalidAreaId);
      }
    } else {
      if (x460_knockBackController.GetActiveParms().x0_animState == EKnockBackAnimationState::Hurled &&
          x460_knockBackController.GetActiveParms().x4_animFollowup != EKnockBackAnimationFollowUp::LaggedBurnDeath &&
          x460_knockBackController.GetActiveParms().x4_animFollowup != EKnockBackAnimationFollowUp::BurnDeath) {
        CSfxManager::AddEmitter(x568_pirateData.xc2_Sound_Death, GetTranslation(), zeus::skZero3f, 1.f, true, false,
                                0x7f, kInvalidAreaId);
      }
    }
  }
}

bool CSpacePirate::IsListening() const { return true; }

bool CSpacePirate::Listen(const zeus::CVector3f& pos, EListenNoiseType type) {
  bool ret = false;
  if (x400_25_alive) {
    zeus::CVector3f delta = pos - GetTranslation();
    if (delta.magSquared() < x568_pirateData.x14_HearingRadius * x568_pirateData.x14_HearingRadius &&
        (x3c0_detectionHeightRange == 0.f ||
         delta.z() * delta.z() < x3c0_detectionHeightRange * x3c0_detectionHeightRange))
      x636_25_hearNoise = true;
    if (type == EListenNoiseType::PlayerFire)
      x637_26_hearPlayerFire = true;
  }
  return ret;
}

zeus::CVector3f CSpacePirate::GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                                        const zeus::CVector3f& aimPos) const {
  return GetTranslation();
}

void CSpacePirate::AvoidActors(CStateManager& mgr) {
  for (CEntity* ent : mgr.GetListeningAiObjectList()) {
    if (TCastToPtr<CPatterned> ai = ent) {
      if (ai.GetPtr() != this && ai->GetAreaIdAlways() == GetAreaIdAlways()) {
        zeus::CVector3f deltaVec =
            x45c_steeringBehaviors.Separation(*this, ai->GetTranslation(), x568_pirateData.xc8_AvoidDistance);
        if (!deltaVec.isZero()) {
          x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(deltaVec, zeus::skZero3f, 1.f));
          if (x748_steeringDelayTimer == 0.f) {
            if (CSpacePirate* otherSp = CPatterned::CastTo<CSpacePirate>(ai.GetPtr())) {
              if (otherSp->x748_steeringDelayTimer == 0.f &&
                  (otherSp->GetTranslation() - GetTranslation()).dot(GetTransform().basis[1]) > 0.f &&
                  otherSp->GetTransform().basis[1].dot(otherSp->GetVelocity()) > 0.f) {
                x748_steeringDelayTimer = 1.f;
              }
            }
          }
        }
      }
    }
  }
}

bool CSpacePirate::AttachActorToPirate(TUniqueId id) {
  if (x7b4_attachedActor == kInvalidUniqueId) {
    x7b4_attachedActor = id;
    return true;
  }
  return false;
}

void CSpacePirate::SetAttackTarget(TUniqueId id) {
  x7c0_targetId = id;
  x7c4_burstFire.SetBurstType(1);
  x7bc_attackRemTime = 0.f;
}

void CSpacePirate::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    x644_steeringSpeed = x450_bodyController->GetBodyStateInfo().GetLocomotionSpeed(pas::ELocomotionAnim::Walk) /
                         x450_bodyController->GetBodyStateInfo().GetLocomotionSpeed(pas::ELocomotionAnim::Run);
    break;
  case EStateMsg::Deactivate:
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Combat);
    if (!x639_31_sentAttackMsg) {
      x639_31_sentAttackMsg = true;
      SendScriptMsgs(EScriptObjectState::Attack, mgr, EScriptObjectMessage::None);
    }
    break;
  default:
    break;
  }
  if (x637_24_enablePatrol) {
    CPatterned::Patrol(mgr, msg, dt);
    switch (msg) {
    case EStateMsg::Activate:
      x450_bodyController->GetCommandMgr().SetSteeringBlendMode(ESteeringBlendMode::FullSpeed);
      x450_bodyController->SetTurnSpeed(x450_bodyController->GetTurnSpeed() / 1.25f);
      break;
    case EStateMsg::Update:
      AvoidActors(mgr);
      x828_patrolDestPos = x2e0_destPos;
      break;
    case EStateMsg::Deactivate:
      x450_bodyController->GetCommandMgr().SetSteeringBlendMode(ESteeringBlendMode::Normal);
      x450_bodyController->SetTurnSpeed(x450_bodyController->GetTurnSpeed() * 1.25f);
      break;
    }
  }
}

void CSpacePirate::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::Dead(mgr, msg, dt);
  switch (msg) {
  case EStateMsg::Activate:
    x764_boneTracking.SetActive(false);
    SetEyeParticleActive(mgr, false);
    SquadReset(mgr);
    break;
  case EStateMsg::Update:
    if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Death) {
      RemoveMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
      RemoveMaterial(EMaterialTypes::GroundCollider, EMaterialTypes::NoStaticCollision, EMaterialTypes::AIBlock, mgr);
      AddMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
      SetMomentumWR(zeus::skZero3f);
      CPhysicsActor::Stop();
    }
    break;
  default:
    break;
  }
}

bool CSpacePirate::LineOfSightTest(const CStateManager& mgr, const zeus::CVector3f& eyePos,
                                   const zeus::CVector3f& targetPos, const CMaterialList& excludeList) const {
  return mgr.RayCollideWorld(eyePos, targetPos,
                             CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, excludeList), this);
}

void CSpacePirate::UpdateCantSeePlayer(CStateManager& mgr) {
  if ((++x7b0_cantSeePlayerCycleCounter + 1) % 7 == 0) {
    zeus::CVector3f eyePos = GetTranslation() + zeus::CVector3f(0.f, 0.f, x7a8_eyeHeight);
    zeus::CVector3f aimPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
    if (CScriptCoverPoint* cp = GetCoverPoint(mgr, x640_coverPoint)) {
      switch (x79c_coverDir) {
      case pas::ECoverDirection::Left:
        eyePos -= GetTransform().basis[1] * 2.f;
        break;
      case pas::ECoverDirection::Right:
        eyePos += GetTransform().basis[1] * 2.f;
        break;
      default:
        break;
      }
    } else {
      eyePos += (aimPos - eyePos).normalized().cross(zeus::skUp) * 1.1f;
    }
    x637_28_noPlayerLos = !LineOfSightTest(mgr, eyePos, mgr.GetPlayer().GetAimPosition(mgr, 0.f),
                                           {EMaterialTypes::Player, EMaterialTypes::ProjectilePassthrough});
  }
}

void CSpacePirate::UpdateHeldPosition(CStateManager& mgr, float dt) {
  if ((mgr.GetPlayer().GetTranslation().toVec2f() - x8d0_heldPosition).magSquared() < 3.f) {
    x8d8_holdPositionTime += dt;
  } else {
    x8d0_heldPosition = mgr.GetPlayer().GetTranslation().toVec2f();
    x8d8_holdPositionTime = 0.f;
  }
}

void CSpacePirate::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x840_jumpPoint = kInvalidUniqueId;
    if (CScriptCoverPoint* cp = GetCoverPoint(mgr, x640_coverPoint)) {
      x2ec_reflectedDestPos = GetTranslation();
      x328_24_inPosition = false;
      x2dc_destObj = cp->GetUniqueId();
      x2e0_destPos = cp->GetTranslation();
    }
    if (GetSearchPath()->Search(GetTranslation(), x2e0_destPos) == CPathFindSearch::EResult::Success) {
      x2ec_reflectedDestPos = GetTranslation();
      x2e0_destPos = (GetSearchPath()->GetCurrentWaypoint() + 1 < GetSearchPath()->GetWaypoints().size())
                         ? GetSearchPath()->GetWaypoints()[GetSearchPath()->GetCurrentWaypoint() + 1]
                         : GetSearchPath()->GetWaypoints()[GetSearchPath()->GetCurrentWaypoint()];
      x328_24_inPosition = false;
      x450_bodyController->GetCommandMgr().DeliverCmd(
          CBCLocomotionCmd(x2e0_destPos - GetTranslation(), zeus::skZero3f, 1.f));
    } else {
      CScriptAiJumpPoint* bestJp = nullptr;
      float minDist = FLT_MAX;
      for (CEntity* ent : mgr.GetAiWaypointObjectList()) {
        if (TCastToPtr<CScriptAiJumpPoint> jp = ent) {
          if (jp->GetActive() && !jp->GetInUse(GetUniqueId()) && jp->GetJumpTarget() == kInvalidUniqueId &&
              GetAreaIdAlways() == jp->GetAreaIdAlways()) {
            zeus::CVector3f toJp = jp->GetTranslation() - GetTranslation();
            float f30 = toJp.magSquared();
            if (f30 > 25.f && jp->GetTransform().basis[1].dot(toJp) > 0.f) {
              if (TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(jp->GetJumpPoint())) {
                if ((wp->GetTranslation().z() - jp->GetTranslation().z()) * (x2e0_destPos.z() - GetTranslation().z()) >
                    0.f) {
                  zeus::CVector3f delta = x2e0_destPos - wp->GetTranslation();
                  f30 += 4.f * toJp.z() * toJp.z();
                  f30 += delta.magSquared() + delta.z() * delta.z() * 9.f;
                  if (f30 < minDist && GetSearchPath()->PathExists(GetTranslation(), jp->GetTranslation()) ==
                                           CPathFindSearch::EResult::Success) {
                    bool r24 = false;
                    auto res = GetSearchPath()->PathExists(wp->GetTranslation(), x2e0_destPos);
                    if (res != CPathFindSearch::EResult::Success)
                      f30 += 1000.f;
                    if (res == CPathFindSearch::EResult::Success)
                      r24 = true;
                    if (f30 < minDist) {
                      minDist = f30;
                      bestJp = jp.GetPtr();
                      if (r24)
                        break;
                    }
                  }
                }
              }
            }
          }
        }
      }
      if (bestJp) {
        x2e0_destPos = bestJp->GetTranslation();
        if (GetSearchPath()->Search(GetTranslation(), x2e0_destPos) == CPathFindSearch::EResult::Success) {
          x2ec_reflectedDestPos = GetTranslation();
          x2e0_destPos = (GetSearchPath()->GetCurrentWaypoint() + 1 < GetSearchPath()->GetWaypoints().size())
                             ? GetSearchPath()->GetWaypoints()[GetSearchPath()->GetCurrentWaypoint() + 1]
                             : GetSearchPath()->GetWaypoints()[GetSearchPath()->GetCurrentWaypoint()];
          x328_24_inPosition = false;
          x840_jumpPoint = bestJp->GetUniqueId();
          x824_jumpHeight = bestJp->GetJumpApex();
          if (TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(bestJp->GetJumpPoint())) {
            x828_patrolDestPos = wp->GetTranslation();
            x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(x2e0_destPos, zeus::skZero3f, 1.f));
            x30c_behaviourOrient = EBehaviourOrient::MoveDir;
          }
        }
      }
    }
    x450_bodyController->GetCommandMgr().SetSteeringBlendMode(ESteeringBlendMode::FullSpeed);
    if (x637_25_enableAim)
      x644_steeringSpeed = 1.f;
    x639_27_inRange = false;
    x63a_24_normalDodge = true;
    break;
  case EStateMsg::Update:
    CPatterned::PathFind(mgr, msg, dt);
    if (x840_jumpPoint != kInvalidUniqueId) {
      if (TCastToPtr<CScriptAiJumpPoint> jp = mgr.ObjectById(x840_jumpPoint)) {
        float f0 =
            (1.5f * dt + 0.1f) * x64_modelData->GetScale().y() * x450_bodyController->GetBodyStateInfo().GetMaxSpeed() +
            x7a4_intoJumpDist;
        if ((GetTranslation() - jp->GetTranslation()).magSquared() < f0 * f0) {
          x32c_animState = EAnimState::Ready;
          TryCommand(mgr, pas::EAnimationState::Jump, &CPatterned::TryJump, 0);
        }
      }
    }
    AvoidActors(mgr);
    if (!x639_27_inRange) {
      if (CScriptCoverPoint* cp = GetCoverPoint(mgr, x640_coverPoint)) {
        x754_fsmRange =
            (1.5f * dt + 0.1f) * x64_modelData->GetScale().y() * x450_bodyController->GetBodyStateInfo().GetMaxSpeed();
        if (cp->ShouldWallHang())
          x754_fsmRange += x7a4_intoJumpDist;
        x639_27_inRange = (GetTranslation() - cp->GetTranslation()).magSquared() < x754_fsmRange * x754_fsmRange;
      }
    }
    UpdateCantSeePlayer(mgr);
    UpdateHeldPosition(mgr, dt);
    break;
  case EStateMsg::Deactivate:
    x32c_animState = EAnimState::NotReady;
    x840_jumpPoint = kInvalidUniqueId;
    x30c_behaviourOrient = EBehaviourOrient::Constant;
    x639_27_inRange = false;
    x450_bodyController->GetCommandMgr().SetSteeringBlendMode(ESteeringBlendMode::Normal);
    break;
  }
}

void CSpacePirate::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::Patrol(mgr, msg, dt);
  switch (msg) {
  case EStateMsg::Activate:
    x644_steeringSpeed = 1.f;
    x450_bodyController->GetCommandMgr().SetSteeringBlendMode(ESteeringBlendMode::FullSpeed);
    x2dc_destObj = GetWaypointForState(mgr, EScriptObjectState::Attack, EScriptObjectMessage::Follow);
    break;
  case EStateMsg::Update:
    if (TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(x2dc_destObj)) {
      if ((wp->GetBehaviourModifiers() & 0x2) != 0 || (wp->GetBehaviourModifiers() & 0x4)) {
        float f0 =
            (1.5f * dt + 0.1f) * x64_modelData->GetScale().y() * x450_bodyController->GetBodyStateInfo().GetMaxSpeed() +
            x7a4_intoJumpDist;
        if ((GetTranslation() - wp->GetTranslation()).magSquared() < f0 * f0) {
          x328_24_inPosition = true;
          x824_jumpHeight = (wp->GetBehaviourModifiers() & 0x2) ? 3.f : 0.f;
        }
      }
    }
    if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Jump) {
      bool r28 = true;
      if (TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(x2dc_destObj)) {
        for (const auto& conn : wp->GetConnectionList()) {
          if (conn.x0_state == EScriptObjectState::Arrived && conn.x4_msg == EScriptObjectMessage::Next) {
            r28 = false;
          }
        }
      }
      if (r28)
        x450_bodyController->GetCommandMgr().DeliverTargetVector(mgr.GetPlayer().GetTranslation() - GetTranslation());
    }
    x828_patrolDestPos = x2e0_destPos;
    break;
  case EStateMsg::Deactivate:
    x450_bodyController->GetCommandMgr().SetSteeringBlendMode(ESteeringBlendMode::Normal);
    break;
  }
}

void CSpacePirate::TargetCover(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    if (CScriptCoverPoint* cp = GetCoverPoint(mgr, x640_coverPoint)) {
      x2dc_destObj = x640_coverPoint;
      x2e0_destPos = cp->GetTranslation();
    }
    x2ec_reflectedDestPos = GetTranslation();
    x328_24_inPosition = false;
    break;
  default:
    break;
  }
}

void CSpacePirate::Halt(CStateManager& mgr, EStateMsg msg, float dt) { x644_steeringSpeed = 0.f; }

void CSpacePirate::Run(CStateManager& mgr, EStateMsg msg, float dt) { x644_steeringSpeed = 1.f; }

void CSpacePirate::Generate(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x637_25_enableAim = true;
    if (!x639_31_sentAttackMsg) {
      x639_31_sentAttackMsg = true;
      SendScriptMsgs(EScriptObjectState::Attack, mgr, EScriptObjectMessage::None);
    }
    x32c_animState = EAnimState::Ready;
    if (x634_25_ceilingAmbush) {
      x2e0_destPos = GetTranslation() + zeus::skDown;
      x828_patrolDestPos = x2e0_destPos;
      x824_jumpHeight = 0.f;
    } else {
      TUniqueId wpId = GetWaypointForState(mgr, EScriptObjectState::Attack, EScriptObjectMessage::Follow);
      if (TCastToConstPtr<CActor> act = mgr.GetObjectById(wpId)) {
        x2e0_destPos = act->GetTranslation();
        x828_patrolDestPos = x2e0_destPos;
        x824_jumpHeight = 3.f;
      }
      x450_bodyController->SetLocomotionType(pas::ELocomotionType::Combat);
    }
    break;
  case EStateMsg::Update:
    TryCommand(mgr, pas::EAnimationState::Jump, &CPatterned::TryJump, x634_25_ceilingAmbush ? 2 : 0);
    if (x32c_animState == EAnimState::Repeat)
      x450_bodyController->SetLocomotionType(pas::ELocomotionType::Combat);
    x450_bodyController->GetCommandMgr().DeliverTargetVector(mgr.GetPlayer().GetTranslation() - GetTranslation());
    break;
  case EStateMsg::Deactivate:
    x32c_animState = EAnimState::NotReady;
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Combat);
    x824_jumpHeight = 3.f;
    x634_25_ceilingAmbush = false;
    x764_boneTracking.SetActive(true);
    x764_boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
    break;
  }
}

void CSpacePirate::Deactivate(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate)
    x401_30_pendingDeath = true;
}

void CSpacePirate::CheckBlade(CStateManager& mgr) {
  if (x638_25_appliedBladeDamage || x7b9_swooshSeg.IsInvalid()) {
    return;
  }

  if (TCastToPtr<CPhysicsActor> act = mgr.ObjectById(x7c0_targetId)) {
    zeus::CVector3f extent = x64_modelData->GetScale() * 0.5f;
    zeus::CVector3f swooshPos = GetLctrTransform(x7b9_swooshSeg).origin;
    if (zeus::CAABox(swooshPos - extent, swooshPos + extent).intersects(act->GetBoundingBox())) {
      mgr.ApplyDamage(GetUniqueId(), act->GetUniqueId(), GetUniqueId(), x568_pirateData.x4c_BladeDamage,
                      CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), zeus::skZero3f);
      x638_25_appliedBladeDamage = true;
    }
  }
}

void CSpacePirate::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x32c_animState = EAnimState::Ready;
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Combat);
    x2e0_destPos = GetTargetPos(mgr);
    x648_targetDelta = x2e0_destPos - GetBoundingBox().center();
    x644_steeringSpeed = 0.f;
    x636_26_enableMeleeAttack = false;
    if (!x635_24_noMeleeAttack && x648_targetDelta.magSquared() < x2fc_minAttackRange * x2fc_minAttackRange &&
        x648_targetDelta.z() * x648_targetDelta.z() < 4.f) {
      x636_26_enableMeleeAttack = true;
      x638_25_appliedBladeDamage = false;
    } else if (x648_targetDelta.normalized().dot(GetTransform().basis[1]) < 0.8f) {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(zeus::skZero3f, x648_targetDelta, 1.f));
    }
    x636_31_inAttackState = true;
    x8bc_maxCloakAlpha = 0.75f;
    break;
  case EStateMsg::Update:
    if (x636_26_enableMeleeAttack) {
      TryCommand(mgr, pas::EAnimationState::MeleeAttack, &CPatterned::TryMeleeAttack, 1);
      x450_bodyController->GetCommandMgr().DeliverTargetVector(x648_targetDelta);
      CheckBlade(mgr);
      if (x635_27_shadowPirate) {
        if (x32c_animState == EAnimState::Over) {
          x3e8_alphaDelta = -0.4f;
        } else {
          x3e8_alphaDelta = 1.f;
          x8bc_maxCloakAlpha = 0.75f;
        }
      }
    }
    UpdateCantSeePlayer(mgr);
    UpdateHeldPosition(mgr, dt);
    break;
  case EStateMsg::Deactivate:
    x636_26_enableMeleeAttack = false;
    x636_31_inAttackState = false;
    x32c_animState = EAnimState::NotReady;
    break;
  }
}

bool CSpacePirate::CantJumpBack(const CStateManager& mgr, const zeus::CVector3f& dir, float dist) const {
  const zeus::CVector3f center = GetBoundingBox().center();
  if (!LineOfSightTest(mgr, center, center + dist * dir, {}))
    return false;
  const zeus::CVector3f center2 = (dist * 0.5f) * dir + center;
  if (LineOfSightTest(mgr, center2, center2 + 5.f * zeus::skDown, {}))
    return false;
  const zeus::CVector3f center3 = dist * dir + center;
  if (LineOfSightTest(mgr, center3, center3 + 5.f * zeus::skDown, {}))
    return false;
  return true;
}

void CSpacePirate::JumpBack(CStateManager& mgr, EStateMsg msg, float dt) {
  if (!ShouldJumpBack(mgr, dt))
    return;
  switch (msg) {
  case EStateMsg::Activate:
    if (!x634_29_onlyAttackInRange && !CantJumpBack(mgr, -GetTransform().basis[1], 5.f)) {
      float backupChHeight = GetSearchPath()->GetCharacterHeight();
      x660_pathFindSearch.SetCharacterHeight(5.f + backupChHeight);
      zeus::CVector3f dest = GetTransform().basis[1] * 10.f + GetTranslation();
      if (GetSearchPath()->Search(GetTranslation(), dest) == CPathFindSearch::EResult::Success &&
          (GetSearchPath()->GetWaypoints().back() - dest).magSquared() < 3.f &&
          std::fabs(GetSearchPath()->RemainingPathDistance(GetTranslation()) - 10.f) < 4.f) {
        x828_patrolDestPos = GetSearchPath()->GetWaypoints().back();
        x824_jumpHeight = 5.f;
        x639_25_useJumpBackJump = true;
        x32c_animState = EAnimState::Ready;
      }
      GetSearchPath()->SetCharacterHeight(backupChHeight);
    }
    break;
  case EStateMsg::Update:
    if (!x639_25_useJumpBackJump) {
      x450_bodyController->GetCommandMgr().DeliverCmd(
          CBCStepCmd(pas::EStepDirection::Backward, pas::EStepType::Normal));
      x450_bodyController->GetCommandMgr().DeliverTargetVector(GetTargetPos(mgr) - GetTranslation());
    } else {
      TryCommand(mgr, pas::EAnimationState::Jump, &CPatterned::TryJump, 0);
    }
    break;
  case EStateMsg::Deactivate:
    if (x639_25_useJumpBackJump) {
      x32c_animState = EAnimState::NotReady;
      x639_25_useJumpBackJump = false;
    }
    x8d8_holdPositionTime = 0.f;
    break;
  }
}

void CSpacePirate::DoubleSnap(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    if (!x635_24_noMeleeAttack)
      x32c_animState = EAnimState::Ready;
    x2e0_destPos = GetTargetPos(mgr);
    x648_targetDelta = x2e0_destPos - GetTranslation();
    x644_steeringSpeed = 0.f;
    x636_26_enableMeleeAttack = true;
    x83c_meleeSeverity = pas::ESeverity::One;
    x638_25_appliedBladeDamage = false;
    x636_31_inAttackState = true;
    x639_30_closeMelee = false;
    mChargePlayerList.remove(GetUniqueId());
    break;
  case EStateMsg::Update:
    TryCommand(mgr, pas::EAnimationState::MeleeAttack, &CPatterned::TryMeleeAttack, int(x83c_meleeSeverity));
    if (x83c_meleeSeverity == pas::ESeverity::One && x32c_animState == EAnimState::Over) {
      zeus::CVector3f delta = GetTargetPos(mgr) - GetTranslation();
      if (delta.magSquared() < x2fc_minAttackRange * x2fc_minAttackRange &&
          delta.normalized().dot(GetTransform().basis[1]) > -0.123f) {
        x32c_animState = EAnimState::Ready;
        x83c_meleeSeverity = pas::ESeverity::Two;
        x638_25_appliedBladeDamage = false;
        x648_targetDelta = delta;
        x639_30_closeMelee = true;
      }
    }
    if (x639_30_closeMelee)
      x648_targetDelta = GetTargetPos(mgr) - GetTranslation();
    x450_bodyController->GetCommandMgr().DeliverTargetVector(x648_targetDelta);
    if (x635_27_shadowPirate) {
      if (x32c_animState == EAnimState::Over) {
        x3e8_alphaDelta = -0.4f;
      } else {
        x3e8_alphaDelta = 1.f;
        x8bc_maxCloakAlpha = 0.75f;
      }
    }
    UpdateCantSeePlayer(mgr);
    UpdateHeldPosition(mgr, dt);
    CheckBlade(mgr);
    break;
  case EStateMsg::Deactivate:
    x636_26_enableMeleeAttack = false;
    x636_31_inAttackState = false;
    x32c_animState = EAnimState::NotReady;
    break;
  }
}

void CSpacePirate::Shuffle(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->GetCommandMgr().SetSteeringBlendMode(ESteeringBlendMode::Normal);
    if (!x634_28_noShuffleCloseCheck && TooClose(mgr, 0.f)) {
      SetDestPos(GetTranslation() +
                 x2fc_minAttackRange * (GetTranslation() - mgr.GetPlayer().GetTranslation()).normalized() +
                 mgr.Random2f(0.f, 5.f));
      x2dc_destObj = kInvalidUniqueId;
      x30c_behaviourOrient = EBehaviourOrient::Constant;
      x636_30_shuffleClose = true;
    } else {
      zeus::CVector3f fromPlayer = GetTranslation() - mgr.GetPlayer().GetTranslation();
      SetDestPos(mgr.GetPlayer().GetTranslation() +
                 (x300_maxAttackRange * mgr.GetActiveRandom()->Float() + x300_maxAttackRange) *
                     fromPlayer.normalized() +
                 zeus::skUp.cross(fromPlayer).normalized() *
                     (2.f * x300_maxAttackRange * (mgr.GetActiveRandom()->Float() - 0.5f)));
      x2dc_destObj = kInvalidUniqueId;
      x30c_behaviourOrient = EBehaviourOrient::MoveDir;
      x636_30_shuffleClose = false;
    }
    x644_steeringSpeed = 1.f;
  }
  CPatterned::PathFind(mgr, msg, dt);
  switch (msg) {
  case EStateMsg::Update:
    AvoidActors(mgr);
    break;
  case EStateMsg::Deactivate:
    x636_30_shuffleClose = false;
    break;
  default:
    break;
  }
}

void CSpacePirate::TurnAround(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    SetDestPos(GetTargetPos(mgr));
    if (GetTransform().basis[1].dot((x2e0_destPos - GetTranslation()).normalized()) < 0.8f)
      x32c_animState = EAnimState::Ready;
    break;
  case EStateMsg::Update:
    TryCommand(mgr, pas::EAnimationState::Turn, &CPatterned::TryTurn, 0);
    UpdateCantSeePlayer(mgr);
    break;
  case EStateMsg::Deactivate:
    x32c_animState = EAnimState::NotReady;
    break;
  }
}

void CSpacePirate::Skid(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x838_strafeDelayTimer = 4.f;
    x636_31_inAttackState = true;
    break;
  case EStateMsg::Update:
    if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Step)
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCStepCmd(x834_skidDir, pas::EStepType::Normal));
    break;
  case EStateMsg::Deactivate:
    x636_31_inAttackState = false;
    break;
  }
}

void CSpacePirate::CoverAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x450_bodyController->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::LeanFromCover));
    x636_31_inAttackState = true;
    break;
  case EStateMsg::Update:
    UpdateCantSeePlayer(mgr);
    break;
  case EStateMsg::Deactivate:
    x636_31_inAttackState = false;
    break;
  }
}

void CSpacePirate::Crouch(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Crouch);
    if (CScriptCoverPoint* cp = GetCoverPoint(mgr, x640_coverPoint))
      x648_targetDelta = cp->GetTransform().basis[1];
    x644_steeringSpeed = 0.f;
    TargetPlayer(mgr, msg, dt);
    x79c_coverDir = pas::ECoverDirection::Invalid;
    break;
  case EStateMsg::Update:
    x450_bodyController->GetCommandMgr().DeliverTargetVector(x648_targetDelta);
    UpdateCantSeePlayer(mgr);
    break;
  default:
    break;
  }
}

void CSpacePirate::UpdateLeashTimer(float dt) {
  if (x450_bodyController->IsFrozen() || x450_bodyController->IsElectrocuting())
    return;
  x8dc_leashTimer += dt;
}

void CSpacePirate::GetUp(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x32c_animState = EAnimState::Ready;
    SquadReset(mgr);
    x8dc_leashTimer = 0.f;
    break;
  case EStateMsg::Update:
    if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::LieOnGround &&
        x660_pathFindSearch.Search(GetTranslation(), GetTranslation()) == CPathFindSearch::EResult::NoSourcePoint)
      x401_30_pendingDeath = true;
    else
      TryCommand(mgr, pas::EAnimationState::Getup, &CPatterned::TryGetUp, int(pas::EGetupType::Zero));
    UpdateLeashTimer(dt);
    break;
  case EStateMsg::Deactivate:
    x32c_animState = EAnimState::Repeat;
    break;
  }
}

void CSpacePirate::Taunt(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x637_25_enableAim = true;
    x764_boneTracking.SetActive(true);
    x764_boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
    if (x7c0_targetId == kInvalidUniqueId)
      x7c0_targetId = mgr.GetPlayer().GetUniqueId();
    if (x450_bodyController->HasBodyState(pas::EAnimationState::Taunt)) {
      if (!x635_27_shadowPirate) {
        bool withOtherPirate = true;
        if (x634_27_melee) {
          const auto bestAnim = x450_bodyController->GetPASDatabase().FindBestAnimation(
              CPASAnimParmData{16, CPASAnimParm::FromEnum(2)}, *mgr.GetActiveRandom(), -1);
          if (bestAnim.first > 0.f) {
            withOtherPirate = false;
            x760_taunt = pas::ETauntType::Two;
          }
        }
        if (withOtherPirate) {
          withOtherPirate = false;
          for (CEntity* ent : mgr.GetListeningAiObjectList()) {
            if (CSpacePirate* otherSp = CPatterned::CastTo<CSpacePirate>(ent)) {
              if (otherSp != this && !otherSp->x637_25_enableAim && otherSp->x400_25_alive &&
                  otherSp->GetAreaIdAlways() == GetAreaIdAlways()) {
                if ((otherSp->GetTranslation() - GetTranslation()).magSquared() <
                    x568_pirateData.x14_HearingRadius * x568_pirateData.x14_HearingRadius)
                  withOtherPirate = true;
              }
            }
          }
          x760_taunt = withOtherPirate ? pas::ETauntType::Zero : pas::ETauntType::One;
        }
      } else {
        x760_taunt = x635_28_alertBeforeCloak ? pas::ETauntType::One : pas::ETauntType::Zero;
      }
      x32c_animState = EAnimState::Ready;
    }
    CSfxManager::AddEmitter(x568_pirateData.xa4_Sound_Alert, GetTranslation(), zeus::skZero3f, true, false, 0x7f,
                            kInvalidAreaId);
    break;
  case EStateMsg::Update:
    TryCommand(mgr, pas::EAnimationState::Taunt, &CPatterned::TryTaunt, int(x760_taunt));
    break;
  case EStateMsg::Deactivate:
    if (x760_taunt == pas::ETauntType::Zero)
      mgr.InformListeners(GetTranslation(), EListenNoiseType::PlayerFire);
    x32c_animState = EAnimState::NotReady;
    break;
  }
}

void CSpacePirate::Flee(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x450_bodyController->GetCommandMgr().SetSteeringBlendMode(ESteeringBlendMode::Normal);
    SetDestPos(GetTranslation() + (GetTranslation() - mgr.GetPlayer().GetTranslation()).normalized() * 15.f);
    x30c_behaviourOrient = EBehaviourOrient::MoveDir;
    x644_steeringSpeed = 1.f;
    break;
  case EStateMsg::Update:
    AvoidActors(mgr);
    break;
  default:
    break;
  }
}

void CSpacePirate::Lurk(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    ReleaseCoverPoint(mgr, x640_coverPoint);
    x644_steeringSpeed = 0.f;
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Combat);
    x637_28_noPlayerLos = true;
    x7ac_timeNoPlayerLos = 0.f;
    x638_26_alwaysAggressive = mgr.GetActiveRandom()->Range(0.f, 100.f) < x568_pirateData.x0_AggressionCheck;
    x638_27_coverCheck = mgr.GetActiveRandom()->Range(0.f, 100.f) < x568_pirateData.x4_CoverCheck;
    x638_28_enableDodge = mgr.GetActiveRandom()->Range(0.f, 100.f) < x568_pirateData.x94_DodgeCheck;
    x637_25_enableAim = true;
    x764_boneTracking.SetActive(true);
    x764_boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
    if (x634_29_onlyAttackInRange) {
      x7c4_burstFire.SetBurstType(4);
      x450_bodyController->SetLocomotionType(pas::ELocomotionType::Combat);
    }
    x63a_24_normalDodge = false;
    break;
  case EStateMsg::Update:
    if (x450_bodyController->HasBodyState(pas::EAnimationState::Turn)) {
      if (x32c_animState != EAnimState::NotReady)
        TryCommand(mgr, pas::EAnimationState::Turn, &CPatterned::TryTurn, 0);
      if (x32c_animState != EAnimState::Repeat) {
        x2e0_destPos = GetTargetPos(mgr);
        if ((x2e0_destPos - GetTranslation()).normalized().dot(GetTransform().basis[1]) < 0.9f)
          x32c_animState = EAnimState::Ready;
      }
    }
    if (x635_26_seated && x639_28_satUp) {
      if (x7bc_attackRemTime > x304_averageAttackTime &&
          x450_bodyController->GetLocomotionType() == pas::ELocomotionType::Combat) {
        x450_bodyController->SetLocomotionType(pas::ELocomotionType::Internal5);
      } else if (x7bc_attackRemTime < 0.5f * x304_averageAttackTime &&
                 x450_bodyController->GetLocomotionType() == pas::ELocomotionType::Internal5) {
        x450_bodyController->SetLocomotionType(pas::ELocomotionType::Combat);
      }
    }
    UpdateCantSeePlayer(mgr);
    UpdateHeldPosition(mgr, dt);
    break;
  case EStateMsg::Deactivate:
    x638_26_alwaysAggressive = false;
    x638_29_noPlayerDodge = false;
    x32c_animState = EAnimState::NotReady;
    break;
  }
}

void CSpacePirate::Jump(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x32c_animState = EAnimState::Ready;
    x828_patrolDestPos = GetTranslation() + zeus::skDown;
    x824_jumpHeight = 0.f;
    x8dc_leashTimer = 0.f;
    break;
  case EStateMsg::Update:
    TryCommand(mgr, pas::EAnimationState::Jump, &CPatterned::TryJumpInLoop, int(pas::EJumpType::Normal));
    UpdateLeashTimer(dt);
    break;
  case EStateMsg::Deactivate:
    x32c_animState = EAnimState::NotReady;
    break;
  }
}

pas::EStepDirection CSpacePirate::GetStrafeDir(CStateManager& mgr, float dist) const {
  float distSq = dist * dist;
  bool left = true;
  bool right = true;
  for (CEntity* ent : mgr.GetListeningAiObjectList()) {
    if (CSpacePirate* otherSp = CPatterned::CastTo<CSpacePirate>(ent)) {
      if (otherSp != this && otherSp->GetAreaIdAlways() == GetAreaIdAlways()) {
        zeus::CVector3f delta = otherSp->GetTranslation() - GetTranslation();
        float deltaSq = delta.magSquared();
        if (deltaSq < distSq) {
          float dot = GetTransform().basis[1].dot(delta);
          if (dot > 0.866f * deltaSq || (dot > 0.f && deltaSq < 3.f))
            right = false;
          else if (dot < -deltaSq * 0.866f || (dot < 0.f && deltaSq < 3.f))
            left = false;
        }
      }
    }
  }
  if (right)
    right = CantJumpBack(mgr, GetTransform().basis[0], dist);
  if (left)
    left = CantJumpBack(mgr, -GetTransform().basis[0], dist);
  if (left && right) {
    if (mgr.GetActiveRandom()->Next() & 0x4000)
      left = false;
    else
      right = false;
  }
  if (left)
    return pas::EStepDirection::Left;
  if (right)
    return pas::EStepDirection::Right;
  return pas::EStepDirection::Invalid;
}

void CSpacePirate::Dodge(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x639_29_enableBreakDodge = false;
    if (!x63a_24_normalDodge && !x635_29_noBreakDodge && x8c0_dodgeDelayTimer <= 0.f) {
      if (mgr.GetActiveRandom()->Float() <
          ((x750_initialHP - HealthInfo(mgr)->GetHP()) * 4.f / x750_initialHP + 1.f) * 0.15f)
        x639_29_enableBreakDodge = true;
      x8c0_dodgeDelayTimer =
          mgr.GetActiveRandom()->Range(x568_pirateData.xb8_dodgeDelayTimeMin, x568_pirateData.xbc_dodgeDelayTimeMax);
    }
    x844_dodgeDir = GetStrafeDir(mgr, x639_29_enableBreakDodge ? x84c_breakDodgeDist : x848_dodgeDist);
    if (x844_dodgeDir != pas::EStepDirection::Invalid)
      x32c_animState = EAnimState::Ready;
    break;
  case EStateMsg::Update:
    if (!x639_29_enableBreakDodge) {
      if (x63a_24_normalDodge || mgr.GetActiveRandom()->Float() < 0.5f)
        TryCommand(mgr, pas::EAnimationState::Step, &CPatterned::TryDodge, int(x844_dodgeDir));
      else
        TryCommand(mgr, pas::EAnimationState::Step, &CPatterned::TryRollingDodge, int(x844_dodgeDir));
    } else {
      TryCommand(mgr, pas::EAnimationState::Step, &CPatterned::TryBreakDodge, int(x844_dodgeDir));
      if (GetMaterialList().HasMaterial(EMaterialTypes::Orbit) && x330_stateMachineState.GetTime() > 0.5f) {
        RemoveMaterial(EMaterialTypes::Orbit, mgr);
        mgr.GetPlayer().SetOrbitRequestForTarget(GetUniqueId(), CPlayer::EPlayerOrbitRequest::ActivateOrbitSource, mgr);
      }
    }
    break;
  case EStateMsg::Deactivate:
    x32c_animState = EAnimState::NotReady;
    x638_29_noPlayerDodge = true;
    if (!GetMaterialList().HasMaterial(EMaterialTypes::Orbit))
      AddMaterial(EMaterialTypes::Orbit, mgr);
    break;
  }
}

void CSpacePirate::Cover(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    if (x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Cover) {
      if (CScriptCoverPoint* cp = GetCoverPoint(mgr, x640_coverPoint)) {
        x79c_coverDir = cp->GetAttackDirection();
        x32c_animState = EAnimState::Ready;
        x2e0_destPos = cp->GetTranslation();
        TryCommand(mgr, pas::EAnimationState::Cover, &CPatterned::TryCover, int(x79c_coverDir));
      }
    }
    break;
  case EStateMsg::Update:
    TryCommand(mgr, pas::EAnimationState::Cover, &CPatterned::TryCover, int(x79c_coverDir));
    if (CScriptCoverPoint* cp = GetCoverPoint(mgr, x640_coverPoint))
      x450_bodyController->GetCommandMgr().DeliverTargetVector(-cp->GetTransform().basis[1]);
    UpdateCantSeePlayer(mgr);
    break;
  case EStateMsg::Deactivate:
    x32c_animState = EAnimState::NotReady;
    break;
  }
}

void CSpacePirate::Approach(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x450_bodyController->GetCommandMgr().SetSteeringBlendMode(ESteeringBlendMode::Normal);
    x30c_behaviourOrient = EBehaviourOrient::MoveDir;
    x644_steeringSpeed = 1.f;
    break;
  case EStateMsg::Update:
    AvoidActors(mgr);
    break;
  default:
    break;
  }
}

void CSpacePirate::WallHang(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x32c_animState = EAnimState::Ready;
    x637_29_inWallHang = true;
    if (CScriptCoverPoint* cp = GetCoverPoint(mgr, x640_coverPoint)) {
      for (const auto& conn : cp->GetConnectionList()) {
        if (conn.x0_state == EScriptObjectState::Arrived && conn.x4_msg == EScriptObjectMessage::Next) {
          if (TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(mgr.GetIdForScript(conn.x8_objId))) {
            x2e0_destPos = wp->GetTranslation();
            x2ec_reflectedDestPos = GetTranslation();
            x328_24_inPosition = false;
            break;
          }
        }
      }
    }
    x636_31_inAttackState = true;
    break;
  case EStateMsg::Update:
    TryCommand(mgr, pas::EAnimationState::WallHang, &CSpacePirate::TryWallHang, 0);
    x450_bodyController->GetCommandMgr().DeliverTargetVector(mgr.GetPlayer().GetTranslation() - GetTranslation());
    x7c4_burstFire.SetBurstType(1);
    break;
  case EStateMsg::Deactivate:
    x637_29_inWallHang = false;
    x32c_animState = EAnimState::NotReady;
    x636_31_inAttackState = false;
    break;
  }
}

void CSpacePirate::WallDetach(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x637_29_inWallHang = true;
    break;
  case EStateMsg::Update:
    x450_bodyController->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::ExitState));
    break;
  case EStateMsg::Deactivate:
    x637_29_inWallHang = false;
    break;
  }
}

void CSpacePirate::Enraged(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate)
    x450_bodyController->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::ExitState));
}

void CSpacePirate::SpecialAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    x32c_animState = EAnimState::Ready;
    x648_targetDelta = mgr.GetPlayer().GetAimPosition(mgr, 0.f) - GetGunEyePos();
    break;
  case EStateMsg::Update:
    TryCommand(mgr, pas::EAnimationState::ProjectileAttack, &CPatterned::TryProjectileAttack, int(pas::ESeverity::One));
    if (x32c_animState == EAnimState::Ready)
      x450_bodyController->GetCommandMgr().DeliverTargetVector(x648_targetDelta);
    break;
  case EStateMsg::Deactivate:
    x32c_animState = EAnimState::NotReady;
    break;
  }
}

void CSpacePirate::Bounce(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case EStateMsg::Activate:
    if (TCastToConstPtr<CScriptAiJumpPoint> jp = mgr.GetObjectById(x840_jumpPoint)) {
      if (TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(jp->GetJumpTarget())) {
        x450_bodyController->GetCommandMgr().DeliverCmd(
            CBCJumpCmd(x828_patrolDestPos, wp->GetTranslation(), pas::EJumpType::Normal));
      }
    }
    break;
  case EStateMsg::Update:
    if (x330_stateMachineState.GetTime() > 0.1f &&
        x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Jump)
      x330_stateMachineState.SetCodeTrigger();
    break;
  default:
    break;
  }
}

void CSpacePirate::PathFindEx(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::PathFind(mgr, msg, dt);
  switch (msg) {
  case EStateMsg::Activate:
    x639_27_inRange = false;
    x30c_behaviourOrient = EBehaviourOrient::MoveDir;
    break;
  case EStateMsg::Update:
    AvoidActors(mgr);
    if (!x639_27_inRange) {
      if (TCastToConstPtr<CScriptAiJumpPoint> jp = mgr.GetObjectById(x840_jumpPoint)) {
        x754_fsmRange =
            (1.5f * dt + 0.1f) * x64_modelData->GetScale().y() * x450_bodyController->GetBodyStateInfo().GetMaxSpeed() +
            x7a4_intoJumpDist;
        x639_27_inRange = (GetTranslation() - jp->GetTranslation()).magSquared() < x754_fsmRange * x754_fsmRange;
      }
    }
    break;
  case EStateMsg::Deactivate:
    x639_27_inRange = false;
    break;
  }
}

bool CSpacePirate::Leash(CStateManager& mgr, float arg) { return x8dc_leashTimer > arg; }

bool CSpacePirate::OffLine(CStateManager& mgr, float arg) { return !IsOnGround(); }

bool CSpacePirate::Attacked(CStateManager& mgr, float arg) {
  return x850_timeSinceHitByPlayer < (arg == 0.f ? 0.5f : arg);
}

bool CSpacePirate::InRange(CStateManager& mgr, float arg) { return x639_27_inRange; }

bool CSpacePirate::SpotPlayer(CStateManager& mgr, float arg) {
  zeus::CVector3f toPlayer = mgr.GetPlayer().GetTranslation() - GetTranslation();
  return toPlayer.dot(GetTransform().basis[1]) > toPlayer.magnitude() * x3c4_detectionAngle;
}

bool CSpacePirate::PatternOver(CStateManager& mgr, float arg) { return x2dc_destObj == kInvalidUniqueId; }

bool CSpacePirate::PatternShagged(CStateManager& mgr, float arg) {
  return CPatterned::Stuck(mgr, arg) || CPatterned::PatternShagged(mgr, arg);
}

bool CSpacePirate::AnimOver(CStateManager& mgr, float arg) {
  if (x637_29_inWallHang)
    return x450_bodyController->GetCurrentStateId() != pas::EAnimationState::WallHang;
  return CPatterned::AnimOver(mgr, arg);
}

bool CSpacePirate::ShouldAttack(CStateManager& mgr, float arg) {
  bool ret = true;
  if (mgr.GetPlayer().GetUniqueId() == x7c0_targetId) {
    zeus::CVector3f targetPos = GetTargetPos(mgr);
    int numCloserPirates = 0;
    float distSq = (GetTranslation() - targetPos).magSquared();
    for (CEntity* ent : mgr.GetListeningAiObjectList()) {
      if (CSpacePirate* otherSp = CPatterned::CastTo<CSpacePirate>(ent)) {
        if (otherSp != this && otherSp->x636_31_inAttackState && otherSp->x400_25_alive &&
            otherSp->GetAreaIdAlways() == GetAreaIdAlways()) {
          if ((otherSp->GetTranslation() - targetPos).magSquared() < distSq) {
            ++numCloserPirates;
            if (numCloserPirates > 3)
              ret = false;
          }
        }
      }
    }
  }
  return ret;
}

bool CSpacePirate::ShouldJumpBack(CStateManager& mgr, float arg) {
  return !x634_28_noShuffleCloseCheck || x8d8_holdPositionTime > 6.f;
}

bool CSpacePirate::Stuck(CStateManager& mgr, float arg) {
  if (x330_stateMachineState.GetTime() > 0.5f)
    return CPatterned::Stuck(mgr, arg) || CPatterned::PatternShagged(mgr, arg);
  return false;
}

bool CSpacePirate::Landed(CStateManager& mgr, float arg) { return IsOnGround(); }

bool CSpacePirate::HearShot(CStateManager& mgr, float arg) {
  bool ret = x636_25_hearNoise;
  x636_25_hearNoise = false;
  return ret;
}

bool CSpacePirate::HearPlayer(CStateManager& mgr, float arg) {
  if (mgr.GetPlayer().GetVelocity().magSquared() > 0.1f)
    return (mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared() <
           x568_pirateData.x14_HearingRadius * x568_pirateData.x14_HearingRadius;
  return false;
}

bool CSpacePirate::CoverCheck(CStateManager& mgr, float arg) { return x638_27_coverCheck; }

bool CSpacePirate::CoverFind(CStateManager& mgr, float arg) {
  bool ret = false;
  float minCpDistSq = x568_pirateData.x8_SearchRadius * x568_pirateData.x8_SearchRadius;
  CScriptCoverPoint* closestCp = nullptr;
  for (CEntity* ent : mgr.GetAiWaypointObjectList()) {
    if (TCastToPtr<CScriptCoverPoint> cp = ent) {
      if (cp->GetActive() && !cp->ShouldLandHere() && !cp->GetInUse(GetUniqueId()) &&
          cp->GetAreaIdAlways() == GetAreaIdAlways() && cp->GetUniqueId() != x642_previousCoverPoint) {
        float fromCpDist = (GetTranslation() - cp->GetTranslation()).magSquared();
        if (fromCpDist < minCpDistSq && !cp->Blown(mgr.GetPlayer().GetTranslation())) {
          minCpDistSq = fromCpDist;
          closestCp = cp.GetPtr();
        }
      }
    }
  }
  if (closestCp) {
    ReleaseCoverPoint(mgr, x640_coverPoint);
    if (TCastToPtr<CScriptCoverPoint> cp = mgr.ObjectById(closestCp->GetUniqueId())) {
      SetCoverPoint(cp.GetPtr(), x640_coverPoint);
      x642_previousCoverPoint = x640_coverPoint;
      x654_coverPointRearDir = -closestCp->GetTransform().basis[1];
      x30c_behaviourOrient = EBehaviourOrient::MoveDir;
      ret = true;
    }
  }
  return ret;
}

bool CSpacePirate::CoverBlown(CStateManager& mgr, float arg) {
  bool ret = true;
  if ((mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared() > x2fc_minAttackRange * x2fc_minAttackRange) {
    if (CScriptCoverPoint* cp = GetCoverPoint(mgr, x640_coverPoint)) {
      if (!(ret = cp->Blown(mgr.GetPlayer().GetTranslation())) && x644_steeringSpeed == 0.f &&
          x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Step &&
          (cp->GetTranslation() - GetTranslation()).magSquared() > 3.f * x64_modelData->GetScale().y()) {
        ret = true;
      }
    }
  }
  return ret;
}

bool CSpacePirate::CoverNearlyBlown(CStateManager& mgr, float arg) {
  bool ret = true;
  if (CScriptCoverPoint* cp = GetCoverPoint(mgr, x640_coverPoint))
    ret = cp->Blown(mgr.GetPlayer().GetTranslation() + mgr.GetPlayer().GetVelocity() * 1.f);
  return ret;
}

bool CSpacePirate::CoveringFire(CStateManager& mgr, float arg) {
  bool ret = false;
  for (CEntity* ent : mgr.GetListeningAiObjectList()) {
    if (CSpacePirate* otherSp = CPatterned::CastTo<CSpacePirate>(ent)) {
      if (otherSp != this && otherSp->x636_31_inAttackState && otherSp->GetAreaIdAlways() == GetAreaIdAlways())
        ret = true;
    }
  }
  return ret;
}

bool CSpacePirate::LineOfSight(CStateManager& mgr, float arg) { return !x637_28_noPlayerLos; }

bool CSpacePirate::AggressionCheck(CStateManager& mgr, float arg) {
  bool ret = false;
  if (!x634_26_nonAggressive) {
    if (x638_26_alwaysAggressive)
      ret = true;
    else if (mChargePlayerList.empty() && x7ac_timeNoPlayerLos > 10.f)
      ret = true;
    if (ret) {
      x30c_behaviourOrient = EBehaviourOrient::MoveDir;
      if (std::find(mChargePlayerList.begin(), mChargePlayerList.end(), GetUniqueId()) == mChargePlayerList.end())
        mChargePlayerList.push_back(GetUniqueId());
    }
  }
  return ret;
}

bool CSpacePirate::ShouldDodge(CStateManager& mgr, float arg) {
  bool ret = false;
  if (x638_28_enableDodge) {
    if (!x634_26_nonAggressive && !x638_29_noPlayerDodge &&
        (GetTargetPos(mgr) - GetTranslation()).dot(GetTransform().basis[1]) > 0.f &&
        (x850_timeSinceHitByPlayer < 0.33f || x854_lowHealthFrenzyTimer < 0.33f) && x7ac_timeNoPlayerLos < 0.5f) {
      ret = true;
    }
    if (!ret) {
      if (const CMetroid* metroid = CPatterned::CastTo<CMetroid>(mgr.GetObjectById(x7c0_targetId))) {
        if (metroid->GetX9BF_29() &&
            (GetTranslation() - metroid->GetTranslation()).dot(metroid->GetTransform().basis[1]) > 0.f) {
          ret = true;
        }
      }
    }
  }
  return ret;
}

bool CSpacePirate::ShouldRetreat(CStateManager& mgr, float arg) {
  bool ret = false;
  if (x636_29_enableRetreat) {
    TUniqueId wpId = GetWaypointForState(mgr, EScriptObjectState::Patrol, EScriptObjectMessage::Follow);
    TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(wpId);
    if (!wp) {
      wpId = GetWaypointForState(mgr, EScriptObjectState::Retreat, EScriptObjectMessage::Follow);
      wp = TCastToConstPtr<CScriptWaypoint>(mgr.GetObjectById(wpId));
    }
    if (wp) {
      x2dc_destObj = wpId;
      SetDestPos(wp->GetTranslation());
    } else {
      x2dc_destObj = kInvalidUniqueId;
      SetDestPos(GetTranslation());
    }
    x636_29_enableRetreat = false;
    x2ec_reflectedDestPos = GetTranslation();
    x328_24_inPosition = false;
    ReleaseCoverPoint(mgr, x640_coverPoint);
    x636_25_hearNoise = false;
    x637_25_enableAim = false;
    x400_24_hitByPlayerProjectile = false;
    ret = true;
  }
  return ret;
}

bool CSpacePirate::ShouldCrouch(CStateManager& mgr, float arg) {
  if (CScriptCoverPoint* cp = GetCoverPoint(mgr, x640_coverPoint))
    return cp->ShouldCrouch();
  return false;
}

bool CSpacePirate::ShouldMove(CStateManager& mgr, float arg) {
  if (CScriptCoverPoint* cp = GetCoverPoint(mgr, x640_coverPoint))
    return !cp->ShouldStay();
  return false;
}

bool CSpacePirate::ShotAt(CStateManager& mgr, float arg) {
  return x854_lowHealthFrenzyTimer < (arg == 0.f ? 0.5f : arg);
}

bool CSpacePirate::HasTargetingPoint(CStateManager& mgr, float arg) {
  bool ret = true;
  TCastToPtr<CActor> act = mgr.ObjectById(x7c0_targetId);
  if (x7c0_targetId == mgr.GetPlayer().GetUniqueId() || !act || !act->GetActive()) {
    ret = false;
    x764_boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
    x7c0_targetId = mgr.GetPlayer().GetUniqueId();
    float margin = x568_pirateData.x8_SearchRadius * 1.f;
    zeus::CAABox nearAABB(GetTranslation() - margin, GetTranslation() + margin);
    rstl::reserved_vector<TUniqueId, 1024> nearList;
    mgr.BuildNearList(nearList, nearAABB, CMaterialFilter::MakeExclude({EMaterialTypes::Solid}), nullptr);
    for (TUniqueId id : nearList) {
      if (TCastToConstPtr<CScriptTargetingPoint> tp = mgr.GetObjectById(id)) {
        if (tp->GetActive() && tp->GetAreaIdAlways() == GetAreaIdAlways() && !tp->GetLocked()) {
          x764_boneTracking.SetTarget(tp->GetUniqueId());
          x7c0_targetId = tp->GetUniqueId();
          ret = true;
        }
      }
    }
  }
  return ret;
}

bool CSpacePirate::ShouldWallHang(CStateManager& mgr, float arg) {
  if (CScriptCoverPoint* cp = GetCoverPoint(mgr, x640_coverPoint))
    return cp->ShouldWallHang();
  return false;
}

bool CSpacePirate::StartAttack(CStateManager& mgr, float arg) {
  if (x638_31_mayStartAttack) {
    x638_31_mayStartAttack = false;
    return true;
  }
  return false;
}

bool CSpacePirate::BreakAttack(CStateManager& mgr, float arg) { return x635_25_breakAttack; }

bool CSpacePirate::ShouldStrafe(CStateManager& mgr, float arg) {
  bool ret = false;
  bool noPlayerStrafe = false;
  x834_skidDir = pas::EStepDirection::Invalid;
  if (!x634_26_nonAggressive && (GetTargetPos(mgr) - GetTranslation()).dot(GetTransform().basis[1]) > 0.f) {
    if ((x854_lowHealthFrenzyTimer < 0.66f || x850_timeSinceHitByPlayer < 0.66f) && x838_strafeDelayTimer == 0.f &&
        (GetTargetPos(mgr) - GetBoundingBox().center()).normalized().dot(GetTransform().basis[1]) > 0.707f) {
      x834_skidDir = GetStrafeDir(mgr, 10.f);
      if (x834_skidDir != pas::EStepDirection::Invalid)
        ret = true;
      else
        noPlayerStrafe = true;
    }
    if (!noPlayerStrafe && !ret && x7c0_targetId == mgr.GetPlayer().GetUniqueId() && x7ac_timeNoPlayerLos > 1.f &&
        (mgr.GetPlayer().GetTranslation() - GetTranslation()).magnitude() < 15.f &&
        x834_skidDir == pas::EStepDirection::Invalid) {
      x834_skidDir = GetStrafeDir(mgr, 5.f);
      if (x834_skidDir != pas::EStepDirection::Invalid)
        ret = true;
    }
  }
  return ret;
}

bool CSpacePirate::ShouldSpecialAttack(CStateManager& mgr, float arg) {
  return x634_29_onlyAttackInRange && !x7c4_burstFire.IsBurstSet() && x7bc_attackRemTime > 2.f;
}

bool CSpacePirate::LostInterest(CStateManager& mgr, float arg) {
  return x634_29_onlyAttackInRange && x7bc_attackRemTime < 1.5f;
}

bool CSpacePirate::BounceFind(CStateManager& mgr, float arg) {
  float minDistSq = FLT_MAX;
  CScriptAiJumpPoint* bestJp = nullptr;
  bool ret = false;
  for (CEntity* ent : mgr.GetAiWaypointObjectList()) {
    if (TCastToPtr<CScriptAiJumpPoint> jp = ent) {
      if (jp->GetActive() && !jp->GetInUse(GetUniqueId()) && jp->GetJumpTarget() != kInvalidUniqueId &&
          jp->GetAreaIdAlways() == GetAreaIdAlways()) {
        zeus::CVector3f toJp = jp->GetTranslation() - GetTranslation();
        float distSq = toJp.magSquared();
        if (distSq < minDistSq && jp->GetTransform().basis[1].dot(toJp) > 0.f) {
          if (TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(jp->GetJumpTarget())) {
            zeus::CVector3f wpToDest = x2e0_destPos - wp->GetTranslation();
            distSq += wpToDest.magSquared();
            if (distSq < minDistSq && wp->GetTransform().basis[1].dot(wpToDest) > 0.f &&
                GetSearchPath()->PathExists(GetTranslation(), jp->GetTranslation()) ==
                    CPathFindSearch::EResult::Success) {
              bool good = false;
              if (GetSearchPath()->PathExists(wp->GetTranslation(), x2e0_destPos) != CPathFindSearch::EResult::Success)
                distSq += 1000.f;
              else
                good = true;
              if (distSq < minDistSq) {
                minDistSq = distSq;
                bestJp = jp.GetPtr();
                if (good)
                  break;
              }
            }
          }
        }
      }
    }
  }
  if (bestJp) {
    if (TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(bestJp->GetJumpPoint())) {
      SetDestPos(bestJp->GetTranslation());
      x840_jumpPoint = bestJp->GetUniqueId();
      x824_jumpHeight = bestJp->GetJumpApex();
      x828_patrolDestPos = wp->GetTranslation();
      ret = true;
    }
  }
  return ret;
}

CPathFindSearch* CSpacePirate::GetSearchPath() { return &x660_pathFindSearch; }

u8 CSpacePirate::GetModelAlphau8(const CStateManager& mgr) const {
  if ((mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::EPlayerVisor::XRay &&
       mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::EPlayerVisor::Thermal) ||
      !x400_25_alive) {
    if (!x635_27_shadowPirate)
      return u8(x42c_color.a() * 255.f);
    else
      return u8(x8b4_shadowPirateAlpha * 255.f);
  }
  return 255;
}

float CSpacePirate::GetGravityConstant() const { return 50.f; }

CProjectileInfo* CSpacePirate::GetProjectileInfo() { return &x568_pirateData.x20_Projectile; }

} // namespace urde::MP1
