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
: mAggressionCheck(in.ReadFloat())
, mCoverCheck(in.ReadFloat())
, mSearchRadius(in.ReadFloat())
, mFallBackCheck(in.ReadFloat())
, mFallBackRadius(in.ReadFloat())
, mHearingRadius(in.ReadFloat())
, mFlags(in.ReadLong())
, x1c_(in.ReadBool())
, mProjectile(in)
, mSound_Projectile(CSfxManager::TranslateSFXID(in.ReadLong()))
, mBladeDamage(in)
, mKneelAttackChance(in.ReadFloat())
, mKneelAttackShot(in)
, mDodgeCheck(in.ReadFloat())
, mSound_Impact(CSfxManager::TranslateSFXID(in.ReadLong()))
, mAverageNextShotTime(in.ReadFloat())
, mNextShotTimeVariation(in.ReadFloat())
, mSound_Alert(CSfxManager::TranslateSFXID(in.ReadLong()))
, mGunTrackDelay(in.ReadFloat())
, mFirstBurstCount(in.ReadLong())
, mCloakOpacity(in.ReadFloat())
, mMaxCloakOpacity(in.ReadFloat())
, mDodgeDelayTimeMin(in.ReadFloat())
, mDodgeDelayTimeMax(in.ReadFloat())
, mSound_Hurled(CSfxManager::TranslateSFXID(in.ReadLong()))
, mSound_Death(CSfxManager::TranslateSFXID(in.ReadLong()))
, xc4_(propCount >= 35 ? in.ReadFloat() : 0.2f)
, mAvoidDistance(propCount >= 36 ? in.ReadFloat() : 8.f) {
  mProjectile.Token().Lock();
}

CPirateRagDoll::CPirateRagDoll(CStateManager& mgr, CSpacePirate* pirate, ushort thudSfx, uint flags)
: CRagDoll(-pirate->GetGravityConstant(), -pirate->GetFloatingGravityConstant(), 8.f, flags)
, mSpacePirate(pirate)
, mThudSfx(thudSfx)
, mSfxTimer(0.f)
, mLastSFXPos(CVector3f::Zero())
, mTorsoImpulse(CVector3f::Zero())
, mInitSfx(true) {
  mSpacePirate->RemoveMaterial(kMT_Solid, kMT_AIBlock, kMT_GroundCollider, mgr);
  mSpacePirate->HealthInfo(mgr)->SetHP(-1.f);
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
  AddMinLengthConstraint(1, 8, mLengthConstraints[2].GetLength());
  AddMinLengthConstraint(1, 11, mLengthConstraints[3].GetLength());
  AddMinLengthConstraint(1, 2, mLengthConstraints[1].GetLength() * 0.9f);
  AddMinLengthConstraint(1, 5, mLengthConstraints[4].GetLength() * 0.9f);
  AddMinLengthConstraint(1, 4, mLengthConstraints[0].GetLength() * 2.5f);
  AddMinLengthConstraint(1, 7, mLengthConstraints[0].GetLength() * 2.5f);
  AddMinLengthConstraint(4, 2, mLengthConstraints[5].GetLength());
  AddMinLengthConstraint(7, 5, mLengthConstraints[7].GetLength());
  AddMinLengthConstraint(
      3, 5, mLengthConstraints[5].GetLength() * 0.5f + mLengthConstraints[9].GetLength());
  AddMinLengthConstraint(
      6, 2, mLengthConstraints[7].GetLength() * 0.5f + mLengthConstraints[9].GetLength());
  AddMinLengthConstraint(
      4, 5, mLengthConstraints[5].GetLength() * 0.5f + mLengthConstraints[9].GetLength());
  AddMinLengthConstraint(
      7, 2, mLengthConstraints[7].GetLength() * 0.5f + mLengthConstraints[9].GetLength());
  AddMinLengthConstraint(4, 7, mLengthConstraints[9].GetLength());
  AddMinLengthConstraint(4, 8, mLengthConstraints[14].GetLength());
  AddMinLengthConstraint(7, 11, mLengthConstraints[14].GetLength());
  AddMinLengthConstraint(10, 8, mLengthConstraints[15].GetLength());
  AddMinLengthConstraint(13, 11, mLengthConstraints[17].GetLength());
  AddMinLengthConstraint(
      9, 2, mLengthConstraints[15].GetLength() * 0.707f + mLengthConstraints[10].GetLength());
  AddMinLengthConstraint(12, 5,
                         mLengthConstraints[17].GetLength() * 0.707f +
                             mLengthConstraints[13].GetLength());
  AddMinLengthConstraint(9, 11, mLengthConstraints[15].GetLength());
  AddMinLengthConstraint(12, 8, mLengthConstraints[17].GetLength());
  AddMinLengthConstraint(
      10, 0, mLengthConstraints[2].GetLength() + mLengthConstraints[15].GetLength());
  AddMinLengthConstraint(
      13, 0, mLengthConstraints[3].GetLength() + mLengthConstraints[17].GetLength());
  AddMinLengthConstraint(10, 13, mLengthConstraints[14].GetLength());
  AddMinLengthConstraint(9, 12, mLengthConstraints[14].GetLength());
  AddMinLengthConstraint(10, 12, mLengthConstraints[14].GetLength());
  AddMinLengthConstraint(13, 9, mLengthConstraints[14].GetLength());
  AddMaxLengthConstraint(10, 13, mLengthConstraints[14].GetLength() * 5.f);
  AddJointConstraint(8, 2, 5, 8, 9, 10);
  AddJointConstraint(11, 2, 5, 11, 12, 13);
  AddJointConstraint(2, 11, 5, 2, 3, 4);
  AddJointConstraint(5, 2, 8, 5, 6, 7);
  for (AUTO(conn, mSpacePirate->GetConnectionList().begin());
       conn != mSpacePirate->GetConnectionList().end(); ++conn) {
    if (conn->mState == kSS_Modify && conn->mMsg == kSM_Follow) {
      TUniqueId id = mgr.GetIdForScript(conn->mObjId);
      if (const CScriptWaypoint* wp = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id))) {
        mWaypoints.push_back(id);
        mWpParticleIdxs.push_back(wp->GetAnimation());
        if (mWaypoints.capacity() - mWaypoints.size() <= 0) {
          break;
        }
      }
    }
  }
}

void CPirateRagDoll::Prime(CStateManager& mgr, const CTransform4f& xf, CModelData& mData) {
  const CAABox& bounds = mSpacePirate->GetBaseBoundingBox();
  CVector3f max = bounds.GetMaxPoint();
  max.SetZ((bounds.GetMaxPoint().GetZ() - bounds.GetMinPoint().GetZ()) * 0.5f +
           bounds.GetMinPoint().GetZ());
  mSpacePirate->SetBoundingBox(CAABox(bounds.GetMinPoint(), max));
  CRagDoll::Prime(mgr, xf, mData);
}

