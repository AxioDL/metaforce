#include "MetroidPrime/Enemies/CFlyingPirate.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "MetroidPrime/Enemies/CFlyingPirateRagDoll.hpp"

#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetaRender/IRenderer.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CCameraShakeData.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/Enemies/CSpacePirate.hpp"
#include "MetroidPrime/Enemies/CTeamAiMgr.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "rstl/math.hpp"
#include <float.h>

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"

const SBurst CFlyingPirate::skBurstsFlying[] = {
    {10, {3, 4, 11, 12, -1, 0, 0, 0}, 0.1f, 0.05f},
    {20, {2, 3, 4, 5, -1, 0, 0, 0}, 0.1f, 0.05f},
    {20, {10, 11, 12, 13, -1, 0, 0, 0}, 0.1f, 0.05f},
    {25, {15, 16, 1, 2, -1, 0, 0, 0}, 0.1f, 0.05f},
    {25, {5, 6, 7, 8, -1, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CFlyingPirate::skBurstsFlyingOutOfView[] = {
    {5, {3, 4, 11, 12, -1, 0, 0, 0}, 0.1f, 0.05f},
    {10, {2, 3, 4, 5, -1, 0, 0, 0}, 0.1f, 0.05f},
    {10, {10, 11, 12, 13, -1, 0, 0, 0}, 0.1f, 0.05f},
    {40, {15, 16, 1, 2, -1, 0, 0, 0}, 0.1f, 0.05f},
    {35, {5, 6, 7, 8, -1, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CFlyingPirate::skBurstsLanded[] = {
    {30, {3, 4, 5, 11, 12, 4, -1, 0}, 0.1f, 0.05f},  {20, {2, 3, 4, 5, 4, 3, -1, 0}, 0.1f, 0.05f},
    {20, {5, 4, 3, 13, 12, 11, -1, 0}, 0.1f, 0.05f}, {30, {1, 2, 3, 4, 5, 6, -1, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CFlyingPirate::skBurstsLandedOutOfView[] = {
    {10, {6, 5, 4, 14, 13, 12, -1, 0}, 0.1f, 0.05f},
    {20, {14, 13, 12, 11, 10, 9, -1, 0}, 0.1f, 0.05f},
    {20, {14, 15, 16, 11, 10, 9, -1, 0}, 0.1f, 0.05f},
    {50, {11, 10, 9, 8, 7, 6, -1, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const uint CFlyingPirate::skNumProperties = 35;

const SBurst* CFlyingPirate::skBursts[] = {
    skBurstsFlying, skBurstsFlyingOutOfView, skBurstsLanded, skBurstsLandedOutOfView, nullptr,
};

rstl::string skJetPack = rstl::string_l("JetPack");
rstl::string skScubaGear = rstl::string_l("ScubaGear");
rstl::string skScubaBubbles = rstl::string_l("ScubaBubbles");
rstl::string skSparks = rstl::string_l("Sparks");
rstl::string skLandingSmoke = rstl::string_l("LandingSmoke");
rstl::string skEyes = rstl::string_l("Eyes");

static const rstl::string skParts[] = {
    rstl::string_l("Collar"),  rstl::string_l("Head_1"),  rstl::string_l("R_shoulder"),
    rstl::string_l("R_elbow"), rstl::string_l("R_wrist"), rstl::string_l("L_shoulder"),
    rstl::string_l("L_elbow"), rstl::string_l("L_wrist"), rstl::string_l("R_hip"),
    rstl::string_l("R_knee"),  rstl::string_l("R_ankle"), rstl::string_l("L_hip"),
    rstl::string_l("L_knee"),  rstl::string_l("L_ankle"), rstl::string_l("rocket_LCTR")};
static const float skRadii[] = {0.45f, 0.52f, 0.35f, 0.1f,  0.15f, 0.35f, 0.1f, 0.15f,
                                0.15f, 0.15f, 0.15f, 0.15f, 0.15f, 0.15f, 0.35f};

CFlyingPirateRagDoll::CFlyingPirateRagDoll(CStateManager& mgr, CFlyingPirate* actor,
                                           const ushort impactSfx, const ushort spinSfx)
: CRagDoll(-actor->GetGravityConstant(), -actor->GetFloatingGravityConstant(), 8.f, 0)
, x6c_actor(actor)
, x70_(0.f)
, x74_(CVector3f::Up())
, x80_(0.f)
, x84_(5.f)
, x88_impactSfx(impactSfx)
, x8c_impactTimer(0.f)
, x90_lastImpactPos(CVector3f::Zero())
, x9c_spinSfx(spinSfx)
, xa4_(actor->x2e0_destPos - actor->GetTranslation())
, xb0_24_firstImpact(true) {
  x6c_actor->RemoveMaterial(kMT_Solid, kMT_AIBlock, kMT_GroundCollider, mgr);
  x6c_actor->HealthInfo(mgr)->SetHP(-1.f);
  SetNumParticles(15);
  SetNumLengthConstraints(45);
  SetNumJointConstraints(4);
  const CVector3f& scale = CVector3f(actor->GetModelData()->GetScale());
  const CTransform4f& xf = actor->GetTransform();
  CAnimData* animData = actor->AnimationData();
  animData->BuildPose();
  CVector3f center = actor->GetBoundingBox().GetCenterPoint();
  for (int i = 0; i < 15; ++i) {
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
  AddLengthConstraint(14, 0);
  AddLengthConstraint(14, 2);
  AddLengthConstraint(14, 5);
  AddLengthConstraint(14, 8);
  AddLengthConstraint(14, 11);
  AddMinLengthConstraint(1, 8, x14_lengthConstraints[2].GetLength());
  AddMinLengthConstraint(1, 11, x14_lengthConstraints[3].GetLength());
  AddMinLengthConstraint(4, 2, x14_lengthConstraints[5].GetLength());
  AddMinLengthConstraint(7, 5, x14_lengthConstraints[7].GetLength());
  AddMinLengthConstraint(
      3, 5, 0.5f * x14_lengthConstraints[5].GetLength() + x14_lengthConstraints[9].GetLength());
  AddMinLengthConstraint(
      6, 2, 0.5f * x14_lengthConstraints[7].GetLength() + x14_lengthConstraints[9].GetLength());
  AddMinLengthConstraint(
      4, 5, 0.5f * x14_lengthConstraints[5].GetLength() + x14_lengthConstraints[9].GetLength());
  AddMinLengthConstraint(
      7, 2, 0.5f * x14_lengthConstraints[7].GetLength() + x14_lengthConstraints[9].GetLength());
  AddMinLengthConstraint(10, 8, x14_lengthConstraints[15].GetLength());
  AddMinLengthConstraint(13, 11, x14_lengthConstraints[17].GetLength());
  AddMinLengthConstraint(
      9, 2, 0.707f * x14_lengthConstraints[15].GetLength() + x14_lengthConstraints[10].GetLength());
  AddMinLengthConstraint(12, 5,
                         0.707f * x14_lengthConstraints[17].GetLength() +
                             x14_lengthConstraints[13].GetLength());
  AddMinLengthConstraint(9, 11, x14_lengthConstraints[15].GetLength());
  AddMinLengthConstraint(12, 8, x14_lengthConstraints[17].GetLength());
  AddMinLengthConstraint(
      10, 0, x14_lengthConstraints[2].GetLength() + x14_lengthConstraints[15].GetLength());
  AddMinLengthConstraint(
      13, 0, x14_lengthConstraints[3].GetLength() + x14_lengthConstraints[17].GetLength());
  AddMinLengthConstraint(10, 13, x14_lengthConstraints[14].GetLength());
  AddMinLengthConstraint(9, 12, 0.5f * x14_lengthConstraints[14].GetLength());
  AddMinLengthConstraint(10, 12, 0.5f * x14_lengthConstraints[14].GetLength());
  AddMinLengthConstraint(13, 9, 0.5f * x14_lengthConstraints[14].GetLength());
  AddMaxLengthConstraint(10, 13, 4.f * x14_lengthConstraints[14].GetLength());
  AddJointConstraint(8, 2, 5, 8, 9, 10);
  AddJointConstraint(11, 2, 5, 11, 12, 13);
  AddJointConstraint(2, 11, 5, 2, 3, 4);
  AddJointConstraint(5, 2, 8, 5, 6, 7);
}

CFlyingPirateRagDoll::~CFlyingPirateRagDoll() { CSfxManager::RemoveEmitter(xa0_spinEmitter); }

void CFlyingPirateRagDoll::Prime(CStateManager& mgr, const CTransform4f& xf, CModelData& mData) {
  if (x6c_actor->x6a1_30_spinToDeath) {
    xa0_spinEmitter = CSfxManager::AddEmitter(x9c_spinSfx, x6c_actor->GetTranslation(),
                                              CVector3f::Zero(), true, true);
  }
  CRagDoll::Prime(mgr, xf, mData);
}

void CFlyingPirateRagDoll::Update(CStateManager& mgr, float dt, float waterTop) {
  if (!IsOver()) {
    if (x6c_actor->x6a1_30_spinToDeath) {
      x84_ -= dt;
      const CVector3f direction =
          (x6c_actor->x2e0_destPos - x4_particles[14].GetPosition()).AsNormalized();
      x74_ = CVector3f::Slerp(x74_, direction, CRelAngle::FromDegrees(360.f * dt));
      x70_ = 25.f;
      const CVector3f force = x70_ * x74_;
      if (x84_ > 0.f) {
        x4_particles[14].Velocity() += 25.f * force;
      } else {
        x4_particles[14].Velocity() += 11.f * force;
      }
      CVector3f counterForce = force;
      counterForce *= -4.f;
      x4_particles[4].Velocity() += -force;
      x4_particles[7].Velocity() += -force;
      x4_particles[10].Velocity() += counterForce;
      x4_particles[10].Velocity() += counterForce;
      x4_particles[1].Velocity() += force;
      CVector3f torque =
          CVector3f::Cross(x4_particles[5].GetPosition() - x4_particles[2].GetPosition(),
                           x4_particles[8].GetPosition() - x4_particles[2].GetPosition());
      torque += 0.25f * (x4_particles[2].GetPosition() - x4_particles[5].GetPosition());
      x80_ = CMath::Min(1000.f * dt + x80_, 1000.f);
      torque = x80_ * torque.AsNormalized();
      x4_particles[2].Velocity() += torque;
      x4_particles[5].Velocity() += -torque;
      x44_normalGravity = 0.f;
      CSfxManager::UpdateEmitter(xa0_spinEmitter, x6c_actor->GetTranslation(),
                                 CVector3f(x58_averageVel), 127);
    }
    CVector3f oldCenter = x4_particles[8].GetPosition() * 0.25f +
                          x4_particles[11].GetPosition() * 0.25f +
                          x4_particles[0].GetPosition() * 0.5f;
    oldCenter[kDZ] =
        CMath::Min(x4_particles[8].GetPosition().GetZ() - x4_particles[8].GetRadius(),
                   x4_particles[11].GetPosition().GetZ() - x4_particles[11].GetRadius());
    oldCenter[kDZ] = CMath::Min(x4_particles[0].GetPosition().GetZ() - x4_particles[0].GetRadius(),
                                oldCenter[kDZ]);
    CRagDoll::Update(mgr, dt, waterTop);
    CVector3f newCenter = x4_particles[8].GetPosition() * 0.25f +
                          x4_particles[11].GetPosition() * 0.25f +
                          x4_particles[0].GetPosition() * 0.5f;
    newCenter[kDZ] =
        CMath::Min(x4_particles[8].GetPosition().GetZ() - x4_particles[8].GetRadius(),
                   x4_particles[11].GetPosition().GetZ() - x4_particles[11].GetRadius());
    newCenter[kDZ] = CMath::Min(x4_particles[0].GetPosition().GetZ() - x4_particles[0].GetRadius(),
                                newCenter[kDZ]);
    const CVector3f velocity = (newCenter - oldCenter) / dt;
    x6c_actor->SetTransform(CTransform4f::Identity());
    x6c_actor->SetTranslation(newCenter);
    x6c_actor->SetVelocityWR(velocity);
    if (x6c_actor->x6a1_30_spinToDeath) {
      if (CVector3f::Dot(newCenter - x6c_actor->x2e0_destPos, xa4_) > 0.f) {
        x6c_actor->x88c_ragDollTimer = 0.5f * dt;
      }
    }
    x8c_impactTimer -= dt;
    const float impactVel = x54_impactVel;
    if (impactVel > 2.f && x8c_impactTimer < 0.f) {
      const CVector3f delta = x6c_actor->GetTranslation() - x90_lastImpactPos;
      if (xb0_24_firstImpact || delta.MagSquared() > 0.1f) {
        float volume = CMath::Min(10.f * impactVel, 127.f);
        CSfxManager::AddEmitter(x88_impactSfx, x6c_actor->GetTranslation(), CVector3f::Zero(),
                                CCast::ToUint8(volume), true, false);
        x8c_impactTimer = 0.222f * mgr.Random()->Float() + 0.222f;
        xb0_24_firstImpact = false;
        x90_lastImpactPos = x6c_actor->GetTranslation();
      }
    }
  } else {
    x6c_actor->SetMomentumWR(CVector3f::Zero());
    x6c_actor->Stop();
  }
}

void CFlyingPirateRagDoll::PreRender(const CVector3f& pos, CModelData& mData) {
  if (!IsOver()) {
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
    rootOffset[kDX] /= mData.ScaleCopy()[kDX];
    rootOffset[kDY] /= mData.ScaleCopy()[kDY];
    rootOffset[kDZ] /= mData.ScaleCopy()[kDZ];
    builder.Insert(rootId, rootOffset);
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
    CVector3f rest = layout.GetFromParentUnrotated(x4_particles[1].GetBone());
    CVector3f neck = x4_particles[1].GetPosition() - x4_particles[0].GetPosition();
    neck = rootRot.BuildInverted().Transform(neck);
    CQuaternion neckRot = CQuaternion::ShortestRotationArc(rest, neck);
    builder.Insert(x4_particles[1].GetBone(), neckRot);
    CQuaternion jointRot = BoneAlign(builder, layout, 2, 3, rootRot);
    BoneAlign(builder, layout, 3, 4, rootRot * jointRot);
    jointRot = BoneAlign(builder, layout, 5, 6, rootRot);
    BoneAlign(builder, layout, 6, 7, rootRot * jointRot);
    jointRot = BoneAlign(builder, layout, 8, 9, rootRot);
    BoneAlign(builder, layout, 9, 10, rootRot * jointRot);
    jointRot = BoneAlign(builder, layout, 11, 12, rootRot);
    BoneAlign(builder, layout, 12, 13, rootRot * jointRot);
    animData->SetPoseBuilderValid(false);
  }
}

CFlyingPirate::CFlyingPirateData::CFlyingPirateData(CInputStream& in, int propCount)
: x0_maxCoverDistance(in.ReadFloat())
, x4_hearingDistance(in.ReadFloat())
, x8_type(in.ReadLong())
, xc_gunProjectileInfo(in)
, x34_gunSfx(CSfxManager::TranslateSFXID(in.ReadLong()))
, x38_altProjectileInfo1(in)
, x60_altProjectileInfo2(in.ReadLong(), CDamageInfo())
, x88_knockBackDelay(in.ReadFloat())
, x8c_flyingHeight(in.ReadFloat())
, x90_particleGenDesc(gpSimplePool->GetObj(SObjectTag('PART', in.ReadLong())))
, x9c_dInfo(in)
, xb8_(in.ReadFloat())
, xbc_(in.ReadFloat())
, xc0_(in.ReadFloat())
, xc4_(in.ReadFloat())
, xc8_ragDollSfx1(CSfxManager::TranslateSFXID(in.ReadLong()))
, xca_ragDollSfx2(CSfxManager::TranslateSFXID(in.ReadLong()))
, xcc_coverCheckChance(in.ReadFloat())
, xd0_(in.ReadFloat())
, xd4_(in.ReadFloat())
, xd8_particleGen1(in.ReadLong())
, xdc_particleGen2(in.ReadLong())
, xe0_particleGen3(in.ReadLong())
, xe4_knockBackSfx(CSfxManager::TranslateSFXID(in.ReadLong()))
, xe6_deathSfx(CSfxManager::TranslateSFXID(in.ReadLong()))
, xe8_aggressionChance(in.ReadFloat())
, xec_(in.ReadFloat())
, xf0_projectileHomingDistance(propCount > 35 ? in.ReadFloat() : 8.f) {
  xc_gunProjectileInfo.Token().Lock();
  x38_altProjectileInfo1.Token().Lock();
  x60_altProjectileInfo2.Token().Lock();
}

CFlyingPirate::CFlyingPirate(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                             const CTransform4f& xf, const CModelData& mData,
                             const CActorParameters& actParms, const CPatternedInfo& pInfo,
                             CInputStream& in, int propCount)
: CPatterned(kC_FlyingPirate, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Ground, kCT_One,
             kBT_NewFlyer, actParms, kCS_Medium)
, x568_data(in, propCount)
, x6a0_24_isFlyingPirate(x568_data.x8_type & 1)
, x6a0_25_isAquaPirate(x568_data.x8_type & 2)
, x6a0_26_hearShot(false)
, x6a0_27_canPatrol(false)
, x6a0_28_(false)
, x6a0_29_checkForProjectiles(false)
, x6a0_30_(false)
, x6a0_31_cantSeePlayer(true)
, x6a1_24_prevInCineCam(false)
, x6a1_25_(false)
, x6a1_26_isAttackingObject(false)
, x6a1_28_(false)
, x6a1_29_isMoving(false)
, x6a1_30_spinToDeath(false)
, x6a1_31_stopped(false)
, x6a2_24_aggressive(false)
, x6a2_25_aggressionChecked(false)
, x6a2_26_jetpackActive(false)
, x6a2_27_sparksActive(false)
, x6a2_28_(false)
, x6a4_currentCoverPoint(kInvalidUniqueId)
, x6a6_id2(kInvalidUniqueId)
, x6a8_pathFindSearch(nullptr, x6a0_25_isAquaPirate ? 4 : 3, pInfo.GetPathfindingIndex(),
                      pInfo.GetHalfExtent(), pInfo.GetHeight())
, x790_(0)
, x794_initialHealth(pInfo.GetHealthInfo().GetHP())
, x798_headSegId(CSegId::Invalid())
, x79c_(-1)
, x7a0_boneTracking(*GetModelData()->GetAnimationData(), rstl::string_l("Head_1"),
                    CMath::Deg2Rad(80.f), CMath::Deg2Rad(180.f), kBTF_None)
, x7d8_(0.f)
, x7dc_(0)
, x7e0_gunSegId(CSegId::Invalid())
, x7e4_(1.f)
, x7e8_targetId(kInvalidUniqueId)
, x7ec_burstFire(skBursts, 0)
, x84c_dodgeDirection(pas::kSD_Invalid)
, x850_height(3.f)
, x854_(FLT_MAX)
, x858_(FLT_MAX)
, x85c_attackObjectId(kInvalidUniqueId)
, x860_(15.f)
, x86c_(0.f)
, x870_(CVector3f::Zero())
, x87c_(CVector3f::Zero())
, x888_(10.f)
, x88c_ragDollTimer(3.f)
, x890_teamAiMgr(kInvalidUniqueId)
, x894_pitchBend(1.f)
, x898_(1.f)
, x89c_ragDoll(nullptr)
, x8a0_patrolTarget(kInvalidUniqueId)
, x8a4_(0.f) {
  const CAnimData* animData = GetModelData()->GetAnimationData();
  x798_headSegId = animData->GetLocatorSegId(rstl::string_l("Head_1"));
  x7e0_gunSegId = animData->GetLocatorSegId(rstl::string_l("L_gun_LCTR"));
  x864_missileSegments.push_back(animData->GetLocatorSegId(rstl::string_l("L_Missile_LCTR")));
  x864_missileSegments.push_back(animData->GetLocatorSegId(rstl::string_l("R_Missile_LCTR")));
  const CPASAnimParmData parms(pas::kAS_Step, CPASAnimParm::FromEnum(3), CPASAnimParm::FromEnum(1));
  const CModelData& modelData = *GetModelData();
  const float distance = GetAnimationDistance(parms);
  float height = modelData.GetScale().GetX();
  height *= distance;
  x850_height = height;
  if (x568_data.GetParticleGen1() != kInvalidAssetId &&
      x568_data.GetParticleGen2() != kInvalidAssetId &&
      x568_data.GetParticleGen3() != kInvalidAssetId) {
    x65c_particleGenDescs.push_back(
        gpSimplePool->GetObj(SObjectTag('PART', x568_data.GetParticleGen1())));
    x65c_particleGenDescs.push_back(
        gpSimplePool->GetObj(SObjectTag('PART', x568_data.GetParticleGen2())));
    x65c_particleGenDescs.push_back(
        gpSimplePool->GetObj(SObjectTag('PART', x568_data.GetParticleGen3())));
    for (int i = 0; i < x65c_particleGenDescs.size(); ++i) {
      x684_particleGens.push_back(rs_new CElementGen(x65c_particleGenDescs[i]));
      x684_particleGens[i]->SetParticleEmission(false);
    }
  }
  KnockBackCtrl().SetLocomotionDuringElectrocution(true);
}

ENTITY_ACCEPT_IMPL(CFlyingPirate)

bool CFlyingPirate::Listen(const CVector3f& pos, EListenNoiseType type) {
  bool heard = false;
  if (x400_25_alive) {
    const float hearingDistance = x568_data.GetHearingDistance() * x568_data.GetHearingDistance();
    const CVector3f delta = pos - GetTranslation();
    if (delta.MagSquared() < hearingDistance &&
        (x3c0_detectionHeightRange == 0.f ||
         delta.GetZ() * delta.GetZ() < x3c0_detectionHeightRange * x3c0_detectionHeightRange)) {
      x6a0_26_hearShot = true;
      heard = true;
    }
    if (type == kLNT_PlayerFire) {
      x6a0_29_checkForProjectiles = true;
    }
  }
  const bool result = heard;
  return result;
}

void CFlyingPirate::DeliverGetUp() {
  if (x450_bodyController->GetCurrentStateId() == pas::kAS_LieOnGround) {
    x450_bodyController->CommandMgr().DeliverCmd(CBCGetupCmd(pas::kGetup_Zero));
  }
}

void CFlyingPirate::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case kSM_Alert:
    if (GetActive()) {
      x400_24_hitByPlayerProjectile = true;
    }
    break;
  case kSM_Activate:
    AddToTeam(mgr);
    break;
  case kSM_Deactivate:
  case kSM_Deleted:
    RemoveFromTeam(mgr);
    break;
  }
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case kSM_InitializedInArea:
    for (AUTO(it, GetConnectionList().begin()); it != GetConnectionList().end(); ++it) {
      if (it->x0_state == kSS_Retreat) {
        const TUniqueId id = mgr.GetIdForScript(it->x8_objId);
        if (CScriptCoverPoint* cover = TCastToPtr< CScriptCoverPoint >(mgr.ObjectById(id))) {
          cover->Reserve(GetUniqueId());
        }
      } else if (it->x0_state == kSS_Patrol && it->x4_msg == kSM_Follow) {
        x6a0_27_canPatrol = true;
      } else if (it->x0_state == kSS_Attack && it->x4_msg == kSM_Action) {
        x85c_attackObjectId = mgr.GetIdForScript(it->x8_objId);
      }
    }
    x6a8_pathFindSearch.SetArea(
        mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetPostConstructed()->x10bc_pathArea);
    if (GetActive()) {
      AddToTeam(mgr);
    }
    UpdateParticleEffects(mgr, 0.f, x6a0_24_isFlyingPirate);
    ModelData()->AnimationData()->SetParticleEffectState(skEyes, true, mgr);
    break;
  case kSM_Registered: {
    const float range = x568_data.xc0_;
    const float delay = x568_data.xbc_;
    x86c_ = range * mgr.Random()->Float() + delay;
    break;
  }
  case kSM_Falling:
    if (x450_bodyController->GetPercentageFrozen() == 0.f && !x400_27_fadeToDeath &&
        !x6a1_30_spinToDeath) {
      SetMomentumWR(CVector3f(0.f, 0.f, -GetGravityConstant() * GetMass()));
    }
    x7ec_burstFire.SetBurstType(0);
    break;
  case kSM_OnFloor:
    x7ec_burstFire.SetBurstType(2);
    break;
  case kSM_Jumped:
    if (CScriptCoverPoint* cover = GetCoverPoint(mgr, x6a4_currentCoverPoint)) {
      x328_25_verticalMovement = false;
      SetMomentumWR(CVector3f(0.f, 0.f, -GetMass() * GetGravityConstant()));
      AddMaterial(kMT_GroundCollider, mgr);
      SetDestPos(cover->GetTranslation());
      const CVector3f delta = cover->GetTranslation() - GetTranslation();
      if (delta.GetZ() < 0.f) {
        CVector3f velocity = GetVelocityWR();
        const float gravity = GetGravityConstant();
        const float root =
            CMath::FastSqrtF(-(2.f * gravity * delta.GetZ() - velocity[kDZ] * velocity[kDZ]));
        float verticalVelocity = -velocity[kDZ];
        verticalVelocity += root;
        const float time = verticalVelocity / gravity;
        if (time > 0.f) {
          const CVector2f normal(delta.ToVec2f().AsNormalized());
          const float speed = delta.ToVec2f().Magnitude() / time;
          velocity.SetX(speed * normal[0]);
          velocity.SetY(speed * normal[1]);
          SetVelocityWR(velocity);
          x870_ = CVector3f::Zero();
          x87c_ = CVector3f::Zero();
          x898_ = 1.f;
        }
      }
    }
    break;
  case kSM_Start:
    x6a1_31_stopped = false;
    break;
  case kSM_Stop:
    x6a1_31_stopped = true;
    break;
  case kSM_SetToZero:
    x6a2_28_ = true;
    break;
  }
}

void CFlyingPirate::UpdateParticleEffects(CStateManager& mgr, float intensity, const bool active) {
  CAnimData* animData = AnimationData();
  const rstl::string& name = x6a0_25_isAquaPirate ? skScubaGear : skJetPack;
  if (active != x6a2_26_jetpackActive) {
    animData->SetParticleEffectState(name, active, mgr);
    if (x6a0_25_isAquaPirate) {
      animData->SetParticleEffectState(skScubaBubbles, active, mgr);
    }
    x6a2_26_jetpackActive = active;
  }
  if (active) {
    animData->SetParticleCEXTValue(name, 0, 0.75f * intensity + 2.25f);
    animData->SetParticleCEXTValue(name, 1, -0.13f * intensity + -0.1f);
  }
  if (!x6a0_25_isAquaPirate) {
    bool sparks = active && intensity > 0.8f;
    if (sparks != x6a2_27_sparksActive) {
      animData->SetParticleEffectState(skSparks, sparks, mgr);
      x6a2_27_sparksActive = sparks;
    }
  }
}

void CFlyingPirate::UpdateLandingSmoke(CStateManager& mgr, bool active) {
  if (active) {
    if (!x684_particleGens.empty()) {
      float particleLevel = GetTranslation().GetZ() - 5.f;
      CScriptCoverPoint* cover = GetCoverPoint(mgr, x6a4_currentCoverPoint);
      if (cover != nullptr) {
        particleLevel = cover->GetTranslation().GetZ() - 1.f;
      }
      const CRayCastResult result = mgr.RayStaticIntersection(
          GetTranslation(), CVector3f::Down(), GetTranslation()[kDZ] - particleLevel,
          CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid)));
      int index = 1;
      if (result.IsValid()) {
        const CMaterialList& material = result.GetMaterial();
        if (material.HasMaterial(kMT_Ice) || material.HasMaterial(kMT_Snow)) {
          index = 2;
        } else if (material.HasMaterial(kMT_Dirt) || material.HasMaterial(kMT_MudSlow) ||
                   material.HasMaterial(kMT_Sand)) {
          index = 0;
        }
        particleLevel = GetTranslation().GetZ() - result.GetTime();
      }
      x684_particleGens[index]->SetParticleEmission(true);
      const CVector3f& origin = GetTranslation();
      x684_particleGens[index]->SetTranslation(
          CVector3f(origin.GetX(), origin.GetY(), particleLevel));
    }
    AnimationData()->SetParticleEffectState(skLandingSmoke, true, mgr);
  } else {
    for (int i = 0; i < x684_particleGens.size(); ++i) {
      x684_particleGens[i]->SetParticleEmission(false);
    }
    AnimationData()->SetParticleEffectState(skLandingSmoke, false, mgr);
  }
}

CVector3f CFlyingPirate::GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                                   const CVector3f& aimPos) const {
  return GetTranslation();
}

void CFlyingPirate::AddToTeam(CStateManager& mgr) {
  if (x890_teamAiMgr == kInvalidUniqueId) {
    x890_teamAiMgr = CTeamAiMgr::GetTeamAiMgr(*this, mgr);
  }
  if (x890_teamAiMgr != kInvalidUniqueId) {
    if (CTeamAiMgr* team = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(x890_teamAiMgr))) {
      team->AssignTeamAiRole(*this, CTeamAiRole::kTAR_Projectile, CTeamAiRole::kTAR_Unknown,
                             CTeamAiRole::kTAR_Invalid);
    }
  }
}

void CFlyingPirate::RemoveFromTeam(CStateManager& mgr) {
  if (x890_teamAiMgr != kInvalidUniqueId) {
    if (CTeamAiMgr* team = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(x890_teamAiMgr))) {
      if (team->IsPartOfTeam(GetUniqueId())) {
        team->RemoveTeamAiRole(GetUniqueId());
        x890_teamAiMgr = kInvalidUniqueId;
      }
    }
  }
}

void CFlyingPirate::CheckForProjectiles(CStateManager& mgr) {
  if (x6a0_29_checkForProjectiles) {
    const CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
    const CVector3f extent(5.f, 5.f, 5.f);
    const CAABox box(playerPos - extent, playerPos + extent);
    x6a0_30_ = false;
    TEntityList nearList;
    mgr.BuildNearList(nearList, box, CMaterialFilter::MakeInclude(CMaterialList(kMT_Projectile)),
                      nullptr);
    for (int i = 0; i < nearList.size(); ++i) {
      if (const CGameProjectile* const projectile =
              TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(nearList[i]))) {
        CVector3f delta = GetBoundingBox().GetCenterPoint() - projectile->GetTranslation();
        if (delta.IsMagnitudeSafe()) {
          if (CVector3f::Dot(GetTransform().GetForward(), delta) < 0.f) {
            delta.Normalize();
            CVector3f movement = projectile->GetTranslation() - projectile->GetPreviousPos();
            if (movement.IsMagnitudeSafe()) {
              movement.Normalize();
              if (CVector3f::Dot(movement, delta) > 0.939f) {
                x6a0_30_ = true;
              }
            }
          }
        } else {
          x6a0_30_ = true;
        }
        if (x6a0_30_) {
          break;
        }
      }
    }
    x6a0_29_checkForProjectiles = false;
  }
}

bool CFlyingPirate::LineOfSightTest(CStateManager& mgr, const CVector3f& start,
                                    const CVector3f& end, const CMaterialList& exclude) {
  const CMaterialFilter filter =
      CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), exclude);
  return mgr.RayCollideWorld(start, end, filter, this);
}

void CFlyingPirate::UpdateCantSeePlayer(CStateManager& mgr) {
  if (x7dc_ % 7 == 0) {
    const CVector3f target = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    const CVector3f origin = GetGunEyePos();
    bool cantSee = true;
    if (LineOfSightTest(mgr, origin - GetTransform().GetRight(), target,
                        CMaterialList(kMT_Player, kMT_ProjectilePassthrough))) {
      if (LineOfSightTest(mgr, origin + GetTransform().GetRight(), target,
                          CMaterialList(kMT_Player, kMT_ProjectilePassthrough))) {
        cantSee = false;
      }
    }
    x6a0_31_cantSeePlayer = cantSee;
  }
  ++x7dc_;
}

CVector3f CFlyingPirate::AvoidActors(CStateManager& mgr) {
  CVector3f separation = CVector3f::Zero();
  const CVector3f extent(8.f, 8.f, 8.f);
  const CAABox box(GetTranslation() - extent, GetTranslation() + extent);
  TEntityList nearList;
  mgr.BuildNearList(nearList, box, CMaterialFilter::MakeInclude(CMaterialList(kMT_Character)),
                    this);
  for (int i = 0; i < nearList.size(); ++i) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(nearList[i]))) {
      separation += x45c_steeringBehaviors.Separation(*this, actor->GetTranslation(), 10.f);
    }
  }
  CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  delta.SetZ(0.f);
  separation += x45c_steeringBehaviors.Separation(*this, GetTranslation() + delta, 20.f);
  return separation;
}

