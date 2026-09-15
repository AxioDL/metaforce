#include "MetroidPrime/Enemies/CSpacePirate.hpp"

#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CTimeProvider.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetaRender/IRenderer.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyState.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CActorModelParticles.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Enemies/CMetroid.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/Enemies/CTeamAiMgr.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAiJumpPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTargetingPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "rstl/algorithm.hpp"
#include <float.h>

static const rstl::string skParts[] = {
    rstl::string_l("Collar"),  rstl::string_l("Neck_1"),  rstl::string_l("R_shoulder"),
    rstl::string_l("R_elbow"), rstl::string_l("R_wrist"), rstl::string_l("L_shoulder"),
    rstl::string_l("L_elbow"), rstl::string_l("L_wrist"), rstl::string_l("R_hip"),
    rstl::string_l("R_knee"),  rstl::string_l("R_ankle"), rstl::string_l("L_hip"),
    rstl::string_l("L_knee"),  rstl::string_l("L_ankle")};
static const float skRadii[] = {0.45f, 0.52f, 0.35f, 0.1f,  0.15f, 0.35f, 0.1f,
                                0.15f, 0.15f, 0.15f, 0.15f, 0.15f, 0.15f, 0.15f};

const uint CSpacePirate::skNumProperties = 34;

const SBurst CSpacePirate::skBurstsQuick[] = {
    {20, {3, 4, 5, -1, 0, 0, 0, 0}, 0.1f, 0.05f}, {20, {2, 3, 4, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {20, {6, 5, 4, -1, 0, 0, 0, 0}, 0.1f, 0.05f}, {20, {1, 2, 3, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {20, {7, 6, 5, -1, 0, 0, 0, 0}, 0.1f, 0.05f}, {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CSpacePirate::skBurstsStandard[] = {
    {15, {5, 3, 2, 1, -1, 0, 0, 0}, 0.1f, 0.05f}, {20, {1, 2, 3, 4, -1, 0, 0, 0}, 0.1f, 0.05f},
    {20, {7, 6, 5, 4, -1, 0, 0, 0}, 0.1f, 0.05f}, {15, {3, 4, 5, 6, -1, 0, 0, 0}, 0.1f, 0.05f},
    {15, {6, 5, 4, 3, -1, 0, 0, 0}, 0.1f, 0.05f}, {15, {2, 3, 4, 5, -1, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CSpacePirate::skBurstsFrenzied[] = {
    {40, {1, 2, 3, 4, 5, 6, -1, 0}, 0.1f, 0.05f}, {40, {7, 6, 5, 4, 3, 2, -1, 0}, 0.1f, 0.05f},
    {10, {2, 3, 4, 5, 4, 3, -1, 0}, 0.1f, 0.05f}, {10, {6, 5, 4, 3, 4, 5, -1, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CSpacePirate::skBurstsJumping[] = {
    {20, {16, 4, -1, 0, 0, 0, 0, 0}, 0.1f, 0.05f},
    {40, {5, 7, -1, 0, 0, 0, 0, 0}, 0.1f, 0.05f},
    {40, {1, 10, -1, 0, 0, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CSpacePirate::skBurstsInjured[] = {
    {15, {16, 1, 3, -1, 0, 0, 0, 0}, 0.1f, 0.05f}, {20, {3, 4, 6, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {25, {7, 5, 4, -1, 0, 0, 0, 0}, 0.1f, 0.05f},  {25, {2, 6, 4, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {15, {7, 5, 3, -1, 0, 0, 0, 0}, 0.1f, 0.05f},  {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CSpacePirate::skBurstsSeated[] = {
    {35, {7, 13, -1, 0, 0, 0, 0, 0}, 0.1f, 0.05f},
    {35, {9, 1, -1, 0, 0, 0, 0, 0}, 0.1f, 0.05f},
    {30, {16, 12, -1, 0, 0, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CSpacePirate::skBurstsQuickOOV[] = {
    {10, {16, 15, 13, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {20, {13, 12, 10, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {30, {9, 11, 12, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {30, {14, 10, 12, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {10, {9, 11, 13, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CSpacePirate::skBurstsStandardOOV[] = {
    {26, {16, 8, 11, 14, -1, 0, 0, 0}, 0.1f, 0.05f},
    {26, {16, 13, 11, 12, -1, 0, 0, 0}, 0.1f, 0.05f},
    {16, {9, 11, 13, 10, -1, 0, 0, 0}, 0.1f, 0.05f},
    {16, {14, 13, 12, 11, -1, 0, 0, 0}, 0.1f, 0.05f},
    {8, {10, 11, 12, 13, -1, 0, 0, 0}, 0.1f, 0.05f},
    {8, {6, 8, 11, 13, -1, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CSpacePirate::skBurstsFrenziedOOV[] = {
    {40, {1, 16, 14, 12, 10, 11, -1, 0}, 0.1f, 0.05f},
    {40, {9, 11, 12, 13, 11, 7, -1, 0}, 0.1f, 0.05f},
    {10, {8, 10, 11, 12, 13, 12, -1, 0}, 0.1f, 0.05f},
    {10, {15, 13, 12, 10, 12, 9, -1, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CSpacePirate::skBurstsJumpingOOV[] = {
    {40, {7, 13, -1, 0, 0, 0, 0, 0}, 0.1f, 0.05f},
    {40, {9, 1, -1, 0, 0, 0, 0, 0}, 0.1f, 0.05f},
    {20, {16, 12, -1, 0, 0, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CSpacePirate::skBurstsInjuredOOV[] = {
    {30, {9, 11, 13, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {10, {13, 12, 10, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {15, {9, 11, 12, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {15, {14, 10, 12, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {30, {16, 15, 13, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CSpacePirate::skBurstsSeatedOOV[] = {
    {35, {7, 13, -1, 0, 0, 0, 0, 0}, 0.1f, 0.05f},
    {35, {9, 1, -1, 0, 0, 0, 0, 0}, 0.1f, 0.05f},
    {30, {16, 12, -1, 0, 0, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst* CSpacePirate::skBursts[] = {skBurstsQuick,
                                          skBurstsStandard,
                                          skBurstsFrenzied,
                                          skBurstsJumping,
                                          skBurstsInjured,
                                          skBurstsSeated,
                                          skBurstsQuickOOV,
                                          skBurstsStandardOOV,
                                          skBurstsFrenziedOOV,
                                          skBurstsJumpingOOV,
                                          skBurstsInjuredOOV,
                                          skBurstsSeatedOOV,
                                          nullptr};

rstl::list< TUniqueId > CSpacePirate::mChargePlayerList;

rstl::string skOneEye = rstl::string_l("OneEye");
rstl::string skTwoEyes = rstl::string_l("TwoEyes");

CSpacePirate::CSpacePirateData::CSpacePirateData(CInputStream& in, int propCount)
: x0_AggressionCheck(in.ReadFloat())
, x4_CoverCheck(in.ReadFloat())
, x8_SearchRadius(in.ReadFloat())
, xc_FallBackCheck(in.ReadFloat())
, x10_FallBackRadius(in.ReadFloat())
, x14_HearingRadius(in.ReadFloat())
, x18_flags(in.ReadLong())
, x1c_(in.ReadBool())
, x20_Projectile(in)
, x48_Sound_Projectile(CSfxManager::TranslateSFXID(in.ReadLong()))
, x4c_BladeDamage(in)
, x68_KneelAttackChance(in.ReadFloat())
, x6c_KneelAttackShot(in)
, x94_DodgeCheck(in.ReadFloat())
, x98_Sound_Impact(CSfxManager::TranslateSFXID(in.ReadLong()))
, x9c_averageNextShotTime(in.ReadFloat())
, xa0_nextShotTimeVariation(in.ReadFloat())
, xa4_Sound_Alert(CSfxManager::TranslateSFXID(in.ReadLong()))
, xa8_GunTrackDelay(in.ReadFloat())
, xac_firstBurstCount(in.ReadLong())
, xb0_CloakOpacity(in.ReadFloat())
, xb4_MaxCloakOpacity(in.ReadFloat())
, xb8_dodgeDelayTimeMin(in.ReadFloat())
, xbc_dodgeDelayTimeMax(in.ReadFloat())
, xc0_Sound_Hurled(CSfxManager::TranslateSFXID(in.ReadLong()))
, xc2_Sound_Death(CSfxManager::TranslateSFXID(in.ReadLong()))
, xc4_(propCount >= 35 ? in.ReadFloat() : 0.2f)
, xc8_AvoidDistance(propCount >= 36 ? in.ReadFloat() : 8.f) {
  x20_Projectile.Token().Lock();
}

CPirateRagDoll::CPirateRagDoll(CStateManager& mgr, CSpacePirate* pirate, ushort thudSfx, uint flags)
: CRagDoll(-pirate->GetGravityConstant(), -pirate->GetFloatingGravityConstant(), 8.f, flags)
, x6c_spacePirate(pirate)
, x70_thudSfx(thudSfx)
, x74_sfxTimer(0.f)
, x78_lastSFXPos(CVector3f::Zero())
, x84_torsoImpulse(CVector3f::Zero())
, xb0_24_initSfx(true) {
  x6c_spacePirate->RemoveMaterial(kMT_Solid, kMT_AIBlock, kMT_GroundCollider, mgr);
  x6c_spacePirate->HealthInfo(mgr)->SetHP(-1.f);
  SetNumParticles(14);
  SetNumLengthConstraints(47);
  SetNumJointConstraints(4);
  const CVector3f& scale = CVector3f(pirate->GetModelData()->GetScale());
  const CTransform4f& xf = pirate->GetTransform();
  CAnimData* animData = pirate->AnimationData();
  animData->BuildPose();
  CVector3f center = pirate->GetBoundingBox().GetCenterPoint();
  for (int i = 0; i < 14; ++i) {
    CSegId id = animData->GetLocatorSegId(skParts[i]);
    CVector3f pos = xf * CVector3f::ByElementMultiply(scale, animData->GetPose().GetOffset(id));
    AddParticle(id, center, pos, skRadii[i] * scale.GetZ());
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
  AddMinLengthConstraint(
      3, 5, x14_lengthConstraints[5].GetLength() * 0.5f + x14_lengthConstraints[9].GetLength());
  AddMinLengthConstraint(
      6, 2, x14_lengthConstraints[7].GetLength() * 0.5f + x14_lengthConstraints[9].GetLength());
  AddMinLengthConstraint(
      4, 5, x14_lengthConstraints[5].GetLength() * 0.5f + x14_lengthConstraints[9].GetLength());
  AddMinLengthConstraint(
      7, 2, x14_lengthConstraints[7].GetLength() * 0.5f + x14_lengthConstraints[9].GetLength());
  AddMinLengthConstraint(4, 7, x14_lengthConstraints[9].GetLength());
  AddMinLengthConstraint(4, 8, x14_lengthConstraints[14].GetLength());
  AddMinLengthConstraint(7, 11, x14_lengthConstraints[14].GetLength());
  AddMinLengthConstraint(10, 8, x14_lengthConstraints[15].GetLength());
  AddMinLengthConstraint(13, 11, x14_lengthConstraints[17].GetLength());
  AddMinLengthConstraint(
      9, 2, x14_lengthConstraints[15].GetLength() * 0.707f + x14_lengthConstraints[10].GetLength());
  AddMinLengthConstraint(12, 5,
                         x14_lengthConstraints[17].GetLength() * 0.707f +
                             x14_lengthConstraints[13].GetLength());
  AddMinLengthConstraint(9, 11, x14_lengthConstraints[15].GetLength());
  AddMinLengthConstraint(12, 8, x14_lengthConstraints[17].GetLength());
  AddMinLengthConstraint(
      10, 0, x14_lengthConstraints[2].GetLength() + x14_lengthConstraints[15].GetLength());
  AddMinLengthConstraint(
      13, 0, x14_lengthConstraints[3].GetLength() + x14_lengthConstraints[17].GetLength());
  AddMinLengthConstraint(10, 13, x14_lengthConstraints[14].GetLength());
  AddMinLengthConstraint(9, 12, x14_lengthConstraints[14].GetLength());
  AddMinLengthConstraint(10, 12, x14_lengthConstraints[14].GetLength());
  AddMinLengthConstraint(13, 9, x14_lengthConstraints[14].GetLength());
  AddMaxLengthConstraint(10, 13, x14_lengthConstraints[14].GetLength() * 5.f);
  AddJointConstraint(8, 2, 5, 8, 9, 10);
  AddJointConstraint(11, 2, 5, 11, 12, 13);
  AddJointConstraint(2, 11, 5, 2, 3, 4);
  AddJointConstraint(5, 2, 8, 5, 6, 7);
  for (AUTO(conn, x6c_spacePirate->GetConnectionList().begin());
       conn != x6c_spacePirate->GetConnectionList().end(); ++conn) {
    if (conn->x0_state == kSS_Modify && conn->x4_msg == kSM_Follow) {
      TUniqueId id = mgr.GetIdForScript(conn->x8_objId);
      if (const CScriptWaypoint* wp = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id))) {
        x90_waypoints.push_back(id);
        x9c_wpParticleIdxs.push_back(wp->GetAnimation());
        if (x90_waypoints.capacity() - x90_waypoints.size() <= 0) {
          break;
        }
      }
    }
  }
}

void CPirateRagDoll::Prime(CStateManager& mgr, const CTransform4f& xf, CModelData& mData) {
  const CAABox& bounds = x6c_spacePirate->GetBaseBoundingBox();
  CVector3f max = bounds.GetMaxPoint();
  max.SetZ((bounds.GetMaxPoint().GetZ() - bounds.GetMinPoint().GetZ()) * 0.5f +
           bounds.GetMinPoint().GetZ());
  x6c_spacePirate->SetBoundingBox(CAABox(bounds.GetMinPoint(), max));
  CRagDoll::Prime(mgr, xf, mData);
}

void CPirateRagDoll::Update(CStateManager& mgr, float dt, float waterTop) {
  if (!IsOver() || WillContinueSmallMovements()) {
    if (x6c_spacePirate->x7b4_attachedActor != kInvalidUniqueId) {
      float delta = x4_particles[2].GetPosition().GetZ() - x4_particles[5].GetPosition().GetZ();
      if (delta * delta > 0.0625f) {
        CVector3f adjustment(0.f, 0.f, (delta > 0.f ? delta - 0.25f : delta + 0.25f) * 0.1f);
        x4_particles[2].Position() = x4_particles[2].GetPosition() - adjustment;
        x4_particles[5].Position() = x4_particles[5].GetPosition() + adjustment;
      }
      delta = x4_particles[0].GetPosition().GetZ() -
              (x4_particles[8].GetPosition().GetZ() + x4_particles[11].GetPosition().GetZ()) * 0.5f;
      if (delta * delta > 0.0625f) {
        CVector3f adjustment(0.f, 0.f, (delta > 0.f ? delta - 0.25f : delta + 0.25f) * 0.1f);
        x4_particles[0].Position() = x4_particles[0].GetPosition() - adjustment;
        adjustment[kDZ] = 0.5f * adjustment[kDZ];
        x4_particles[8].Position() = x4_particles[8].GetPosition() + adjustment;
        x4_particles[11].Position() = x4_particles[11].GetPosition() + adjustment;
      }
    }
    CVector3f oldCenter = x4_particles[8].GetPosition() * 0.25f +
                          x4_particles[11].GetPosition() * 0.25f +
                          x4_particles[0].GetPosition() * 0.5f;
    oldCenter[kDZ] =
        CMath::Min(x4_particles[8].GetPosition().GetZ() - x4_particles[8].GetRadius(),
                   x4_particles[11].GetPosition().GetZ() - x4_particles[11].GetRadius());
    oldCenter[kDZ] = CMath::Min(x4_particles[0].GetPosition().GetZ() - x4_particles[0].GetRadius(),
                                oldCenter[kDZ]);
    if (oldCenter.GetZ() < 0.5f + waterTop) {
      x84_torsoImpulse *= 1000.f;
    }
    CVector3f acceleration = x84_torsoImpulse * 0.333f * (1.f / x6c_spacePirate->GetMass());
    x4_particles[11].Velocity() += acceleration;
    x4_particles[8].Velocity() += acceleration;
    x4_particles[0].Velocity() += acceleration;
    x84_torsoImpulse = CVector3f::Zero();
    CRagDoll::Update(mgr, dt, waterTop);
    for (int i = 0; i < x90_waypoints.size(); ++i) {
      if (const CScriptWaypoint* wp =
              static_cast< const CScriptWaypoint* >(mgr.GetObjectById(x90_waypoints[i]))) {
        if (wp->GetActive()) {
          x4_particles[x9c_wpParticleIdxs[i]].Position() = wp->GetTranslation();
        }
      }
    }
    CVector3f newCenter = x4_particles[8].GetPosition() * 0.25f +
                          x4_particles[11].GetPosition() * 0.25f +
                          x4_particles[0].GetPosition() * 0.5f;
    newCenter[kDZ] =
        CMath::Min(x4_particles[8].GetPosition().GetZ() - x4_particles[8].GetRadius(),
                   x4_particles[11].GetPosition().GetZ() - x4_particles[11].GetRadius());
    newCenter[kDZ] = CMath::Min(x4_particles[0].GetPosition().GetZ() - x4_particles[0].GetRadius(),
                                newCenter[kDZ]);
    CVector3f velocity = (newCenter - oldCenter) / dt;
    x6c_spacePirate->SetTransform(CTransform4f::Identity());
    x6c_spacePirate->SetTranslation(newCenter);
    x6c_spacePirate->SetVelocityWR(velocity);
    x74_sfxTimer -= dt;
    float impactVel = x54_impactVel;
    if (impactVel > 2.5f && x74_sfxTimer < 0.f) {
      CVector3f delta = x6c_spacePirate->GetTranslation() - x78_lastSFXPos;
      if (xb0_24_initSfx || delta.MagSquared() > 0.1f) {
        float volume = CMath::Min(25.f * impactVel, 127.f);
        CSfxManager::AddEmitter(x70_thudSfx, x6c_spacePirate->GetTranslation(), CVector3f::Zero(),
                                CCast::ToUint8(volume), true, false);
        x74_sfxTimer = mgr.Random()->Float() * 0.222f + 0.222f;
        xb0_24_initSfx = false;
        x78_lastSFXPos = x6c_spacePirate->GetTranslation();
      }
    }
  } else {
    x6c_spacePirate->SetMomentumWR(CVector3f::Zero());
    x6c_spacePirate->Stop();
  }
}

void CPirateRagDoll::PreRender(const CVector3f& pos, CModelData& mData) {
  if (!IsOver() || WillContinueSmallMovements()) {
    CAnimData* animData = mData.AnimationData();
    const CSegIdList& parts = animData->GetCharLayoutInfo()->GetBodyPartSegIds();
    CHierarchyPoseBuilder& builder = animData->PoseBuilder();
    for (AUTO(it, parts.begin()); it != parts.end(); ++it) {
      CSegId id = *it;
      if (animData->GetCharLayoutInfo()->GetSegmentData(id).GetNumConnectedParts() > 1) {
        builder.Insert(id, CQuaternion::NoRotation());
      }
    }
    CSegId rootId = animData->GetLocatorSegId(rstl::string_l("Skeleton_Root"));
    CVector3f rootOffset =
        0.5f * (x4_particles[8].GetPosition() + x4_particles[11].GetPosition()) - pos;
    const CVector3f& scale = CVector3f(mData.GetScale());
    builder.Insert(rootId,
                   CVector3f(rootOffset.GetX() / scale.GetX(), rootOffset.GetY() / scale.GetY(),
                             rootOffset.GetZ() / scale.GetZ()));
    CVector3f right = x4_particles[2].GetPosition() - x4_particles[5].GetPosition();
    CVector3f up = (x4_particles[0].GetPosition() -
                    (x4_particles[8].GetPosition() + x4_particles[11].GetPosition()) * 0.5f)
                       .AsNormalized();
    CVector3f forward = CVector3f::Cross(up, right).AsNormalized();
    right = CVector3f::Cross(forward, up);
    CMatrix3f matrix(right, forward, up);
    CQuaternion rootRot = CQuaternion::FromMatrix(matrix.GetTranspose());
    builder.Insert(rootId, rootRot);
    const CCharLayoutInfo& layout = *animData->GetCharLayoutInfo();
    if (x6c_spacePirate->x7b4_attachedActor == kInvalidUniqueId) {
      CVector3f rest = layout.GetFromParentUnrotated(x4_particles[1].GetBone());
      CVector3f neck = x4_particles[1].GetPosition() - x4_particles[0].GetPosition();
      neck = rootRot.BuildInverted().Transform(neck);
      CQuaternion neckRot = CQuaternion::ShortestRotationArc(rest, neck);
      builder.Insert(x4_particles[1].GetBone(), neckRot);
    }
    CQuaternion jointRot = BoneAlign(builder, layout, 2, 3, rootRot);
    BoneAlign(builder, layout, 3, 4, rootRot * jointRot);
    jointRot = BoneAlign(builder, layout, 5, 6, rootRot);
    BoneAlign(builder, layout, 6, 7, rootRot * jointRot);
    jointRot = BoneAlign(builder, layout, 8, 9, rootRot);
    BoneAlign(builder, layout, 9, 10, rootRot * jointRot);
    jointRot = BoneAlign(builder, layout, 11, 12, rootRot);
    BoneAlign(builder, layout, 12, 13, rootRot * jointRot);
    CQuaternion ankleRot = CQuaternion::XRotation(CRelAngle::FromDegrees(-70.f));
    builder.Insert(x4_particles[10].GetBone(), ankleRot);
    builder.Insert(x4_particles[13].GetBone(), ankleRot);
    animData->SetPoseBuilderValid(false);
  }
}

CSpacePirate::CSpacePirate(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CTransform4f& xf, const CModelData& mData,
                           const CActorParameters& aParams, const CPatternedInfo& pInfo,
                           CInputStream& in, int propCount)
: CPatterned(kC_SpacePirate, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Ground, kCT_One,
             kBT_BiPedal, aParams, kCS_Medium)
, x568_pirateData(in, propCount)
, x634_24_pendingAmbush(x568_pirateData.x18_flags & 0x1)
, x634_25_ceilingAmbush(x568_pirateData.x18_flags & 0x2)
, x634_26_nonAggressive(x568_pirateData.x18_flags & 0x4)
, x634_27_melee(x568_pirateData.x18_flags & 0x8)
, x634_28_noShuffleCloseCheck(x568_pirateData.x18_flags & 0x10)
, x634_29_onlyAttackInRange(x568_pirateData.x18_flags & 0x20)
, x634_30_(x568_pirateData.x18_flags & 0x40)
, x634_31_noKnockbackImpulseReset(x568_pirateData.x18_flags & 0x80)
, x635_24_noMeleeAttack(x568_pirateData.x18_flags & 0x200)
, x635_25_breakAttack(x568_pirateData.x18_flags & 0x400)
, x635_26_seated(x568_pirateData.x18_flags & 0x1000)
, x635_27_shadowPirate(x568_pirateData.x18_flags & 0x2000)
, x635_28_alertBeforeCloak(x568_pirateData.x18_flags & 0x4000)
, x635_29_noBreakDodge(x568_pirateData.x18_flags & 0x8000)
, x635_30_floatingCorpse(x568_pirateData.x18_flags & 0x10000)
, x635_31_ragdollNoAiCollision(x568_pirateData.x18_flags & 0x20000)
, x636_24_trooper(x568_pirateData.x18_flags & 0x40000)
, x636_25_hearNoise(false)
, x636_26_enableMeleeAttack(false)
, x636_27_(false)
, x636_28_(false)
, x636_29_enableRetreat(false)
, x636_30_shuffleClose(false)
, x637_24_enablePatrol(false)
, x637_25_enableAim(false)
, x637_26_hearPlayerFire(false)
, x637_27_inProjectilePath(false)
, x637_28_noPlayerLos(false)
, x637_29_inWallHang(false)
, x637_30_jumpVelSet(false)
, x637_31_prevInCineCam(false)
, x638_24_pendingFrenzyChance(false)
, x638_25_appliedBladeDamage(false)
, x638_26_alwaysAggressive(false)
, x638_27_coverCheck(false)
, x638_28_enableDodge(false)
, x638_29_noPlayerDodge(false)
, x638_30_allEnergyDrained(false)
, x638_31_mayStartAttack(false)
, x639_24_(false)
, x639_25_useJumpBackJump(false)
, x639_26_started(false)
, x639_27_inRange(false)
, x639_28_satUp(false)
, x639_30_closeMelee(false)
, x639_31_sentAttackMsg(false)
, x63a_24_normalDodge(false)
, x63c_frenzyFrames(0)
, x640_coverPoint(kInvalidUniqueId)
, x642_previousCoverPoint(kInvalidUniqueId)
, x644_steeringSpeed(1.f)
, x648_targetDelta(CVector3f::Forward())
, x654_coverPointRearDir(CVector3f::Zero())
, x660_pathFindSearch(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x748_steeringDelayTimer(0.f)
, x74c_(0)
, x750_initialHP(pInfo.GetHealthInfo().GetHP())
, x754_coverRange(0.f)
, x758_headSeg(CSegId::Invalid())
, x75c_(0)
, x760_taunt(pas::kTT_Invalid)
, x764_boneTracking(*GetAnimationData(), rstl::string_l("Head_1"), CMath::Deg2Rad(70.f),
                    CMath::Deg2Rad(180.f), kBTF_None)
, x79c_coverDir(pas::kCD_Invalid)
, x7a4_intoJumpDist(1.f)
, x7a8_eyeHeight(2.f)
, x7ac_timeNoPlayerLos(0.f)
, x7b0_cantSeePlayerCycleCounter(0)
, x7b4_attachedActor(kInvalidUniqueId)
, x7b6_gunSeg(CSegId::Invalid())
, x7b7_elbowSeg(CSegId::Invalid())
, x7b8_wristSeg(CSegId::Invalid())
, x7b9_swooshSeg(CSegId::Invalid())
, x7bc_attackRemTime(1.f)
, x7c0_targetId(kInvalidUniqueId)
, x7c4_burstFire(skBursts, x568_pirateData.xac_firstBurstCount)
, x824_jumpHeight(3.f)
, x828_patrolDestPos(CVector3f::Zero())
, x834_skidDir(pas::kSD_Invalid)
, x838_strafeDelayTimer(0.f)
, x83c_meleeSeverity(pas::kS_Invalid)
, x840_jumpPoint(kInvalidUniqueId)
, x844_dodgeDir(pas::kSD_Invalid)
, x848_dodgeDist(3.f)
, x84c_breakDodgeDist(3.f)
, x850_timeSinceHitByPlayer(FLT_MAX)
, x854_lowHealthFrenzyTimer(FLT_MAX)
, x858_ragdollDelayTimer(0.f)
, x85c_ragDoll(nullptr)
, x860_ikChain()
, x8a8_cloakDelayTimer(0.f)
, x8ac_electricParticleTimer(0.f)
, x8b0_cloakStepTime(0.f)
, x8b4_shadowPirateAlpha(0.5f)
, x8b8_minCloakAlpha(x568_pirateData.xb0_CloakOpacity)
, x8bc_maxCloakAlpha(x568_pirateData.xb4_MaxCloakOpacity)
, x8c0_dodgeDelayTimer(x568_pirateData.xb8_dodgeDelayTimeMin)
, x8c4_aimDelayTimer(x568_pirateData.xa8_GunTrackDelay)
, x8c8_teamAiMgrId(kInvalidUniqueId)
, x8cc_trooperColor(CColor::White())
, x8d0_heldPosition(CVector2f::Zero())
, x8d8_holdPositionTime(0.f)
, x8dc_leashTimer(0.f) {
  CAnimData* animData = AnimationData();
  x758_headSeg = animData->GetLocatorSegId(rstl::string_l("Head_1"));
  x7b6_gunSeg = animData->GetLocatorSegId(rstl::string_l("R_gun_LCTR"));
  x7b7_elbowSeg = animData->GetLocatorSegId(rstl::string_l("R_elbow"));
  x7b8_wristSeg = animData->GetLocatorSegId(rstl::string_l("R_wrist"));
  x7b9_swooshSeg = animData->GetLocatorSegId(rstl::string_l("Swoosh_LCTR"));

  if (!x634_29_onlyAttackInRange) {
    const CPASAnimParmData jump(pas::kAS_Jump, CPASAnimParm::FromEnum(0),
                                CPASAnimParm::FromEnum(0));
    const CVector3f& scale = CVector3f(GetModelData()->GetScale());
    x7a4_intoJumpDist = scale[kDY] * GetAnimationDistance(jump);
    const CPASAnimParmData dodge(pas::kAS_Step, CPASAnimParm::FromEnum(3),
                                 CPASAnimParm::FromEnum(1));
    x848_dodgeDist = GetModelData()->GetScale()[kDX] * GetAnimationDistance(dodge);
    const CPASAnimParmData breakDodge(pas::kAS_Step, CPASAnimParm::FromEnum(3),
                                      CPASAnimParm::FromEnum(2));
    x84c_breakDodgeDist = GetModelData()->GetScale()[kDX] * GetAnimationDistance(breakDodge);
  } else {
    BodyCtrl()->BodyStateInfo().SetLocoAnimChangeAtEndOfAnimOnly(true);
  }

  const CAABox& baseAABB = GetBaseBoundingBox();
  x7a8_eyeHeight = (baseAABB.GetMaxPoint().GetZ() - baseAABB.GetMinPoint().GetZ()) * 0.6f;

  if (ActorLights()) {
    ActorLights()->SetAmbienceGenerated(false);
  }

  KnockBackCtrl().sub80233d40(3, 3.f, FLT_MAX);
  KnockBackCtrl().SetLocomotionDuringElectrocution(true);

  if (x634_29_onlyAttackInRange) {
    KnockBackCtrl().SetCreatureSize(kCS_Small);
  } else if (x636_24_trooper &&
             static_cast< const CSpacePirate* >(this)->GetDamageVulnerability()->WeaponHurts(
                 CWeaponMode(kWT_Plasma), CDamageVulnerability::kRD_No)) {
    KnockBackCtrl().SetCreatureSize(kCS_Large);
  }

  if (!BodyCtrl()->HasBodyState(pas::kAS_AdditiveAim)) {
    x634_27_melee = true;
  }

  if (x636_24_trooper) {
    const CDamageVulnerability& vulnerability =
        *static_cast< const CSpacePirate* >(this)->GetDamageVulnerability();
    if (vulnerability.WeaponHurts(CWeaponMode(kWT_Plasma), CDamageVulnerability::kRD_No)) {
      x8cc_trooperColor = CColor(uchar(254), uchar(0), uchar(40), uchar(255));
    } else if (vulnerability.WeaponHurts(CWeaponMode(kWT_Ice), CDamageVulnerability::kRD_No)) {
      x8cc_trooperColor = CColor::White();
    } else if (vulnerability.WeaponHurts(CWeaponMode(kWT_Power), CDamageVulnerability::kRD_No)) {
      x8cc_trooperColor = CColor(uchar(253), uchar(239), uchar(86), uchar(255));
    } else if (vulnerability.WeaponHurts(CWeaponMode(kWT_Wave), CDamageVulnerability::kRD_No)) {
      x8cc_trooperColor = CColor(uchar(198), uchar(14), uchar(255), uchar(255));
    }
  }
}

ENTITY_ACCEPT_IMPL(CSpacePirate)

void CSpacePirate::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  if (x637_29_inWallHang || x634_25_ceilingAmbush) {
    switch (msg) {
    case kSM_Falling:
      if ((x637_29_inWallHang && BodyCtrl()->GetCurrentStateId() == pas::kAS_WallHang &&
           !BodyCtrl()->GetBodyStateInfo().GetCurrentState()->ApplyGravity()) ||
          (x634_25_ceilingAmbush && (BodyCtrl()->GetCurrentStateId() == pas::kAS_Locomotion ||
                                     (BodyCtrl()->GetCurrentStateId() == pas::kAS_Jump &&
                                      !BodyCtrl()->GetBodyStateInfo().IsInAir())))) {
        CPhysicsActor::Stop();
        SetMomentumWR(CVector3f::Zero());
        return;
      }
      break;
    case kSM_OnFloor:
      x850_timeSinceHitByPlayer = FLT_MAX;
      break;
    default:
      break;
    }
  }
  switch (msg) {
  case kSM_Alert:
  case kSM_Activate:
    if (GetActive()) {
      if (x634_29_onlyAttackInRange) {
        x638_31_mayStartAttack = true;
      } else {
        x400_24_hitByPlayerProjectile = true;
      }
      SquadAdd(mgr);
    } else if (x634_25_ceilingAmbush) {
      RemoveMaterial(kMT_GroundCollider, mgr);
      x328_27_onGround = false;
    }
    break;
  default:
    break;
  }
  CPatterned::AcceptScriptMsg(msg, sender, mgr);
  switch (msg) {
  case kSM_InitializedInArea: {
    const rstl::vector< SConnection >& connections = GetConnectionList();
    for (AUTO(it, connections.begin()); it != connections.end(); ++it) {
      if (it->x0_state == kSS_Retreat && it->x4_msg == kSM_Next) {
        TUniqueId id = mgr.GetIdForScript(it->x8_objId);
        if (CScriptCoverPoint* cp = TCastToPtr< CScriptCoverPoint >(mgr.ObjectById(id))) {
          cp->Reserve(GetUniqueId());
        }
      } else if (it->x0_state == kSS_Patrol && it->x4_msg == kSM_Follow) {
        x637_24_enablePatrol = true;
      }
    }
    x660_pathFindSearch.SetArea(
        mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetPostConstructed()->x10bc_pathArea);
    if (x635_30_floatingCorpse) {
      x858_ragdollDelayTimer = 0.01f;
      RemoveMaterial(kMT_Character, kMT_Solid, kMT_Target, kMT_Orbit, mgr);
      x400_25_alive = false;
      HealthInfo(mgr)->SetHP(-1.f);
    } else {
      SetEyeParticleActive(mgr, true);
    }
    break;
  }
  case kSM_Decrement:
    if (x85c_ragDoll.get()) {
      x85c_ragDoll->SetNoOverTimer(false);
      x85c_ragDoll->SetContinueSmallMovements(false);
    }
    break;
  case kSM_Registered: {
    if (x634_25_ceilingAmbush) {
      x634_24_pendingAmbush = true;
      if (x635_27_shadowPirate) {
        x42c_color.SetAlpha(x568_pirateData.xb0_CloakOpacity);
        x3e8_alphaDelta = -1.f;
      }
    }
    x75c_ = mgr.Random()->Next() % 6;
    CMaterialList include = GetMaterialFilter().GetIncludeList();
    CMaterialList exclude = GetMaterialFilter().GetExcludeList();
    CMaterialList passthrough(kMT_AIPassthrough);
    include.Remove(passthrough);
    exclude.Add(passthrough);
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
    break;
  }
  case kSM_SetToZero:
    if (GetActive()) {
      x636_29_enableRetreat = true;
    }
    break;
  case kSM_Falling:
    if (!BodyCtrl()->IsFrozen()) {
      float momentum = GetGravityConstant() * GetMass();
      if (x634_25_ceilingAmbush) {
        momentum *= 3.f;
      }
      SetMomentumWR(CVector3f(0.f, 0.f, -momentum));
    }
    if (BodyCtrl()->GetCurrentStateId() == pas::kAS_Step) {
      SetVelocityWR(CVector3f(0.f, 0.f, GetVelocityWR().GetZ()));
    }
    x7c4_burstFire.SetBurstType(3);
    break;
  case kSM_Jumped:
    CPatterned::AcceptScriptMsg(kSM_Falling, sender, mgr);
    SetMomentumWR(CVector3f(0.f, 0.f, -GetGravityConstant() * GetMass()));
    SetVelocityForJump();
    break;
  case kSM_OnFloor:
    if (!x634_29_onlyAttackInRange) {
      x7c4_burstFire.SetBurstType(1);
    } else {
      x7c4_burstFire.SetBurstType(4);
    }
    x637_30_jumpVelSet = false;
    if (x635_27_shadowPirate && GetVelocityWR().GetZ() < -1.f) {
      x3e8_alphaDelta = 1.f;
      x8a8_cloakDelayTimer += -0.05f * GetVelocityWR().GetZ();
      x8a8_cloakDelayTimer = CMath::Clamp(0.f, x8a8_cloakDelayTimer, 1.f);
      x8bc_maxCloakAlpha = 0.5f;
      if (x400_25_alive) {
        mgr.ActorModelParticles()->StartElectric(*this);
        x8ac_electricParticleTimer = 1.f + x8a8_cloakDelayTimer;
      }
    }
    break;
  case kSM_Action:
    if (CScriptTargetingPoint* point =
            TCastToPtr< CScriptTargetingPoint >(mgr.ObjectById(sender))) {
      if (point->GetActive()) {
        x764_boneTracking.SetTarget(sender);
        x7c0_targetId = sender;
        x400_24_hitByPlayerProjectile = true;
      } else {
        x764_boneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
        x7c0_targetId = mgr.GetPlayer()->GetUniqueId();
      }
      x7bc_attackRemTime = 0.f;
    }
    break;
  case kSM_Deactivate:
  case kSM_Deleted:
    SquadRemove(mgr);
    mChargePlayerList.remove(GetUniqueId());
    break;
  case kSM_Start:
    x639_26_started = false;
    break;
  case kSM_Stop:
    x639_26_started = true;
    break;
  default:
    break;
  }
}

void CSpacePirate::Touch(CActor& actor, CStateManager& mgr) {
  CPatterned::Touch(actor, mgr);
  if (x85c_ragDoll.get() && x85c_ragDoll->IsPrimed()) {
    if (const CScriptTrigger* trigger = TCastToConstPtr< CScriptTrigger >(actor)) {
      if (trigger->GetActive() && (trigger->GetTriggerFlags() & kTFL_DetectAI) &&
          trigger->GetForceMagnitude() > 0.f) {
        CVector3f& impulse = x85c_ragDoll->TorsoImpulse();
        impulse += trigger->GetForceField();
      }
    }
  }
}

bool CSpacePirate::Listen(const CVector3f& pos, EListenNoiseType type) {
  bool heard = false;
  if (IsAlive()) {
    CVector3f delta = pos - GetTranslation();
    if (delta.MagSquared() <
            x568_pirateData.x14_HearingRadius * x568_pirateData.x14_HearingRadius &&
        (x3c0_detectionHeightRange == 0.f ||
         delta.GetZ() * delta.GetZ() < x3c0_detectionHeightRange * x3c0_detectionHeightRange)) {
      heard = true;
      x636_25_hearNoise = true;
    }
    if (type == kLNT_PlayerFire) {
      x637_26_hearPlayerFire = true;
    }
  }
  const bool result = heard;
  return result;
}

void CSpacePirate::SetEyeParticleActive(CStateManager& mgr, const bool active) {
  if (!x636_24_trooper) {
    CAnimData* animData = AnimationData();
    if (!x634_29_onlyAttackInRange || x635_26_seated) {
      if (!x635_27_shadowPirate) {
        animData->SetParticleEffectState(skTwoEyes, active, mgr);
      }
    } else {
      animData->SetParticleEffectState(skOneEye, active, mgr);
    }
  }
}

bool CSpacePirate::CheckTargetable(CStateManager& mgr) { return GetModelAlphau8(mgr) > 127; }

void CSpacePirate::SetVelocityForJump() {
  if (!x637_30_jumpVelSet) {
    CVector3f velocity = CVector3f::Zero();
    CVector3f position = GetTranslation();
    CVector3f delta = x828_patrolDestPos - position;
    float gravity = GetGravityConstant();
    float jumpZ = x824_jumpHeight + CMath::Max(x828_patrolDestPos[kDZ], position[kDZ]);
    velocity.SetZ(CMath::SqrtF(2.f * gravity * (jumpZ - position.GetZ())));
    float riseTime = velocity.GetZ() / gravity;
    riseTime += CMath::SqrtF(2.f * (jumpZ - x828_patrolDestPos[kDZ]) / gravity);
    float invTime = 1.f / riseTime;
    velocity.SetX(invTime * delta.GetX());
    velocity.SetY(invTime * delta.GetY());
    SetVelocityWR(velocity);
    x637_30_jumpVelSet = true;
  }
}

void CSpacePirate::SetAttackTarget(TUniqueId id) {
  x7c0_targetId = id;
  x7c4_burstFire.SetBurstType(1);
  x7bc_attackRemTime = 0.f;
}

void CSpacePirate::DetachActorFromPirate() { x7b4_attachedActor = kInvalidUniqueId; }

bool CSpacePirate::AttachActorToPirate(TUniqueId id) {
  if (x7b4_attachedActor == kInvalidUniqueId) {
    x7b4_attachedActor = id;
    return true;
  }
  return false;
}

CVector3f CSpacePirate::GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                                  const CVector3f& aimPos) const {
  return GetTranslation();
}

void CSpacePirate::SquadAdd(CStateManager& mgr) {
  if (x8c8_teamAiMgrId == kInvalidUniqueId) {
    x8c8_teamAiMgrId = CTeamAiMgr::GetTeamAiMgr(*this, mgr);
  }
  if (x8c8_teamAiMgrId != kInvalidUniqueId) {
    if (CTeamAiMgr* teamMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(x8c8_teamAiMgrId))) {
      teamMgr->AssignTeamAiRole(
          *this, x634_27_melee ? CTeamAiRole::kTAR_Melee : CTeamAiRole::kTAR_Projectile,
          CTeamAiRole::kTAR_Unknown, CTeamAiRole::kTAR_Invalid);
    }
  }
}

void CSpacePirate::SquadRemove(CStateManager& mgr) {
  if (x8c8_teamAiMgrId != kInvalidUniqueId) {
    if (CTeamAiMgr* teamMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(x8c8_teamAiMgrId))) {
      if (teamMgr->IsPartOfTeam(GetUniqueId())) {
        teamMgr->RemoveTeamAiRole(GetUniqueId());
        x8c8_teamAiMgrId = kInvalidUniqueId;
      }
    }
  }
}

void CSpacePirate::SquadReset(CStateManager& mgr) {
  CTeamAiMgr::ResetTeamAiRole(x634_27_melee ? kAT_Melee : kAT_Projectile, mgr, x8c8_teamAiMgrId,
                              GetUniqueId(), true);
}

void CSpacePirate::CheckForProjectiles(CStateManager& mgr) {
  if (x637_26_hearPlayerFire) {
    CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    CVector3f extent(5.f, 5.f, 5.f);
    CAABox bounds(aimPos - extent, aimPos + extent);
    x637_27_inProjectilePath = false;
    TEntityList nearList;
    mgr.BuildNearList(nearList, bounds, CMaterialFilter::MakeInclude(CMaterialList(kMT_Projectile)),
                      nullptr);
    for (int i = 0; i < nearList.size(); ++i) {
      const CGameProjectile* const projectile =
          TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(nearList[i]));
      if (projectile) {
        CVector3f delta = GetBoundingBox().GetCenterPoint() - projectile->GetTranslation();
        if (delta.IsMagnitudeSafe()) {
          if (CVector3f::Dot(GetTransform().GetForward(), delta) < 0.f) {
            delta.Normalize();
            CVector3f projDelta = projectile->GetTranslation() - projectile->GetPreviousPos();
            if (projDelta.IsMagnitudeSafe()) {
              projDelta.Normalize();
              if (CVector3f::Dot(projDelta, delta) > 0.939f) {
                x637_27_inProjectilePath = true;
              }
            }
          }
        } else {
          x637_27_inProjectilePath = true;
        }
        if (x637_27_inProjectilePath) {
          break;
        }
      }
    }
    x637_26_hearPlayerFire = false;
  }
}

bool CSpacePirate::LineOfSightTest(CStateManager& mgr, const CVector3f& eyePos,
                                   const CVector3f& targetPos, const CMaterialList& excludeList) {
  CMaterialFilter filter =
      CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), excludeList);
  return mgr.RayCollideWorld(eyePos, targetPos, filter, this);
}

void CPatterned::TryCover(CStateManager& mgr, int arg) {
  CBodyStateCmdMgr& cmdMgr = BodyCtrl()->CommandMgr();
  if (CScriptCoverPoint* cp = GetCoverPoint(mgr, x2dc_destObj)) {
    cmdMgr.DeliverCmd(CBCCoverCmd(static_cast< pas::ECoverDirection >(arg), cp->GetTranslation(),
                                  -cp->GetTransform().GetForward()));
  }
}

void CPatterned::TryWallHang(CStateManager&, int) {
  CBodyStateCmdMgr& cmdMgr = BodyCtrl()->CommandMgr();
  cmdMgr.DeliverCmd(CBCWallHangCmd(x2dc_destObj));
}

void CSpacePirate::UpdateCantSeePlayer(CStateManager& mgr) {
  if (++x7b0_cantSeePlayerCycleCounter % 7 == 0) {
    CVector3f eyePos = GetTranslation() + CVector3f(0.f, 0.f, x7a8_eyeHeight);
    CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    if (GetCoverPoint(mgr, x640_coverPoint)) {
      switch (x79c_coverDir) {
      case pas::kCD_Left:
        eyePos -= 2.f * GetTransform().GetRight();
        break;
      case pas::kCD_Right:
        eyePos += 2.f * GetTransform().GetRight();
        break;
      default:
        break;
      }
    } else {
      CVector3f toPlayer = (aimPos - eyePos).AsNormalized();
      eyePos += 1.1f * CVector3f::Cross(toPlayer, CVector3f::Up());
    }
    x637_28_noPlayerLos = !LineOfSightTest(mgr, eyePos, mgr.GetPlayer()->GetAimPosition(mgr, 0.f),
                                           CMaterialList(kMT_Player, kMT_ProjectilePassthrough));
  }
}

static CVector3f Random2f(CStateManager& mgr, float min, float max) {
  CVector3f result(mgr.Random()->Float() - 0.5f, mgr.Random()->Float() - 0.5f, 0.f);
  if (CMath::AbsF(result.GetX()) < 0.001f) {
    result.SetX(0.001f);
  }
  result.Normalize();
  result *= (max - min) * mgr.Random()->Float() + min;
  return result;
}

void CSpacePirate::UpdateHeldPosition(CStateManager& mgr, float dt) {
  CVector2f pos = mgr.GetPlayer()->GetTranslation().ToVec2f();
  if ((pos - x8d0_heldPosition).MagSquared() < 3.f) {
    x8d8_holdPositionTime += dt;
  } else {
    x8d0_heldPosition = pos;
    x8d8_holdPositionTime = 0.f;
  }
}

void CSpacePirate::AvoidActors(CStateManager& mgr) {
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (const CPatterned* ai = TCastToConstPtr< CPatterned >(list[i])) {
      if (ai != this && ai->GetCurrentAreaId() == GetCurrentAreaId()) {
        float avoidDistance = x568_pirateData.xc8_AvoidDistance;
        CVector3f separation =
            x45c_steeringBehaviors.Separation(*this, ai->GetTranslation(), avoidDistance);
        if (separation.IsNonZero()) {
          BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(separation, CVector3f::Zero(), 1.f));
          if (!x748_steeringDelayTimer) {
            if (CSpacePirate* pirate = PATTERNED_CAST_TO(CSpacePirate, const_cast< CPatterned* >(ai))) {
              if (!pirate->x748_steeringDelayTimer) {
                CVector3f delta = pirate->GetTranslation() - GetTranslation();
                if (CVector3f::Dot(GetTransform().GetForward(), delta) > 0.f &&
                    CVector3f::Dot(pirate->GetVelocityWR(), pirate->GetTransform().GetForward()) >
                        0.f) {
                  x748_steeringDelayTimer = 1.f;
                }
              }
            }
          }
        }
      }
    }
  }
}

bool CSpacePirate::CantJumpBack(CStateManager& mgr, const CVector3f& dir, float dist) {
  CVector3f center = GetBoundingBox().GetCenterPoint();
  bool clear = true;
  if (!LineOfSightTest(mgr, center, center + dist * dir, CMaterialList()) ||
      LineOfSightTest(mgr, center + (0.5f * dist) * dir,
                      center + (0.5f * dist) * dir + CVector3f::Down() * 5.f, CMaterialList()) ||
      LineOfSightTest(mgr, center + dist * dir, center + dist * dir + CVector3f::Down() * 5.f,
                      CMaterialList())) {
    clear = false;
  }
  const bool result = clear;
  return result;
}

pas::EStepDirection CSpacePirate::GetStrafeDir(CStateManager& mgr, float dist) {
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  float distSq = dist * dist;
  pas::EStepDirection result = pas::kSD_Invalid;
  bool right = true;
  bool left = true;
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (CSpacePirate* pirate = PATTERNED_CAST_TO(CSpacePirate, const_cast< CEntity* >(list[i]))) {
      if (pirate != this && pirate->GetCurrentAreaId() == GetCurrentAreaId()) {
        CVector3f delta = pirate->GetTranslation() - GetTranslation();
        float deltaSq = delta.MagSquared();
        if (deltaSq < distSq) {
          float dot = CVector3f::Dot(delta, GetTransform().GetRight());
          if (dot > 0.866f * deltaSq || (dot > 0.f && deltaSq < 3.f)) {
            right = false;
          } else if (dot < -deltaSq * 0.866f || (dot < 0.f && deltaSq < 3.f)) {
            left = false;
          }
        }
      }
    }
  }
  if (right) {
    right = CantJumpBack(mgr, GetTransform().GetRight(), dist);
  }
  if (left) {
    left = CantJumpBack(mgr, -GetTransform().GetRight(), dist);
  }
  if (left && right) {
    if ((mgr.Random()->Next() & 0x4000) != 0) {
      left = false;
    } else {
      right = false;
    }
  }
  if (left) {
    result = pas::kSD_Left;
  } else if (right) {
    result = pas::kSD_Right;
  }
  return result;
}

void CSpacePirate::CheckBlade(CStateManager& mgr) {
  if (!x638_25_appliedBladeDamage && x7b9_swooshSeg != CSegId::Invalid()) {
    if (CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(mgr.ObjectById(x7c0_targetId))) {
      CTransform4f swoosh = GetLctrTransform(x7b9_swooshSeg);
      const CVector3f& scale = GetModelData()->GetScale();
      CVector3f extent(scale.GetX() / 2.f, scale.GetY() / 2.f, scale.GetZ() / 2.f);
      CAABox bounds(swoosh.GetTranslation() - extent, swoosh.GetTranslation() + extent);
      if (bounds.DoBoundsOverlap(actor->GetBoundingBox())) {
        mgr.ApplyDamage(
            GetUniqueId(), actor->GetUniqueId(), GetUniqueId(), x568_pirateData.x4c_BladeDamage,
            CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
            CVector3f::Zero());
        x638_25_appliedBladeDamage = true;
      }
    }
  }
}

CVector3f CSpacePirate::GetTargetPos(CStateManager& mgr) {
  if (x7c0_targetId != mgr.GetPlayer()->GetUniqueId()) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(x7c0_targetId))) {
      if (actor->GetActive()) {
        return actor->GetTranslation();
      }
    }
    x764_boneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    x7c0_targetId = mgr.GetPlayer()->GetUniqueId();
  }
  return mgr.GetPlayer()->GetTranslation();
}

void CSpacePirate::SetCinematicCollision(CStateManager& mgr) {
  RemoveMaterial(kMT_AIBlock, mgr);
  CMaterialList include = GetMaterialFilter().GetIncludeList();
  include.Remove(kMT_AIBlock);
  SetMaterialFilter(
      CMaterialFilter::MakeIncludeExclude(include, GetMaterialFilter().GetExcludeList()));
}

void CSpacePirate::SetNonCinematicCollision(CStateManager& mgr) {
  AddMaterial(kMT_AIBlock, mgr);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      GetMaterialFilter().GetIncludeList().Union(CMaterialList(kMT_AIBlock)),
      GetMaterialFilter().GetExcludeList()));
}

void CSpacePirate::KnockBack(const CVector3f& dir, CStateManager& mgr, const CDamageInfo& info,
                             float magnitude, bool direct, const bool inDeferred) {
  if (!x634_25_ceilingAmbush || !x400_25_alive || inDeferred) {
    x460_knockBackController.SetAutoResetImpulse(!x634_31_noKnockbackImpulseReset);
    x460_knockBackController.EnableAnimReaction(kAR_KnockBack, IsOnGround());
    bool enableFreeze = true;
    if ((x636_24_trooper || x635_27_shadowPirate) && !info.GetWeaponMode().IsCharged() &&
        !info.GetWeaponMode().IsComboed()) {
      enableFreeze = false;
    }
    x460_knockBackController.SetEnableFreeze(enableFreeze);
    CPatterned::KnockBack(dir, mgr, info, magnitude, direct, inDeferred);
    if (x635_27_shadowPirate) {
      if (x400_25_alive) {
        if (magnitude >= 4.f && !BodyCtrl()->IsFrozen()) {
          x3e8_alphaDelta = 1.f;
          x8a8_cloakDelayTimer += 0.1f * magnitude;
          x8a8_cloakDelayTimer = CMath::Clamp(0.f, x8a8_cloakDelayTimer, 1.f);
          x8bc_maxCloakAlpha = 0.5f;
          mgr.ActorModelParticles()->StartElectric(*this);
          x8ac_electricParticleTimer = x8a8_cloakDelayTimer + 1.f;
        }
      } else {
        x8bc_maxCloakAlpha = x3e8_alphaDelta = 1.f;
        x8b8_minCloakAlpha = 0.f;
        mgr.ActorModelParticles()->StartElectric(*this);
        x8ac_electricParticleTimer = 2.f;
      }
    }
    if (x635_30_floatingCorpse && x85c_ragDoll.get()) {
      x85c_ragDoll->TorsoImpulse() += (20.f * magnitude) * dir;
    }
    if (x400_25_alive) {
      switch (x460_knockBackController.GetActiveParms().x0_animState) {
      case kAR_Hurled: {
        x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), rstl::string_l("GetUpNow"));
        CSfxManager::AddEmitter(x568_pirateData.xc0_Sound_Hurled, GetTranslation(),
                                CVector3f::Zero(), 127, true, false);
        break;
      }
      default:
        break;
      }
    } else {
      const CKnockBackMgr::KnockBackParms& parms = x460_knockBackController.GetActiveParms();
      switch (parms.x0_animState) {
      case kAR_Hurled:
        if (parms.x4_animFollowup != kKBAFU_LaggedBurnDeath &&
            parms.x4_animFollowup != kKBAFU_BurnDeath) {
          CSfxManager::AddEmitter(x568_pirateData.xc2_Sound_Death, GetTranslation(),
                                  CVector3f::Zero(), 127, true, false);
        }
        break;
      default:
        break;
      }
    }
  }
}

void CSpacePirate::Death(CStateManager& mgr, const CVector3f& dir, EScriptObjectState state) {
  if (IsAlive()) {
    CPatterned::Death(mgr, dir, state);
    if (x7b4_attachedActor != kInvalidUniqueId) {
      BodyCtrl()->CommandMgr().DeliverCmd(
          CBCKnockDownCmd(GetTransform().GetForward(), pas::kS_Two));
    }
  }
}

bool CSpacePirate::Stuck(CStateManager& mgr, float arg) {
  if (GetStateMachineTime() > 0.5f) {
    return CPatterned::Stuck(mgr, arg) || CPatterned::PathShagged(mgr, arg);
  }
  return false;
}

bool CSpacePirate::PatternShagged(CStateManager& mgr, float arg) {
  return CPatterned::Stuck(mgr, arg) || CPatterned::PatternShagged(mgr, arg);
}

void CSpacePirate::Generate(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x637_25_enableAim = true;
    if (!x639_31_sentAttackMsg) {
      x639_31_sentAttackMsg = true;
      SendScriptMsgs(kSS_Attack, mgr, kSM_None);
    }
    x32c_animState = kAS_Ready;
    if (x634_25_ceilingAmbush) {
      x2e0_destPos = GetTranslation() + CVector3f::Down();
      x828_patrolDestPos = x2e0_destPos;
      x824_jumpHeight = 0.f;
    } else {
      TUniqueId wpId = GetConnectedObject(mgr, kSS_Attack, kSM_Follow);
      if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(wpId))) {
        x2e0_destPos = actor->GetTranslation();
        x828_patrolDestPos = x2e0_destPos;
        x824_jumpHeight = 3.f;
      }
      BodyCtrl()->SetLocomotionType(pas::kLT_Combat);
    }
    break;
  case kStateMsg_Update: {
    int jumpType = 0;
    if (x634_25_ceilingAmbush) {
      jumpType = 2;
    }
    TryCommand(mgr, pas::kAS_Jump, &CPatterned::TryJump, jumpType);
    if (x32c_animState == kAS_Repeat) {
      BodyCtrl()->SetLocomotionType(pas::kLT_Combat);
    }
    CVector3f target = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    target.SetZ(0.f);
    BodyCtrl()->CommandMgr().DeliverTargetVector(target);
    break;
  }
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    BodyCtrl()->SetLocomotionType(pas::kLT_Combat);
    x824_jumpHeight = 3.f;
    x634_25_ceilingAmbush = false;
    x764_boneTracking.SetActive(true);
    x764_boneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    break;
  }
}

void CSpacePirate::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    x644_steeringSpeed = BodyCtrl()->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_Walk) /
                         BodyCtrl()->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_Run);
    break;
  case kStateMsg_Deactivate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Combat);
    if (!x639_31_sentAttackMsg) {
      x639_31_sentAttackMsg = true;
      SendScriptMsgs(kSS_Attack, mgr, kSM_None);
    }
    break;
  }
  if (x637_24_enablePatrol) {
    CPatterned::Patrol(mgr, msg, dt);
    switch (msg) {
    case kStateMsg_Activate:
      BodyCtrl()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
      BodyCtrl()->SetTurnSpeed(BodyCtrl()->GetTurnSpeed() / 1.25f);
      break;
    case kStateMsg_Update:
      AvoidActors(mgr);
      x828_patrolDestPos = x2e0_destPos;
      break;
    case kStateMsg_Deactivate:
      BodyCtrl()->CommandMgr().SetSteeringBlendMode(kSBM_Normal);
      BodyCtrl()->SetTurnSpeed(BodyCtrl()->GetTurnSpeed() * 1.25f);
      break;
    }
  }
}

void CSpacePirate::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::Patrol(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Activate:
    x644_steeringSpeed = 1.f;
    BodyCtrl()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
    x2dc_destObj = GetConnectedObject(mgr, kSS_Attack, kSM_Follow);
    break;
  case kStateMsg_Update:
    if (CScriptWaypoint* wp = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(x2dc_destObj))) {
      const uint jump = (wp->GetBehaviourModifiers() >> 1) & 1;
      const uint drop = (wp->GetBehaviourModifiers() >> 2) & 1;
      if (jump || drop) {
        float maxSpeed = BodyCtrl()->GetBodyStateInfo().GetMaxSpeed();
        const CVector3f& scale = CVector3f(GetModelData()->GetScale());
        float distance = maxSpeed * ((1.5f * dt + 0.1f) * scale.GetY()) + x7a4_intoJumpDist;
        if ((GetTranslation() - wp->GetTranslation()).MagSquared() < distance * distance) {
          x328_24_inPosition = true;
          x824_jumpHeight = jump ? 3.f : 0.f;
        }
      }
    }
    if (BodyCtrl()->GetCurrentStateId() == pas::kAS_Jump) {
      bool targetPlayer = true;
      if (CScriptWaypoint* wp = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(x2dc_destObj))) {
        for (AUTO(conn, wp->GetConnectionList().begin()); conn != wp->GetConnectionList().end();
             ++conn) {
          if (conn->x0_state == kSS_Arrived && conn->x4_msg == kSM_Next) {
            targetPlayer = false;
          }
        }
      }
      if (targetPlayer) {
        BodyCtrl()->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
                                                     GetTranslation());
      }
    }
    x828_patrolDestPos = x2e0_destPos;
    break;
  case kStateMsg_Deactivate:
    BodyCtrl()->CommandMgr().SetSteeringBlendMode(kSBM_Normal);
    break;
  }
}

bool CSpacePirate::PatternOver(CStateManager& mgr, float arg) {
  return GetDestObj() == kInvalidUniqueId;
}

bool CSpacePirate::HearShot(CStateManager& mgr, float arg) {
  const bool heard = x636_25_hearNoise;
  x636_25_hearNoise = false;
  return heard;
}

bool CSpacePirate::HearPlayer(CStateManager& mgr, float arg) {
  bool heard = false;
  if (mgr.GetPlayer()->GetVelocityWR().MagSquared() > 0.1f) {
    CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    if (delta.MagSquared() <
        x568_pirateData.x14_HearingRadius * x568_pirateData.x14_HearingRadius) {
      heard = true;
    }
  }
  return heard;
}

void CSpacePirate::Halt(CStateManager& mgr, EStateMsg msg, float dt) { x644_steeringSpeed = 0.f; }

void CSpacePirate::Run(CStateManager& mgr, EStateMsg msg, float dt) { x644_steeringSpeed = 1.f; }

void CSpacePirate::Taunt(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x637_25_enableAim = true;
    x764_boneTracking.SetActive(true);
    x764_boneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    if (x7c0_targetId == kInvalidUniqueId) {
      x7c0_targetId = mgr.GetPlayer()->GetUniqueId();
    }
    if (BodyCtrl()->HasBodyState(pas::kAS_Taunt)) {
      if (!x635_27_shadowPirate) {
        bool findOtherPirate = true;
        if (x634_27_melee) {
          const CPASAnimParmData parms(pas::kAS_Taunt, CPASAnimParm::FromEnum(2));
          const rstl::pair< float, int > anim =
              BodyCtrl()->GetPASDatabase().FindBestAnimation(parms, *mgr.Random(), -1);
          if (anim.first > 0.f) {
            findOtherPirate = false;
            x760_taunt = pas::kTT_Two;
          }
        }
        if (findOtherPirate) {
          bool withOtherPirate = false;
          const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
          for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
            if (CSpacePirate* pirate = PATTERNED_CAST_TO(CSpacePirate, const_cast< CEntity* >(list[i]))) {
              if (pirate != this && !pirate->x637_25_enableAim && pirate->IsAlive() &&
                  pirate->GetCurrentAreaId() == GetCurrentAreaId()) {
                if ((pirate->GetTranslation() - GetTranslation()).MagSquared() <
                    x568_pirateData.x14_HearingRadius * x568_pirateData.x14_HearingRadius) {
                  withOtherPirate = true;
                }
              }
            }
          }
          x760_taunt = withOtherPirate ? pas::kTT_Zero : pas::kTT_One;
        }
      } else {
        x760_taunt = x635_28_alertBeforeCloak ? pas::kTT_One : pas::kTT_Zero;
      }
      x32c_animState = kAS_Ready;
    } else {
      CSfxManager::AddEmitter(x568_pirateData.xa4_Sound_Alert, GetTranslation(), CVector3f::Zero(),
                              true, false);
    }
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Taunt, &CPatterned::TryTaunt, x760_taunt);
    break;
  case kStateMsg_Deactivate:
    if (x760_taunt == pas::kTT_Zero) {
      mgr.InformListeners(GetTranslation(), kLNT_PlayerFire);
    }
    x32c_animState = kAS_NotReady;
    break;
  }
}

void CSpacePirate::GetUp(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    SquadReset(mgr);
    x8dc_leashTimer = 0.f;
    break;
  case kStateMsg_Update:
    if (BodyCtrl()->GetCurrentStateId() == pas::kAS_LieOnGround &&
        x660_pathFindSearch.Search(GetTranslation(), GetTranslation()) ==
            CPathFindSearch::kR_NoSourcePoint) {
      x401_30_pendingDeath = true;
    } else {
      TryCommand(mgr, pas::kAS_Getup, &CPatterned::TryGetUp, 0);
    }
    UpdateLeashTimer(dt);
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    break;
  }
}

void CSpacePirate::Lurk(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    ReleaseCoverPoint(mgr, x640_coverPoint);
    x644_steeringSpeed = 0.f;
    BodyCtrl()->SetLocomotionType(pas::kLT_Combat);
    x637_28_noPlayerLos = true;
    x7ac_timeNoPlayerLos = 0.f;
    float aggression = x568_pirateData.x0_AggressionCheck;
    x638_26_alwaysAggressive = mgr.Random()->Range(0.f, 100.f) < aggression;
    float cover = x568_pirateData.x4_CoverCheck;
    x638_27_coverCheck = mgr.Random()->Range(0.f, 100.f) < cover;
    float dodge = x568_pirateData.x94_DodgeCheck;
    x638_28_enableDodge = mgr.Random()->Range(0.f, 100.f) < dodge;
    x637_25_enableAim = true;
    x764_boneTracking.SetActive(true);
    x764_boneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    if (x634_29_onlyAttackInRange) {
      x7c4_burstFire.SetBurstType(4);
      BodyCtrl()->SetLocomotionType(pas::kLT_Combat);
    }
    x63a_24_normalDodge = false;
    break;
  }
  case kStateMsg_Update:
    if (BodyCtrl()->HasBodyState(pas::kAS_Turn)) {
      if (x32c_animState != kAS_NotReady) {
        TryCommand(mgr, pas::kAS_Turn, &CPatterned::TryTurn, 0);
      }
      if (x32c_animState != kAS_Repeat) {
        x2e0_destPos = GetTargetPos(mgr);
        CVector3f delta = x2e0_destPos - GetTranslation();
        delta.SetZ(0.f);
        if (CVector3f::Dot(GetTransform().GetForward(), delta.AsNormalized()) < 0.9f) {
          x32c_animState = kAS_Ready;
        }
      }
    }
    if (x635_26_seated && x639_28_satUp) {
      if (x7bc_attackRemTime > x304_averageAttackTime &&
          BodyCtrl()->GetLocomotionType() == pas::kLT_Combat) {
        BodyCtrl()->SetLocomotionType(pas::kLT_Internal5);
      } else if (x7bc_attackRemTime < 0.5f * x304_averageAttackTime &&
                 BodyCtrl()->GetLocomotionType() == pas::kLT_Internal5) {
        BodyCtrl()->SetLocomotionType(pas::kLT_Combat);
      }
    }
    UpdateCantSeePlayer(mgr);
    UpdateHeldPosition(mgr, dt);
    break;
  case kStateMsg_Deactivate:
    x638_26_alwaysAggressive = false;
    x638_29_noPlayerDodge = false;
    x32c_animState = kAS_NotReady;
    break;
  }
}

bool CSpacePirate::AggressionCheck(CStateManager& mgr, float arg) {
  bool result = false;
  if (!x634_26_nonAggressive) {
    if (x638_26_alwaysAggressive) {
      result = true;
    } else if (mChargePlayerList.empty() && x7ac_timeNoPlayerLos > 10.f) {
      result = true;
    }
    if (result) {
      x30c_behaviourOrient = kBO_MoveDir;
      if (rstl::find< rstl::list< TUniqueId >::const_iterator, TUniqueId >(
              mChargePlayerList.begin(), mChargePlayerList.end(), GetUniqueId()) ==
          mChargePlayerList.end()) {
        mChargePlayerList.push_back(GetUniqueId());
      }
    }
  }
  return result;
}

bool CSpacePirate::CoverCheck(CStateManager& mgr, float arg) { return x638_27_coverCheck; }

bool CSpacePirate::CoverFind(CStateManager& mgr, float arg) {
  bool result = false;
  float minDistSq = x568_pirateData.x8_SearchRadius * x568_pirateData.x8_SearchRadius;
  const CScriptCoverPoint* closest = nullptr;
  const CObjectList& list = mgr.GetObjectListById(kOL_AiWaypoint);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (const CScriptCoverPoint* cp = TCastToConstPtr< CScriptCoverPoint >(list[i])) {
      if (cp->GetActive() && !cp->ShouldLandHere() && !cp->GetInUse(GetUniqueId()) &&
          cp->GetCurrentAreaId() == GetCurrentAreaId() &&
          cp->GetUniqueId() != x642_previousCoverPoint) {
        float distSq = (GetTranslation() - cp->GetTranslation()).MagSquared();
        if (distSq < minDistSq && !cp->Blown(mgr.GetPlayer()->GetTranslation())) {
          minDistSq = distSq;
          closest = cp;
        }
      }
    }
  }
  if (closest) {
    ReleaseCoverPoint(mgr, x640_coverPoint);
    if (CScriptCoverPoint* cp =
            TCastToPtr< CScriptCoverPoint >(mgr.ObjectById(closest->GetUniqueId()))) {
      SetCoverPoint(cp, x640_coverPoint);
      result = true;
      x642_previousCoverPoint = x640_coverPoint;
      x654_coverPointRearDir = -closest->GetTransform().GetForward();
      x30c_behaviourOrient = kBO_MoveDir;
    }
  }
  return result;
}

bool CSpacePirate::CoverBlown(CStateManager& mgr, float arg) {
  bool result = true;
  CVector3f toPlayer = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  if (toPlayer.MagSquared() > x2fc_minAttackRange * x2fc_minAttackRange) {
    if (CScriptCoverPoint* cp = GetCoverPoint(mgr, x640_coverPoint)) {
      result = cp->Blown(mgr.GetPlayer()->GetTranslation());
      if (!result && x644_steeringSpeed == 0.f &&
          BodyCtrl()->GetCurrentStateId() != pas::kAS_Step) {
        CVector3f toCover = cp->GetTranslation() - GetTranslation();
        if (toCover.MagSquared() > 3.f * CVector3f(GetModelData()->GetScale()).GetY()) {
          result = true;
        }
      }
    }
  }
  return result;
}

bool CSpacePirate::CoverNearlyBlown(CStateManager& mgr, float arg) {
  bool result = true;
  if (CScriptCoverPoint* cp = GetCoverPoint(mgr, x640_coverPoint)) {
    CVector3f pos = mgr.GetPlayer()->GetTranslation() + 1.f * mgr.GetPlayer()->GetVelocityWR();
    result = cp->Blown(pos);
  }
  return result;
}

bool CSpacePirate::CoveringFire(CStateManager& mgr, float arg) {
  bool result = false;
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (CSpacePirate* pirate = PATTERNED_CAST_TO(CSpacePirate, const_cast< CEntity* >(list[i]))) {
      if (pirate != this && pirate->x636_31_inAttackState &&
          pirate->GetCurrentAreaId() == GetCurrentAreaId()) {
        result = true;
      }
    }
  }
  return result;
}

bool CSpacePirate::ShouldAttack(CStateManager& mgr, float arg) {
  bool result = true;
  if (x7c0_targetId == mgr.GetPlayer()->GetUniqueId()) {
    CVector3f target = GetTargetPos(mgr);
    int numCloserPirates = 0;
    float distSq = (GetTranslation() - target).MagSquared();
    const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
    for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
      if (CSpacePirate* pirate = PATTERNED_CAST_TO(CSpacePirate, const_cast< CEntity* >(list[i]))) {
        if (pirate != this && pirate->x636_31_inAttackState && pirate->x400_25_alive &&
            pirate->GetCurrentAreaId() == GetCurrentAreaId()) {
          if ((pirate->GetTranslation() - target).MagSquared() < distSq) {
            ++numCloserPirates;
            if (numCloserPirates > 3) {
              result = false;
            }
          }
        }
      }
    }
  }
  return result;
}

void CSpacePirate::Cover(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (BodyCtrl()->GetCurrentStateId() != pas::kAS_Cover) {
      if (CScriptCoverPoint* cp = GetCoverPoint(mgr, x640_coverPoint)) {
        x79c_coverDir = static_cast< pas::ECoverDirection >(
            (static_cast< uint >(cp->GetAttackDirection()) >> 1) & 1);
        x32c_animState = kAS_Ready;
        x2e0_destPos = cp->GetTranslation();
        TryCommand(mgr, pas::kAS_Cover, &CPatterned::TryCover, x79c_coverDir);
      }
    }
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Cover, &CPatterned::TryCover, x79c_coverDir);
    if (CScriptCoverPoint* cp = GetCoverPoint(mgr, x640_coverPoint)) {
      BodyCtrl()->CommandMgr().DeliverTargetVector(-cp->GetTransform().GetForward());
    }
    UpdateCantSeePlayer(mgr);
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    break;
  }
}

void CSpacePirate::CoverAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_LeanFromCover));
    x636_31_inAttackState = true;
    break;
  case kStateMsg_Update:
    UpdateCantSeePlayer(mgr);
    break;
  case kStateMsg_Deactivate:
    x636_31_inAttackState = false;
    break;
  }
}

void CSpacePirate::Enraged(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    break;
  }
}

void CSpacePirate::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    BodyCtrl()->SetLocomotionType(pas::kLT_Combat);
    x2e0_destPos = GetTargetPos(mgr);
    x648_targetDelta = x2e0_destPos - GetBoundingBox().GetCenterPoint();
    x644_steeringSpeed = 0.f;
    x636_26_enableMeleeAttack = false;
    if (!x635_24_noMeleeAttack &&
        x648_targetDelta.MagSquared() < x2fc_minAttackRange * x2fc_minAttackRange &&
        x648_targetDelta[kDZ] * x648_targetDelta[kDZ] < 4.f) {
      x636_26_enableMeleeAttack = true;
      x638_25_appliedBladeDamage = false;
    } else {
      if (CVector3f::Dot(GetTransform().GetForward(), x648_targetDelta.AsNormalized()) < 0.8f) {
        BodyCtrl()->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), x648_targetDelta, 1.f));
      }
    }
    x636_31_inAttackState = true;
    x8bc_maxCloakAlpha = 0.75f;
    break;
  case kStateMsg_Update:
    if (x636_26_enableMeleeAttack) {
      TryCommand(mgr, pas::kAS_MeleeAttack, &CPatterned::TryMeleeAttack, 1);
      BodyCtrl()->CommandMgr().DeliverTargetVector(x648_targetDelta);
      CheckBlade(mgr);
      if (x635_27_shadowPirate) {
        if (x32c_animState == kAS_Over) {
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
  case kStateMsg_Deactivate:
    x636_26_enableMeleeAttack = false;
    x636_31_inAttackState = false;
    x32c_animState = kAS_NotReady;
    break;
  }
}

void CSpacePirate::DoubleSnap(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (!x635_24_noMeleeAttack) {
      x32c_animState = kAS_Ready;
    }
    x2e0_destPos = GetTargetPos(mgr);
    x648_targetDelta = x2e0_destPos - GetTranslation();
    x644_steeringSpeed = 0.f;
    x636_26_enableMeleeAttack = true;
    x83c_meleeSeverity = pas::kS_One;
    x638_25_appliedBladeDamage = false;
    x636_31_inAttackState = true;
    x639_30_closeMelee = false;
    mChargePlayerList.remove(GetUniqueId());
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_MeleeAttack, &CPatterned::TryMeleeAttack, x83c_meleeSeverity);
    if (x83c_meleeSeverity == pas::kS_One && x32c_animState == kAS_Over) {
      CVector3f delta = GetTargetPos(mgr) - GetTranslation();
      if (delta.MagSquared() < x2fc_minAttackRange * x2fc_minAttackRange &&
          CVector3f::Dot(delta.AsNormalized(), GetTransform().GetForward()) > -0.123f) {
        x32c_animState = kAS_Ready;
        x83c_meleeSeverity = pas::kS_Two;
        x638_25_appliedBladeDamage = false;
        x648_targetDelta = delta;
        x639_30_closeMelee = true;
      }
    }
    if (x639_30_closeMelee) {
      x648_targetDelta = GetTargetPos(mgr) - GetTranslation();
    }
    BodyCtrl()->CommandMgr().DeliverTargetVector(x648_targetDelta);
    if (x635_27_shadowPirate) {
      if (x32c_animState == kAS_Over) {
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
  case kStateMsg_Deactivate:
    x636_26_enableMeleeAttack = false;
    x636_31_inAttackState = false;
    x32c_animState = kAS_NotReady;
    break;
  }
}

bool CSpacePirate::ShouldCrouch(CStateManager& mgr, float arg) {
  bool result = false;
  if (CScriptCoverPoint* cp = GetCoverPoint(mgr, x640_coverPoint)) {
    result = cp->ShouldCrouch();
  }
  return result;
}

void CSpacePirate::Crouch(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Crouch);
    if (CScriptCoverPoint* cp = GetCoverPoint(mgr, x640_coverPoint)) {
      x648_targetDelta = cp->GetTransform().GetForward();
    }
    x644_steeringSpeed = 0.f;
    TargetPlayer(mgr, msg, dt);
    x79c_coverDir = pas::kCD_Invalid;
    break;
  case kStateMsg_Update:
    BodyCtrl()->CommandMgr().DeliverTargetVector(x648_targetDelta);
    UpdateCantSeePlayer(mgr);
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

bool CSpacePirate::ShouldStrafe(CStateManager& mgr, float arg) {
  bool result = false;
  bool noPlayerStrafe = false;
  x834_skidDir = pas::kSD_Invalid;
  if (!x634_26_nonAggressive) {
    CVector3f toTarget = GetTargetPos(mgr) - GetTranslation();
    if (CVector3f::Dot(toTarget, GetTransform().GetForward()) > 0.f) {
      if ((x854_lowHealthFrenzyTimer < 0.66f || x850_timeSinceHitByPlayer < 0.66f) &&
          x838_strafeDelayTimer == 0.f) {
        CVector3f center = GetBoundingBox().GetCenterPoint();
        const CVector3f& delta = (GetTargetPos(mgr) - center).AsNormalized();
        if (CVector3f::Dot(delta, GetTransform().GetForward()) > 0.707f) {
          x834_skidDir = GetStrafeDir(mgr, 10.f);
          if (x834_skidDir != pas::kSD_Invalid) {
            result = true;
          } else {
            noPlayerStrafe = true;
          }
        }
      }
      if (!noPlayerStrafe && !result && x7c0_targetId == mgr.GetPlayer()->GetUniqueId() &&
          x7ac_timeNoPlayerLos > 1.f) {
        if ((mgr.GetPlayer()->GetTranslation() - GetTranslation()).Magnitude() < 15.f &&
            x834_skidDir == pas::kSD_Invalid) {
          x834_skidDir = GetStrafeDir(mgr, 5.f);
          if (x834_skidDir != pas::kSD_Invalid) {
            result = true;
          }
        }
      }
    }
  }
  return result;
}

void CSpacePirate::Skid(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x838_strafeDelayTimer = 4.f;
    x636_31_inAttackState = true;
    break;
  case kStateMsg_Update:
    if (BodyCtrl()->GetCurrentStateId() != pas::kAS_Step) {
      BodyCtrl()->CommandMgr().DeliverCmd(CBCStepCmd(x834_skidDir, pas::kStep_Normal));
    }
    break;
  case kStateMsg_Deactivate:
    x636_31_inAttackState = false;
    break;
  }
}

void CSpacePirate::Flee(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    BodyCtrl()->CommandMgr().SetSteeringBlendMode(kSBM_Normal);
    CVector3f delta = GetTranslation() - mgr.GetPlayer()->GetTranslation();
    SetDestPos(GetTranslation() + delta.AsNormalized() * 15.f);
    x30c_behaviourOrient = kBO_MoveDir;
    x644_steeringSpeed = 1.f;
    break;
  }
  case kStateMsg_Update:
    AvoidActors(mgr);
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CSpacePirate::Approach(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->CommandMgr().SetSteeringBlendMode(kSBM_Normal);
    x30c_behaviourOrient = kBO_MoveDir;
    x644_steeringSpeed = 1.f;
    break;
  case kStateMsg_Update:
    AvoidActors(mgr);
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

bool CSpacePirate::SpotPlayer(CStateManager& mgr, float arg) {
  CVector3f toPlayer = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  float distance = toPlayer.Magnitude();
  float angle = x3c4_detectionAngle;
  return CVector3f::Dot(toPlayer, GetTransform().GetForward()) > distance * angle;
}

bool CSpacePirate::LineOfSight(CStateManager& mgr, float arg) { return !x637_28_noPlayerLos; }

void CSpacePirate::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x840_jumpPoint = kInvalidUniqueId;
    if (CScriptCoverPoint* cp = GetCoverPoint(mgr, x640_coverPoint)) {
      x2ec_reflectedDestPos = GetTranslation();
      x328_24_inPosition = false;
      x2dc_destObj = cp->GetUniqueId();
      x2e0_destPos = cp->GetTranslation();
    }
    if (GetSearchPath()->Search(GetTranslation(), x2e0_destPos) == CPathFindSearch::kR_Success) {
      x2ec_reflectedDestPos = GetTranslation();
      x2e0_destPos = GetSearchPath()->GetPoint();
      x328_24_inPosition = false;
      BodyCtrl()->CommandMgr().DeliverCmd(
          CBCLocomotionCmd(x2e0_destPos - GetTranslation(), CVector3f::Zero(), 1.f));
    } else {
      CScriptAiJumpPoint* best = nullptr;
      float minDistSq = FLT_MAX;
      CObjectList& list = mgr.ObjectListById(kOL_AiWaypoint);
      for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
        if (CScriptAiJumpPoint* jp = TCastToPtr< CScriptAiJumpPoint >(list[i])) {
          if (jp->GetActive() && !jp->GetInUse(GetUniqueId()) &&
              jp->GetJumpTarget() == kInvalidUniqueId &&
              jp->GetCurrentAreaId() == GetCurrentAreaId()) {
            CVector3f toJump = jp->GetTranslation() - GetTranslation();
            float distSq = toJump.MagSquared();
            if (distSq > 25.f && CVector3f::Dot(jp->GetTransform().GetForward(), toJump) > 0.f) {
              if (const CScriptWaypoint* wp =
                      TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(jp->GetJumpPoint()))) {
                if ((x2e0_destPos[kDZ] - GetTranslation().GetZ()) *
                        (wp->GetTranslation().GetZ() - jp->GetTranslation().GetZ()) >
                    0.f) {
                  CVector3f toDest = x2e0_destPos - wp->GetTranslation();
                  distSq += 4.f * toJump.GetZ() * toJump.GetZ();
                  distSq += toDest.MagSquared() + 9.f * toDest.GetZ() * toDest.GetZ();
                  if (distSq < minDistSq &&
                      GetSearchPath()->PathExists(GetTranslation(), jp->GetTranslation()) ==
                          CPathFindSearch::kR_Success) {
                    bool good = false;
                    bool noPath = GetSearchPath()->PathExists(wp->GetTranslation(), x2e0_destPos) !=
                                  CPathFindSearch::kR_Success;
                    if (noPath) {
                      distSq += 1000.f;
                    }
                    if (!noPath) {
                      good = true;
                    }
                    if (distSq < minDistSq) {
                      minDistSq = distSq;
                      best = jp;
                      if (good) {
                        break;
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      if (best) {
        x2e0_destPos = best->GetTranslation();
        if (GetSearchPath()->Search(GetTranslation(), x2e0_destPos) ==
            CPathFindSearch::kR_Success) {
          x2ec_reflectedDestPos = GetTranslation();
          x2e0_destPos = GetSearchPath()->GetPoint();
          x328_24_inPosition = false;
          x840_jumpPoint = best->GetUniqueId();
          x824_jumpHeight = best->GetJumpApex();
          if (const CScriptWaypoint* wp =
                  TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(best->GetJumpPoint()))) {
            x828_patrolDestPos = wp->GetTranslation();
            BodyCtrl()->CommandMgr().DeliverCmd(
                CBCLocomotionCmd(x2e0_destPos, CVector3f::Zero(), 1.f));
            x30c_behaviourOrient = kBO_MoveDir;
          }
        }
      }
    }
    BodyCtrl()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
    if (x637_25_enableAim) {
      x644_steeringSpeed = 1.f;
    }
    x639_27_inRange = false;
    x63a_24_normalDodge = true;
    break;
  case kStateMsg_Update:
    CPatterned::PathFind(mgr, msg, dt);
    if (x840_jumpPoint != kInvalidUniqueId) {
      if (CScriptAiJumpPoint* jp =
              TCastToPtr< CScriptAiJumpPoint >(mgr.ObjectById(x840_jumpPoint))) {
        float maxSpeed = BodyCtrl()->GetBodyStateInfo().GetMaxSpeed();
        const CVector3f& scale = CVector3f(GetModelData()->GetScale());
        float jumpDistance = maxSpeed * ((1.5f * dt + 0.1f) * scale.GetY()) + x7a4_intoJumpDist;
        if ((GetTranslation() - jp->GetTranslation()).MagSquared() < jumpDistance * jumpDistance) {
          x32c_animState = kAS_Ready;
          TryCommand(mgr, pas::kAS_Jump, &CPatterned::TryJump, 0);
        }
      }
    }
    AvoidActors(mgr);
    if (!x639_27_inRange) {
      if (CScriptCoverPoint* cp = GetCoverPoint(mgr, x640_coverPoint)) {
        float maxSpeed = BodyCtrl()->GetBodyStateInfo().GetMaxSpeed();
        const CVector3f& scale = CVector3f(GetModelData()->GetScale());
        x754_coverRange = maxSpeed * ((1.5f * dt + 0.1f) * scale.GetY());
        if (cp->ShouldWallHang()) {
          x754_coverRange += x7a4_intoJumpDist;
        }
        x639_27_inRange = (GetTranslation() - cp->GetTranslation()).MagSquared() <
                          x754_coverRange * x754_coverRange;
      }
    }
    UpdateCantSeePlayer(mgr);
    UpdateHeldPosition(mgr, dt);
    break;
  case kStateMsg_Deactivate:
    CPatterned::PathFind(mgr, msg, dt);
    x32c_animState = kAS_NotReady;
    x840_jumpPoint = kInvalidUniqueId;
    x30c_behaviourOrient = kBO_Constant;
    x639_27_inRange = false;
    BodyCtrl()->CommandMgr().SetSteeringBlendMode(kSBM_Normal);
    break;
  }
}

bool CSpacePirate::InRange(CStateManager& mgr, float arg) { return x639_27_inRange; }

void CSpacePirate::Shuffle(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->CommandMgr().SetSteeringBlendMode(kSBM_Normal);
    if (!x634_28_noShuffleCloseCheck && TooClose(mgr, 0.f)) {
      SetDestPos(GetTranslation() +
                 x2fc_minAttackRange *
                     (GetTranslation() - mgr.GetPlayer()->GetTranslation()).AsNormalized() +
                 Random2f(mgr, 0.f, 5.f));
      x2dc_destObj = kInvalidUniqueId;
      x30c_behaviourOrient = kBO_Constant;
      x636_30_shuffleClose = true;
    } else {
      CVector3f fromPlayer = GetTranslation() - mgr.GetPlayer()->GetTranslation();
      CVector3f side = CVector3f::Cross(CVector3f::Up(), fromPlayer);
      float range = x300_maxAttackRange;
      float distance = range * mgr.Random()->Float() + range;
      float sideDistance = 2.f * x300_maxAttackRange * (mgr.Random()->Float() - 0.5f);
      SetDestPos(mgr.GetPlayer()->GetTranslation() + distance * fromPlayer.AsNormalized() +
                 sideDistance * side.AsNormalized());
      x2dc_destObj = kInvalidUniqueId;
      x30c_behaviourOrient = kBO_MoveDir;
      x636_30_shuffleClose = false;
    }
    x644_steeringSpeed = 1.f;
    break;
  }
  CPatterned::PathFind(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Update:
    AvoidActors(mgr);
    break;
  case kStateMsg_Deactivate:
    x636_30_shuffleClose = false;
    break;
  }
}

void CSpacePirate::TurnAround(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    SetDestPos(GetTargetPos(mgr));
    CVector3f delta = x2e0_destPos - GetTranslation();
    delta.SetZ(0.f);
    if (CVector3f::Dot(GetTransform().GetForward(), delta.AsNormalized()) < 0.8f) {
      x32c_animState = kAS_Ready;
    }
    break;
  }
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Turn, &CPatterned::TryTurn, 0);
    UpdateCantSeePlayer(mgr);
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    break;
  }
}

bool CSpacePirate::ShouldDodge(CStateManager& mgr, float arg) {
  bool result = false;
  if (x638_28_enableDodge) {
    if (!x634_26_nonAggressive && !x638_29_noPlayerDodge) {
      CVector3f toTarget = GetTargetPos(mgr) - GetTranslation();
      if (CVector3f::Dot(toTarget, GetTransform().GetForward()) > 0.f &&
          (x850_timeSinceHitByPlayer < 0.33f || x854_lowHealthFrenzyTimer < 0.33f) &&
          x7ac_timeNoPlayerLos < 0.5f) {
        result = true;
      }
    }
    if (!result) {
      if (const CMetroid* metroid = PATTERNED_CAST_TO(CMetroid, const_cast< CEntity* >(mgr.GetObjectById(x7c0_targetId)))) {
        if (metroid->IsAttacking()) {
          CVector3f delta = GetTranslation() - metroid->GetTranslation();
          if (CVector3f::Dot(delta, metroid->GetTransform().GetForward()) > 0.f) {
            result = true;
          }
        }
      }
    }
  }
  return result;
}

void CPatterned::TryRollingDodge(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = BodyCtrl()->CommandMgr();
  cmdMgr.DeliverCmd(CBCStepCmd(static_cast< pas::EStepDirection >(arg), pas::kStep_RollDodge));
}

void CSpacePirate::Dodge(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x639_29_enableBreakDodge = false;
    if (!x63a_24_normalDodge && !x635_29_noBreakDodge && x8c0_dodgeDelayTimer <= 0.f) {
      float chance =
          0.15f * (1.f + (4.f * (x750_initialHP - HealthInfo(mgr)->GetHP())) / x750_initialHP);
      if (mgr.Random()->Float() < chance) {
        x639_29_enableBreakDodge = true;
      }
      float delayMin = x568_pirateData.xb8_dodgeDelayTimeMin;
      float delayMax = x568_pirateData.xbc_dodgeDelayTimeMax;
      x8c0_dodgeDelayTimer = mgr.Random()->Range(delayMin, delayMax);
    }
    x844_dodgeDir =
        GetStrafeDir(mgr, x639_29_enableBreakDodge ? x84c_breakDodgeDist : x848_dodgeDist);
    if (x844_dodgeDir != pas::kSD_Invalid) {
      x32c_animState = kAS_Ready;
    }
    break;
  case kStateMsg_Update:
    if (!x639_29_enableBreakDodge) {
      if (x63a_24_normalDodge || mgr.Random()->Float() < 0.5f) {
        TryCommand(mgr, pas::kAS_Step, &CPatterned::TryDodge, x844_dodgeDir);
      } else {
        TryCommand(mgr, pas::kAS_Step, &CPatterned::TryRollingDodge, x844_dodgeDir);
      }
    } else {
      TryCommand(mgr, pas::kAS_Step, &CPatterned::TryBreakDodge, x844_dodgeDir);
      if (GetMaterialList().HasMaterial(kMT_Orbit) && x330_stateMachineState.GetTime() > 0.5f) {
        RemoveMaterial(kMT_Orbit, mgr);
        mgr.Player()->TryToBreakOrbit(GetUniqueId(), CPlayer::kOB_ActivateOrbitSource, mgr);
      }
    }
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    x638_29_noPlayerDodge = true;
    if (!GetMaterialList().HasMaterial(kMT_Orbit)) {
      AddMaterial(kMT_Orbit, mgr);
    }
    break;
  }
}

bool CSpacePirate::ShouldRetreat(CStateManager& mgr, float arg) {
  bool result = false;
  if (x636_29_enableRetreat) {
    TUniqueId wpId = GetConnectedObject(mgr, kSS_Patrol, kSM_Follow);
    const CScriptWaypoint* wp = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(wpId));
    if (!wp) {
      wpId = GetConnectedObject(mgr, kSS_Retreat, kSM_Follow);
      wp = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(wpId));
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
    result = true;
    ReleaseCoverPoint(mgr, x640_coverPoint);
    x636_25_hearNoise = false;
    x637_25_enableAim = false;
    x400_24_hitByPlayerProjectile = false;
  }
  return result;
}

bool CSpacePirate::ShouldMove(CStateManager& mgr, float arg) {
  CScriptCoverPoint* cp = GetCoverPoint(mgr, x640_coverPoint);
  return cp && !cp->ShouldStay();
}

bool CSpacePirate::ShotAt(CStateManager& mgr, float arg) {
  return x854_lowHealthFrenzyTimer < (arg ? arg : 0.5f);
}

bool CSpacePirate::Attacked(CStateManager& mgr, float arg) {
  return x850_timeSinceHitByPlayer < (arg ? arg : 0.5f);
}

bool CSpacePirate::HasTargetingPoint(CStateManager& mgr, float arg) {
  bool result = true;
  CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(x7c0_targetId));
  if (x7c0_targetId == mgr.GetPlayer()->GetUniqueId() || !actor || !actor->GetActive()) {
    result = false;
    x764_boneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    x7c0_targetId = mgr.GetPlayer()->GetUniqueId();
    float scale = 1.f;
    float margin = x568_pirateData.x8_SearchRadius * scale;
    CVector3f extent(margin, margin, margin);
    CAABox bounds(GetTranslation() - extent, GetTranslation() + extent);
    TEntityList nearList;
    mgr.BuildNearList(nearList, bounds, CMaterialFilter::MakeExclude(CMaterialList(kMT_Solid)),
                      nullptr);
    for (int i = 0; i < nearList.size(); ++i) {
      const CScriptTargetingPoint* const point =
          TCastToConstPtr< CScriptTargetingPoint >(mgr.GetObjectById(nearList[i]));
      if (point) {
        if (point->GetActive() && point->GetCurrentAreaId() == GetCurrentAreaId() &&
            !point->GetLocked()) {
          result = true;
          x764_boneTracking.SetTarget(point->GetUniqueId());
          x7c0_targetId = point->GetUniqueId();
          break;
        }
      }
    }
  }
  return result;
}

void CSpacePirate::TargetCover(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (CScriptCoverPoint* cp = GetCoverPoint(mgr, x640_coverPoint)) {
      x2dc_destObj = x640_coverPoint;
      x2e0_destPos = cp->GetTranslation();
    }
    x2ec_reflectedDestPos = GetTranslation();
    x328_24_inPosition = false;
    break;
  }
}

bool CSpacePirate::ShouldWallHang(CStateManager& mgr, float arg) {
  CScriptCoverPoint* cp = GetCoverPoint(mgr, x640_coverPoint);
  return cp && cp->ShouldWallHang();
}

void CSpacePirate::WallHang(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    x637_29_inWallHang = true;
    if (CScriptCoverPoint* cp = GetCoverPoint(mgr, x640_coverPoint)) {
      const rstl::vector< SConnection >& connections = cp->GetConnectionList();
      for (AUTO(it, connections.begin()); it != connections.end(); ++it) {
        if (it->x0_state == kSS_Arrived && it->x4_msg == kSM_Next) {
          TUniqueId id = mgr.GetIdForScript(it->x8_objId);
          if (CScriptWaypoint* wp = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(id))) {
            x2dc_destObj = id;
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
  case kStateMsg_Update: {
    TryCommand(mgr, pas::kAS_WallHang, &CPatterned::TryWallHang, 0);
    CVector3f toPlayer = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    toPlayer.SetZ(0.f);
    BodyCtrl()->CommandMgr().DeliverTargetVector(toPlayer);
    x7c4_burstFire.SetBurstType(1);
    break;
  }
  case kStateMsg_Deactivate:
    x637_29_inWallHang = false;
    x32c_animState = kAS_NotReady;
    x636_31_inAttackState = false;
    break;
  }
}

void CSpacePirate::WallDetach(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x637_29_inWallHang = true;
    break;
  case kStateMsg_Update:
    BodyCtrl()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    break;
  case kStateMsg_Deactivate:
    x637_29_inWallHang = false;
    break;
  }
}

bool CSpacePirate::AnimOver(CStateManager& mgr, float arg) {
  if (x637_29_inWallHang) {
    return BodyCtrl()->GetCurrentStateId() != pas::kAS_WallHang;
  }
  return CPatterned::AnimOver(mgr, arg);
}

bool CSpacePirate::ShouldJumpBack(CStateManager& mgr, float arg) {
  return !x634_28_noShuffleCloseCheck || x8d8_holdPositionTime > 6.f;
}

void CSpacePirate::JumpBack(CStateManager& mgr, EStateMsg msg, float dt) {
  if (!ShouldJumpBack(mgr, dt)) {
    return;
  }
  switch (msg) {
  case kStateMsg_Activate:
    if (!x634_29_onlyAttackInRange && !CantJumpBack(mgr, -GetTransform().GetForward(), 5.f)) {
      float height = GetSearchPath()->GetCharacterHeight();
      x660_pathFindSearch.SetCharacterHeight(5.f + height);
      CVector3f dest = GetTranslation() + 10.f * GetTransform().GetForward();
      if (GetSearchPath()->Search(GetTranslation(), dest) == CPathFindSearch::kR_Success &&
          (GetSearchPath()->GetWaypoints().back() - dest).MagSquared() < 3.f) {
        if (CMath::AbsF(GetSearchPath()->RemainingPathDistance(GetTranslation()) - 10.f) < 4.f) {
          x828_patrolDestPos = GetSearchPath()->GetWaypoints().back();
          x824_jumpHeight = 5.f;
          x639_25_useJumpBackJump = true;
          x32c_animState = kAS_Ready;
        }
      }
      GetSearchPath()->SetCharacterHeight(height);
    }
    break;
  case kStateMsg_Update:
    if (!x639_25_useJumpBackJump) {
      BodyCtrl()->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Backward, pas::kStep_Normal));
      BodyCtrl()->CommandMgr().DeliverTargetVector(GetTargetPos(mgr) - GetTranslation());
    } else {
      TryCommand(mgr, pas::kAS_Jump, &CPatterned::TryJump, 0);
    }
    break;
  case kStateMsg_Deactivate:
    if (x639_25_useJumpBackJump) {
      x32c_animState = kAS_NotReady;
      x639_25_useJumpBackJump = false;
    }
    x8d8_holdPositionTime = 0.f;
    break;
  }
}

bool CSpacePirate::ShouldSpecialAttack(CStateManager& mgr, float arg) {
  if (x634_29_onlyAttackInRange && !x7c4_burstFire.IsBurstSet() && x7bc_attackRemTime > 2.f) {
    return true;
  }
  return false;
}

bool CSpacePirate::LostInterest(CStateManager& mgr, float arg) {
  if (x634_29_onlyAttackInRange && x7bc_attackRemTime < 1.5f) {
    return true;
  }
  return false;
}

void CSpacePirate::SpecialAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    x648_targetDelta = mgr.GetPlayer()->GetAimPosition(mgr, 0.f) - GetGunEyePos();
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_ProjectileAttack, &CPatterned::TryProjectileAttack, pas::kS_One);
    if (x32c_animState == kAS_Ready) {
      BodyCtrl()->CommandMgr().DeliverTargetVector(x648_targetDelta);
    }
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    break;
  }
}

bool CSpacePirate::StartAttack(CStateManager& mgr, float arg) {
  if (x638_31_mayStartAttack) {
    x638_31_mayStartAttack = false;
    return true;
  }
  return false;
}

bool CSpacePirate::BreakAttack(CStateManager& mgr, float arg) { return x635_25_breakAttack; }

bool CSpacePirate::BounceFind(CStateManager& mgr, float arg) {
  bool result = false;
  float minDistSq = FLT_MAX;
  CScriptAiJumpPoint* best = nullptr;
  CObjectList& list = mgr.ObjectListById(kOL_AiWaypoint);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (CScriptAiJumpPoint* jp = TCastToPtr< CScriptAiJumpPoint >(list[i])) {
      if (jp->GetActive() && !jp->GetInUse(GetUniqueId()) &&
          jp->GetJumpTarget() != kInvalidUniqueId && jp->GetCurrentAreaId() == GetCurrentAreaId()) {
        CVector3f toJump = jp->GetTranslation() - GetTranslation();
        float distSq = toJump.MagSquared();
        if (distSq < minDistSq && CVector3f::Dot(jp->GetTransform().GetForward(), toJump) > 0.f) {
          if (const CScriptWaypoint* wp =
                  TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(jp->GetJumpTarget()))) {
            CVector3f toDest = x2e0_destPos - wp->GetTranslation();
            distSq += toDest.MagSquared() + 9.f * toDest.GetZ() * toDest.GetZ();
            if (distSq < minDistSq &&
                CVector3f::Dot(wp->GetTransform().GetForward(), toDest) > 0.f &&
                GetSearchPath()->PathExists(GetTranslation(), jp->GetTranslation()) ==
                    CPathFindSearch::kR_Success) {
              bool good = false;
              bool noPath = GetSearchPath()->PathExists(wp->GetTranslation(), x2e0_destPos) !=
                            CPathFindSearch::kR_Success;
              if (noPath) {
                distSq += 1000.f;
              }
              if (!noPath) {
                good = true;
              }
              if (distSq < minDistSq) {
                minDistSq = distSq;
                best = jp;
                if (good) {
                  break;
                }
              }
            }
          }
        }
      }
    }
  }
  if (best) {
    if (const CScriptWaypoint* wp =
            TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(best->GetJumpPoint()))) {
      SetDestPos(best->GetTranslation());
      result = true;
      x840_jumpPoint = best->GetUniqueId();
      x824_jumpHeight = best->GetJumpApex();
      x828_patrolDestPos = wp->GetTranslation();
    }
  }
  return result;
}