void CPirateRagDoll::Update(CStateManager& mgr, float dt, float waterTop) {
  if (!IsOver() || WillContinueSmallMovements()) {
    if (mSpacePirate->mAttachedActor != kInvalidUniqueId) {
      float delta = mParticles[2].GetPosition().GetZ() - mParticles[5].GetPosition().GetZ();
      if (delta * delta > 0.0625f) {
        CVector3f adjustment(0.f, 0.f, (delta > 0.f ? delta - 0.25f : delta + 0.25f) * 0.1f);
        mParticles[2].Position() = mParticles[2].GetPosition() - adjustment;
        mParticles[5].Position() = mParticles[5].GetPosition() + adjustment;
      }
      delta = mParticles[0].GetPosition().GetZ() -
              (mParticles[8].GetPosition().GetZ() + mParticles[11].GetPosition().GetZ()) * 0.5f;
      if (delta * delta > 0.0625f) {
        CVector3f adjustment(0.f, 0.f, (delta > 0.f ? delta - 0.25f : delta + 0.25f) * 0.1f);
        mParticles[0].Position() = mParticles[0].GetPosition() - adjustment;
        adjustment[kDZ] = 0.5f * adjustment[kDZ];
        mParticles[8].Position() = mParticles[8].GetPosition() + adjustment;
        mParticles[11].Position() = mParticles[11].GetPosition() + adjustment;
      }
    }
    CVector3f oldCenter = mParticles[8].GetPosition() * 0.25f +
                          mParticles[11].GetPosition() * 0.25f +
                          mParticles[0].GetPosition() * 0.5f;
    oldCenter[kDZ] =
        CMath::Min(mParticles[8].GetPosition().GetZ() - mParticles[8].GetRadius(),
                   mParticles[11].GetPosition().GetZ() - mParticles[11].GetRadius());
    oldCenter[kDZ] = CMath::Min(mParticles[0].GetPosition().GetZ() - mParticles[0].GetRadius(),
                                oldCenter[kDZ]);
    if (oldCenter.GetZ() < 0.5f + waterTop) {
      mTorsoImpulse *= 1000.f;
    }
    CVector3f acceleration = mTorsoImpulse * 0.333f * (1.f / mSpacePirate->GetMass());
    mParticles[11].Velocity() += acceleration;
    mParticles[8].Velocity() += acceleration;
    mParticles[0].Velocity() += acceleration;
    mTorsoImpulse = CVector3f::Zero();
    CRagDoll::Update(mgr, dt, waterTop);
    for (int i = 0; i < mWaypoints.size(); ++i) {
      if (const CScriptWaypoint* wp =
              static_cast< const CScriptWaypoint* >(mgr.GetObjectById(mWaypoints[i]))) {
        if (wp->GetActive()) {
          mParticles[mWpParticleIdxs[i]].Position() = wp->GetTranslation();
        }
      }
    }
    CVector3f newCenter = mParticles[8].GetPosition() * 0.25f +
                          mParticles[11].GetPosition() * 0.25f +
                          mParticles[0].GetPosition() * 0.5f;
    newCenter[kDZ] =
        CMath::Min(mParticles[8].GetPosition().GetZ() - mParticles[8].GetRadius(),
                   mParticles[11].GetPosition().GetZ() - mParticles[11].GetRadius());
    newCenter[kDZ] = CMath::Min(mParticles[0].GetPosition().GetZ() - mParticles[0].GetRadius(),
                                newCenter[kDZ]);
    CVector3f velocity = (newCenter - oldCenter) / dt;
    mSpacePirate->SetTransform(CTransform4f::Identity());
    mSpacePirate->SetTranslation(newCenter);
    mSpacePirate->SetVelocityWR(velocity);
    mSfxTimer -= dt;
    float impactVel = mImpactVel;
    if (impactVel > 2.5f && mSfxTimer < 0.f) {
      CVector3f delta = mSpacePirate->GetTranslation() - mLastSFXPos;
      if (mInitSfx || delta.MagSquared() > 0.1f) {
        float volume = CMath::Min(25.f * impactVel, 127.f);
        CSfxManager::AddEmitter(mThudSfx, mSpacePirate->GetTranslation(), CVector3f::Zero(),
                                CCast::ToUint8(volume), true, false);
        mSfxTimer = mgr.Random()->Float() * 0.222f + 0.222f;
        mInitSfx = false;
        mLastSFXPos = mSpacePirate->GetTranslation();
      }
    }
  } else {
    mSpacePirate->SetMomentumWR(CVector3f::Zero());
    mSpacePirate->Stop();
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
        0.5f * (mParticles[8].GetPosition() + mParticles[11].GetPosition()) - pos;
    const CVector3f& scale = CVector3f(mData.GetScale());
    builder.Insert(rootId,
                   CVector3f(rootOffset.GetX() / scale.GetX(), rootOffset.GetY() / scale.GetY(),
                             rootOffset.GetZ() / scale.GetZ()));
    CVector3f right = mParticles[2].GetPosition() - mParticles[5].GetPosition();
    CVector3f up = (mParticles[0].GetPosition() -
                    (mParticles[8].GetPosition() + mParticles[11].GetPosition()) * 0.5f)
                       .AsNormalized();
    CVector3f forward = CVector3f::Cross(up, right).AsNormalized();
    right = CVector3f::Cross(forward, up);
    CMatrix3f matrix(right, forward, up);
    CQuaternion rootRot = CQuaternion::FromMatrix(matrix.GetTranspose());
    builder.Insert(rootId, rootRot);
    const CCharLayoutInfo& layout = *animData->GetCharLayoutInfo();
    if (mSpacePirate->mAttachedActor == kInvalidUniqueId) {
      CVector3f rest = layout.GetFromParentUnrotated(mParticles[1].GetBone());
      CVector3f neck = mParticles[1].GetPosition() - mParticles[0].GetPosition();
      neck = rootRot.BuildInverted().Transform(neck);
      CQuaternion neckRot = CQuaternion::ShortestRotationArc(rest, neck);
      builder.Insert(mParticles[1].GetBone(), neckRot);
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
    builder.Insert(mParticles[10].GetBone(), ankleRot);
    builder.Insert(mParticles[13].GetBone(), ankleRot);
    animData->SetPoseBuilderValid(false);
  }
}

CSpacePirate::CSpacePirate(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CTransform4f& xf, const CModelData& mData,
                           const CActorParameters& aParams, const CPatternedInfo& pInfo,
                           CInputStream& in, int propCount)
: CPatterned(kC_SpacePirate, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Ground, kCT_One,
             kBT_BiPedal, aParams, kCS_Medium)
, mPirateData(in, propCount)
, mPendingAmbush(mPirateData.mFlags & 0x1)
, mCeilingAmbush(mPirateData.mFlags & 0x2)
, mNonAggressive(mPirateData.mFlags & 0x4)
, mMelee(mPirateData.mFlags & 0x8)
, mNoShuffleCloseCheck(mPirateData.mFlags & 0x10)
, mOnlyAttackInRange(mPirateData.mFlags & 0x20)
, x634_30_(mPirateData.mFlags & 0x40)
, mNoKnockbackImpulseReset(mPirateData.mFlags & 0x80)
, mNoMeleeAttack(mPirateData.mFlags & 0x200)
, mBreakAttack(mPirateData.mFlags & 0x400)
, mSeated(mPirateData.mFlags & 0x1000)
, mShadowPirate(mPirateData.mFlags & 0x2000)
, mAlertBeforeCloak(mPirateData.mFlags & 0x4000)
, mNoBreakDodge(mPirateData.mFlags & 0x8000)
, mFloatingCorpse(mPirateData.mFlags & 0x10000)
, mRagdollNoAiCollision(mPirateData.mFlags & 0x20000)
, mTrooper(mPirateData.mFlags & 0x40000)
, mHearNoise(false)
, mEnableMeleeAttack(false)
, x636_27_(false)
, x636_28_(false)
, mEnableRetreat(false)
, mShuffleClose(false)
, mEnablePatrol(false)
, mEnableAim(false)
, mHearPlayerFire(false)
, mInProjectilePath(false)
, mNoPlayerLos(false)
, mInWallHang(false)
, mJumpVelSet(false)
, mPrevInCineCam(false)
, mPendingFrenzyChance(false)
, mAppliedBladeDamage(false)
, mAlwaysAggressive(false)
, mCoverCheck(false)
, mEnableDodge(false)
, mNoPlayerDodge(false)
, mAllEnergyDrained(false)
, mMayStartAttack(false)
, x639_24_(false)
, mUseJumpBackJump(false)
, mStarted(false)
, mInRange(false)
, mSatUp(false)
, mCloseMelee(false)
, mSentAttackMsg(false)
, mNormalDodge(false)
, mFrenzyFrames(0)
, mCoverPoint(kInvalidUniqueId)
, mPreviousCoverPoint(kInvalidUniqueId)
, mSteeringSpeed(1.f)
, mTargetDelta(CVector3f::Forward())
, mCoverPointRearDir(CVector3f::Zero())
, mPathFindSearch(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, mSteeringDelayTimer(0.f)
, x74c_(0)
, mInitialHP(pInfo.GetHealthInfo().GetHP())
, mCoverRange(0.f)
, mHeadSeg(CSegId::Invalid())
, x75c_(0)
, mTaunt(pas::kTT_Invalid)
, mBoneTracking(*GetAnimationData(), rstl::string_l("Head_1"), CMath::Deg2Rad(70.f),
                    CMath::Deg2Rad(180.f), kBTF_None)
, mCoverDir(pas::kCD_Invalid)
, mIntoJumpDist(1.f)
, mEyeHeight(2.f)
, mTimeNoPlayerLos(0.f)
, mCantSeePlayerCycleCounter(0)
, mAttachedActor(kInvalidUniqueId)
, mGunSeg(CSegId::Invalid())
, mElbowSeg(CSegId::Invalid())
, mWristSeg(CSegId::Invalid())
, mSwooshSeg(CSegId::Invalid())
, mAttackRemTime(1.f)
, mTargetId(kInvalidUniqueId)
, mBurstFire(skBursts, mPirateData.mFirstBurstCount)
, mJumpHeight(3.f)
, mPatrolDestPos(CVector3f::Zero())
, mSkidDir(pas::kSD_Invalid)
, mStrafeDelayTimer(0.f)
, mMeleeSeverity(pas::kS_Invalid)
, mJumpPoint(kInvalidUniqueId)
, mDodgeDir(pas::kSD_Invalid)
, mDodgeDist(3.f)
, mBreakDodgeDist(3.f)
, mTimeSinceHitByPlayer(FLT_MAX)
, mLowHealthFrenzyTimer(FLT_MAX)
, mRagdollDelayTimer(0.f)
, mRagDoll(nullptr)
, mIkChain()
, mCloakDelayTimer(0.f)
, mElectricParticleTimer(0.f)
, mCloakStepTime(0.f)
, mShadowPirateAlpha(0.5f)
, mMinCloakAlpha(mPirateData.mCloakOpacity)
, mMaxCloakAlpha(mPirateData.mMaxCloakOpacity)
, mDodgeDelayTimer(mPirateData.mDodgeDelayTimeMin)
, mAimDelayTimer(mPirateData.mGunTrackDelay)
, mTeamAiMgrId(kInvalidUniqueId)
, mTrooperColor(CColor::White())
, mHeldPosition(CVector2f::Zero())
, mHoldPositionTime(0.f)
, mLeashTimer(0.f) {
  CAnimData* animData = AnimationData();
  mHeadSeg = animData->GetLocatorSegId(rstl::string_l("Head_1"));
  mGunSeg = animData->GetLocatorSegId(rstl::string_l("R_gun_LCTR"));
  mElbowSeg = animData->GetLocatorSegId(rstl::string_l("R_elbow"));
  mWristSeg = animData->GetLocatorSegId(rstl::string_l("R_wrist"));
  mSwooshSeg = animData->GetLocatorSegId(rstl::string_l("Swoosh_LCTR"));

  if (!mOnlyAttackInRange) {
    const CPASAnimParmData jump(pas::kAS_Jump, CPASAnimParm::FromEnum(0),
                                CPASAnimParm::FromEnum(0));
    const CVector3f& scale = CVector3f(GetModelData()->GetScale());
    mIntoJumpDist = scale[kDY] * GetAnimationDistance(jump);
    const CPASAnimParmData dodge(pas::kAS_Step, CPASAnimParm::FromEnum(3),
                                 CPASAnimParm::FromEnum(1));
    mDodgeDist = GetModelData()->GetScale()[kDX] * GetAnimationDistance(dodge);
    const CPASAnimParmData breakDodge(pas::kAS_Step, CPASAnimParm::FromEnum(3),
                                      CPASAnimParm::FromEnum(2));
    mBreakDodgeDist = GetModelData()->GetScale()[kDX] * GetAnimationDistance(breakDodge);
  } else {
    BodyCtrl()->BodyStateInfo().SetLocoAnimChangeAtEndOfAnimOnly(true);
  }

  const CAABox& baseAABB = GetBaseBoundingBox();
  mEyeHeight = (baseAABB.GetMaxPoint().GetZ() - baseAABB.GetMinPoint().GetZ()) * 0.6f;

  if (ActorLights()) {
    ActorLights()->SetAmbienceGenerated(false);
  }

  KnockBackCtrl().sub80233d40(3, 3.f, FLT_MAX);
  KnockBackCtrl().SetLocomotionDuringElectrocution(true);

  if (mOnlyAttackInRange) {
    KnockBackCtrl().SetCreatureSize(kCS_Small);
  } else if (mTrooper &&
             static_cast< const CSpacePirate* >(this)->GetDamageVulnerability()->WeaponHurts(
                 CWeaponMode(kWT_Plasma), CDamageVulnerability::kRD_No)) {
    KnockBackCtrl().SetCreatureSize(kCS_Large);
  }

  if (!BodyCtrl()->HasBodyState(pas::kAS_AdditiveAim)) {
    mMelee = true;
  }

  if (mTrooper) {
    const CDamageVulnerability& vulnerability =
        *static_cast< const CSpacePirate* >(this)->GetDamageVulnerability();
    if (vulnerability.WeaponHurts(CWeaponMode(kWT_Plasma), CDamageVulnerability::kRD_No)) {
      mTrooperColor = CColor(uchar(254), uchar(0), uchar(40), uchar(255));
    } else if (vulnerability.WeaponHurts(CWeaponMode(kWT_Ice), CDamageVulnerability::kRD_No)) {
      mTrooperColor = CColor::White();
    } else if (vulnerability.WeaponHurts(CWeaponMode(kWT_Power), CDamageVulnerability::kRD_No)) {
      mTrooperColor = CColor(uchar(253), uchar(239), uchar(86), uchar(255));
    } else if (vulnerability.WeaponHurts(CWeaponMode(kWT_Wave), CDamageVulnerability::kRD_No)) {
      mTrooperColor = CColor(uchar(198), uchar(14), uchar(255), uchar(255));
    }
  }
}

ENTITY_ACCEPT_IMPL(CSpacePirate)

void CSpacePirate::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  if (mInWallHang || mCeilingAmbush) {
    switch (msg) {
    case kSM_Falling:
      if ((mInWallHang && BodyCtrl()->GetCurrentStateId() == pas::kAS_WallHang &&
           !BodyCtrl()->GetBodyStateInfo().GetCurrentState()->ApplyGravity()) ||
          (mCeilingAmbush && (BodyCtrl()->GetCurrentStateId() == pas::kAS_Locomotion ||
                                     (BodyCtrl()->GetCurrentStateId() == pas::kAS_Jump &&
                                      !BodyCtrl()->GetBodyStateInfo().IsInAir())))) {
        CPhysicsActor::Stop();
        SetMomentumWR(CVector3f::Zero());
        return;
      }
      break;
    case kSM_OnFloor:
      mTimeSinceHitByPlayer = FLT_MAX;
      break;
    default:
      break;
    }
  }
  switch (msg) {
  case kSM_Alert:
  case kSM_Activate:
    if (GetActive()) {
      if (mOnlyAttackInRange) {
        mMayStartAttack = true;
      } else {
        mHitByPlayerProjectile = true;
      }
      SquadAdd(mgr);
    } else if (mCeilingAmbush) {
      RemoveMaterial(kMT_GroundCollider, mgr);
      mOnGround = false;
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
      if (it->mState == kSS_Retreat && it->mMsg == kSM_Next) {
        TUniqueId id = mgr.GetIdForScript(it->mObjId);
        if (CScriptCoverPoint* cp = TCastToPtr< CScriptCoverPoint >(mgr.ObjectById(id))) {
          cp->Reserve(GetUniqueId());
        }
      } else if (it->mState == kSS_Patrol && it->mMsg == kSM_Follow) {
        mEnablePatrol = true;
      }
    }
    mPathFindSearch.SetArea(
        mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetPostConstructed()->mPathArea);
    if (mFloatingCorpse) {
      mRagdollDelayTimer = 0.01f;
      RemoveMaterial(kMT_Character, kMT_Solid, kMT_Target, kMT_Orbit, mgr);
      mAlive = false;
      HealthInfo(mgr)->SetHP(-1.f);
    } else {
      SetEyeParticleActive(mgr, true);
    }
    break;
  }
  case kSM_Decrement:
    if (mRagDoll.get()) {
      mRagDoll->SetNoOverTimer(false);
      mRagDoll->SetContinueSmallMovements(false);
    }
    break;
  case kSM_Registered: {
    if (mCeilingAmbush) {
      mPendingAmbush = true;
      if (mShadowPirate) {
        mColor.SetAlpha(mPirateData.mCloakOpacity);
        mAlphaDelta = -1.f;
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
      mEnableRetreat = true;
    }
    break;
  case kSM_Falling:
    if (!BodyCtrl()->IsFrozen()) {
      float momentum = GetGravityConstant() * GetMass();
      if (mCeilingAmbush) {
        momentum *= 3.f;
      }
      SetMomentumWR(CVector3f(0.f, 0.f, -momentum));
    }
    if (BodyCtrl()->GetCurrentStateId() == pas::kAS_Step) {
      SetVelocityWR(CVector3f(0.f, 0.f, GetVelocityWR().GetZ()));
    }
    mBurstFire.SetBurstType(3);
    break;
  case kSM_Jumped:
    CPatterned::AcceptScriptMsg(kSM_Falling, sender, mgr);
    SetMomentumWR(CVector3f(0.f, 0.f, -GetGravityConstant() * GetMass()));
    SetVelocityForJump();
    break;
  case kSM_OnFloor:
    if (!mOnlyAttackInRange) {
      mBurstFire.SetBurstType(1);
    } else {
      mBurstFire.SetBurstType(4);
    }
    mJumpVelSet = false;
    if (mShadowPirate && GetVelocityWR().GetZ() < -1.f) {
      mAlphaDelta = 1.f;
      mCloakDelayTimer += -0.05f * GetVelocityWR().GetZ();
      mCloakDelayTimer = CMath::Clamp(0.f, mCloakDelayTimer, 1.f);
      mMaxCloakAlpha = 0.5f;
      if (mAlive) {
        mgr.ActorModelParticles()->StartElectric(*this);
        mElectricParticleTimer = 1.f + mCloakDelayTimer;
      }
    }
    break;
  case kSM_Action:
    if (CScriptTargetingPoint* point =
            TCastToPtr< CScriptTargetingPoint >(mgr.ObjectById(sender))) {
      if (point->GetActive()) {
        mBoneTracking.SetTarget(sender);
        mTargetId = sender;
        mHitByPlayerProjectile = true;
      } else {
        mBoneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
        mTargetId = mgr.GetPlayer()->GetUniqueId();
      }
      mAttackRemTime = 0.f;
    }
    break;
  case kSM_Deactivate:
  case kSM_Deleted:
    SquadRemove(mgr);
    mChargePlayerList.remove(GetUniqueId());
    break;
  case kSM_Start:
    mStarted = false;
    break;
  case kSM_Stop:
    mStarted = true;
    break;
  default:
    break;
  }
}

void CSpacePirate::Touch(CActor& actor, CStateManager& mgr) {
  CPatterned::Touch(actor, mgr);
  if (mRagDoll.get() && mRagDoll->IsPrimed()) {
    if (const CScriptTrigger* trigger = TCastToConstPtr< CScriptTrigger >(actor)) {
      if (trigger->GetActive() && (trigger->GetTriggerFlags() & kTFL_DetectAI) &&
          trigger->GetForceMagnitude() > 0.f) {
        CVector3f& impulse = mRagDoll->TorsoImpulse();
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
            mPirateData.mHearingRadius * mPirateData.mHearingRadius &&
        (mDetectionHeightRange == 0.f ||
         delta.GetZ() * delta.GetZ() < mDetectionHeightRange * mDetectionHeightRange)) {
      heard = true;
      mHearNoise = true;
    }
    if (type == kLNT_PlayerFire) {
      mHearPlayerFire = true;
    }
  }
  const bool result = heard;
  return result;
}

void CSpacePirate::SetEyeParticleActive(CStateManager& mgr, const bool active) {
  if (!mTrooper) {
    CAnimData* animData = AnimationData();
    if (!mOnlyAttackInRange || mSeated) {
      if (!mShadowPirate) {
        animData->SetParticleEffectState(skTwoEyes, active, mgr);
      }
    } else {
      animData->SetParticleEffectState(skOneEye, active, mgr);
    }
  }
}

bool CSpacePirate::CheckTargetable(CStateManager& mgr) { return GetModelAlphau8(mgr) > 127; }

void CSpacePirate::SetVelocityForJump() {
  if (!mJumpVelSet) {
    CVector3f velocity = CVector3f::Zero();
    CVector3f position = GetTranslation();
    CVector3f delta = mPatrolDestPos - position;
    float gravity = GetGravityConstant();
    float jumpZ = mJumpHeight + CMath::Max(mPatrolDestPos[kDZ], position[kDZ]);
    velocity.SetZ(CMath::SqrtF(2.f * gravity * (jumpZ - position.GetZ())));
    float riseTime = velocity.GetZ() / gravity;
    riseTime += CMath::SqrtF(2.f * (jumpZ - mPatrolDestPos[kDZ]) / gravity);
    float invTime = 1.f / riseTime;
    velocity.SetX(invTime * delta.GetX());
    velocity.SetY(invTime * delta.GetY());
    SetVelocityWR(velocity);
    mJumpVelSet = true;
  }
}

void CSpacePirate::SetAttackTarget(TUniqueId id) {
  mTargetId = id;
  mBurstFire.SetBurstType(1);
  mAttackRemTime = 0.f;
}

void CSpacePirate::DetachActorFromPirate() { mAttachedActor = kInvalidUniqueId; }

bool CSpacePirate::AttachActorToPirate(TUniqueId id) {
  if (mAttachedActor == kInvalidUniqueId) {
    mAttachedActor = id;
    return true;
  }
  return false;
}

CVector3f CSpacePirate::GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                                  const CVector3f& aimPos) const {
  return GetTranslation();
}

void CSpacePirate::SquadAdd(CStateManager& mgr) {
  if (mTeamAiMgrId == kInvalidUniqueId) {
    mTeamAiMgrId = CTeamAiMgr::GetTeamAiMgr(*this, mgr);
  }
  if (mTeamAiMgrId != kInvalidUniqueId) {
    if (CTeamAiMgr* teamMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId))) {
      teamMgr->AssignTeamAiRole(
          *this, mMelee ? CTeamAiRole::kTAR_Melee : CTeamAiRole::kTAR_Projectile,
          CTeamAiRole::kTAR_Unknown, CTeamAiRole::kTAR_Invalid);
    }
  }
}