pas::EStepDirection CFlyingPirate::GetDodgeDirection(CStateManager& mgr, float arg) {
  const float argSquared = arg * arg;
  bool canDodgeLeft = true;
  bool canDodgeRight = true;
  bool canDodgeUp = true;
  bool canDodgeDown = true;
  pas::EStepDirection direction = pas::kSD_Invalid;
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (const CPhysicsActor* actor = TCastToConstPtr< CPhysicsActor >(list[i])) {
      if (actor != this && actor->GetCurrentAreaId() == GetCurrentAreaId()) {
        const CVector3f delta = actor->GetTranslation() - GetTranslation();
        const float magSquared = delta.MagSquared();
        if (magSquared < argSquared) {
          float rightDot = CVector3f::Dot(delta, GetTransform().GetRight());
          if (rightDot > 0.866f * magSquared || (rightDot > 0.f && magSquared < 3.f)) {
            canDodgeRight = false;
          } else if (rightDot < 0.866f * -magSquared || (rightDot < 0.f && magSquared < 3.f)) {
            canDodgeLeft = false;
          }
          float upDot = CVector3f::Dot(delta, GetTransform().GetUp());
          if (upDot > 0.866f * magSquared || (upDot > 0.f && magSquared < 3.f)) {
            canDodgeUp = false;
          } else if (upDot < 0.866f * -magSquared || (upDot < 0.f && magSquared < 3.f)) {
            canDodgeDown = false;
          }
        }
      }
    }
  }
  const CVector3f center = GetBoundingBox().GetCenterPoint();
  if (canDodgeRight) {
    if (!LineOfSightTest(mgr, center, center + arg * GetTransform().GetRight(), CMaterialList())) {
      canDodgeRight = false;
    }
  }
  if (canDodgeLeft) {
    if (!LineOfSightTest(mgr, center, center - arg * GetTransform().GetRight(), CMaterialList())) {
      canDodgeLeft = false;
    }
  }
  if (canDodgeUp) {
    if (!LineOfSightTest(mgr, center, center + arg * GetTransform().GetUp(), CMaterialList())) {
      canDodgeUp = false;
    }
  }
  if (canDodgeDown) {
    if (!LineOfSightTest(mgr, center, center - arg * GetTransform().GetUp(), CMaterialList())) {
      canDodgeDown = false;
    }
  }
  if ((canDodgeLeft || canDodgeRight) && (canDodgeUp || canDodgeDown)) {
    if ((mgr.Random()->Next() & 0x4000) != 0) {
      canDodgeLeft = false;
      canDodgeRight = false;
    } else {
      canDodgeUp = false;
      canDodgeDown = false;
    }
  }
  if (canDodgeLeft && canDodgeRight) {
    if ((mgr.Random()->Next() & 0x4000) != 0) {
      canDodgeLeft = false;
    } else {
      canDodgeRight = false;
    }
  }
  if (canDodgeUp && canDodgeDown) {
    const float height = x568_data.x8c_flyingHeight;
    if (GetTargetPos(mgr).GetZ() - (GetTranslation()[kDZ] - -height) > 0.f) {
      canDodgeDown = false;
    } else {
      canDodgeUp = false;
    }
  }
  if (canDodgeUp) {
    direction = pas::kSD_Up;
  } else if (canDodgeDown) {
    direction = pas::kSD_Down;
  } else if (canDodgeLeft) {
    direction = pas::kSD_Left;
  } else if (canDodgeRight) {
    direction = pas::kSD_Right;
  }
  return direction;
}