void CSpacePirate::PathFindEx(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::PathFind(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Activate:
    x639_27_inRange = false;
    x30c_behaviourOrient = kBO_MoveDir;
    break;
  case kStateMsg_Update:
    AvoidActors(mgr);
    if (!x639_27_inRange) {
      if (const CScriptAiJumpPoint* jp =
              TCastToConstPtr< CScriptAiJumpPoint >(mgr.GetObjectById(x840_jumpPoint))) {
        float maxSpeed = BodyCtrl()->GetBodyStateInfo().GetMaxSpeed();
        const CVector3f& scale = CVector3f(GetModelData()->GetScale());
        x754_coverRange = maxSpeed * ((1.5f * dt + 0.1f) * scale.GetY()) + x7a4_intoJumpDist;
        x639_27_inRange = (GetTranslation() - jp->GetTranslation()).MagSquared() <
                          x754_coverRange * x754_coverRange;
      }
    }
    break;
  case kStateMsg_Deactivate:
    x639_27_inRange = false;
    break;
  }
}

void CSpacePirate::Bounce(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (const CScriptAiJumpPoint* jp =
            TCastToConstPtr< CScriptAiJumpPoint >(mgr.GetObjectById(x840_jumpPoint))) {
      TUniqueId target = jp->GetJumpTarget();
      if (const CScriptWaypoint* wp =
              TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(target))) {
        CBodyStateCmdMgr& cmdMgr = BodyCtrl()->CommandMgr();
        cmdMgr.DeliverCmd(CBCJumpCmd(x828_patrolDestPos, wp->GetTranslation(), pas::kJT_Normal));
      }
    }
    break;
  case kStateMsg_Update:
    if (x330_stateMachineState.GetTime() > 0.1f &&
        BodyCtrl()->GetCurrentStateId() != pas::kAS_Jump) {
      x330_stateMachineState.SetCodeTrigger();
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CSpacePirate::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::Dead(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Activate:
    x764_boneTracking.SetActive(false);
    SetEyeParticleActive(mgr, false);
    SquadReset(mgr);
    break;
  case kStateMsg_Update:
    if (BodyCtrl()->GetCurrentStateId() == pas::kAS_Death) {
      RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
      RemoveMaterial(kMT_GroundCollider, kMT_Solid, kMT_AIBlock, mgr);
      AddMaterial(kMT_ProjectilePassthrough, mgr);
      SetMomentumWR(CVector3f::Zero());
      CPhysicsActor::Stop();
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CSpacePirate::Deactivate(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    x401_30_pendingDeath = true;
  }
}

bool CSpacePirate::OffLine(CStateManager& mgr, float arg) { return !IsOnGround(); }

bool CSpacePirate::Landed(CStateManager& mgr, float arg) { return IsOnGround(); }

void CPatterned::TryJumpInLoop(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = x450_bodyController->CommandMgr();
  CBCJumpCmd cmd(x2e0_destPos, static_cast< pas::EJumpType >(arg), true);
  cmdMgr.DeliverCmd(cmd);
}

void CSpacePirate::Jump(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    x828_patrolDestPos = GetTranslation() + CVector3f::Down();
    x824_jumpHeight = 0.f;
    x8dc_leashTimer = 0.f;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Jump, &CPatterned::TryJumpInLoop, 0);
    UpdateLeashTimer(dt);
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    break;
  }
}