void CSpacePirate::SquadRemove(CStateManager& mgr) {
  if (mTeamAiMgrId != kInvalidUniqueId) {
    if (CTeamAiMgr* teamMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId))) {
      if (teamMgr->IsPartOfTeam(GetUniqueId())) {
        teamMgr->RemoveTeamAiRole(GetUniqueId());
        mTeamAiMgrId = kInvalidUniqueId;
      }
    }
  }
}

void CSpacePirate::SquadReset(CStateManager& mgr) {
  CTeamAiMgr::ResetTeamAiRole(mMelee ? kAT_Melee : kAT_Projectile, mgr, mTeamAiMgrId,
                              GetUniqueId(), true);
}

void CSpacePirate::CheckForProjectiles(CStateManager& mgr) {
  if (mHearPlayerFire) {
    CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    CVector3f extent(5.f, 5.f, 5.f);
    CAABox bounds(aimPos - extent, aimPos + extent);
    mInProjectilePath = false;
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
                mInProjectilePath = true;
              }
            }
          }
        } else {
          mInProjectilePath = true;
        }
        if (mInProjectilePath) {
          break;
        }
      }
    }
    mHearPlayerFire = false;
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
  if (CScriptCoverPoint* cp = GetCoverPoint(mgr, mDestObj)) {
    cmdMgr.DeliverCmd(CBCCoverCmd(static_cast< pas::ECoverDirection >(arg), cp->GetTranslation(),
                                  -cp->GetTransform().GetForward()));
  }
}

void CPatterned::TryWallHang(CStateManager&, int) {
  CBodyStateCmdMgr& cmdMgr = BodyCtrl()->CommandMgr();
  cmdMgr.DeliverCmd(CBCWallHangCmd(mDestObj));
}