CVector3f CFlyingPirate::GetTargetPos(CStateManager& mgr) {
  if (x7e8_targetId != mgr.GetPlayer()->GetUniqueId()) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(x7e8_targetId))) {
      if (actor->GetActive()) {
        return actor->GetTranslation();
      }
    }
    x7a0_boneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    x7e8_targetId = mgr.GetPlayer()->GetUniqueId();
  }
  return mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
}

bool CFlyingPirate::CanFireMissiles(CStateManager& mgr) {
  bool canFire = true;
  for (int i = 0; i < 2; ++i) {
    const CTransform4f xf = GetLctrTransform(x864_missileSegments[i]);
    const CVector3f end = xf.GetTranslation() + 3.f * xf.GetForward();
    if (!LineOfSightTest(mgr, xf.GetTranslation(), end,
                         CMaterialList(kMT_Player, kMT_ProjectilePassthrough)) ||
        !LineOfSightTest(mgr, end, GetTargetPos(mgr),
                         CMaterialList(kMT_Player, kMT_ProjectilePassthrough))) {
      x6a1_28_ = true;
      canFire = false;
      break;
    }
  }
  const bool result = canFire;
  return result;
}

void CFlyingPirate::KnockBack(const CVector3f& pos, CStateManager& mgr, const CDamageInfo& info,
                              float magnitude, bool direct, const bool inDeferred) {
  if (IsAlive()) {
    KnockBackCtrl().SetSeverity(x328_25_verticalMovement ? pas::kS_Zero : pas::kS_One);
  } else if (!IsOnGround()) {
    const float chance = x568_data.xb8_;
    if (mgr.Random()->Range(0.f, 100.f) < chance) {
      x6a1_30_spinToDeath = true;
      SetMomentumWR(CVector3f::Zero());
    } else {
      UpdateParticleEffects(mgr, 0.f, false);
      SetMomentumWR(CVector3f(0.f, 0.f, -GetGravityConstant() * GetMass()));
    }
    KnockBackCtrl().SetAnimationStateRange(kAR_Hurled, kAR_Hurled);
    x328_25_verticalMovement = false;
    const TUniqueId waypointId = GetConnectedObject(mgr, kSS_Retreat, kSM_Next);
    if (waypointId != kInvalidUniqueId) {
      const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(waypointId));
    }
    const CVector3f homingPos = mgr.GetPlayer()->GetHomingPosition(mgr, 0.f);
    const CVector3f delta = homingPos - GetTranslation();
    CVector3f cross = CVector3f::Cross(delta, CVector3f::Up());
    if (close_enough(cross, CVector3f::Zero(), 0.0001f)) {
      cross = CVector3f::Cross(delta, CVector3f::Forward());
    }
    cross = mgr.Random()->Range(-5.f, 5.f) * cross.AsNormalized();
    SetDestPos(homingPos + cross);
    x7a0_boneTracking.SetActive(false);
  }
  CPatterned::KnockBack(pos, mgr, info, magnitude, direct, inDeferred);
  if (IsAlive()) {
    switch (KnockBackCtrl().GetActiveParms().x0_animState) {
    case kAR_Hurled:
      if (!x450_bodyController->IsFrozen()) {
        x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), rstl::string_l("GetUpNow"));
        x330_stateMachineState.SetDelay(x568_data.x88_knockBackDelay);
      }
      x6a1_28_ = false;
      x328_25_verticalMovement = false;
      CSfxManager::AddEmitter(x568_data.xe4_knockBackSfx, GetTranslation(), CVector3f::Zero(),
                              uchar(127), true, false);
      break;
    }
  } else
    switch (KnockBackCtrl().GetActiveParms().x0_animState) {
    case kAR_Hurled:
      CSfxManager::AddEmitter(x568_data.xe6_deathSfx, GetTranslation(), CVector3f::Zero(),
                              uchar(127), true, false);
      if (x400_27_fadeToDeath) {
        x6a1_30_spinToDeath = false;
        UpdateParticleEffects(mgr, 0.f, false);
        SetMomentumWR(CVector3f(0.f, 0.f, -GetGravityConstant() * GetMass()));
      }
      break;
    }
}