bool CSpacePirate::Leash(CStateManager& mgr, float arg) { return x8dc_leashTimer > arg; }

void CSpacePirate::UpdateLeashTimer(float dt) {
  if (!BodyCtrl()->IsFrozen() && !BodyCtrl()->IsElectrocuting()) {
    x8dc_leashTimer += dt;
  }
}

bool CSpacePirate::FireProjectile(float dt, CStateManager& mgr) {
  bool result = false;
  CTransform4f gunXf = GetLctrTransform(x7b6_gunSeg);
  if (!x400_25_alive) {
    LaunchProjectile(gunXf, mgr, 6, CWeapon::kPA_None, false,
                     rstl::optional_object< TLockedToken< CGenDescription > >(),
                     CSfxManager::kInternalInvalidSfxId, false, CVector3f(1.f, 1.f, 1.f));
    result = true;
  } else {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(x7c0_targetId))) {
      CVector3f pos = actor->GetTranslation();
      if (x7c0_targetId == mgr.GetPlayer()->GetUniqueId()) {
        pos = ProjectileInfo()->PredictInterceptPos(gunXf.GetTranslation(),
                                                    mgr.GetPlayer()->GetAimPosition(mgr, 0.f),
                                                    *mgr.GetPlayer(), true, dt);
      }
      CVector3f gunToPos = pos - gunXf.GetTranslation();
      float distance = gunToPos.Magnitude();
      gunToPos /= distance;
      float dot = CVector3f::Dot((GetLctrTransform(x7b8_wristSeg).GetTranslation() -
                                  GetLctrTransform(x7b7_elbowSeg).GetTranslation())
                                     .AsNormalized(),
                                 gunToPos);
      if ((dot > 0.707f || (distance < 6.f && dot > 0.5f)) &&
          LineOfSightTest(mgr, gunXf.GetTranslation(), pos,
                          CMaterialList(kMT_Player, kMT_ProjectilePassthrough))) {
        pos += GetTransform().Rotate(x7c4_burstFire.GetDistanceCompensatedError(distance, 6.f));
        CTransform4f shotXf = CTransform4f::LookAt(gunXf.GetTranslation(), pos);
        LaunchProjectile(shotXf, mgr, 6, CWeapon::kPA_None, false,
                         rstl::optional_object< TLockedToken< CGenDescription > >(),
                         CSfxManager::kInternalInvalidSfxId, false, CVector3f(1.f, 1.f, 1.f));
        result = true;
      }
    }
  }
  if (result) {
    const CPASDatabase& database = BodyCtrl()->GetPASDatabase();
    const CPASAnimParmData parms(pas::kAS_AdditiveReaction, CPASAnimParm::FromEnum(2));
    const rstl::pair< float, int > best = database.FindBestAnimation(parms, *mgr.Random(), -1);
    if (best.first > 0.f) {
      ModelData()->AnimationData()->AddAdditiveAnimation(best.second, 1.f, false, true);
    }
    CSfxManager::AddEmitter(x568_pirateData.x48_Sound_Projectile, GetTranslation(),
                            CVector3f::Zero(), true, false);
  }
  const bool fired = result;
  return fired;
}