void CSpacePirate::UpdateCantSeePlayer(CStateManager& mgr) {
  if (++mCantSeePlayerCycleCounter % 7 == 0) {
    CVector3f eyePos = GetTranslation() + CVector3f(0.f, 0.f, mEyeHeight);
    CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    if (GetCoverPoint(mgr, mCoverPoint)) {
      switch (mCoverDir) {
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
    mNoPlayerLos = !LineOfSightTest(mgr, eyePos, mgr.GetPlayer()->GetAimPosition(mgr, 0.f),
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
  if ((pos - mHeldPosition).MagSquared() < 3.f) {
    mHoldPositionTime += dt;
  } else {
    mHeldPosition = pos;
    mHoldPositionTime = 0.f;
  }
}

void CSpacePirate::AvoidActors(CStateManager& mgr) {
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (const CPatterned* ai = TCastToConstPtr< CPatterned >(list[i])) {
      if (ai != this && ai->GetCurrentAreaId() == GetCurrentAreaId()) {
        float avoidDistance = mPirateData.mAvoidDistance;
        CVector3f separation =
            mSteeringBehaviors.Separation(*this, ai->GetTranslation(), avoidDistance);
        if (separation.IsNonZero()) {
          BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(separation, CVector3f::Zero(), 1.f));
          if (!mSteeringDelayTimer) {
            if (CSpacePirate* pirate = PATTERNED_CAST_TO(CSpacePirate, const_cast< CPatterned* >(ai))) {
              if (!pirate->mSteeringDelayTimer) {
                CVector3f delta = pirate->GetTranslation() - GetTranslation();
                if (CVector3f::Dot(GetTransform().GetForward(), delta) > 0.f &&
                    CVector3f::Dot(pirate->GetVelocityWR(), pirate->GetTransform().GetForward()) >
                        0.f) {
                  mSteeringDelayTimer = 1.f;
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
  if (!mAppliedBladeDamage && mSwooshSeg != CSegId::Invalid()) {
    if (CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(mgr.ObjectById(mTargetId))) {
      CTransform4f swoosh = GetLctrTransform(mSwooshSeg);
      const CVector3f& scale = GetModelData()->GetScale();
      CVector3f extent(scale.GetX() / 2.f, scale.GetY() / 2.f, scale.GetZ() / 2.f);
      CAABox bounds(swoosh.GetTranslation() - extent, swoosh.GetTranslation() + extent);
      if (bounds.DoBoundsOverlap(actor->GetBoundingBox())) {
        mgr.ApplyDamage(
            GetUniqueId(), actor->GetUniqueId(), GetUniqueId(), mPirateData.mBladeDamage,
            CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
            CVector3f::Zero());
        mAppliedBladeDamage = true;
      }
    }
  }
}

CVector3f CSpacePirate::GetTargetPos(CStateManager& mgr) {
  if (mTargetId != mgr.GetPlayer()->GetUniqueId()) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mTargetId))) {
      if (actor->GetActive()) {
        return actor->GetTranslation();
      }
    }
    mBoneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    mTargetId = mgr.GetPlayer()->GetUniqueId();
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
  if (!mCeilingAmbush || !mAlive || inDeferred) {
    mKnockBackController.SetAutoResetImpulse(!mNoKnockbackImpulseReset);
    mKnockBackController.EnableAnimReaction(kAR_KnockBack, IsOnGround());
    bool enableFreeze = true;
    if ((mTrooper || mShadowPirate) && !info.GetWeaponMode().IsCharged() &&
        !info.GetWeaponMode().IsComboed()) {
      enableFreeze = false;
    }
    mKnockBackController.SetEnableFreeze(enableFreeze);
    CPatterned::KnockBack(dir, mgr, info, magnitude, direct, inDeferred);
    if (mShadowPirate) {
      if (mAlive) {
        if (magnitude >= 4.f && !BodyCtrl()->IsFrozen()) {
          mAlphaDelta = 1.f;
          mCloakDelayTimer += 0.1f * magnitude;
          mCloakDelayTimer = CMath::Clamp(0.f, mCloakDelayTimer, 1.f);
          mMaxCloakAlpha = 0.5f;
          mgr.ActorModelParticles()->StartElectric(*this);
          mElectricParticleTimer = mCloakDelayTimer + 1.f;
        }
      } else {
        mMaxCloakAlpha = mAlphaDelta = 1.f;
        mMinCloakAlpha = 0.f;
        mgr.ActorModelParticles()->StartElectric(*this);
        mElectricParticleTimer = 2.f;
      }
    }
    if (mFloatingCorpse && mRagDoll.get()) {
      mRagDoll->TorsoImpulse() += (20.f * magnitude) * dir;
    }
    if (mAlive) {
      switch (mKnockBackController.GetActiveParms().mAnimState) {
      case kAR_Hurled: {
        mStateMachineState.SetState(mgr, *this, GetStateMachine(), rstl::string_l("GetUpNow"));
        CSfxManager::AddEmitter(mPirateData.mSound_Hurled, GetTranslation(),
                                CVector3f::Zero(), 127, true, false);
        break;
      }
      default:
        break;
      }
    } else {
      const CKnockBackMgr::KnockBackParms& parms = mKnockBackController.GetActiveParms();
      switch (parms.mAnimState) {
      case kAR_Hurled:
        if (parms.mAnimFollowup != kKBAFU_LaggedBurnDeath &&
            parms.mAnimFollowup != kKBAFU_BurnDeath) {
          CSfxManager::AddEmitter(mPirateData.mSound_Death, GetTranslation(),
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
    if (mAttachedActor != kInvalidUniqueId) {
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
    mEnableAim = true;
    if (!mSentAttackMsg) {
      mSentAttackMsg = true;
      SendScriptMsgs(kSS_Attack, mgr, kSM_None);
    }
    mAnimState = kAS_Ready;
    if (mCeilingAmbush) {
      mDestPos = GetTranslation() + CVector3f::Down();
      mPatrolDestPos = mDestPos;
      mJumpHeight = 0.f;
    } else {
      TUniqueId wpId = GetConnectedObject(mgr, kSS_Attack, kSM_Follow);
      if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(wpId))) {
        mDestPos = actor->GetTranslation();
        mPatrolDestPos = mDestPos;
        mJumpHeight = 3.f;
      }
      BodyCtrl()->SetLocomotionType(pas::kLT_Combat);
    }
    break;
  case kStateMsg_Update: {
    int jumpType = 0;
    if (mCeilingAmbush) {
      jumpType = 2;
    }
    TryCommand(mgr, pas::kAS_Jump, &CPatterned::TryJump, jumpType);
    if (mAnimState == kAS_Repeat) {
      BodyCtrl()->SetLocomotionType(pas::kLT_Combat);
    }
    CVector3f target = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    target.SetZ(0.f);
    BodyCtrl()->CommandMgr().DeliverTargetVector(target);
    break;
  }
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    BodyCtrl()->SetLocomotionType(pas::kLT_Combat);
    mJumpHeight = 3.f;
    mCeilingAmbush = false;
    mBoneTracking.SetActive(true);
    mBoneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    break;
  }
}

void CSpacePirate::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    mSteeringSpeed = BodyCtrl()->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_Walk) /
                         BodyCtrl()->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_Run);
    break;
  case kStateMsg_Deactivate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Combat);
    if (!mSentAttackMsg) {
      mSentAttackMsg = true;
      SendScriptMsgs(kSS_Attack, mgr, kSM_None);
    }
    break;
  }
  if (mEnablePatrol) {
    CPatterned::Patrol(mgr, msg, dt);
    switch (msg) {
    case kStateMsg_Activate:
      BodyCtrl()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
      BodyCtrl()->SetTurnSpeed(BodyCtrl()->GetTurnSpeed() / 1.25f);
      break;
    case kStateMsg_Update:
      AvoidActors(mgr);
      mPatrolDestPos = mDestPos;
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
    mSteeringSpeed = 1.f;
    BodyCtrl()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
    mDestObj = GetConnectedObject(mgr, kSS_Attack, kSM_Follow);
    break;
  case kStateMsg_Update:
    if (CScriptWaypoint* wp = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(mDestObj))) {
      const uint jump = (wp->GetBehaviourModifiers() >> 1) & 1;
      const uint drop = (wp->GetBehaviourModifiers() >> 2) & 1;
      if (jump || drop) {
        float maxSpeed = BodyCtrl()->GetBodyStateInfo().GetMaxSpeed();
        const CVector3f& scale = CVector3f(GetModelData()->GetScale());
        float distance = maxSpeed * ((1.5f * dt + 0.1f) * scale.GetY()) + mIntoJumpDist;
        if ((GetTranslation() - wp->GetTranslation()).MagSquared() < distance * distance) {
          mInPosition = true;
          mJumpHeight = jump ? 3.f : 0.f;
        }
      }
    }
    if (BodyCtrl()->GetCurrentStateId() == pas::kAS_Jump) {
      bool targetPlayer = true;
      if (CScriptWaypoint* wp = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(mDestObj))) {
        for (AUTO(conn, wp->GetConnectionList().begin()); conn != wp->GetConnectionList().end();
             ++conn) {
          if (conn->mState == kSS_Arrived && conn->mMsg == kSM_Next) {
            targetPlayer = false;
          }
        }
      }
      if (targetPlayer) {
        BodyCtrl()->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
                                                     GetTranslation());
      }
    }
    mPatrolDestPos = mDestPos;
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
  const bool heard = mHearNoise;
  mHearNoise = false;
  return heard;
}

bool CSpacePirate::HearPlayer(CStateManager& mgr, float arg) {
  bool heard = false;
  if (mgr.GetPlayer()->GetVelocityWR().MagSquared() > 0.1f) {
    CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    if (delta.MagSquared() <
        mPirateData.mHearingRadius * mPirateData.mHearingRadius) {
      heard = true;
    }
  }
  return heard;
}

void CSpacePirate::Halt(CStateManager& mgr, EStateMsg msg, float dt) { mSteeringSpeed = 0.f; }

void CSpacePirate::Run(CStateManager& mgr, EStateMsg msg, float dt) { mSteeringSpeed = 1.f; }

void CSpacePirate::Taunt(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mEnableAim = true;
    mBoneTracking.SetActive(true);
    mBoneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    if (mTargetId == kInvalidUniqueId) {
      mTargetId = mgr.GetPlayer()->GetUniqueId();
    }
    if (BodyCtrl()->HasBodyState(pas::kAS_Taunt)) {
      if (!mShadowPirate) {
        bool findOtherPirate = true;
        if (mMelee) {
          const CPASAnimParmData parms(pas::kAS_Taunt, CPASAnimParm::FromEnum(2));
          const rstl::pair< float, int > anim =
              BodyCtrl()->GetPASDatabase().FindBestAnimation(parms, *mgr.Random(), -1);
          if (anim.first > 0.f) {
            findOtherPirate = false;
            mTaunt = pas::kTT_Two;
          }
        }
        if (findOtherPirate) {
          bool withOtherPirate = false;
          const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
          for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
            if (CSpacePirate* pirate = PATTERNED_CAST_TO(CSpacePirate, const_cast< CEntity* >(list[i]))) {
              if (pirate != this && !pirate->mEnableAim && pirate->IsAlive() &&
                  pirate->GetCurrentAreaId() == GetCurrentAreaId()) {
                if ((pirate->GetTranslation() - GetTranslation()).MagSquared() <
                    mPirateData.mHearingRadius * mPirateData.mHearingRadius) {
                  withOtherPirate = true;
                }
              }
            }
          }
          mTaunt = withOtherPirate ? pas::kTT_Zero : pas::kTT_One;
        }
      } else {
        mTaunt = mAlertBeforeCloak ? pas::kTT_One : pas::kTT_Zero;
      }
      mAnimState = kAS_Ready;
    } else {
      CSfxManager::AddEmitter(mPirateData.mSound_Alert, GetTranslation(), CVector3f::Zero(),
                              true, false);
    }
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Taunt, &CPatterned::TryTaunt, mTaunt);
    break;
  case kStateMsg_Deactivate:
    if (mTaunt == pas::kTT_Zero) {
      mgr.InformListeners(GetTranslation(), kLNT_PlayerFire);
    }
    mAnimState = kAS_NotReady;
    break;
  }
}

void CSpacePirate::GetUp(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimState = kAS_Ready;
    SquadReset(mgr);
    mLeashTimer = 0.f;
    break;
  case kStateMsg_Update:
    if (BodyCtrl()->GetCurrentStateId() == pas::kAS_LieOnGround &&
        mPathFindSearch.Search(GetTranslation(), GetTranslation()) ==
            CPathFindSearch::kR_NoSourcePoint) {
      mPendingDeath = true;
    } else {
      TryCommand(mgr, pas::kAS_Getup, &CPatterned::TryGetUp, 0);
    }
    UpdateLeashTimer(dt);
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    break;
  }
}

void CSpacePirate::Lurk(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    ReleaseCoverPoint(mgr, mCoverPoint);
    mSteeringSpeed = 0.f;
    BodyCtrl()->SetLocomotionType(pas::kLT_Combat);
    mNoPlayerLos = true;
    mTimeNoPlayerLos = 0.f;
    float aggression = mPirateData.mAggressionCheck;
    mAlwaysAggressive = mgr.Random()->Range(0.f, 100.f) < aggression;
    float cover = mPirateData.mCoverCheck;
    mCoverCheck = mgr.Random()->Range(0.f, 100.f) < cover;
    float dodge = mPirateData.mDodgeCheck;
    mEnableDodge = mgr.Random()->Range(0.f, 100.f) < dodge;
    mEnableAim = true;
    mBoneTracking.SetActive(true);
    mBoneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    if (mOnlyAttackInRange) {
      mBurstFire.SetBurstType(4);
      BodyCtrl()->SetLocomotionType(pas::kLT_Combat);
    }
    mNormalDodge = false;
    break;
  }
  case kStateMsg_Update:
    if (BodyCtrl()->HasBodyState(pas::kAS_Turn)) {
      if (mAnimState != kAS_NotReady) {
        TryCommand(mgr, pas::kAS_Turn, &CPatterned::TryTurn, 0);
      }
      if (mAnimState != kAS_Repeat) {
        mDestPos = GetTargetPos(mgr);
        CVector3f delta = mDestPos - GetTranslation();
        delta.SetZ(0.f);
        if (CVector3f::Dot(GetTransform().GetForward(), delta.AsNormalized()) < 0.9f) {
          mAnimState = kAS_Ready;
        }
      }
    }
    if (mSeated && mSatUp) {
      if (mAttackRemTime > mAverageAttackTime &&
          BodyCtrl()->GetLocomotionType() == pas::kLT_Combat) {
        BodyCtrl()->SetLocomotionType(pas::kLT_Internal5);
      } else if (mAttackRemTime < 0.5f * mAverageAttackTime &&
                 BodyCtrl()->GetLocomotionType() == pas::kLT_Internal5) {
        BodyCtrl()->SetLocomotionType(pas::kLT_Combat);
      }
    }
    UpdateCantSeePlayer(mgr);
    UpdateHeldPosition(mgr, dt);
    break;
  case kStateMsg_Deactivate:
    mAlwaysAggressive = false;
    mNoPlayerDodge = false;
    mAnimState = kAS_NotReady;
    break;
  }
}