void CFlyingPirate::MassiveDeath(CStateManager& mgr) {
  CExplosion* explosion =
      rs_new CExplosion(x568_data.x90_particleGenDesc, mgr.AllocateUniqueId(), true,
                        CEntityInfo(GetCurrentAreaId(), NullConnectionList), rstl::string_l(""),
                        GetTransform(), 0, CVector3f(1.5f, 1.5f, 1.5f), CColor::White());
  if (explosion != nullptr) {
    mgr.AddObject(*explosion);
    mgr.ApplyDamageToWorld(
        GetUniqueId(), *this, GetTranslation(), x568_data.x9c_dInfo,
        CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()));
    mgr.CameraManager()->AddCameraShaker(CCameraShakeData::HardHorizShake(0.5f, 0.3f), true);
  }
  CPatterned::MassiveDeath(mgr);
}

void CFlyingPirate::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  if (x6a0_27_canPatrol) {
    CPatterned::Patrol(mgr, msg, dt);
    switch (msg) {
    case kStateMsg_Activate:
      x450_bodyController->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
      x8a0_patrolTarget = x2dc_destObj;
      x8a4_ = 0.f;
      break;
    case kStateMsg_Update:
      if (x2dc_destObj != x8a0_patrolTarget) {
        x8a0_patrolTarget = x2dc_destObj;
        x8a4_ = 0.f;
      }
      if (x2d8_patrolState == kPS_Patrol) {
        const CVector3f direction = (x2e0_destPos - GetTranslation()).AsNormalized();
        const float speed = x3b0_moveSpeed * x568_data.xc4_;
        x8a4_ = dt * speed + x8a4_;
        x8a4_ = CMath::Min(x8a4_, speed);
        x87c_ = (dt * (x8a4_ * dt)) * direction;
        x898_ = 1.5f * x3b0_moveSpeed;
        x870_ += x87c_;
      }
      switch (x30c_behaviourOrient) {
      case kBO_Constant:
        x450_bodyController->CommandMgr().DeliverTargetVector(GetTargetPos(mgr) - GetTranslation());
        break;
      }
      UpdateCantSeePlayer(mgr);
      break;
    case kStateMsg_Deactivate:
      x450_bodyController->CommandMgr().SetSteeringBlendMode(kSBM_Normal);
      break;
    }
  }
}