void CSpacePirate::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                   EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case kUE_BeginAction:
    RemoveMaterial(kMT_Solid, mgr);
    x638_30_allEnergyDrained = true;
    handled = true;
    break;
  case kUE_EndAction:
    x639_30_closeMelee = false;
    handled = true;
    break;
  case kUE_DeGenerate:
  case kUE_BecomeRagDoll:
    if (x634_29_onlyAttackInRange || HealthInfo(mgr)->GetHP() <= 0.f) {
      x858_ragdollDelayTimer = mgr.Random()->Float() * 0.05f + 0.001f;
    }
    handled = true;
    break;
  case kUE_IkLock:
    if (!x860_ikChain.GetActive()) {
      const CSegId& bone =
          GetModelData()->GetAnimationData()->GetLocatorSegId(node.GetLocatorName());
      if (bone != CSegId(3)) {
        CTransform4f xf = GetLctrTransform(bone);
        x860_ikChain.Activate(*GetModelData()->GetAnimationData(), bone, xf);
        x639_28_satUp = true;
      }
    }
    handled = true;
    break;
  case kUE_IkRelease:
    x860_ikChain.Deactivate();
    handled = true;
    break;
  case kUE_ScreenShake:
    SendScriptMsgs(kSS_Play, mgr, kSM_None);
    handled = true;
    break;
  case kUE_FadeOut:
    if (x635_27_shadowPirate) {
      x3e8_alphaDelta = -0.8f;
      mgr.ActorModelParticles()->StartElectric(*this);
      x8ac_electricParticleTimer = 1.f;
    }
    handled = true;
    break;
  default:
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