bool CSpacePirate::AggressionCheck(CStateManager& mgr, float arg) {
  bool result = false;
  if (!mNonAggressive) {
    if (mAlwaysAggressive) {
      result = true;
    } else if (mChargePlayerList.empty() && mTimeNoPlayerLos > 10.f) {
      result = true;
    }
    if (result) {
      mBehaviourOrient = kBO_MoveDir;
      if (rstl::find< rstl::list< TUniqueId >::const_iterator, TUniqueId >(
              mChargePlayerList.begin(), mChargePlayerList.end(), GetUniqueId()) ==
          mChargePlayerList.end()) {
        mChargePlayerList.push_back(GetUniqueId());
      }
    }
  }
  return result;
}

bool CSpacePirate::CoverCheck(CStateManager& mgr, float arg) { return mCoverCheck; }

bool CSpacePirate::CoverFind(CStateManager& mgr, float arg) {
  bool result = false;
  float minDistSq = mPirateData.mSearchRadius * mPirateData.mSearchRadius;
  const CScriptCoverPoint* closest = nullptr;
  const CObjectList& list = mgr.GetObjectListById(kOL_AiWaypoint);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (const CScriptCoverPoint* cp = TCastToConstPtr< CScriptCoverPoint >(list[i])) {
      if (cp->GetActive() && !cp->ShouldLandHere() && !cp->GetInUse(GetUniqueId()) &&
          cp->GetCurrentAreaId() == GetCurrentAreaId() &&
          cp->GetUniqueId() != mPreviousCoverPoint) {
        float distSq = (GetTranslation() - cp->GetTranslation()).MagSquared();
        if (distSq < minDistSq && !cp->Blown(mgr.GetPlayer()->GetTranslation())) {
          minDistSq = distSq;
          closest = cp;
        }
      }
    }
  }
  if (closest) {
    ReleaseCoverPoint(mgr, mCoverPoint);
    if (CScriptCoverPoint* cp =
            TCastToPtr< CScriptCoverPoint >(mgr.ObjectById(closest->GetUniqueId()))) {
      SetCoverPoint(cp, mCoverPoint);
      result = true;
      mPreviousCoverPoint = mCoverPoint;
      mCoverPointRearDir = -closest->GetTransform().GetForward();
      mBehaviourOrient = kBO_MoveDir;
    }
  }
  return result;
}

bool CSpacePirate::CoverBlown(CStateManager& mgr, float arg) {
  bool result = true;
  CVector3f toPlayer = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  if (toPlayer.MagSquared() > mMinAttackRange * mMinAttackRange) {
    if (CScriptCoverPoint* cp = GetCoverPoint(mgr, mCoverPoint)) {
      result = cp->Blown(mgr.GetPlayer()->GetTranslation());
      if (!result && mSteeringSpeed == 0.f &&
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
  if (CScriptCoverPoint* cp = GetCoverPoint(mgr, mCoverPoint)) {
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
      if (pirate != this && pirate->mInAttackState &&
          pirate->GetCurrentAreaId() == GetCurrentAreaId()) {
        result = true;
      }
    }
  }
  return result;
}

bool CSpacePirate::ShouldAttack(CStateManager& mgr, float arg) {
  bool result = true;
  if (mTargetId == mgr.GetPlayer()->GetUniqueId()) {
    CVector3f target = GetTargetPos(mgr);
    int numCloserPirates = 0;
    float distSq = (GetTranslation() - target).MagSquared();
    const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
    for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
      if (CSpacePirate* pirate = PATTERNED_CAST_TO(CSpacePirate, const_cast< CEntity* >(list[i]))) {
        if (pirate != this && pirate->mInAttackState && pirate->mAlive &&
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
      if (CScriptCoverPoint* cp = GetCoverPoint(mgr, mCoverPoint)) {
        mCoverDir = static_cast< pas::ECoverDirection >(
            (static_cast< uint >(cp->GetAttackDirection()) >> 1) & 1);
        mAnimState = kAS_Ready;
        mDestPos = cp->GetTranslation();
        TryCommand(mgr, pas::kAS_Cover, &CPatterned::TryCover, mCoverDir);
      }
    }
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Cover, &CPatterned::TryCover, mCoverDir);
    if (CScriptCoverPoint* cp = GetCoverPoint(mgr, mCoverPoint)) {
      BodyCtrl()->CommandMgr().DeliverTargetVector(-cp->GetTransform().GetForward());
    }
    UpdateCantSeePlayer(mgr);
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    break;
  }
}

void CSpacePirate::CoverAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_LeanFromCover));
    mInAttackState = true;
    break;
  case kStateMsg_Update:
    UpdateCantSeePlayer(mgr);
    break;
  case kStateMsg_Deactivate:
    mInAttackState = false;
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
    mAnimState = kAS_Ready;
    BodyCtrl()->SetLocomotionType(pas::kLT_Combat);
    mDestPos = GetTargetPos(mgr);
    mTargetDelta = mDestPos - GetBoundingBox().GetCenterPoint();
    mSteeringSpeed = 0.f;
    mEnableMeleeAttack = false;
    if (!mNoMeleeAttack &&
        mTargetDelta.MagSquared() < mMinAttackRange * mMinAttackRange &&
        mTargetDelta[kDZ] * mTargetDelta[kDZ] < 4.f) {
      mEnableMeleeAttack = true;
      mAppliedBladeDamage = false;
    } else {
      if (CVector3f::Dot(GetTransform().GetForward(), mTargetDelta.AsNormalized()) < 0.8f) {
        BodyCtrl()->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), mTargetDelta, 1.f));
      }
    }
    mInAttackState = true;
    mMaxCloakAlpha = 0.75f;
    break;
  case kStateMsg_Update:
    if (mEnableMeleeAttack) {
      TryCommand(mgr, pas::kAS_MeleeAttack, &CPatterned::TryMeleeAttack, 1);
      BodyCtrl()->CommandMgr().DeliverTargetVector(mTargetDelta);
      CheckBlade(mgr);
      if (mShadowPirate) {
        if (mAnimState == kAS_Over) {
          mAlphaDelta = -0.4f;
        } else {
          mAlphaDelta = 1.f;
          mMaxCloakAlpha = 0.75f;
        }
      }
    }
    UpdateCantSeePlayer(mgr);
    UpdateHeldPosition(mgr, dt);
    break;
  case kStateMsg_Deactivate:
    mEnableMeleeAttack = false;
    mInAttackState = false;
    mAnimState = kAS_NotReady;
    break;
  }
}

void CSpacePirate::DoubleSnap(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (!mNoMeleeAttack) {
      mAnimState = kAS_Ready;
    }
    mDestPos = GetTargetPos(mgr);
    mTargetDelta = mDestPos - GetTranslation();
    mSteeringSpeed = 0.f;
    mEnableMeleeAttack = true;
    mMeleeSeverity = pas::kS_One;
    mAppliedBladeDamage = false;
    mInAttackState = true;
    mCloseMelee = false;
    mChargePlayerList.remove(GetUniqueId());
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_MeleeAttack, &CPatterned::TryMeleeAttack, mMeleeSeverity);
    if (mMeleeSeverity == pas::kS_One && mAnimState == kAS_Over) {
      CVector3f delta = GetTargetPos(mgr) - GetTranslation();
      if (delta.MagSquared() < mMinAttackRange * mMinAttackRange &&
          CVector3f::Dot(delta.AsNormalized(), GetTransform().GetForward()) > -0.123f) {
        mAnimState = kAS_Ready;
        mMeleeSeverity = pas::kS_Two;
        mAppliedBladeDamage = false;
        mTargetDelta = delta;
        mCloseMelee = true;
      }
    }
    if (mCloseMelee) {
      mTargetDelta = GetTargetPos(mgr) - GetTranslation();
    }
    BodyCtrl()->CommandMgr().DeliverTargetVector(mTargetDelta);
    if (mShadowPirate) {
      if (mAnimState == kAS_Over) {
        mAlphaDelta = -0.4f;
      } else {
        mAlphaDelta = 1.f;
        mMaxCloakAlpha = 0.75f;
      }
    }
    UpdateCantSeePlayer(mgr);
    UpdateHeldPosition(mgr, dt);
    CheckBlade(mgr);
    break;
  case kStateMsg_Deactivate:
    mEnableMeleeAttack = false;
    mInAttackState = false;
    mAnimState = kAS_NotReady;
    break;
  }
}

bool CSpacePirate::ShouldCrouch(CStateManager& mgr, float arg) {
  bool result = false;
  if (CScriptCoverPoint* cp = GetCoverPoint(mgr, mCoverPoint)) {
    result = cp->ShouldCrouch();
  }
  return result;
}

void CSpacePirate::Crouch(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Crouch);
    if (CScriptCoverPoint* cp = GetCoverPoint(mgr, mCoverPoint)) {
      mTargetDelta = cp->GetTransform().GetForward();
    }
    mSteeringSpeed = 0.f;
    TargetPlayer(mgr, msg, dt);
    mCoverDir = pas::kCD_Invalid;
    break;
  case kStateMsg_Update:
    BodyCtrl()->CommandMgr().DeliverTargetVector(mTargetDelta);
    UpdateCantSeePlayer(mgr);
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