void CFlyingPirate::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::Patrol(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Activate: {
    x450_bodyController->CommandMgr().SetSteeringBlendMode(kSBM_Normal);
    x2dc_destObj = GetConnectedObject(mgr, kSS_Attack, kSM_Follow);
    const TUniqueId& id = x2dc_destObj;
    if (id != kInvalidUniqueId) {
      if (const CScriptWaypoint* waypoint =
              TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id))) {
        x30c_behaviourOrient = waypoint->GetBehaviourOrient();
        x3b0_moveSpeed = waypoint->GetSpeed();
      }
    }
    x8a0_patrolTarget = x2dc_destObj;
    x8a4_ = 0.f;
    break;
  }
  case kStateMsg_Update:
    if (x2dc_destObj != x8a0_patrolTarget) {
      x8a0_patrolTarget = x2dc_destObj;
      x8a4_ = 0.f;
    }
    if (x2d8_patrolState == kPS_Patrol) {
      const CVector3f direction = (x2e0_destPos - GetTranslation()).AsNormalized();
      const float speed = x3b0_moveSpeed * x568_data.xc4_;
      x8a4_ = dt * speed + x8a4_;
      x8a4_ = CMath::Min(x8a4_, speed);
      x87c_ = (dt * (x8a4_ * dt)) * direction;
      x898_ = 1.5f * x3b0_moveSpeed;
      x870_ += x87c_;
    }
    switch (x30c_behaviourOrient) {
    case kBO_Constant:
      x450_bodyController->CommandMgr().DeliverTargetVector(GetTargetPos(mgr) - GetTranslation());
      break;
    }
    UpdateCantSeePlayer(mgr);
    break;
  }
}

bool CFlyingPirate::PatternOver(CStateManager& mgr, float arg) {
  return x2dc_destObj == kInvalidUniqueId;
}

bool CFlyingPirate::HearShot(CStateManager& mgr, float arg) {
  const bool heard = x6a0_26_hearShot;
  x6a0_26_hearShot = false;
  return heard;
}

bool CFlyingPirate::HearPlayer(CStateManager& mgr, float arg) {
  const CPlayer& player = *mgr.GetPlayer();
  bool heard = false;
  if (player.GetVelocityWR().MagSquared() > 0.1f) {
    const CVector3f delta = player.GetTranslation() - GetTranslation();
    if (delta.MagSquared() < x568_data.GetHearingDistance() * x568_data.GetHearingDistance()) {
      heard = true;
    }
  }
  return heard;
}

void CFlyingPirate::Taunt(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    x6a0_28_ = true;
    x7a0_boneTracking.SetActive(true);
    x7a0_boneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    bool foundPirate = false;
    const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
    for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
      if (const CSpacePirate* pirate = PATTERNED_CAST_TO(CSpacePirate, const_cast< CEntity* >(list[i]))) {
        if (!pirate->GetEnableAim() && pirate->IsAlive() &&
            pirate->GetCurrentAreaId() == GetCurrentAreaId() &&
            (pirate->GetTranslation() - GetTranslation()).MagSquared() <
                x568_data.x4_hearingDistance * x568_data.x4_hearingDistance) {
          foundPirate = true;
        }
      }
    }
    x79c_ = foundPirate ? 0 : 1;
    if (x7e8_targetId == kInvalidUniqueId) {
      x7e8_targetId = mgr.GetPlayer()->GetUniqueId();
    }
    break;
  }
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    if (x79c_ == 0) {
      mgr.InformListeners(GetTranslation(), kLNT_PlayerFire);
    }
    break;
  }
}

void CFlyingPirate::GetUp(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    CTeamAiMgr::ResetTeamAiRole(kAT_Projectile, mgr, x890_teamAiMgr, GetUniqueId(), true);
    break;
  case kStateMsg_Update:
    if (x450_bodyController->GetCurrentStateId() == pas::kAS_LieOnGround) {
      CPathFindSearch::EResult result =
          x6a8_pathFindSearch.Search(GetTranslation(), GetTranslation());
      if (result == CPathFindSearch::kR_NoSourcePoint) {
        x401_30_pendingDeath = true;
        return;
      }
    }
    TryCommand(mgr, pas::kAS_Getup, &CPatterned::TryGetUp, 0);
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    break;
  }
}

void CFlyingPirate::Bounce(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    CTeamAiMgr::ResetTeamAiRole(kAT_Projectile, mgr, x890_teamAiMgr, GetUniqueId(), true);
    break;
  case kStateMsg_Update:
    switch (x450_bodyController->GetCurrentStateId()) {
    case pas::kAS_Hurled:
      x450_bodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
      x328_25_verticalMovement = true;
      break;
    case pas::kAS_LieOnGround:
      x450_bodyController->CommandMgr().DeliverCmd(CBCGetupCmd(pas::kGetup_Zero));
      break;
    case pas::kAS_Locomotion:
      x330_stateMachineState.SetCodeTrigger();
      break;
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CFlyingPirate::Lurk(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    ReleaseCoverPoint(mgr, x6a4_currentCoverPoint);
    x6a0_31_cantSeePlayer = true;
    x7d8_ = 0.f;
    x7dc_ = 0;
    CTeamAiMgr::ResetTeamAiRole(kAT_Projectile, mgr, x890_teamAiMgr, GetUniqueId(), true);
    x330_stateMachineState.SetDelay(3.f);
    UpdateParticleEffects(mgr, 0.f, true);
    x6a2_25_aggressionChecked = false;
    break;
  case kStateMsg_Update:
    UpdateCantSeePlayer(mgr);
    if (x32c_animState != kAS_NotReady) {
      TryCommand(mgr, pas::kAS_Turn, &CPatterned::TryTurn, 0);
    }
    if (x32c_animState != kAS_Repeat) {
      x2e0_destPos = GetTargetPos(mgr);
      CVector3f delta = x2e0_destPos - GetTranslation();
      delta.SetZ(0.f);
      if (CVector3f::Dot(GetTransform().GetForward(), delta.AsNormalized()) < 0.8f) {
        x32c_animState = kAS_Ready;
      }
    }
    break;
  case kStateMsg_Deactivate:
    x6a1_25_ = false;
    x6a1_28_ = false;
    x32c_animState = kAS_NotReady;
    break;
  }
}

bool CFlyingPirate::CoverCheck(CStateManager& mgr, float arg) {
  if (x888_ <= 0.f) {
    x888_ = 10.f;
    const float chance = x568_data.xcc_coverCheckChance;
    return mgr.Random()->Range(0.f, 100.f) < chance;
  }
  return false;
}

bool CFlyingPirate::CoverFind(CStateManager& mgr, float arg) {
  bool found = false;
  float closestMag = x568_data.x0_maxCoverDistance * x568_data.x0_maxCoverDistance;
  const CScriptCoverPoint* closest = nullptr;
  const CObjectList& list = mgr.GetObjectListById(kOL_AiWaypoint);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (const CScriptCoverPoint* cover = TCastToConstPtr< CScriptCoverPoint >(list[i])) {
      if (cover->GetActive() && cover->ShouldLandHere() && !cover->GetInUse(GetUniqueId()) &&
          cover->GetCurrentAreaId() == GetCurrentAreaId()) {
        float mag = (GetTranslation() - cover->GetTranslation()).MagSquared();
        if (mag < closestMag) {
          closestMag = mag;
          closest = cover;
        }
      }
    }
  }
  if (closest != nullptr) {
    ReleaseCoverPoint(mgr, x6a4_currentCoverPoint);
    if (CScriptCoverPoint* cover =
            TCastToPtr< CScriptCoverPoint >(mgr.ObjectById(closest->GetUniqueId()))) {
      SetCoverPoint(cover, x6a4_currentCoverPoint);
      found = true;
      x6a6_id2 = x6a4_currentCoverPoint;
    }
  }
  return found;
}

bool CFlyingPirate::ShouldAttack(CStateManager& mgr, float arg) {
  bool shouldAttack = false;
  const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x890_teamAiMgr, GetUniqueId());
  if ((role == nullptr || role->GetTeamAiRole() == CTeamAiRole::kTAR_Projectile) &&
      x7e8_targetId == mgr.GetPlayer()->GetUniqueId() && (x86c_ <= 0.f || x854_ < 1.f) &&
      CanFireMissiles(mgr)) {
    const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    if (delta.GetZ() * delta.GetZ() < delta.GetX() * delta.GetX() + delta.GetY() * delta.GetY()) {
      if (x890_teamAiMgr == kInvalidUniqueId ||
          CTeamAiMgr::AddAttacker(kAT_Projectile, mgr, x890_teamAiMgr, GetUniqueId())) {
        float range = x568_data.xc0_;
        float delay = x568_data.xbc_;
        shouldAttack = true;
        x86c_ = range * mgr.Random()->Float() + delay;
      }
    }
  }
  return shouldAttack;
}

void CFlyingPirate::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    if (!x6a2_25_aggressionChecked) {
      float chance;
      if (x858_ < 3.f) {
        chance = 2.f * x568_data.xe8_aggressionChance;
      } else {
        chance = x568_data.xe8_aggressionChance;
      }
      x6a2_24_aggressive = mgr.Random()->Range(0.f, 100.f) < chance;
      x6a2_25_aggressionChecked = true;
    }
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_ProjectileAttack, &CPatterned::TryProjectileAttack, 1);
    x450_bodyController->FaceDirection(
        (mgr.GetPlayer()->GetTranslation() - GetTranslation()).AsNormalized(), dt);
    DeliverGetUp();
    UpdateCantSeePlayer(mgr);
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    x6a2_24_aggressive = false;
    break;
  }
}