uchar CSpacePirate::GetModelAlphau8(const CStateManager& mgr) const {
  uchar alpha = 255;
  if ((mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::kPV_XRay &&
       mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::kPV_Thermal) ||
      !IsAlive()) {
    if (!x635_27_shadowPirate) {
      alpha = x42c_color.GetAlphau8();
    } else {
      alpha = static_cast< uchar >(x8b4_shadowPirateAlpha * 255.f);
    }
  }
  const uchar result = alpha;
  return result;
}

void CSpacePirate::CalculateRenderBounds() {
  if (x85c_ragDoll.get() && x85c_ragDoll->IsPrimed()) {
    CAABox bounds = x85c_ragDoll->CalculateRenderBounds();
    CVector3f margin = GetModelData()->GetScale() * 0.2f;
    SetRenderBounds(CAABox(bounds.GetMinPoint() - margin, bounds.GetMaxPoint() + margin));
  } else {
    CActor::CalculateRenderBounds();
  }
}

void CSpacePirate::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  if (x85c_ragDoll.get() && x85c_ragDoll->IsPrimed()) {
    x85c_ragDoll->PreRender(GetTranslation(), *ModelData());
  }
  CPatterned::PreRender(mgr, frustum);
  if (!x85c_ragDoll.get() || !x85c_ragDoll->IsPrimed()) {
    x764_boneTracking.PreRender(mgr, *ModelData()->AnimationData(), GetTransform(),
                                CVector3f(ModelData()->GetScale()), *BodyCtrl());
    x860_ikChain.PreRender(*ModelData()->AnimationData(), GetTransform(),
                           CVector3f(ModelData()->GetScale()));
  }
}