bool CSpacePirate::ShouldStrafe(CStateManager& mgr, float arg) {
  bool result = false;
  bool noPlayerStrafe = false;
  mSkidDir = pas::kSD_Invalid;
  if (!mNonAggressive) {
    CVector3f toTarget = GetTargetPos(mgr) - GetTranslation();
    if (CVector3f::Dot(toTarget, GetTransform().GetForward()) > 0.f) {
      if ((mLowHealthFrenzyTimer < 0.66f || mTimeSinceHitByPlayer < 0.66f) &&
          mStrafeDelayTimer == 0.f) {
        CVector3f center = GetBoundingBox().GetCenterPoint();
        const CVector3f& delta = (GetTargetPos(mgr) - center).AsNormalized();
        if (CVector3f::Dot(delta, GetTransform().GetForward()) > 0.707f) {
          mSkidDir = GetStrafeDir(mgr, 10.f);
          if (mSkidDir != pas::kSD_Invalid) {
            result = true;
          } else {
            noPlayerStrafe = true;
          }
        }
      }
      if (!noPlayerStrafe && !result && mTargetId == mgr.GetPlayer()->GetUniqueId() &&
          mTimeNoPlayerLos > 1.f) {
        if ((mgr.GetPlayer()->GetTranslation() - GetTranslation()).Magnitude() < 15.f &&
            mSkidDir == pas::kSD_Invalid) {
          mSkidDir = GetStrafeDir(mgr, 5.f);
          if (mSkidDir != pas::kSD_Invalid) {
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
    mStrafeDelayTimer = 4.f;
    mInAttackState = true;
    break;
  case kStateMsg_Update:
    if (BodyCtrl()->GetCurrentStateId() != pas::kAS_Step) {
      BodyCtrl()->CommandMgr().DeliverCmd(CBCStepCmd(mSkidDir, pas::kStep_Normal));
    }
    break;
  case kStateMsg_Deactivate:
    mInAttackState = false;
    break;
  }
}

void CSpacePirate::Flee(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    BodyCtrl()->CommandMgr().SetSteeringBlendMode(kSBM_Normal);
    CVector3f delta = GetTranslation() - mgr.GetPlayer()->GetTranslation();
    SetDestPos(GetTranslation() + delta.AsNormalized() * 15.f);
    mBehaviourOrient = kBO_MoveDir;
    mSteeringSpeed = 1.f;
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
    mBehaviourOrient = kBO_MoveDir;
    mSteeringSpeed = 1.f;
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
  float angle = mDetectionAngle;
  return CVector3f::Dot(toPlayer, GetTransform().GetForward()) > distance * angle;
}

bool CSpacePirate::LineOfSight(CStateManager& mgr, float arg) { return !mNoPlayerLos; }

void CSpacePirate::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mJumpPoint = kInvalidUniqueId;
    if (CScriptCoverPoint* cp = GetCoverPoint(mgr, mCoverPoint)) {
      mReflectedDestPos = GetTranslation();
      mInPosition = false;
      mDestObj = cp->GetUniqueId();
      mDestPos = cp->GetTranslation();
    }
    if (GetSearchPath()->Search(GetTranslation(), mDestPos) == CPathFindSearch::kR_Success) {
      mReflectedDestPos = GetTranslation();
      mDestPos = GetSearchPath()->GetPoint();
      mInPosition = false;
      BodyCtrl()->CommandMgr().DeliverCmd(
          CBCLocomotionCmd(mDestPos - GetTranslation(), CVector3f::Zero(), 1.f));
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
                if ((mDestPos[kDZ] - GetTranslation().GetZ()) *
                        (wp->GetTranslation().GetZ() - jp->GetTranslation().GetZ()) >
                    0.f) {
                  CVector3f toDest = mDestPos - wp->GetTranslation();
                  distSq += 4.f * toJump.GetZ() * toJump.GetZ();
                  distSq += toDest.MagSquared() + 9.f * toDest.GetZ() * toDest.GetZ();
                  if (distSq < minDistSq &&
                      GetSearchPath()->PathExists(GetTranslation(), jp->GetTranslation()) ==
                          CPathFindSearch::kR_Success) {
                    bool good = false;
                    bool noPath = GetSearchPath()->PathExists(wp->GetTranslation(), mDestPos) !=
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
        mDestPos = best->GetTranslation();
        if (GetSearchPath()->Search(GetTranslation(), mDestPos) ==
            CPathFindSearch::kR_Success) {
          mReflectedDestPos = GetTranslation();
          mDestPos = GetSearchPath()->GetPoint();
          mInPosition = false;
          mJumpPoint = best->GetUniqueId();
          mJumpHeight = best->GetJumpApex();
          if (const CScriptWaypoint* wp =
                  TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(best->GetJumpPoint()))) {
            mPatrolDestPos = wp->GetTranslation();
            BodyCtrl()->CommandMgr().DeliverCmd(
                CBCLocomotionCmd(mDestPos, CVector3f::Zero(), 1.f));
            mBehaviourOrient = kBO_MoveDir;
          }
        }
      }
    }
    BodyCtrl()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
    if (mEnableAim) {
      mSteeringSpeed = 1.f;
    }
    mInRange = false;
    mNormalDodge = true;
    break;
  case kStateMsg_Update:
    CPatterned::PathFind(mgr, msg, dt);
    if (mJumpPoint != kInvalidUniqueId) {
      if (CScriptAiJumpPoint* jp =
              TCastToPtr< CScriptAiJumpPoint >(mgr.ObjectById(mJumpPoint))) {
        float maxSpeed = BodyCtrl()->GetBodyStateInfo().GetMaxSpeed();
        const CVector3f& scale = CVector3f(GetModelData()->GetScale());
        float jumpDistance = maxSpeed * ((1.5f * dt + 0.1f) * scale.GetY()) + mIntoJumpDist;
        if ((GetTranslation() - jp->GetTranslation()).MagSquared() < jumpDistance * jumpDistance) {
          mAnimState = kAS_Ready;
          TryCommand(mgr, pas::kAS_Jump, &CPatterned::TryJump, 0);
        }
      }
    }
    AvoidActors(mgr);
    if (!mInRange) {
      if (CScriptCoverPoint* cp = GetCoverPoint(mgr, mCoverPoint)) {
        float maxSpeed = BodyCtrl()->GetBodyStateInfo().GetMaxSpeed();
        const CVector3f& scale = CVector3f(GetModelData()->GetScale());
        mCoverRange = maxSpeed * ((1.5f * dt + 0.1f) * scale.GetY());
        if (cp->ShouldWallHang()) {
          mCoverRange += mIntoJumpDist;
        }
        mInRange = (GetTranslation() - cp->GetTranslation()).MagSquared() <
                          mCoverRange * mCoverRange;
      }
    }
    UpdateCantSeePlayer(mgr);
    UpdateHeldPosition(mgr, dt);
    break;
  case kStateMsg_Deactivate:
    CPatterned::PathFind(mgr, msg, dt);
    mAnimState = kAS_NotReady;
    mJumpPoint = kInvalidUniqueId;
    mBehaviourOrient = kBO_Constant;
    mInRange = false;
    BodyCtrl()->CommandMgr().SetSteeringBlendMode(kSBM_Normal);
    break;
  }
}

bool CSpacePirate::InRange(CStateManager& mgr, float arg) { return mInRange; }

void CSpacePirate::Shuffle(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->CommandMgr().SetSteeringBlendMode(kSBM_Normal);
    if (!mNoShuffleCloseCheck && TooClose(mgr, 0.f)) {
      SetDestPos(GetTranslation() +
                 mMinAttackRange *
                     (GetTranslation() - mgr.GetPlayer()->GetTranslation()).AsNormalized() +
                 Random2f(mgr, 0.f, 5.f));
      mDestObj = kInvalidUniqueId;
      mBehaviourOrient = kBO_Constant;
      mShuffleClose = true;
    } else {
      CVector3f fromPlayer = GetTranslation() - mgr.GetPlayer()->GetTranslation();
      CVector3f side = CVector3f::Cross(CVector3f::Up(), fromPlayer);
      float range = mMaxAttackRange;
      float distance = range * mgr.Random()->Float() + range;
      float sideDistance = 2.f * mMaxAttackRange * (mgr.Random()->Float() - 0.5f);
      SetDestPos(mgr.GetPlayer()->GetTranslation() + distance * fromPlayer.AsNormalized() +
                 sideDistance * side.AsNormalized());
      mDestObj = kInvalidUniqueId;
      mBehaviourOrient = kBO_MoveDir;
      mShuffleClose = false;
    }
    mSteeringSpeed = 1.f;
    break;
  }
  CPatterned::PathFind(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Update:
    AvoidActors(mgr);
    break;
  case kStateMsg_Deactivate:
    mShuffleClose = false;
    break;
  }
}

void CSpacePirate::TurnAround(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    SetDestPos(GetTargetPos(mgr));
    CVector3f delta = mDestPos - GetTranslation();
    delta.SetZ(0.f);
    if (CVector3f::Dot(GetTransform().GetForward(), delta.AsNormalized()) < 0.8f) {
      mAnimState = kAS_Ready;
    }
    break;
  }
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Turn, &CPatterned::TryTurn, 0);
    UpdateCantSeePlayer(mgr);
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    break;
  }
}

bool CSpacePirate::ShouldDodge(CStateManager& mgr, float arg) {
  bool result = false;
  if (mEnableDodge) {
    if (!mNonAggressive && !mNoPlayerDodge) {
      CVector3f toTarget = GetTargetPos(mgr) - GetTranslation();
      if (CVector3f::Dot(toTarget, GetTransform().GetForward()) > 0.f &&
          (mTimeSinceHitByPlayer < 0.33f || mLowHealthFrenzyTimer < 0.33f) &&
          mTimeNoPlayerLos < 0.5f) {
        result = true;
      }
    }
    if (!result) {
      if (const CMetroid* metroid = PATTERNED_CAST_TO(CMetroid, const_cast< CEntity* >(mgr.GetObjectById(mTargetId)))) {
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
    mEnableBreakDodge = false;
    if (!mNormalDodge && !mNoBreakDodge && mDodgeDelayTimer <= 0.f) {
      float chance =
          0.15f * (1.f + (4.f * (mInitialHP - HealthInfo(mgr)->GetHP())) / mInitialHP);
      if (mgr.Random()->Float() < chance) {
        mEnableBreakDodge = true;
      }
      float delayMin = mPirateData.mDodgeDelayTimeMin;
      float delayMax = mPirateData.mDodgeDelayTimeMax;
      mDodgeDelayTimer = mgr.Random()->Range(delayMin, delayMax);
    }
    mDodgeDir =
        GetStrafeDir(mgr, mEnableBreakDodge ? mBreakDodgeDist : mDodgeDist);
    if (mDodgeDir != pas::kSD_Invalid) {
      mAnimState = kAS_Ready;
    }
    break;
  case kStateMsg_Update:
    if (!mEnableBreakDodge) {
      if (mNormalDodge || mgr.Random()->Float() < 0.5f) {
        TryCommand(mgr, pas::kAS_Step, &CPatterned::TryDodge, mDodgeDir);
      } else {
        TryCommand(mgr, pas::kAS_Step, &CPatterned::TryRollingDodge, mDodgeDir);
      }
    } else {
      TryCommand(mgr, pas::kAS_Step, &CPatterned::TryBreakDodge, mDodgeDir);
      if (GetMaterialList().HasMaterial(kMT_Orbit) && mStateMachineState.GetTime() > 0.5f) {
        RemoveMaterial(kMT_Orbit, mgr);
        mgr.Player()->TryToBreakOrbit(GetUniqueId(), CPlayer::kOB_ActivateOrbitSource, mgr);
      }
    }
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    mNoPlayerDodge = true;
    if (!GetMaterialList().HasMaterial(kMT_Orbit)) {
      AddMaterial(kMT_Orbit, mgr);
    }
    break;
  }
}

bool CSpacePirate::ShouldRetreat(CStateManager& mgr, float arg) {
  bool result = false;
  if (mEnableRetreat) {
    TUniqueId wpId = GetConnectedObject(mgr, kSS_Patrol, kSM_Follow);
    const CScriptWaypoint* wp = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(wpId));
    if (!wp) {
      wpId = GetConnectedObject(mgr, kSS_Retreat, kSM_Follow);
      wp = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(wpId));
    }
    if (wp) {
      mDestObj = wpId;
      SetDestPos(wp->GetTranslation());
    } else {
      mDestObj = kInvalidUniqueId;
      SetDestPos(GetTranslation());
    }
    mEnableRetreat = false;
    mReflectedDestPos = GetTranslation();
    mInPosition = false;
    result = true;
    ReleaseCoverPoint(mgr, mCoverPoint);
    mHearNoise = false;
    mEnableAim = false;
    mHitByPlayerProjectile = false;
  }
  return result;
}

bool CSpacePirate::ShouldMove(CStateManager& mgr, float arg) {
  CScriptCoverPoint* cp = GetCoverPoint(mgr, mCoverPoint);
  return cp && !cp->ShouldStay();
}

bool CSpacePirate::ShotAt(CStateManager& mgr, float arg) {
  return mLowHealthFrenzyTimer < (arg ? arg : 0.5f);
}

bool CSpacePirate::Attacked(CStateManager& mgr, float arg) {
  return mTimeSinceHitByPlayer < (arg ? arg : 0.5f);
}

bool CSpacePirate::HasTargetingPoint(CStateManager& mgr, float arg) {
  bool result = true;
  CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(mTargetId));
  if (mTargetId == mgr.GetPlayer()->GetUniqueId() || !actor || !actor->GetActive()) {
    result = false;
    mBoneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    mTargetId = mgr.GetPlayer()->GetUniqueId();
    float scale = 1.f;
    float margin = mPirateData.mSearchRadius * scale;
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
          mBoneTracking.SetTarget(point->GetUniqueId());
          mTargetId = point->GetUniqueId();
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
    if (CScriptCoverPoint* cp = GetCoverPoint(mgr, mCoverPoint)) {
      mDestObj = mCoverPoint;
      mDestPos = cp->GetTranslation();
    }
    mReflectedDestPos = GetTranslation();
    mInPosition = false;
    break;
  }
}

bool CSpacePirate::ShouldWallHang(CStateManager& mgr, float arg) {
  CScriptCoverPoint* cp = GetCoverPoint(mgr, mCoverPoint);
  return cp && cp->ShouldWallHang();
}