bool CFlyingPirate::SpotPlayer(CStateManager& mgr, float arg) {
  const CVector3f dir = mgr.GetPlayer()->GetAimPosition(mgr, 0.f) - GetGunEyePos();
  float distance = dir.Magnitude();
  float angle = x3c4_detectionAngle;
  return CVector3f::Dot(dir, GetTransform().GetForward()) > distance * angle;
}

bool CFlyingPirate::LineOfSight(CStateManager& mgr, float arg) { return !x6a0_31_cantSeePlayer; }

bool CFlyingPirate::InRange(CStateManager& mgr, float arg) {
  const CPlayer& player = *mgr.GetPlayer();
  const CVector3f pos = player.GetTranslation();
  return CMath::AbsF(pos.GetZ()) < x2fc_minAttackRange &&
         pos.MagSquared() < x300_maxAttackRange * x300_maxAttackRange;
}

void CFlyingPirate::TurnAround(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    x2e0_destPos = GetTargetPos(mgr);
    CVector3f delta = x2e0_destPos - GetTranslation();
    delta.SetZ(0.f);
    if (CVector3f::Dot(GetTransform().GetForward(), delta.AsNormalized()) < 0.8f) {
      x32c_animState = kAS_Ready;
    }
    break;
  }
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Turn, &CPatterned::TryTurn, 0);
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    break;
  }
}

bool CFlyingPirate::ShouldDodge(CStateManager& mgr, float arg) {
  bool dodge = x6a1_28_;
  if (!x6a1_28_ && !x6a1_25_) {
    const CVector3f delta = GetTargetPos(mgr) - GetTranslation();
    if (CVector3f::Dot(delta, GetTransform().GetForward()) > 0.f &&
        (x854_ < 0.33f || x858_ < 0.33f) && x7d8_ < 0.5f) {
      dodge = true;
    }
  }
  return dodge;
}

void CFlyingPirate::Dodge(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    x84c_dodgeDirection = GetDodgeDirection(mgr, x850_height);
    if (x84c_dodgeDirection == pas::kSD_Invalid) {
      x84c_dodgeDirection = (mgr.Random()->Next() & 0x4000) != 0 ? pas::kSD_Left : pas::kSD_Right;
    }
    UpdateParticleEffects(mgr, 1.f, true);
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Step, &CPatterned::TryDodge, x84c_dodgeDirection);
    UpdateCantSeePlayer(mgr);
    x898_ = CMath::Max(1.f, 2.f - x330_stateMachineState.GetTime());
    DeliverGetUp();
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    x6a1_28_ = false;
    break;
  }
}

bool CFlyingPirate::ShotAt(CStateManager& mgr, float arg) { return x858_ < (arg ? arg : 0.5f); }

bool CFlyingPirate::Attacked(CStateManager& mgr, float arg) { return x854_ < (arg ? arg : 0.5f); }

bool CFlyingPirate::ShouldSpecialAttack(CStateManager& mgr, float arg) {
  if (x3fc_flavor == kFT_One && x85c_attackObjectId != kInvalidUniqueId && x860_ <= 0.f) {
    x860_ = 15.f * mgr.Random()->Float() + 15.f;
    if (!mgr.GetPlayer()->CheckOrbitDisableSourceList()) {
      if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(x85c_attackObjectId))) {
        if (x890_teamAiMgr == kInvalidUniqueId ||
            CTeamAiMgr::AddAttacker(kAT_Projectile, mgr, x890_teamAiMgr, GetUniqueId())) {
          SetDestPos(actor->GetTranslation() + 15.f * CVector3f::Down());
          x6a1_29_isMoving = true;
          return true;
        }
      }
    }
  }
  return false;
}

void CFlyingPirate::Jump(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    x450_bodyController->SetLocomotionType(pas::kLT_Combat);
    x328_25_verticalMovement = true;
    RemoveMaterial(kMT_GroundCollider, mgr);
    SetMomentumWR(CVector3f::Zero());
    x888_ = 10.f;
    UpdateParticleEffects(mgr, 1.f, true);
    UpdateLandingSmoke(mgr, true);
    float chance = x568_data.xec_;
    x6a2_24_aggressive = mgr.Random()->Range(0.f, 100.f) < chance;
    break;
  }
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    UpdateParticleEffects(mgr, 0.5f, true);
    UpdateLandingSmoke(mgr, false);
    x6a2_24_aggressive = false;
    break;
  }
}

bool CFlyingPirate::Landed(CStateManager& mgr, float arg) {
  return x450_bodyController->GetCurrentStateId() == pas::kAS_LieOnGround;
}

bool CFlyingPirate::InPosition(CStateManager& mgr, float arg) {
  CScriptCoverPoint* cover = GetCoverPoint(mgr, x6a4_currentCoverPoint);
  if (cover != nullptr) {
    const CVector3f delta = cover->GetTranslation() - GetTranslation();
    return delta.GetZ() < 0.f && delta.Magnitude() < 4.f;
  }
  return true;
}

void CFlyingPirate::Land(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    UpdateLandingSmoke(mgr, true);
    UpdateParticleEffects(mgr, 1.f, true);
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Jump, &CPatterned::TryJump, 0);
    if (x32c_animState == kAS_Repeat) {
      x450_bodyController->SetLocomotionType(pas::kLT_Relaxed);
    }
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    UpdateLandingSmoke(mgr, false);
    UpdateParticleEffects(mgr, 0.f, false);
    break;
  }
}

void CFlyingPirate::Walk(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    UpdateParticleEffects(mgr, 0.f, false);
    break;
  case kStateMsg_Update:
    if (x32c_animState != kAS_NotReady) {
      TryCommand(mgr, pas::kAS_Turn, &CPatterned::TryTurn, 0);
    }
    if (x32c_animState != kAS_Repeat) {
      x2e0_destPos = GetTargetPos(mgr);
      CVector3f delta = x2e0_destPos - GetTranslation();
      delta.SetZ(0.f);
      if (CVector3f::Dot(GetTransform().GetForward(), delta.AsNormalized()) < 0.8f) {
        x32c_animState = kAS_Ready;
      }
    }
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    x450_bodyController->SetLocomotionType(pas::kLT_Combat);
    x328_25_verticalMovement = true;
    SetMomentumWR(CVector3f::Zero());
    break;
  }
}

void CFlyingPirate::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x6a1_26_isAttackingObject = true;
    x32c_animState = kAS_Ready;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_ProjectileAttack, &CPatterned::TryProjectileAttack, 0);
    DeliverGetUp();
    break;
  case kStateMsg_Deactivate:
    x6a1_26_isAttackingObject = false;
    x32c_animState = kAS_NotReady;
    break;
  }
}

void CFlyingPirate::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    CVector3f target = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    if (x6a1_29_isMoving) {
      target = x2e0_destPos;
    } else if (const CScriptCoverPoint* cover = GetCoverPoint(mgr, x6a4_currentCoverPoint)) {
      target = cover->GetTranslation();
    }
    if (GetSearchPath()->Search(GetTranslation(), target) != CPathFindSearch::kR_Success &&
        (GetSearchPath()->GetResult() == CPathFindSearch::kR_NoDestPoint ||
         GetSearchPath()->GetResult() == CPathFindSearch::kR_NoPath)) {
      if (GetSearchPath()->FindClosestReachablePoint(GetTranslation(), target) ==
          CPathFindSearch::kR_Success) {
        GetSearchPath()->Search(GetTranslation(), target);
      }
    }
    UpdateParticleEffects(mgr, 0.5f, true);
    break;
  }
  case kStateMsg_Update: {
    CVector3f move = CVector3f::Zero();
    if (!GetSearchPath()->IsShagged() && !GetSearchPath()->IsOver()) {
      CVector3f out = GetTranslation() + GetTransform().GetForward();
      GetSearchPath()->GetSplinePointWithLookahead(out, GetTranslation(), 3.f);
      if (GetSearchPath()->SegmentOver(out)) {
        GetSearchPath()->Advance();
      }
      move = out - GetTranslation();
      if (move.CanBeNormalized()) {
        move.Normalize();
      }
    }
    move += 3.f * AvoidActors(mgr);
    if (move.CanBeNormalized()) {
      move.Normalize();
    }
    float speed = x858_ < 2.f ? 4.f : 1.f;
    const float multiplier = 1.5f * speed;
    speed = dt * (dt * (speed * x568_data.xc4_));
    x87c_ = speed * move;
    x898_ = multiplier;
    x870_ += x87c_;
    const CVector3f face = (GetTargetPos(mgr) - GetTranslation()).AsNormalized();
    x450_bodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, face, 1.f));
    UpdateCantSeePlayer(mgr);
    break;
  }
  case kStateMsg_Deactivate:
    x6a1_29_isMoving = false;
    break;
  }
}

void CFlyingPirate::Retreat(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    const CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
    CVector3f target =
        GetTranslation() - x2fc_minAttackRange * (playerPos - GetTranslation()).AsNormalized();
    float targetZ = playerPos.GetZ();
    targetZ += x568_data.x8c_flyingHeight;
    target.SetZ(targetZ);
    if (GetSearchPath()->OnPath(target) == CPathFindSearch::kR_NoSourcePoint) {
      GetSearchPath()->FindClosestReachablePoint(GetTranslation(), target);
      target[kDZ] += x568_data.x8c_flyingHeight;
      if ((playerPos - target).MagSquared() < 0.25f * x2fc_minAttackRange * x2fc_minAttackRange) {
        target =
            GetTranslation() + x2fc_minAttackRange * (playerPos - GetTranslation()).AsNormalized();
        float targetZ = playerPos.GetZ();
        targetZ += x568_data.x8c_flyingHeight;
        target.SetZ(targetZ);
        if (GetSearchPath()->OnPath(target) == CPathFindSearch::kR_NoSourcePoint) {
          GetSearchPath()->FindClosestReachablePoint(GetTranslation(), target);
          target[kDZ] += x568_data.x8c_flyingHeight;
        }
      }
    }
    GetSearchPath()->Search(GetTranslation(), target);
    UpdateParticleEffects(mgr, 0.5f, true);
    break;
  }
  case kStateMsg_Update: {
    CVector3f move = CVector3f::Zero();
    if (!GetSearchPath()->IsOver()) {
      CVector3f out = GetTranslation() + GetTransform().GetForward();
      GetSearchPath()->GetSplinePointWithLookahead(out, GetTranslation(), 3.f);
      if (GetSearchPath()->SegmentOver(out)) {
        GetSearchPath()->Advance();
      }
      move = out - GetTranslation();
      if (move.CanBeNormalized()) {
        move.Normalize();
      }
    }
    move += 3.f * AvoidActors(mgr);
    if (move.CanBeNormalized()) {
      move.Normalize();
    }
    float speed = x858_ < 2.f ? 4.f : 1.f;
    const float multiplier = 1.5f * speed;
    speed = dt * (dt * (speed * x568_data.xc4_));
    x87c_ = speed * move;
    x898_ = multiplier;
    x870_ += x87c_;
    const CVector3f face = (GetTargetPos(mgr) - GetTranslation()).AsNormalized();
    x450_bodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, face, 1.f));
    UpdateCantSeePlayer(mgr);
    break;
  }
  case kStateMsg_Deactivate:
    break;
  }
}