void CSpacePirate::Render(const CStateManager& mgr) const {
  float time = IsAlive() ? CGraphics::GetSecondsMod900() : 0.f;
  CTimeProvider provider(time);
  gpRender->SetGXRegister1Color(x8cc_trooperColor);
  CPatterned::Render(mgr);
}

CAABox CSpacePirate::GetSortingBounds(const CStateManager& mgr) const {
  CAABox bounds = GetModelData()->GetBounds(GetTransform());
  CVector3f center = bounds.GetCenterPoint();
  CVector3f radius = (bounds.GetMaxPoint() - bounds.GetMinPoint()) * 0.25f;
  return CAABox(center - radius, center + radius);
}

bool CSpacePirate::ShouldFrenzy(CStateManager& mgr) {
  bool reset = false;
  if (x638_24_pendingFrenzyChance) {
    x638_24_pendingFrenzyChance = false;
    if (mgr.Random()->Next() % 100 < 25) {
      reset = true;
    }
  }
  if (!mChargePlayerList.empty()) {
    reset = true;
  }
  if (mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
    reset = true;
  }
  if (HealthInfo(mgr)->GetHP() < 0.3f * x750_initialHP && mgr.Random()->Next() % 100 < 60 &&
      x854_lowHealthFrenzyTimer < 0.5f) {
    reset = true;
  }
  if (reset) {
    x63c_frenzyFrames = mgr.Random()->Range(2, 4);
  }
  return --x63c_frenzyFrames >= 0;
}