void CSpacePirate::WallHang(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimState = kAS_Ready;
    mInWallHang = true;
    if (CScriptCoverPoint* cp = GetCoverPoint(mgr, mCoverPoint)) {
      const rstl::vector< SConnection >& connections = cp->GetConnectionList();
      for (AUTO(it, connections.begin()); it != connections.end(); ++it) {
        if (it->mState == kSS_Arrived && it->mMsg == kSM_Next) {
          TUniqueId id = mgr.GetIdForScript(it->mObjId);
          if (CScriptWaypoint* wp = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(id))) {
            mDestObj = id;
            mDestPos = wp->GetTranslation();
            mReflectedDestPos = GetTranslation();
            mInPosition = false;
            break;
          }
        }
      }
    }
    mInAttackState = true;
    break;
  case kStateMsg_Update: {
    TryCommand(mgr, pas::kAS_WallHang, &CPatterned::TryWallHang, 0);
    CVector3f toPlayer = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    toPlayer.SetZ(0.f);
    BodyCtrl()->CommandMgr().DeliverTargetVector(toPlayer);
    mBurstFire.SetBurstType(1);
    break;
  }
  case kStateMsg_Deactivate:
    mInWallHang = false;
    mAnimState = kAS_NotReady;
    mInAttackState = false;
    break;
  }
}

void CSpacePirate::WallDetach(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mInWallHang = true;
    break;
  case kStateMsg_Update:
    BodyCtrl()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    break;
  case kStateMsg_Deactivate:
    mInWallHang = false;
    break;
  }
}

bool CSpacePirate::AnimOver(CStateManager& mgr, float arg) {
  if (mInWallHang) {
    return BodyCtrl()->GetCurrentStateId() != pas::kAS_WallHang;
  }
  return CPatterned::AnimOver(mgr, arg);
}

bool CSpacePirate::ShouldJumpBack(CStateManager& mgr, float arg) {
  return !mNoShuffleCloseCheck || mHoldPositionTime > 6.f;
}

void CSpacePirate::JumpBack(CStateManager& mgr, EStateMsg msg, float dt) {
  if (!ShouldJumpBack(mgr, dt)) {
    return;
  }
  switch (msg) {
  case kStateMsg_Activate:
    if (!mOnlyAttackInRange && !CantJumpBack(mgr, -GetTransform().GetForward(), 5.f)) {
      float height = GetSearchPath()->GetCharacterHeight();
      mPathFindSearch.SetCharacterHeight(5.f + height);
      CVector3f dest = GetTranslation() + 10.f * GetTransform().GetForward();
      if (GetSearchPath()->Search(GetTranslation(), dest) == CPathFindSearch::kR_Success &&
          (GetSearchPath()->GetWaypoints().back() - dest).MagSquared() < 3.f) {
        if (CMath::AbsF(GetSearchPath()->RemainingPathDistance(GetTranslation()) - 10.f) < 4.f) {
          mPatrolDestPos = GetSearchPath()->GetWaypoints().back();
          mJumpHeight = 5.f;
          mUseJumpBackJump = true;
          mAnimState = kAS_Ready;
        }
      }
      GetSearchPath()->SetCharacterHeight(height);
    }
    break;
  case kStateMsg_Update:
    if (!mUseJumpBackJump) {
      BodyCtrl()->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Backward, pas::kStep_Normal));
      BodyCtrl()->CommandMgr().DeliverTargetVector(GetTargetPos(mgr) - GetTranslation());
    } else {
      TryCommand(mgr, pas::kAS_Jump, &CPatterned::TryJump, 0);
    }
    break;
  case kStateMsg_Deactivate:
    if (mUseJumpBackJump) {
      mAnimState = kAS_NotReady;
      mUseJumpBackJump = false;
    }
    mHoldPositionTime = 0.f;
    break;
  }
}

bool CSpacePirate::ShouldSpecialAttack(CStateManager& mgr, float arg) {
  if (mOnlyAttackInRange && !mBurstFire.IsBurstSet() && mAttackRemTime > 2.f) {
    return true;
  }
  return false;
}

bool CSpacePirate::LostInterest(CStateManager& mgr, float arg) {
  if (mOnlyAttackInRange && mAttackRemTime < 1.5f) {
    return true;
  }
  return false;
}

void CSpacePirate::SpecialAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimState = kAS_Ready;
    mTargetDelta = mgr.GetPlayer()->GetAimPosition(mgr, 0.f) - GetGunEyePos();
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_ProjectileAttack, &CPatterned::TryProjectileAttack, pas::kS_One);
    if (mAnimState == kAS_Ready) {
      BodyCtrl()->CommandMgr().DeliverTargetVector(mTargetDelta);
    }
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    break;
  }
}

bool CSpacePirate::StartAttack(CStateManager& mgr, float arg) {
  if (mMayStartAttack) {
    mMayStartAttack = false;
    return true;
  }
  return false;
}

bool CSpacePirate::BreakAttack(CStateManager& mgr, float arg) { return mBreakAttack; }

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
            CVector3f toDest = mDestPos - wp->GetTranslation();
            distSq += toDest.MagSquared() + 9.f * toDest.GetZ() * toDest.GetZ();
            if (distSq < minDistSq &&
                CVector3f::Dot(wp->GetTransform().GetForward(), toDest) > 0.f &&
                GetSearchPath()->PathExists(GetTranslation(), jp->GetTranslation()) ==
                    CPathFindSearch::kR_Success) {
              bool good = false;
              bool noPath = GetSearchPath()->PathExists(wp->GetTranslation(), mDestPos) !=
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
      mJumpPoint = best->GetUniqueId();
      mJumpHeight = best->GetJumpApex();
      mPatrolDestPos = wp->GetTranslation();
    }
  }
  return result;
}

void CSpacePirate::PathFindEx(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::PathFind(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Activate:
    mInRange = false;
    mBehaviourOrient = kBO_MoveDir;
    break;
  case kStateMsg_Update:
    AvoidActors(mgr);
    if (!mInRange) {
      if (const CScriptAiJumpPoint* jp =
              TCastToConstPtr< CScriptAiJumpPoint >(mgr.GetObjectById(mJumpPoint))) {
        float maxSpeed = BodyCtrl()->GetBodyStateInfo().GetMaxSpeed();
        const CVector3f& scale = CVector3f(GetModelData()->GetScale());
        mCoverRange = maxSpeed * ((1.5f * dt + 0.1f) * scale.GetY()) + mIntoJumpDist;
        mInRange = (GetTranslation() - jp->GetTranslation()).MagSquared() <
                          mCoverRange * mCoverRange;
      }
    }
    break;
  case kStateMsg_Deactivate:
    mInRange = false;
    break;
  }
}

void CSpacePirate::Bounce(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (const CScriptAiJumpPoint* jp =
            TCastToConstPtr< CScriptAiJumpPoint >(mgr.GetObjectById(mJumpPoint))) {
      TUniqueId target = jp->GetJumpTarget();
      if (const CScriptWaypoint* wp =
              TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(target))) {
        CBodyStateCmdMgr& cmdMgr = BodyCtrl()->CommandMgr();
        cmdMgr.DeliverCmd(CBCJumpCmd(mPatrolDestPos, wp->GetTranslation(), pas::kJT_Normal));
      }
    }
    break;
  case kStateMsg_Update:
    if (mStateMachineState.GetTime() > 0.1f &&
        BodyCtrl()->GetCurrentStateId() != pas::kAS_Jump) {
      mStateMachineState.SetCodeTrigger();
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
    mBoneTracking.SetActive(false);
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
    mPendingDeath = true;
  }
}

bool CSpacePirate::OffLine(CStateManager& mgr, float arg) { return !IsOnGround(); }

bool CSpacePirate::Landed(CStateManager& mgr, float arg) { return IsOnGround(); }

void CPatterned::TryJumpInLoop(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = mBodyController->CommandMgr();
  CBCJumpCmd cmd(mDestPos, static_cast< pas::EJumpType >(arg), true);
  cmdMgr.DeliverCmd(cmd);
}

void CSpacePirate::Jump(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimState = kAS_Ready;
    mPatrolDestPos = GetTranslation() + CVector3f::Down();
    mJumpHeight = 0.f;
    mLeashTimer = 0.f;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Jump, &CPatterned::TryJumpInLoop, 0);
    UpdateLeashTimer(dt);
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    break;
  }
}

bool CSpacePirate::Leash(CStateManager& mgr, float arg) { return mLeashTimer > arg; }

void CSpacePirate::UpdateLeashTimer(float dt) {
  if (!BodyCtrl()->IsFrozen() && !BodyCtrl()->IsElectrocuting()) {
    mLeashTimer += dt;
  }
}