bool CFlyingPirate::ShouldMove(CStateManager& mgr, float arg) {
  CVector3f delta = GetTranslation() - mgr.GetPlayer()->GetTranslation();
  float random = mgr.Random()->Float();
  if (random < 0.5f) {
    random = mgr.Random()->Range(-25.f, -15.f);
  } else {
    random = mgr.Random()->Range(15.f, 25.f);
  }
  CVector3f cross = CVector3f::Cross(delta, CVector3f::Up()).AsNormalized();
  CVector3f dest = GetTranslation() + random * cross;
  dest.SetZ(mgr.GetPlayer()->GetTranslation().GetZ() + x568_data.GetFlyingHeight());
  SetDestPos(dest);
  x6a1_29_isMoving = true;
  return true;
}

bool CFlyingPirate::Stuck(CStateManager& mgr, float arg) {
  return x330_stateMachineState.GetTime() > 0.5f &&
         (CPatterned::Stuck(mgr, arg) ||
          GetSearchPath()->GetResult() != CPathFindSearch::kR_Success);
}

bool CFlyingPirate::AggressionCheck(CStateManager& mgr, float arg) { return x6a2_24_aggressive; }

void CFlyingPirate::Enraged(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Update:
    x87c_ = (dt * (dt * x568_data.xc4_)) * CVector3f::Up();
    x898_ = 1.5f;
    x870_ += x87c_;
    x450_bodyController->CommandMgr().DeliverCmd(CBCLocomotionCmd(
        CVector3f::Up(), (GetTargetPos(mgr) - GetTranslation()).AsNormalized(), 1.f));
    break;
  }
}

void CFlyingPirate::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::Dead(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Activate:
    x7a0_boneTracking.SetActive(false);
    AnimationData()->SetParticleEffectState(skEyes, false, mgr);
    CTeamAiMgr::ResetTeamAiRole(kAT_Projectile, mgr, x890_teamAiMgr, GetUniqueId(), true);
    break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

bool CFlyingPirate::AnimOver(CStateManager& mgr, float arg) {
  if (x450_bodyController->GetCurrentStateId() == pas::kAS_Death) {
    return true;
  }
  return CPatterned::AnimOver(mgr, arg);
}

void CFlyingPirate::Explode(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    RemoveMaterial(kMT_Target, kMT_Orbit, kMT_GroundCollider, kMT_Solid, mgr);
    SetMomentumWR(CVector3f::Zero());
    if (!x400_27_fadeToDeath) {
      MassiveDeath(mgr);
    }
    break;
  case kStateMsg_Update:
    if (x330_stateMachineState.GetTime() > 0.1f) {
      DeathDelete(mgr);
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CFlyingPirate::Deactivate(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    x401_30_pendingDeath = true;
  }
}

bool CFlyingPirate::ShouldRetreat(CStateManager& mgr, float arg) {
  bool shouldRetreat = false;
  if (x6a2_28_) {
    TUniqueId id = GetConnectedObject(mgr, kSS_Patrol, kSM_Follow);
    const CScriptWaypoint* waypoint = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id));
    if (waypoint == nullptr) {
      id = GetConnectedObject(mgr, kSS_Retreat, kSM_Follow);
      waypoint = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id));
    }
    if (waypoint != nullptr) {
      x6a2_28_ = false;
      shouldRetreat = true;
      x2dc_destObj = id;
      SetDestPos(waypoint->GetTranslation());
      x2ec_reflectedDestPos = GetTranslation();
      x328_24_inPosition = false;
      x6a1_29_isMoving = true;
      x6a0_26_hearShot = false;
      x6a0_28_ = false;
      x400_24_hitByPlayerProjectile = false;
    }
  }
  return shouldRetreat;
}

bool CFlyingPirate::FireProjectile(CStateManager& mgr, float dt) {
  bool fired = false;
  const CTransform4f xf = GetLctrTransform(x7e0_gunSegId);
  if (!IsAlive()) {
    LaunchProjectile(xf, mgr, 8, CWeapon::kPA_None, false,
                     rstl::optional_object< TLockedToken< CGenDescription > >(),
                     CSfxManager::kInternalInvalidSfxId, false, CVector3f(1.f, 1.f, 1.f));
    fired = true;
  } else {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(x7e8_targetId))) {
      CVector3f origin = actor->GetTranslation();
      const CPlayer& player = *mgr.GetPlayer();
      if (x7e8_targetId == player.GetUniqueId()) {
        origin = ProjectileInfo()->PredictInterceptPos(
            xf.GetTranslation(), player.GetAimPosition(mgr, 0.f), player, true, dt);
      }
      CVector3f delta = origin - xf.GetTranslation();
      float distance = delta.Magnitude();
      delta *= 1.f / distance;
      float dot = CVector3f::Dot(xf.GetForward(), delta);
      if (dot > 0.707f || (distance < 6.f && dot > 0.5f)) {
        if (LineOfSightTest(mgr, xf.GetTranslation(), origin,
                            CMaterialList(kMT_Player, kMT_ProjectilePassthrough))) {
          origin +=
              GetTransform().Rotate(x7ec_burstFire.GetDistanceCompensatedError(distance, 6.f));
          const CTransform4f aimXf =
              CTransform4f::LookAt(xf.GetTranslation(), origin, CVector3f::Up());
          LaunchProjectile(aimXf, mgr, 8, CWeapon::kPA_None, false,
                           rstl::optional_object< TLockedToken< CGenDescription > >(),
                           CSfxManager::kInternalInvalidSfxId, false, CVector3f(1.f, 1.f, 1.f));
          fired = true;
        }
      }
    }
  }
  if (fired) {
    const CPASDatabase& database = x450_bodyController->GetPASDatabase();
    const CPASAnimParmData parms(pas::kAS_AdditiveReaction, CPASAnimParm::FromEnum(2));
    const rstl::pair< float, int > anim = database.FindBestAnimation(parms, *mgr.Random(), -1);
    if (anim.first > 0.f) {
      ModelData()->AnimationData()->AddAdditiveAnimation(anim.second, 1.f, false, true);
    }
    CSfxManager::AddEmitter(x568_data.x34_gunSfx, GetTranslation(), CVector3f::Zero(), true, false,
                            CSfxManager::kMedPriority, CSfxManager::kAllAreas);
  }
  const bool result = fired;
  return result;
}

CProjectileInfo* CFlyingPirate::ProjectileInfo() { return &x568_data.GunProjectileInfo(); }

void CFlyingPirate::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                    EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case kUE_Projectile: {
    CProjectileInfo& info = x6a1_26_isAttackingObject ? x568_data.x60_altProjectileInfo2
                                                      : x568_data.x38_altProjectileInfo1;
    if (info.Token().TryCache() && mgr.CanCreateProjectile(GetUniqueId(), kWT_AI, 16)) {
      const CTransform4f xf = GetLctrTransform(node.GetLocatorName());
      CEnergyProjectile* projectile = rs_new CEnergyProjectile(
          true, info.Token(), kWT_AI, xf, kMT_Character, info.GetDamage(), mgr.AllocateUniqueId(),
          GetCurrentAreaId(), GetUniqueId(),
          x6a1_26_isAttackingObject ? x85c_attackObjectId : mgr.GetPlayer()->GetUniqueId(),
          CWeapon::kPA_None, false, CVector3f(1.f, 1.f, 1.f),
          rstl::optional_object< TLockedToken< CGenDescription > >(),
          CSfxManager::kInternalInvalidSfxId, false);
      if (projectile != nullptr) {
        mgr.AddObject(projectile);
        if (!x6a1_26_isAttackingObject) {
          projectile->SetCameraShake(CCameraShakeData::HardHorizShakeDistance(
              projectile->GetTranslation(), 0.3f, 0.2f, 50.f));
          if (x6a0_25_isAquaPirate) {
            projectile->SetMinHomingDistance(x568_data.xf0_projectileHomingDistance);
          }
        }
      }
    }
    handled = true;
    break;
  }
  case kUE_DeGenerate:
  case kUE_BecomeRagDoll:
    if (x89c_ragDoll.null() && HealthInfo(mgr)->GetHP() <= 0.f) {
      x89c_ragDoll = rs_new CFlyingPirateRagDoll(mgr, this, x568_data.xc8_ragDollSfx1,
                                                 x568_data.xca_ragDollSfx2);
    }
    handled = true;
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CFlyingPirate::CalculateRenderBounds() {
  if (!x89c_ragDoll.null() && x89c_ragDoll->IsPrimed()) {
    const CAABox bounds = x89c_ragDoll->CalculateRenderBounds();
    const CVector3f scale = 0.25f * GetModelData()->GetScale();
    SetRenderBounds(CAABox(bounds.GetMinPoint() - scale, bounds.GetMaxPoint() + scale));
  } else {
    CActor::CalculateRenderBounds();
  }
}