void CSpacePirate::UpdateCloak(float dt, CStateManager& mgr) {
  if (x635_27_shadowPirate) {
    if (x400_25_alive) {
      if (x8a8_cloakDelayTimer > 0.f) {
        x8a8_cloakDelayTimer -= dt;
        if (x8a8_cloakDelayTimer <= 0.f) {
          x3e8_alphaDelta = -0.4f;
        }
      }
    } else {
      x8b8_minCloakAlpha = 0.f;
      x8bc_maxCloakAlpha = 1.f;
    }
    if (x8ac_electricParticleTimer > 0.f) {
      x8ac_electricParticleTimer -= dt;
      if (x8ac_electricParticleTimer <= 0.f && !BodyCtrl()->IsElectrocuting()) {
        mgr.ActorModelParticles()->StopElectric(*this);
      }
    }
    if (BodyCtrl()->IsFrozen()) {
      x3e8_alphaDelta = 2.f;
    }
    if (x3e8_alphaDelta < 0.f && x42c_color.GetAlpha() < x8b8_minCloakAlpha) {
      x42c_color.SetAlpha(x8b8_minCloakAlpha);
      x3e8_alphaDelta = 0.f;
      RemoveMaterial(kMT_Target, mgr);
    }
    if (x3e8_alphaDelta > 0.f && x42c_color.GetAlpha() > x8bc_maxCloakAlpha) {
      x42c_color.SetAlpha(x8bc_maxCloakAlpha);
      AddMaterial(kMT_Target, mgr);
    }
    x8b0_cloakStepTime -= dt;
    if (x8b0_cloakStepTime < 0.f) {
      float random = mgr.Random()->Float();
      x8b0_cloakStepTime = (1.f - random) * 0.08f;
      if (x3e8_alphaDelta < 0.f) {
        x8b4_shadowPirateAlpha = x42c_color.GetAlpha();
        if (x400_25_alive) {
          x8b4_shadowPirateAlpha -= random * (x42c_color.GetAlpha() - x8b8_minCloakAlpha);
        }
      } else if (x3e8_alphaDelta > 0.f) {
        x8b4_shadowPirateAlpha =
            x42c_color.GetAlpha() + random * (x8bc_maxCloakAlpha - x42c_color.GetAlpha());
      } else {
        x8b4_shadowPirateAlpha = x42c_color.GetAlpha();
      }
    }
  }
}