bool CSpacePirate::FireProjectile(float dt, CStateManager& mgr) {
  bool result = false;
  CTransform4f gunXf = GetLctrTransform(mGunSeg);
  if (!mAlive) {
    LaunchProjectile(gunXf, mgr, 6, CWeapon::kPA_None, false,
                     rstl::optional_object< TLockedToken< CGenDescription > >(),
                     CSfxManager::kInternalInvalidSfxId, false, CVector3f(1.f, 1.f, 1.f));
    result = true;
  } else {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mTargetId))) {
      CVector3f pos = actor->GetTranslation();
      if (mTargetId == mgr.GetPlayer()->GetUniqueId()) {
        pos = ProjectileInfo()->PredictInterceptPos(gunXf.GetTranslation(),
                                                    mgr.GetPlayer()->GetAimPosition(mgr, 0.f),
                                                    *mgr.GetPlayer(), true, dt);
      }
      CVector3f gunToPos = pos - gunXf.GetTranslation();
      float distance = gunToPos.Magnitude();
      gunToPos /= distance;
      float dot = CVector3f::Dot((GetLctrTransform(mWristSeg).GetTranslation() -
                                  GetLctrTransform(mElbowSeg).GetTranslation())
                                     .AsNormalized(),
                                 gunToPos);
      if ((dot > 0.707f || (distance < 6.f && dot > 0.5f)) &&
          LineOfSightTest(mgr, gunXf.GetTranslation(), pos,
                          CMaterialList(kMT_Player, kMT_ProjectilePassthrough))) {
        pos += GetTransform().Rotate(mBurstFire.GetDistanceCompensatedError(distance, 6.f));
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
    CSfxManager::AddEmitter(mPirateData.mSound_Projectile, GetTranslation(),
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
    mAllEnergyDrained = true;
    handled = true;
    break;
  case kUE_EndAction:
    mCloseMelee = false;
    handled = true;
    break;
  case kUE_DeGenerate:
  case kUE_BecomeRagDoll:
    if (mOnlyAttackInRange || HealthInfo(mgr)->GetHP() <= 0.f) {
      mRagdollDelayTimer = mgr.Random()->Float() * 0.05f + 0.001f;
    }
    handled = true;
    break;
  case kUE_IkLock:
    if (!mIkChain.GetActive()) {
      const CSegId& bone =
          GetModelData()->GetAnimationData()->GetLocatorSegId(node.GetLocatorName());
      if (bone != CSegId(3)) {
        CTransform4f xf = GetLctrTransform(bone);
        mIkChain.Activate(*GetModelData()->GetAnimationData(), bone, xf);
        mSatUp = true;
      }
    }
    handled = true;
    break;
  case kUE_IkRelease:
    mIkChain.Deactivate();
    handled = true;
    break;
  case kUE_ScreenShake:
    SendScriptMsgs(kSS_Play, mgr, kSM_None);
    handled = true;
    break;
  case kUE_FadeOut:
    if (mShadowPirate) {
      mAlphaDelta = -0.8f;
      mgr.ActorModelParticles()->StartElectric(*this);
      mElectricParticleTimer = 1.f;
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
    if (!mShadowPirate) {
      alpha = mColor.GetAlphau8();
    } else {
      alpha = static_cast< uchar >(mShadowPirateAlpha * 255.f);
    }
  }
  const uchar result = alpha;
  return result;
}

void CSpacePirate::CalculateRenderBounds() {
  if (mRagDoll.get() && mRagDoll->IsPrimed()) {
    CAABox bounds = mRagDoll->CalculateRenderBounds();
    CVector3f margin = GetModelData()->GetScale() * 0.2f;
    SetRenderBounds(CAABox(bounds.GetMinPoint() - margin, bounds.GetMaxPoint() + margin));
  } else {
    CActor::CalculateRenderBounds();
  }
}

void CSpacePirate::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  if (mRagDoll.get() && mRagDoll->IsPrimed()) {
    mRagDoll->PreRender(GetTranslation(), *ModelData());
  }
  CPatterned::PreRender(mgr, frustum);
  if (!mRagDoll.get() || !mRagDoll->IsPrimed()) {
    mBoneTracking.PreRender(mgr, *ModelData()->AnimationData(), GetTransform(),
                                CVector3f(ModelData()->GetScale()), *BodyCtrl());
    mIkChain.PreRender(*ModelData()->AnimationData(), GetTransform(),
                           CVector3f(ModelData()->GetScale()));
  }
}

void CSpacePirate::Render(const CStateManager& mgr) const {
  float time = IsAlive() ? CGraphics::GetSecondsMod900() : 0.f;
  CTimeProvider provider(time);
  gpRender->SetGXRegister1Color(mTrooperColor);
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
  if (mPendingFrenzyChance) {
    mPendingFrenzyChance = false;
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
  if (HealthInfo(mgr)->GetHP() < 0.3f * mInitialHP && mgr.Random()->Next() % 100 < 60 &&
      mLowHealthFrenzyTimer < 0.5f) {
    reset = true;
  }
  if (reset) {
    mFrenzyFrames = mgr.Random()->Range(2, 4);
  }
  return --mFrenzyFrames >= 0;
}

void CSpacePirate::UpdateCloak(float dt, CStateManager& mgr) {
  if (mShadowPirate) {
    if (mAlive) {
      if (mCloakDelayTimer > 0.f) {
        mCloakDelayTimer -= dt;
        if (mCloakDelayTimer <= 0.f) {
          mAlphaDelta = -0.4f;
        }
      }
    } else {
      mMinCloakAlpha = 0.f;
      mMaxCloakAlpha = 1.f;
    }
    if (mElectricParticleTimer > 0.f) {
      mElectricParticleTimer -= dt;
      if (mElectricParticleTimer <= 0.f && !BodyCtrl()->IsElectrocuting()) {
        mgr.ActorModelParticles()->StopElectric(*this);
      }
    }
    if (BodyCtrl()->IsFrozen()) {
      mAlphaDelta = 2.f;
    }
    if (mAlphaDelta < 0.f && mColor.GetAlpha() < mMinCloakAlpha) {
      mColor.SetAlpha(mMinCloakAlpha);
      mAlphaDelta = 0.f;
      RemoveMaterial(kMT_Target, mgr);
    }
    if (mAlphaDelta > 0.f && mColor.GetAlpha() > mMaxCloakAlpha) {
      mColor.SetAlpha(mMaxCloakAlpha);
      AddMaterial(kMT_Target, mgr);
    }
    mCloakStepTime -= dt;
    if (mCloakStepTime < 0.f) {
      float random = mgr.Random()->Float();
      mCloakStepTime = (1.f - random) * 0.08f;
      if (mAlphaDelta < 0.f) {
        mShadowPirateAlpha = mColor.GetAlpha();
        if (mAlive) {
          mShadowPirateAlpha -= random * (mColor.GetAlpha() - mMinCloakAlpha);
        }
      } else if (mAlphaDelta > 0.f) {
        mShadowPirateAlpha =
            mColor.GetAlpha() + random * (mMaxCloakAlpha - mColor.GetAlpha());
      } else {
        mShadowPirateAlpha = mColor.GetAlpha();
      }
    }
  }
}

void CSpacePirate::UpdateAttacks(float dt, CStateManager& mgr) {
  bool reset = true;
  if ((!mAlive || (BodyCtrl()->GetBodyStateInfo().GetCurrentState()->CanShoot() &&
                          mEnableAim && !mMelee && !mCeilingAmbush &&
                          !mStarted && !BodyCtrl()->IsElectrocuting())) &&
      mBurstFire.GetBurstType() != -1) {
    if (mAlive) {
      if (!mOnlyAttackInRange ||
          (mgr.GetPlayer()->GetTranslation() - GetTranslation()).MagSquared() <
              mLeashRadius * mLeashRadius) {
        reset = false;
        mAttackRemTime -= dt;
        if (mAttackRemTime < 0.f) {
          const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, mTeamAiMgrId, GetUniqueId());
          if (!role || role->GetTeamAiRole() == CTeamAiRole::kTAR_Projectile) {
            if (mTeamAiMgrId == kInvalidUniqueId ||
                CTeamAiMgr::AddAttacker(kAT_Projectile, mgr, mTeamAiMgrId, GetUniqueId())) {
              if (ShouldFrenzy(mgr)) {
                mBurstFire.SetBurstType(2);
              }
              if (mSeated) {
                mBurstFire.SetBurstType(5);
              }
              if (!PlayerSpot(mgr, 0.f) && mBurstFire.GetBurstType() < 6) {
                mBurstFire.SetBurstType(mBurstFire.GetBurstType() + 6);
              }
              mBurstFire.Start(mgr);
              mAttackRemTime =
                  mgr.Random()->Float() * mAttackTimeVariation + mAverageAttackTime;
              const CVector3f& fromPlayer =
                  (GetGunEyePos() - mgr.GetPlayer()->GetAimPosition(mgr, 0.f)).AsNormalized();
              const CVector3f& forward = mgr.GetPlayer()->GetTransform().GetForward();
              if (CVector3f::Dot(fromPlayer, forward) < 0.9f) {
                const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
                for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
                  if (CSpacePirate* pirate = PATTERNED_CAST_TO(CSpacePirate, const_cast< CEntity* >(list[i]))) {
                    if (pirate != this && pirate->mEnableAim &&
                        pirate->GetCurrentAreaId() == GetCurrentAreaId()) {
                      mAttackRemTime += 0.2f;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    mBurstFire.Update(mgr, dt);
    if (mBurstFire.ShouldFire()) {
      if (mgr.GetPlayer()->IsSidewaysDashing() && mgr.Random()->Float() < 0.5f) {
        mBurstFire.SetAvoidAccuracy(true);
      }
      FireProjectile(dt, mgr);
      mBurstFire.SetAvoidAccuracy(false);
      float variation = mPirateData.mNextShotTimeVariation;
      float average = mPirateData.mAverageNextShotTime;
      mBurstFire.SetTimeToNextShot(variation * (mgr.Random()->Float() - 0.5f) + average);
    } else if (!mBurstFire.IsBurstSet()) {
      reset = true;
    }
  }
  if (reset) {
    SquadReset(mgr);
  }
  SetTargetable(CheckTargetable(mgr));
}

void CSpacePirate::UpdateAimBodyState(float dt, CStateManager& mgr) {
  if (mAlive && mEnableAim && !mInWallHang && !BodyCtrl()->IsFrozen() &&
      !mMelee && !mRagDoll.get() && (!mSeated || mSatUp) &&
      !BodyCtrl()->IsElectrocuting()) {
    mAimDelayTimer = CMath::Max(0.f, mAimDelayTimer - dt);
    if (!mAimDelayTimer) {
      BodyCtrl()->CommandMgr().DeliverCmd(CBCAdditiveAimCmd());
      CVector3f direction = GetTransform().TransposeMultiply(GetTargetPos(mgr));
      BodyCtrl()->CommandMgr().DeliverAdditiveTargetVector(direction);
    }
  } else if (mEnableAim && !mMelee) {
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
  if (inCineCam && !mPrevInCineCam) {
    SetCinematicCollision(mgr);
  } else if (!inCineCam && mPrevInCineCam && !mRagdollNoAiCollision) {
    SetNonCinematicCollision(mgr);
  }
  mPrevInCineCam = inCineCam;
  float steeringSpeed = mSteeringDelayTimer ? 0.f : mSteeringSpeed;
  BodyCtrl()->CommandMgr().SetSteeringSpeedRange(steeringSpeed, steeringSpeed);
  mUnkTimer = CMath::Max(0.f, mUnkTimer - dt);
  if (mAlive) {
    mTimeSinceHitByPlayer += dt;
    mLowHealthFrenzyTimer += dt;
    if (mInProjectilePath) {
      mLowHealthFrenzyTimer = 0.f;
      mInProjectilePath = false;
    }
    if (mHitByPlayerProjectile) {
      mTimeSinceHitByPlayer = 0.f;
      mHitByPlayerProjectile = false;
    }
  }
  UpdateCloak(dt, mgr);
  if (!BodyCtrl()->IsFrozen()) {
    if (mAlive) {
      mSteeringDelayTimer = CMath::Max(0.f, mSteeringDelayTimer - dt);
      if (mNoPlayerLos) {
        mTimeNoPlayerLos += dt;
      } else {
        mTimeNoPlayerLos = 0.f;
      }
      mStrafeDelayTimer = CMath::Max(0.f, mStrafeDelayTimer - dt);
      mDodgeDelayTimer = CMath::Max(0.f, mDodgeDelayTimer - dt);
      CheckForProjectiles(mgr);
    }
    UpdateAttacks(dt, mgr);
    UpdateAimBodyState(dt, mgr);
    mIkChain.Update(dt);
  }
  if (mPendingAmbush) {
    mPendingAmbush = false;
    if (mCeilingAmbush) {
      BodyCtrl()->SetLocomotionType(pas::kLT_Internal6);
    } else {
      BodyCtrl()->SetLocomotionType(pas::kLT_Crouch);
    }
    mStateMachineState.SetState(mgr, *this, GetStateMachine(), rstl::string_l("Ambushing"));
  }
  bool noRagDoll = mRagDoll.null();
  if (noRagDoll || !mRagDoll->IsPrimed()) {
    CPatterned::Think(dt, mgr);
    if (!BodyCtrl()->IsFrozen()) {
      mBoneTracking.Update(dt);
    }
  } else {
    UpdateAlphaDelta(dt, mgr);
    UpdateDamageColor(dt);
    if (CSfxHandle handle = GetSfxHandle()) {
      CSfxManager::UpdateEmitter(handle, GetTranslation(), CVector3f::Zero(), 127);
    }
  }
  if (!noRagDoll) {
    if (!mRagDoll->IsPrimed()) {
      mRagDoll->Prime(mgr, GetTransform(), *ModelData());
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
      mRagDoll->Update(mgr, dt * CalcDyingThinkRate(), waterTop);
      ModelData()->AdvanceParticles(GetTransform(), dt, mgr);
    }
    if (mRagDoll->IsOver() && !mRagDoll->WillContinueSmallMovements() &&
        !GetFadeToDeath()) {
      SetFadeToDeath(true);
      mAlphaDelta = -1.f / 3.f;
      AddMaterial(kMT_ProjectilePassthrough, mgr);
      mAllEnergyDrained = true;
      SetMomentumWR(CVector3f::Zero());
      CPhysicsActor::Stop();
    }
  }
  if (mRagdollDelayTimer > 0.f) {
    mRagdollDelayTimer -= dt;
    if (mRagdollDelayTimer <= 0.f) {
      if (mRagDoll.null()) {
        mRagDoll = rs_new CPirateRagDoll(mgr, this, mPirateData.mSound_Impact,
                                             (mFloatingCorpse ? 3 : 0) |
                                                 (mRagdollNoAiCollision ? 4 : 0));
        RemoveMaterial(kMT_Orbit, kMT_Target, mgr);
      }
      mRagdollDelayTimer = 0.f;
    }
  }
}

const float CSpacePirate::skGravityConstant = 50.f;
const float CSpacePirate::skFloatingGravityConstant = -3.f;