void CFlyingPirate::Think(const float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  if (!x450_bodyController->GetIsActive()) {
    x450_bodyController->Activate(mgr);
    if (x6a0_24_isFlyingPirate) {
      x450_bodyController->SetLocomotionType(pas::kLT_Combat);
      x328_25_verticalMovement = true;
    }
  }
  const bool inCineCam = mgr.CameraManager()->IsInCinematicCamera();
  if (inCineCam && !x6a1_24_prevInCineCam) {
    RemoveMaterial(kMT_AIBlock, mgr);
    CMaterialList include = GetMaterialFilter().GetIncludeList();
    include.Remove(kMT_AIBlock);
    SetMaterialFilter(
        CMaterialFilter::MakeIncludeExclude(include, GetMaterialFilter().GetExcludeList()));
  } else if (!inCineCam && x6a1_24_prevInCineCam) {
    AddMaterial(kMT_AIBlock, mgr);
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
        GetMaterialFilter().GetIncludeList().Union(CMaterialList(kMT_AIBlock)),
        GetMaterialFilter().GetExcludeList()));
  }
  x6a1_24_prevInCineCam = inCineCam;
  for (int i = 0; i < x684_particleGens.size(); ++i) {
    x684_particleGens[i]->Update(dt);
  }
  x78c_ = CMath::Max(0.f, x78c_ - dt);
  if (IsAlive()) {
    x854_ += dt;
    x858_ += dt;
    if (x6a0_30_) {
      x858_ = 0.f;
      x6a0_30_ = false;
    }
    if (x400_24_hitByPlayerProjectile) {
      x854_ = 0.f;
      x400_24_hitByPlayerProjectile = false;
    }
    if (!x6a0_25_isAquaPirate && InFluidId() != kInvalidUniqueId) {
      if (const CScriptWater* water =
              TCastToConstPtr< CScriptWater >(mgr.GetObjectById(InFluidId()))) {
        const CVector3f& pos = GetTranslation();
        const float height = pos.GetZ();
        if (water->GetTriggerBoundsWR().GetMaxPoint().GetZ() > 2.f + height) {
          x401_30_pendingDeath = true;
        }
      }
    }
  }
  if (x450_bodyController->GetPercentageFrozen() == 0.f) {
    x86c_ = CMath::Max(0.f, x86c_ - dt);
    x860_ = CMath::Max(0.f, x860_ - dt);
    x888_ = CMath::Max(0.f, x888_ - dt);
    if (x6a0_31_cantSeePlayer) {
      x7d8_ += dt;
    } else {
      x7d8_ = 0.f;
    }
    if (IsAlive()) {
      CheckForProjectiles(mgr);
    }
    if (!x6a0_25_isAquaPirate &&
        (!IsAlive() ||
         (x450_bodyController->GetBodyStateInfo().GetCurrentState()->CanShoot() && x6a0_28_ &&
          x450_bodyController->GetCurrentStateId() != pas::kAS_ProjectileAttack &&
          !x6a1_31_stopped && !x450_bodyController->IsElectrocuting()))) {
      if (x7ec_burstFire.GetBurstType() != -1) {
        x7e4_ -= dt;
        if (x7e4_ < 0.f) {
          int type = x7ec_burstFire.GetBurstType() & ~1;
          if (!PlayerSpot(mgr, 0.f)) {
            ++type;
          }
          x7ec_burstFire.SetBurstType(type);
          x7ec_burstFire.Start(mgr);
          if (IsAlive()) {
            x7e4_ = x308_attackTimeVariation * mgr.Random()->Float() + x304_averageAttackTime;
            const CVector3f& delta =
                (GetBoundingBox().GetCenterPoint() - mgr.GetPlayer()->GetAimPosition(mgr, 0.f))
                    .AsNormalized();
            if (CVector3f::Dot(delta, mgr.GetPlayer()->GetTransform().GetForward()) < 0.9f) {
              const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
              for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
                const CSpacePirate* pirate = PATTERNED_CAST_TO(CSpacePirate, const_cast< CEntity* >(list[i]));
                if (pirate != nullptr && pirate->GetEnableAim() &&
                    pirate->GetCurrentAreaId() == GetCurrentAreaId()) {
                  x7e4_ += 0.2f;
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
          const float variation = x568_data.xd4_;
          const float delay = x568_data.xd0_;
          x7ec_burstFire.SetTimeToNextShot(variation * (mgr.Random()->Float() - 0.5f) + delay);
        }
      }
    }
  }
  const bool ragDollNull = x89c_ragDoll.null();
  if (ragDollNull || !x89c_ragDoll->IsPrimed()) {
    if (IsAlive() && !x450_bodyController->IsFrozen() && !x450_bodyController->IsElectrocuting() &&
        x6a0_28_ && x89c_ragDoll.null() && !IsAquaPirate()) {
      x450_bodyController->CommandMgr().DeliverCmd(CBCAdditiveAimCmd());
      const CVector3f target = GetTargetPos(mgr);
      const CVector3f delta = target - GetTranslation();
      const CVector3f aim = GetTransform().TransposeRotate(delta);
      x450_bodyController->CommandMgr().DeliverAdditiveTargetVector(aim);
    } else {
      x450_bodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_AdditiveIdle));
    }
    if (x870_.MagSquared() > 0.f) {
      const float mag = x870_.Magnitude();
      const CVector3f direction = (1.f / mag) * x870_;
      float damping = 0.2f;
      if (x87c_.MagSquared() == 0.f) {
        damping *= 3.f;
      }
      const float speed = -(dt * (mag * (damping * mag)) - mag);
      x870_ = speed * direction;
    }
    if (IsAlive() && x450_bodyController->GetCurrentStateId() != pas::kAS_LoopReaction &&
        x450_bodyController->GetCurrentStateId() != pas::kAS_Hurled &&
        x450_bodyController->GetCurrentStateId() != pas::kAS_LieOnGround &&
        x450_bodyController->GetCurrentStateId() != pas::kAS_Getup) {
      ApplyImpulseWR(GetMass() * x870_, CAxisAngle::Identity());
    } else {
      x870_ = CVector3f::Zero();
      x87c_ = CVector3f::Zero();
    }
    const CSfxHandle handle = GetSfxHandle();
    if (handle) {
      x898_ = CMath::Clamp(1.f, x898_, 1.999f);
      float change = x898_ - x894_pitchBend;
      change = CMath::Clamp(-dt, change, dt);
      x894_pitchBend += change;
      CSfxManager::PitchBend(handle, static_cast< int >(8192.f * x894_pitchBend));
    }
    x87c_ = CVector3f::Zero();
    x898_ = 1.f;
    CPatterned::Think(dt, mgr);
    CVector3f movement = x87c_;
    if (movement.CanBeNormalized()) {
      movement.Normalize();
    }
    const CVector3f& offset = CMath::Min(0.333f * x87c_.Magnitude(), 0.333f) * movement;
    const CVector3f targetUp = (CVector3f::Up() + offset).AsNormalized();
    const CVector3f& currentUp = GetTransform().GetUp();
    const float angle = CMath::AbsF(CVector3f::GetAngleDiff(currentUp, targetUp));
    if (angle > 0.f) {
      const float maxStep = 30.f * ((M_PIF * dt) / 180.f);
      const float step = CMath::Min(angle, maxStep);
      const CVector3f up = (step * targetUp + (angle - step) * currentUp).AsNormalized();
      CVector3f right = CVector3f::Cross(GetTransform().GetForward(), up);
      const CVector3f forward = CVector3f::Cross(up, right).AsNormalized();
      right = CVector3f::Cross(forward, up);
      SetTransform(CTransform4f::FromColumns(right, forward, up, GetTranslation()));
    }
    if (!x450_bodyController->IsFrozen()) {
      x7a0_boneTracking.Update(dt);
    }
  } else {
    UpdateAlphaDelta(dt, mgr);
    UpdateDamageColor(dt);
  }
  if (!ragDollNull) {
    if (!x89c_ragDoll->IsPrimed()) {
      SetMuted(true);
      SetMuted(false);
      x89c_ragDoll->Prime(mgr, GetTransform(), *ModelData());
      const CVector3f& pos = GetTranslation();
      SetTransform(CTransform4f::Identity());
      SetTranslation(pos);
      x450_bodyController->SetPlaybackRate(0.f);
    } else {
      float waterTop = -FLT_MAX;
      if (InFluidId() != kInvalidUniqueId) {
        if (const CScriptWater* water =
                TCastToConstPtr< CScriptWater >(mgr.GetObjectById(InFluidId()))) {
          waterTop = water->GetTriggerBoundsWR().GetMaxPoint().GetZ();
        }
      }
      x89c_ragDoll->Update(mgr, dt * CalcDyingThinkRate(), waterTop);
      ModelData()->AdvanceParticles(GetTransform(), dt, mgr);
    }
    if (x89c_ragDoll->IsOver() && !GetFadeToDeath()) {
      SetFadeToDeath(true);
      x3e8_alphaDelta = -1.f / 3.f;
      SetVelocityWR(CVector3f::Zero());
      SetMomentumWR(CVector3f::Zero());
      x870_ = CVector3f::Zero();
    }
    if (!x89c_ragDoll.null()) {
      const bool wasPositive = x88c_ragDollTimer > 0.f;
      x88c_ragDollTimer -= dt;
      if (x88c_ragDollTimer < 2.f) {
        if (static_cast< int >(x89c_ragDoll->GetImpactCount()) > 2) {
          x88c_ragDollTimer = CMath::Min(0.1f, x88c_ragDollTimer);
        }
        if (wasPositive && x88c_ragDollTimer <= 0.f) {
          x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), rstl::string_l("Explode"));
        }
      }
    }
  }
}

void CFlyingPirate::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  for (int i = 0; i < x684_particleGens.size(); ++i) {
    CElementGen* gen = x684_particleGens[i].get();
    if (frustum.BoxInFrustumPlanes(gen->GetBounds())) {
      gpRender->AddParticleGen(*gen);
    }
  }
  CPatterned::AddToRenderer(frustum, mgr);
}

void CFlyingPirate::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  if (!x89c_ragDoll.null() && x89c_ragDoll->IsPrimed()) {
    x89c_ragDoll->PreRender(GetTranslation(), *ModelData());
  }
  CPatterned::PreRender(mgr, frustum);
  x7a0_boneTracking.PreRender(mgr, *AnimationData(), GetTransform(), GetModelScale(),
                              *x450_bodyController);
}

bool CFlyingPirate::KnockbackWhenFrozen() const { return false; }

float CFlyingPirate::GetGravityConstant() const {
  return x6a0_25_isAquaPirate ? skAquaGravityConstant : skGravityConstant;
}

bool CFlyingPirate::IsListening() const { return true; }

CPathFindSearch* CFlyingPirate::GetSearchPath() { return &x6a8_pathFindSearch; }

const float CFlyingPirate::skGravityConstant = 50.f;
const float CFlyingPirate::skAquaGravityConstant = 5.f;
const float CFlyingPirate::skFloatingGravityConstant = -3.f;