void CSpacePirate::UpdateAttacks(float dt, CStateManager& mgr) {
  bool reset = true;
  if ((!x400_25_alive || (BodyCtrl()->GetBodyStateInfo().GetCurrentState()->CanShoot() &&
                          x637_25_enableAim && !x634_27_melee && !x634_25_ceilingAmbush &&
                          !x639_26_started && !BodyCtrl()->IsElectrocuting())) &&
      x7c4_burstFire.GetBurstType() != -1) {
    if (x400_25_alive) {
      if (!x634_29_onlyAttackInRange ||
          (mgr.GetPlayer()->GetTranslation() - GetTranslation()).MagSquared() <
              x3c8_leashRadius * x3c8_leashRadius) {
        reset = false;
        x7bc_attackRemTime -= dt;
        if (x7bc_attackRemTime < 0.f) {
          const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x8c8_teamAiMgrId, GetUniqueId());
          if (!role || role->GetTeamAiRole() == CTeamAiRole::kTAR_Projectile) {
            if (x8c8_teamAiMgrId == kInvalidUniqueId ||
                CTeamAiMgr::AddAttacker(kAT_Projectile, mgr, x8c8_teamAiMgrId, GetUniqueId())) {
              if (ShouldFrenzy(mgr)) {
                x7c4_burstFire.SetBurstType(2);
              }
              if (x635_26_seated) {
                x7c4_burstFire.SetBurstType(5);
              }
              if (!PlayerSpot(mgr, 0.f) && x7c4_burstFire.GetBurstType() < 6) {
                x7c4_burstFire.SetBurstType(x7c4_burstFire.GetBurstType() + 6);
              }
              x7c4_burstFire.Start(mgr);
              x7bc_attackRemTime =
                  mgr.Random()->Float() * x308_attackTimeVariation + x304_averageAttackTime;
              const CVector3f& fromPlayer =
                  (GetGunEyePos() - mgr.GetPlayer()->GetAimPosition(mgr, 0.f)).AsNormalized();
              const CVector3f& forward = mgr.GetPlayer()->GetTransform().GetForward();
              if (CVector3f::Dot(fromPlayer, forward) < 0.9f) {
                const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
                for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
                  if (CSpacePirate* pirate = PATTERNED_CAST_TO(CSpacePirate, const_cast< CEntity* >(list[i]))) {
                    if (pirate != this && pirate->x637_25_enableAim &&
                        pirate->GetCurrentAreaId() == GetCurrentAreaId()) {
                      x7bc_attackRemTime += 0.2f;
                    }
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
      if (mgr.GetPlayer()->IsSidewaysDashing() && mgr.Random()->Float() < 0.5f) {
        x7c4_burstFire.SetAvoidAccuracy(true);
      }
      FireProjectile(dt, mgr);
      x7c4_burstFire.SetAvoidAccuracy(false);
      float variation = x568_pirateData.xa0_nextShotTimeVariation;
      float average = x568_pirateData.x9c_averageNextShotTime;
      x7c4_burstFire.SetTimeToNextShot(variation * (mgr.Random()->Float() - 0.5f) + average);
    } else if (!x7c4_burstFire.IsBurstSet()) {
      reset = true;
    }
  }
  if (reset) {
    SquadReset(mgr);
  }
  SetTargetable(CheckTargetable(mgr));
}

void CSpacePirate::UpdateAimBodyState(float dt, CStateManager& mgr) {
  if (x400_25_alive && x637_25_enableAim && !x637_29_inWallHang && !BodyCtrl()->IsFrozen() &&
      !x634_27_melee && !x85c_ragDoll.get() && (!x635_26_seated || x639_28_satUp) &&
      !BodyCtrl()->IsElectrocuting()) {
    x8c4_aimDelayTimer = CMath::Max(0.f, x8c4_aimDelayTimer - dt);
    if (!x8c4_aimDelayTimer) {
      BodyCtrl()->CommandMgr().DeliverCmd(CBCAdditiveAimCmd());
      CVector3f direction = GetTransform().TransposeMultiply(GetTargetPos(mgr));
      BodyCtrl()->CommandMgr().DeliverAdditiveTargetVector(direction);
    }
  } else if (x637_25_enableAim && !x634_27_melee) {
    BodyCtrl()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_AdditiveIdle));
  }
}

void CSpacePirate::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  if (!BodyCtrl()->GetIsActive()) {
    BodyCtrl()->Activate(mgr);
  }
  bool inCineCam = mgr.GetCameraManager()->IsInCinematicCamera();
  if (inCineCam && !x637_31_prevInCineCam) {
    SetCinematicCollision(mgr);
  } else if (!inCineCam && x637_31_prevInCineCam && !x635_31_ragdollNoAiCollision) {
    SetNonCinematicCollision(mgr);
  }
  x637_31_prevInCineCam = inCineCam;
  float steeringSpeed = x748_steeringDelayTimer ? 0.f : x644_steeringSpeed;
  BodyCtrl()->CommandMgr().SetSteeringSpeedRange(steeringSpeed, steeringSpeed);
  x744_unkTimer = CMath::Max(0.f, x744_unkTimer - dt);
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
  if (!BodyCtrl()->IsFrozen()) {
    if (x400_25_alive) {
      x748_steeringDelayTimer = CMath::Max(0.f, x748_steeringDelayTimer - dt);
      if (x637_28_noPlayerLos) {
        x7ac_timeNoPlayerLos += dt;
      } else {
        x7ac_timeNoPlayerLos = 0.f;
      }
      x838_strafeDelayTimer = CMath::Max(0.f, x838_strafeDelayTimer - dt);
      x8c0_dodgeDelayTimer = CMath::Max(0.f, x8c0_dodgeDelayTimer - dt);
      CheckForProjectiles(mgr);
    }
    UpdateAttacks(dt, mgr);
    UpdateAimBodyState(dt, mgr);
    x860_ikChain.Update(dt);
  }
  if (x634_24_pendingAmbush) {
    x634_24_pendingAmbush = false;
    if (x634_25_ceilingAmbush) {
      BodyCtrl()->SetLocomotionType(pas::kLT_Internal6);
    } else {
      BodyCtrl()->SetLocomotionType(pas::kLT_Crouch);
    }
    x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), rstl::string_l("Ambushing"));
  }
  bool noRagDoll = x85c_ragDoll.null();
  if (noRagDoll || !x85c_ragDoll->IsPrimed()) {
    CPatterned::Think(dt, mgr);
    if (!BodyCtrl()->IsFrozen()) {
      x764_boneTracking.Update(dt);
    }
  } else {
    UpdateAlphaDelta(dt, mgr);
    UpdateDamageColor(dt);
    if (CSfxHandle handle = GetSfxHandle()) {
      CSfxManager::UpdateEmitter(handle, GetTranslation(), CVector3f::Zero(), 127);
    }
  }
  if (!noRagDoll) {
    if (!x85c_ragDoll->IsPrimed()) {
      x85c_ragDoll->Prime(mgr, GetTransform(), *ModelData());
      CVector3f translation = GetTranslation();
      SetTransform(CTransform4f::Identity());
      SetTranslation(translation);
      BodyCtrl()->SetPlaybackRate(0.f);
    } else {
      float waterTop = -FLT_MAX / 2.f;
      if (InFluidId() != kInvalidUniqueId) {
        if (const CScriptWater* water =
                TCastToConstPtr< CScriptWater >(mgr.GetObjectById(InFluidId()))) {
          if (water->GetActive()) {
            waterTop = water->GetTriggerBoundsWR().GetMaxPoint().GetZ();
          }
        }
      }
      x85c_ragDoll->Update(mgr, dt * CalcDyingThinkRate(), waterTop);
      ModelData()->AdvanceParticles(GetTransform(), dt, mgr);
    }
    if (x85c_ragDoll->IsOver() && !x85c_ragDoll->WillContinueSmallMovements() &&
        !GetFadeToDeath()) {
      SetFadeToDeath(true);
      x3e8_alphaDelta = -1.f / 3.f;
      AddMaterial(kMT_ProjectilePassthrough, mgr);
      x638_30_allEnergyDrained = true;
      SetMomentumWR(CVector3f::Zero());
      CPhysicsActor::Stop();
    }
  }
  if (x858_ragdollDelayTimer > 0.f) {
    x858_ragdollDelayTimer -= dt;
    if (x858_ragdollDelayTimer <= 0.f) {
      if (x85c_ragDoll.null()) {
        x85c_ragDoll = rs_new CPirateRagDoll(mgr, this, x568_pirateData.x98_Sound_Impact,
                                             (x635_30_floatingCorpse ? 3 : 0) |
                                                 (x635_31_ragdollNoAiCollision ? 4 : 0));
        RemoveMaterial(kMT_Orbit, kMT_Target, mgr);
      }
      x858_ragdollDelayTimer = 0.f;
    }
  }
}

const float CSpacePirate::skGravityConstant = 50.f;
const float CSpacePirate::skFloatingGravityConstant = -3.f;
