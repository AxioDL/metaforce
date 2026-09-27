#include "MetroidPrime/Cameras/CBallCamera.hpp"

#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

#include "Collision/CCollidableSphere.hpp"
#include "Collision/CCollisionInfo.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CRayCastResult.hpp"

#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CControlMapper.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Cameras/CPathCamera.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDock.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDoor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpindleCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/Tweaks/CTweakBall.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"

#include "WorldFormat/CMetroidAreaCollider.hpp"

#include "math.h"

static CMaterialList kLineOfSightIncludeList = CMaterialList(kMT_Solid);
static CMaterialList kLineOfSightExcludeList =
    CMaterialList(kMT_ProjectilePassthrough, kMT_Player, kMT_Character, kMT_CameraPassthrough);
static CMaterialFilter kLineOfSightFilter =
    CMaterialFilter::MakeIncludeExclude(kLineOfSightIncludeList, kLineOfSightExcludeList);

CBallCamera::CBallCamera(TUniqueId uid, TUniqueId watchedId, const CTransform4f& xf, float fovY,
                         float nearZ, float farZ, float aspect)
: CGameCamera(uid, true, rstl::string_l("Ball Camera"),
              CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList, kInvalidEditorId), xf, fovY,
              nearZ, farZ, aspect, watchedId, false, 0)
, mBehaviour(kBCB_Default)
, x18c_24_(true)
, mChaseAllowed(true)
, mBoostAllowed(true)
, mObscureAvoidance(true)
, mVolumeCollider(true)
, mClampAttitude(false)
, mClampAzimuth(false)
, mClearLOS(true)
, mPrevClearLOS(true)
, mAvoidGeometryFull(false)
, mLookAtBall(false)
, mForceProcessing(false)
, mObtuseDirection(false)
, mNoElevationInterp(false)
, mDirectElevation(false)
, mOverrideLookDir(false)
, mNoElevationVelClamp(false)
, mNoSpline(false)
, x18e_26_(false)
, mNearbyDoorClosed(false)
, mNearbyDoorClosing(false)
, mCurMinDistance(gpTweakBall->GetBallCameraMinSpeedDistance())
, mTargetMinDistance(gpTweakBall->GetBallCameraMinSpeedDistance())
, mMaxDistance(gpTweakBall->GetBallCameraMaxSpeedDistance())
, mBackwardsDistance(gpTweakBall->GetBallCameraBackwardsDistance())
, mElevation(gpTweakBall->GetBallCameraElevation())
, mCurAnglePerSecond(gpTweakBall->GetBallCameraAnglePerSecond())
, mTargetAnglePerSecond(gpTweakBall->GetBallCameraAnglePerSecond())
, mAttitudeRange(1.553343f)
, mAzimuthRange(1.553343f)
, mLookAtOffset(gpTweakBall->GetBallCameraOffset())
, mLookPosAhead(0.f, 0.f, 0.f)
, mFixedLookPos(CVector3f::Zero())
, mLookPos(0.f, 0.f, 0.f)
, mNextLookXf(CTransform4f::Identity())
, mBallCameraSpring(gpTweakBall->GetBallCameraSpringConstant(),
                        gpTweakBall->GetBallCameraSpringMax(),
                        gpTweakBall->GetBallCameraSpringTardis())
, mBallCameraCentroidSpring(gpTweakBall->GetBallCameraCentroidSpringConstant(),
                                gpTweakBall->GetBallCameraCentroidSpringMax(),
                                gpTweakBall->GetBallCameraCentroidSpringTardis())
, mBallCameraLookAtSpring(gpTweakBall->GetBallCameraLookAtSpringConstant(),
                              gpTweakBall->GetBallCameraLookAtSpringMax(),
                              gpTweakBall->GetBallCameraLookAtSpringTardis() * 1.1f)
, mBallCameraCentroidDistanceSpring(gpTweakBall->GetBallCameraCentroidDistanceSpringConstant(),
                                        gpTweakBall->GetBallCameraCentroidDistanceSpringMax(),
                                        gpTweakBall->GetBallCameraCentroidDistanceSpringTardis())
, mDampedPos(0.f, 0.f, 0.f)
, mSmallCentroid(0.f, 0.f, 1.f)
, mMediumCentroid(0.f, 0.f, 1.f)
, mLargeCentroid(0.f, 0.f, 1.f)
, mSmallCollidersObsCount(0)
, mMediumCollidersObsCount(0)
, mLargeCollidersObsCount(0)
, mSmallColliderIt(0)
, mMediumColliderIt(0)
, mLargeColliderIt(0)
, mPrevBallPos(0.f, 0.f, 0.f)
, mBallVelFlat(0.f)
, mMaxBallVel(0.f)
, mBallDelta(CVector3f::Zero())
, mBallDeltaFlat(CVector3f::Zero())
, mSpeedFactor(0.f)
, mSpeedingTime(0.f)
, mIdealLookVec(0.f, 0.f, 0.f)
, mPredictedLookPos(0.f, 0.f, 0.f)
, mAvoidGeomCycle(0)
, mColliderMag(1.f)
, mClearColliderThreshold(0.2f)
, mCollidersAABB(CAABox::Identity())
, mObscuredTime(0.f)
, mObscuringMaterial(kMT_NoStepLogic)
, mUnobscureMag(0.f)
, mSplineIntermediatePos(CVector3f::Zero())
, mObscuringObjectId(kInvalidUniqueId)
, mSplineState(kBSS_Invalid)
, mReevalSplineEnd(false)
, mSplineCtrl(0.f)
, mCamSpline(false)
, mCollisionExcludeList(kMT_NoStepLogic)
, mCamBehindFloorOrWall(false)
, mElevInterpTimer(0.f)
, mElevInterpStart(0.f)
, mTooCloseActorId(kInvalidUniqueId)
, mTooCloseActorDist(10000.f)
, mPendingFailsafe(false)
, x3e8_(0.f)
, x3ec_(0.f)
, x3f0_(0.f)
, x3f4_(2.f)
, x3f8_(0.f)
, x3fc_(0.f)
, mState(kBCS_Default)
, mChaseElevation(gpTweakBall->GetBallCameraChaseElevation())
, mChaseDistance(gpTweakBall->GetBallCameraChaseDistance())
, mChaseAnglePerSecond(gpTweakBall->GetBallCameraChaseAnglePerSecond())
, mChaseLookAtOffset(gpTweakBall->GetBallCameraChaseLookAtOffset())
, mBallCameraChaseSpring(gpTweakBall->GetBallCameraChaseSpringConstant(),
                             gpTweakBall->GetBallCameraChaseSpringMax(),
                             gpTweakBall->GetBallCameraChaseSpringTardis())
, mBoostElevation(gpTweakBall->GetBallCameraBoostElevation())
, mBoostDistance(gpTweakBall->GetBallCameraBoostDistance())
, mBoostAnglePerSecond(gpTweakBall->GetBallCameraBoostAnglePerSecond())
, mBoostLookAtOffset(gpTweakBall->GetBallCameraBoostLookAtOffset())
, mBallCameraBoostSpring(gpTweakBall->GetBallCameraBoostSpringConstant(),
                             gpTweakBall->GetBallCameraBoostSpringMax(),
                             gpTweakBall->GetBallCameraBoostSpringTardis())
, mOverrideBallToCam(CVector3f::Zero())
, mConservativeDoorCamDistance(gpTweakBall->GetConservativeDoorCameraDistance())
, mCollisionActorId(kInvalidUniqueId)
, mClampVelTimer(0.f)
, mClampVelRange(0.f)
, mShortMoveCount(0)
, mFailsafeState(rs_new SFailsafeState)
, x480_(rs_new SUnknown) {
  SetupColliders(mSmallColliders, 7.f * 0.33f, 7.f * 0.33f, 0.1f, 3, 2.f, 0.5f, -M_PIF / 2.f);
  SetupColliders(mMediumColliders, 7.f * 0.66f, 7.f * 0.66f, 0.1f, 6, 2.f, 0.5f, -M_PIF / 2.f);
  SetupColliders(mLargeColliders, 7.f, 7.f, 0.1f, 12, 2.f, 0.5f, -M_PIF / 2.f);
}

CBallCamera::~CBallCamera() {}

void CBallCamera::TeleportColliders(rstl::vector< CCameraCollider >& colliderList, CVector3f pos) {
  for (int i = 0; i < colliderList.size(); ++i) {
    colliderList[i].SetRealPosition(pos);
    colliderList[i].SetDesiredPosition(pos);
    colliderList[i].SetLookAtPosition(pos);
  }
}

void CBallCamera::TeleportCamera(const CVector3f& pos, CStateManager& mgr) {
  mDampedPos = pos;
  TeleportColliders(mSmallColliders, pos);
  TeleportColliders(mMediumColliders, pos);
  TeleportColliders(mLargeColliders, pos);
  if (CCollisionActor* act = TCastToPtr< CCollisionActor >(mgr.ObjectById(mCollisionActorId))) {
    act->SetTranslation(pos);
  }
}

void CBallCamera::TeleportLookAtStuff(CStateManager& mgr) {
  CVector3f pos = mgr.GetPlayer()->GetBallPosition();
  mLookPos = pos;
  mLookPos.SetZ(mLookPos.GetZ() + mLookAtOffset.GetZ());
  mLookPosAhead = mLookPos;
  mFixedLookPos = mLookPos;
}

void CBallCamera::TeleportCamera(const CTransform4f& xf, CStateManager& mgr) {
  SetTransform(xf);
  TeleportCamera(xf.GetTranslation(), mgr);
}

void CBallCamera::ResetToTweaks(CStateManager& mgr) {
  mBehaviour = kBCB_Default;
  SetAllowChaseCamera(true);
  SetAllowBoostCamera(true);
  SetGeometryAvoidance(true);
  mVolumeCollider = true;
  mClampAttitude = false;
  mClampAzimuth = false;
  SetMinDistance(gpTweakBall->GetBallCameraMinSpeedDistance());
  SetMaxDistance(gpTweakBall->GetBallCameraMaxSpeedDistance());
  SetBackwardsDistance(gpTweakBall->GetBallCameraBackwardsDistance());
  SetDistanceSpring(CCameraSpring(gpTweakBall->GetBallCameraSpringConstant(),
                                  gpTweakBall->GetBallCameraSpringMax(),
                                  gpTweakBall->GetBallCameraSpringTardis()));
  SetCentroidDistanceSpring(
      CCameraSpring(gpTweakBall->GetBallCameraCentroidDistanceSpringConstant(),
                    gpTweakBall->GetBallCameraCentroidDistanceSpringMax(),
                    gpTweakBall->GetBallCameraCentroidDistanceSpringTardis()));
  SetLookAtOffset(gpTweakBall->GetBallCameraOffset());
  SetChaseLookAtOffset(gpTweakBall->GetBallCameraChaseLookAtOffset());
  SetElevation(gpTweakBall->GetBallCameraElevation());
  mAttitudeRange = M_PIF / 2.f;
  mAzimuthRange = M_PIF / 2.f;
  InterpolateFOV(GetFov(), CCameraManager::GetDefaultThirdPersonVerticalFOV(), 1.f, 0.f);
  mTargetAnglePerSecond = gpTweakBall->GetBallCameraAnglePerSecond();
  mNoElevationInterp = false;
  mDirectElevation = false;
  mOverrideLookDir = false;
  mNoElevationVelClamp = false;
  mNoSpline = false;
  x18e_26_ = false;
}

// TODO non-matching
void CBallCamera::SetupColliders(rstl::vector< CCameraCollider >& out, float xMag, float zMag,
                                 float radius, int count, float k, float max, float startAngle) {
  out.reserve(count);
  const float thetaDelta = 2.f * M_PIF;
  for (int i = 0; i < count; ++i) {
    float z = zMag * static_cast< float >(cos(startAngle));
    if (startAngle > M_PIF / 2.f) {
      z *= 0.25f;
    }

    CVector3f pos(xMag * static_cast< float >(sin(startAngle)), 0.f, z);
    CCameraSpring spring(k, max, 1.f);
    {
      CCameraCollider collider(radius, pos, spring, 1.f);
      out.push_back(collider);
    }
    startAngle += thetaDelta / float(count);
  }
}

void CBallCamera::Render(const CStateManager& mgr) const {}

void CBallCamera::Reset(const CTransform4f& xf, CStateManager& mgr) {
  mBallCameraSpring.Reset();
  mBallCameraCentroidSpring.Reset();
  mBallCameraLookAtSpring.Reset();
  mBallCameraCentroidDistanceSpring.Reset();
  mBallCameraChaseSpring.Reset();
  mBallCameraBoostSpring.Reset();

  CVector3f desiredPos =
      FindDesiredPosition(GetDistance(), GetElevation(), xf.GetForward(), mgr, false);

  if (const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(GetWatchedObject()))) {
    TeleportLookAtStuff(mgr);
    mIdealLookVec = mLookAtOffset;
    mPredictedLookPos = GetLookAtPosition();
    CVector3f dist = GetLookAtPosition() - desiredPos;
    dist.SetZ(0.f);
    if (dist.CanBeNormalized()) {
      TeleportCamera(CTransform4f::LookAt(desiredPos, GetLookAtPosition()), mgr);
    } else {
      CTransform4f camXf = player->CreateTransformFromMovementDirection();
      camXf.SetTranslation(desiredPos);
      TeleportCamera(camXf, mgr);
      mgr.CameraManager()->SetPlayerCamera(mgr, GetUniqueId());
    }

    mBallVelFlat = 0.f;
    mMaxBallVel = 0.f;
    mCurMinDistance = mTargetMinDistance;
    mBallDeltaFlat = CVector3f::Zero();
    mBallDelta = CVector3f::Zero();
    mObtuseDirection = false;
    mSpeedFactor = 0.f;
    mPrevBallPos = player->GetBallPosition();
    mDampedPos = GetTranslation();
    mSmallCentroid = CVector3f::Zero();
    mMediumCentroid = CVector3f::Zero();
    mLargeCentroid = CVector3f::Zero();
    mSmallCollidersObsCount = 0;
    mMediumCollidersObsCount = 0;
    mLargeCollidersObsCount = 0;
    mSmallColliderIt = 0;
    mMediumColliderIt = 0;
    mLargeColliderIt = 0;
    mColliderMag = 1.f;
    mAvoidGeometryFull = true;
    mForceProcessing = true;
    Think(0.1f, mgr);
    mAvoidGeometryFull = false;
    mForceProcessing = false;
  }
}

void CBallCamera::UpdateCollidersDistances(rstl::vector< CCameraCollider >& colliderList,
                                           float xMag, float zMag, float angOffset) {
  float theta = angOffset;
  for (int i = 0; i < colliderList.size(); ++i) {
    float z = cosf(theta) * zMag;
    if (theta > M_PIF / 2.f) {
      z *= 0.25f;
    }

    float x = xMag * CMath::Limit(sinf(theta), 1.f);
    CVector3f pos(x, 0.f, z);
    colliderList[i].SetDesiredPosition(pos);
    theta += 2.f * M_PIF / colliderList.size();
  }
}

// TODO non-matching
void CBallCamera::UpdateColliders(const CTransform4f& xf,
                                  rstl::vector< CCameraCollider >& colliderList, int& idx,
                                  int count, float tolerance, const TEntityList& nearList, float dt,
                                  CStateManager& mgr) {
  if (idx < colliderList.size()) {
    const CPlayer& player = *mgr.GetPlayer();
    mIdealLookVec = CVector3f(0.f, gpTweakBall->GetBallCameraOffset().GetY(),
                                  gpTweakPlayer->mPlayerBallHalfExtent);
    mIdealLookVec[kDY] *= mSpeedFactor;
    mPredictedLookPos = player.GetMovementDirection() * mIdealLookVec.GetY();
    mPredictedLookPos[kDZ] = mIdealLookVec.GetZ();
    mPredictedLookPos += player.GetTranslation();
    CTransform4f predictedLookXf = CTransform4f::LookAt(xf.GetTranslation(), mPredictedLookPos);
    CVector3f predictedLookPos = predictedLookXf.GetTranslation();
    for (int i = 0; i < count; ++i) {
      CVector3f localPos = CVector3f(colliderList[idx].GetDesiredPosition());
      CVector3f worldPos = predictedLookXf.Rotate(localPos) + predictedLookXf.GetTranslation();
      if (CVector3f(colliderList[idx].GetRealPosition() - worldPos).Magnitude() < 0.1f) {
        localPos = colliderList[idx].GetPosition();
        worldPos = colliderList[idx].GetRealPosition();
      }
      CVector3f centerToCollider = worldPos - predictedLookPos;
      float mag = centerToCollider.Magnitude();
      if (centerToCollider.CanBeNormalized()) {
        centerToCollider.Normalize();
        TUniqueId intersectId = kInvalidUniqueId;
        const CRayCastResult& result = mgr.RayWorldIntersection(
            intersectId, predictedLookXf.GetTranslation(), centerToCollider,
            mag + colliderList[idx].GetRadius(), kLineOfSightFilter, nearList);
        float t = result.GetTime();
        if (result.IsValid()) {
          worldPos = predictedLookXf.GetTranslation() +
                     centerToCollider * (t - colliderList[idx].GetRadius());
          localPos = predictedLookXf.GetRotation().GetInverse() *
                     CVector3f(worldPos - predictedLookXf.GetTranslation());
        }
      }
      colliderList[idx].SetRealPosition(worldPos);
      colliderList[idx].SetPosition(localPos);
      CVector3f scaledWorldColliderPos = (1.f / tolerance) * (mag * centerToCollider);
      scaledWorldColliderPos *= mSpeedFactor;
      scaledWorldColliderPos += mPredictedLookPos;
      colliderList[idx].SetLookAtPosition(scaledWorldColliderPos);
      if (mgr.RayCollideWorld(worldPos, scaledWorldColliderPos, nearList, kLineOfSightFilter,
                              nullptr)) {
        colliderList[idx].SetOcclusionCount(0);
      } else {
        colliderList[idx].SetOcclusionCount(colliderList[idx].GetOcclusionCount() + 1);
      }
      idx += 1;
      if (idx == colliderList.size()) {
        idx = 0;
      }
    }
  }
}

CVector3f
CBallCamera::CalculateCollidersCentroid(const rstl::vector< CCameraCollider >& colliderList,
                                        int) const {
  int colliderCount = colliderList.size();
  if (colliderCount < 3) {
    return CVector3f(0.f, 1.f, 0.f);
  }

  float accumCross = 0.f;
  float accumX = 0.f;
  float accumZ = 0.f;
  int clearColliders = 0;
  int prevCol = colliderCount - 1;
  for (int i = 0; i < colliderCount; ++i) {
    if (colliderList[prevCol].GetOcclusionCount() < 2 && colliderList[i].GetOcclusionCount() < 2) {
      float scale = colliderList[prevCol].GetScale();
      CVector3f p0 = scale * colliderList[prevCol].GetPosition();
      CVector3f p1 = scale * colliderList[i].GetPosition();
      float cross = p0.GetX() * p1.GetZ() - p1.GetX() * p0.GetZ();
      accumCross += cross;
      accumX += cross * (p1.GetX() + p0.GetX());
      accumZ += cross * (p1.GetZ() + p0.GetZ());
    } else {
      clearColliders += 1;
    }
    prevCol = i;
  }

  if (static_cast< float >(clearColliders) / static_cast< float >(colliderList.size()) <=
      mClearColliderThreshold) {
    return CVector3f(0.f, 1.f, 0.f);
  }

  if (accumCross != 0.f) {
    float baryCross = 3.f * accumCross;
    return CVector3f(accumX / baryCross, 0.f, accumZ / baryCross);
  }

  return CVector3f(0.f, 2.f, 0.f);
}

CVector3f CBallCamera::ApplyColliders() {
  CVector3f smallCentroid =
      CalculateCollidersCentroid(mSmallColliders, mSmallCollidersObsCount);
  CVector3f mediumCentroid =
      CalculateCollidersCentroid(mMediumColliders, mMediumCollidersObsCount);
  CVector3f largeCentroid =
      CalculateCollidersCentroid(mLargeColliders, mLargeCollidersObsCount);

  if (smallCentroid.GetY() == 0.f) {
    mSmallCentroid = smallCentroid;
  } else {
    mSmallCentroid = CVector3f::Zero();
  }

  float centroidX = mSmallCentroid.GetX();
  float centroidZ = mSmallCentroid.GetZ();

  if (mediumCentroid.GetY() == 0.f) {
    mMediumCentroid = mediumCentroid;
  } else {
    mMediumCentroid = CVector3f::Zero();
  }

  centroidX += mMediumCentroid.GetX();
  centroidZ += mMediumCentroid.GetZ();

  if (largeCentroid.GetY() == 0.f) {
    mLargeCentroid = largeCentroid;
  } else {
    mLargeCentroid = CVector3f::Zero();
  }

  centroidX += mLargeCentroid.GetX();
  centroidZ += mLargeCentroid.GetZ();

  if (mClearLOS) {
    centroidX /= 1.5f;
  }
  centroidZ /= 3.f;

  if (!mClearLOS && mObscuringObjectId == kInvalidUniqueId) {
    float xMul = 1.5f;
    float zMul = 1.f;
    if (mObscuringMaterial.HasMaterial(kMT_Floor)) {
      zMul += 2.f * mUnobscureMag;
    }
    if (mObscuringMaterial.HasMaterial(kMT_Wall)) {
      xMul += 3.f * CMath::Clamp(0.f, mUnobscureMag - 0.25f, 1.f);
    }
    centroidX *= xMul;
    centroidZ *= zMul;
  }

  if (!mVolumeCollider) {
    return CVector3f::Zero();
  }

  if (fabsf(centroidX) < 0.05f) {
    centroidX = 0.f;
  }
  if (fabsf(centroidZ) < 0.05f) {
    centroidZ = 0.f;
  }

  if (mClearLOS) {
    centroidZ *= 0.5f;
  }

  return CVector3f(centroidX, 0.f, centroidZ);
}

int CBallCamera::CountObscuredColliders(const rstl::vector< CCameraCollider >& colliderList) const {
  int ret = 0;
  for (int i = 0; i < colliderList.size(); i++) {
    if (colliderList[i].GetOcclusionCount() >= 2) {
      ++ret;
    }
  }
  return ret;
}

CAABox
CBallCamera::CalculateCollidersBoundingBox(const rstl::vector< CCameraCollider >& colliderList,
                                           const CStateManager& mgr) const {
  CAABox aabb = CAABox::MakeMaxInvertedBox();
  for (int i = 0; i < colliderList.size(); i++) {
    aabb.AccumulateBounds(colliderList[i].GetRealPosition());
  }
  aabb.AccumulateBounds(mgr.GetPlayer()->GetTranslation());
  return aabb;
}

CVector3f CBallCamera::AvoidGeometryFull(const CTransform4f& xf, const TEntityList& nearList,
                                         float dt, CStateManager& mgr) {
  UpdateColliders(xf, mSmallColliders, mSmallColliderIt, mSmallColliders.size(), 4.f,
                  nearList, dt, mgr);
  UpdateColliders(xf, mMediumColliders, mMediumColliderIt, mMediumColliders.size(), 4.f,
                  nearList, dt, mgr);
  UpdateColliders(xf, mLargeColliders, mLargeColliderIt, mLargeColliders.size(), 4.f,
                  nearList, dt, mgr);
  return ApplyColliders();
}

CVector3f CBallCamera::AvoidGeometry(const CTransform4f& xf, const TEntityList& nearList, float dt,
                                     CStateManager& mgr) {
  switch (mAvoidGeomCycle) {
  case 0:
    UpdateColliders(xf, mSmallColliders, mSmallColliderIt, 1, 4.f, nearList, dt, mgr);
    break;
  case 1:
    UpdateColliders(xf, mMediumColliders, mMediumColliderIt, 3, 4.f, nearList, dt, mgr);
    break;
  case 2:
    UpdateColliders(xf, mLargeColliders, mLargeColliderIt, 4, 4.f, nearList, dt, mgr);
    break;
  case 3:
    UpdateColliders(xf, mLargeColliders, mLargeColliderIt, 4, 4.f, nearList, dt, mgr);
    break;
  default:
    break;
  }

  int newCycle = mAvoidGeomCycle + 1;
  mAvoidGeomCycle = newCycle;
  if (newCycle >= 4) {
    mAvoidGeomCycle = 0;
  }

  return ApplyColliders();
}

bool CBallCamera::DetectCollision(const CVector3f& from, const CVector3f& to, float radius,
                                  float& d, const CStateManager& mgr) {
  CVector3f delta = to - from;
  float deltaMag = delta.Magnitude();
  CVector3f deltaNorm = delta * (1.f / deltaMag);
  bool clear = true;

  if (deltaMag > FLT_EPSILON * 10.f) {
    float margin = 2.f * radius;
    CAABox aabb = CAABox::MakeMaxInvertedBox();
    aabb.AccumulateBounds(from);
    aabb.AccumulateBounds(to);
    aabb = CAABox(aabb.GetMinPoint() - CVector3f(margin, margin, margin),
                  aabb.GetMaxPoint() + CVector3f(margin, margin, margin));
    TEntityList nearList;
    mgr.BuildColliderList(nearList, *mgr.GetPlayer(), aabb);
    CAreaCollisionCache cache(aabb);
    CGameCollision::BuildAreaCollisionCache(mgr, cache);
    if (cache.HasCacheOverflowed()) {
      clear = false;
    }
    if (CGameCollision::DetectCollisionBoolean_Cached(
            mgr, cache,
            CCollidableSphere(CSphere(CVector3f::Zero(), radius), CMaterialList(kMT_Solid)),
            CTransform4f::Translate(from),
            CMaterialFilter::MakeIncludeExclude(
                CMaterialList(kMT_Solid), CMaterialList(kMT_ProjectilePassthrough, kMT_Player,
                                                        kMT_Character, kMT_CameraPassthrough)),
            nearList)) {
      d = -1.f;
      return true;
    }
    if (clear) {
      TUniqueId intersectId = kInvalidUniqueId;
      CCollisionInfo info;
      double dTmp = deltaMag;
      if (CGameCollision::DetectCollision_Cached_Moving(
              mgr, cache,
              CCollidableSphere(CSphere(CVector3f::Zero(), radius), CMaterialList(kMT_Solid)),
              CTransform4f::Translate(from),
              CMaterialFilter::MakeIncludeExclude(
                  CMaterialList(kMT_Solid), CMaterialList(kMT_ProjectilePassthrough, kMT_Player,
                                                          kMT_Character, kMT_CameraPassthrough)),
              nearList, deltaNorm, intersectId, info, dTmp)) {
        d = static_cast< float >(dTmp);
        clear = false;
      }
    }
  }
  return !clear;
}

CVector3f CBallCamera::FindDesiredPosition(float distance, float elevation, CVector3f dir,
                                           CStateManager& mgr, bool fullTest) {
  const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(GetWatchedObject()));
  if (!player) {
    return CVector3f(0.f, 0.f, 0.f);
  }

  CVector3f useDir = dir;
  if (!dir.CanBeNormalized()) {
    useDir = CVector3f(0.f, 1.f, 0.f);
  }

  const CTransform4f lookDirXf = CTransform4f::LookAt(CVector3f::Zero(), useDir);
  const CVector3f ballPos = player->GetBallPosition();
  float dist = distance;
  float elev = elevation;
  ConstrainElevationAndDistance(elev, dist, 0.f, mgr);

  CVector3f eyePos = mgr.GetPlayer()->GetEyePosition();
  if (!mgr.RayCollideWorld(ballPos, eyePos, kLineOfSightFilter, nullptr)) {
    eyePos = ballPos;
  }

  CVector3f idealLookVec(0.f, -dist, elev);
  idealLookVec[kDZ] -= eyePos.GetZ() - ballPos.GetZ();
  idealLookVec = lookDirXf.GetRotation() * idealLookVec;
  CVector3f lookVec(0.f, distance, elev);
  lookVec[kDZ] -= eyePos.GetZ() - ballPos.GetZ();
  const float minSeekDist = dist;
  float idealLookDist = idealLookVec.Magnitude();
  const CRelAngle resolveLOSIntervalAng = CRelAngle::FromDegrees(30.f);
  float seekStep = 0.3f;
  const bool clear = !DetectCollision(eyePos, eyePos + idealLookVec, 0.3f, idealLookDist, mgr);
  bool foundClear = false;

  if (!clear && idealLookDist <= 0.f) {
    CAABox aabb(ballPos.GetX() - distance, ballPos.GetY() - distance, ballPos.GetZ(),
                ballPos.GetX() + distance, ballPos.GetY() + distance, elev + ballPos.GetZ());
    TEntityList nearList;
    const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mCollisionActorId));
    mgr.BuildNearList(nearList, aabb, kLineOfSightFilter, actor);

    const CTransform4f xfNeg =
        CQuaternion::ZRotation(CRelAngle(-resolveLOSIntervalAng.AsRadians())).BuildTransform4f();
    const CTransform4f xfPos = CQuaternion::ZRotation(resolveLOSIntervalAng).BuildTransform4f();

    while (!foundClear && idealLookDist > minSeekDist) {
      const float lookDist = idealLookDist;
      idealLookVec = lookDist * idealLookVec.Normalize();
      CVector3f lookVecNeg = xfNeg.Rotate(idealLookVec);
      CVector3f lookVecPos = xfPos.Rotate(idealLookVec);

      for (int i = 0; CCast::LtoF(i) < 180.f / resolveLOSIntervalAng.AsDegrees(); ++i) {
        if (mgr.RayCollideWorld(eyePos, eyePos + lookVecNeg, nearList, kLineOfSightFilter,
                                nullptr)) {
          foundClear = true;
          lookVec = lookVecNeg;
          break;
        }

        if (mgr.RayCollideWorld(eyePos, eyePos + lookVecPos, nearList, kLineOfSightFilter,
                                nullptr)) {
          foundClear = true;
          lookVec = lookVecPos;
          break;
        }

        lookVecNeg = xfNeg * lookVecNeg;
        lookVecPos = xfPos * lookVecPos;
      }

      idealLookDist -= seekStep;
    }
  } else {
    if (idealLookDist < 2.f) {
      idealLookVec = 2.f * idealLookVec.Normalize();
      idealLookDist = 2.f;
    }

    const CTransform4f xfNeg =
        CQuaternion::ZRotation(CRelAngle(-resolveLOSIntervalAng.AsRadians())).BuildTransform4f();
    CVector3f lookVecNeg = xfNeg.Rotate(idealLookVec);
    const CTransform4f xfPos = CQuaternion::ZRotation(resolveLOSIntervalAng).BuildTransform4f();
    CVector3f lookVecPos = xfPos.Rotate(idealLookVec);

    if (clear || (!fullTest && (idealLookDist > 2.f || mBallVelFlat > 1.25f))) {
      const float lookDist = idealLookDist;
      lookVec = lookDist * idealLookVec.Normalize();
      foundClear = true;
    } else {
      for (int i = 0; CCast::LtoF(i) < 180.f / resolveLOSIntervalAng.AsDegrees(); ++i) {
        idealLookDist = lookVecNeg.Magnitude();
        const bool negClear =
            !DetectCollision(eyePos, eyePos + lookVecNeg, 0.3f, idealLookDist, mgr);
        if (negClear || idealLookDist > 2.f) {
          const float lookDist = idealLookDist;
          lookVec = lookDist * lookVecNeg.Normalize();
          foundClear = true;
          break;
        }

        idealLookDist = lookVecPos.Magnitude();
        const bool posClear =
            !DetectCollision(eyePos, eyePos + lookVecPos, 0.3f, idealLookDist, mgr);
        if (posClear || idealLookDist > 2.f) {
          const float lookDist = idealLookDist;
          lookVec = lookDist * lookVecPos.Normalize();
          foundClear = true;
          break;
        }

        lookVecNeg = xfNeg * lookVecNeg;
        lookVecPos = xfPos * lookVecPos;
      }

      if (!foundClear) {
        CAABox findBounds(ballPos.GetX() - distance, ballPos.GetY() - distance, ballPos.GetZ(),
                          ballPos.GetX() + distance, ballPos.GetY() + distance,
                          elev + ballPos.GetZ());
        TEntityList nearList;
        const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mCollisionActorId));
        mgr.BuildNearList(nearList, findBounds, kLineOfSightFilter, actor);

        const CTransform4f xfNeg2 =
            CQuaternion::ZRotation(CRelAngle(-resolveLOSIntervalAng.AsRadians()))
                .BuildTransform4f();
        const CTransform4f xfPos2 =
            CQuaternion::ZRotation(resolveLOSIntervalAng).BuildTransform4f();

        while (!foundClear && idealLookDist > minSeekDist) {
          const float lookDist = idealLookDist;
          idealLookVec = lookDist * idealLookVec.Normalize();
          CVector3f lookVecNeg2 = xfNeg2.Rotate(idealLookVec);
          CVector3f lookVecPos2 = xfPos2.Rotate(idealLookVec);

          for (int i = 0; CCast::LtoF(i) < 180.f / resolveLOSIntervalAng.AsDegrees(); ++i) {
            if (mgr.RayCollideWorld(eyePos, eyePos + lookVecNeg2, nearList, kLineOfSightFilter,
                                    nullptr)) {
              foundClear = true;
              lookVec = lookVecNeg2;
              break;
            }

            if (mgr.RayCollideWorld(eyePos, eyePos + lookVecPos2, nearList, kLineOfSightFilter,
                                    nullptr)) {
              foundClear = true;
              lookVec = lookVecPos2;
              break;
            }

            lookVecNeg2 = xfNeg2 * lookVecNeg2;
            lookVecPos2 = xfPos2 * lookVecPos2;
          }

          idealLookDist -= seekStep;
        }
      }
    }
  }

  if (!foundClear) {
    return GetTranslation();
  }

  CVector3f ret = eyePos + lookVec;
  return ret;
}

CTransform4f CBallCamera::FindDesiredTransform(CVector3f dir, CStateManager& mgr) {
  CVector3f useDir = dir;
  if (!dir.CanBeNormalized()) {
    useDir = CVector3f(0.f, 1.f, 0.f);
  }

  float distance = mCurMinDistance;
  float elevation = mElevation;
  ConstrainElevationAndDistance(elevation, distance, 0.f, mgr);

  CVector3f desiredPos = FindDesiredPosition(distance, elevation, useDir, mgr, false);
  CVector3f pos = desiredPos;
  UpdateLookAtPosition(0.f, mgr);
  CTransform4f xf = CTransform4f::LookAt(pos, mLookPos, CVector3f::Up());
  return xf;
}

void CBallCamera::UpdateObjectTooCloseId(CStateManager& mgr) {
  mTooCloseActorDist = 1000000.f;
  mTooCloseActorId = kInvalidUniqueId;

  CVector3f ballPos = mgr.GetPlayer()->GetBallPosition();
  CVector3f cam = GetTranslation();
  CAABox bounds = CAABox::Identity();
  CObjectList objList = mgr.ObjectListById(kOL_PlatformAndDoor);

  for (int i = objList.GetFirstObjectIndex(); i != -1; i = objList.GetNextObjectIndex(i)) {
    CPhysicsActor* actor = TCastToPtr< CScriptDoor >(objList[i]);
    if (actor != nullptr && actor->GetCurrentAreaId() == mgr.GetPlayer()->GetCurrentAreaId()) {
      bounds = actor->GetBoundingBox();
      const float cameraDist = CVector3f(actor->GetTranslation() - cam).Magnitude();
      const float ballDist = CVector3f(actor->GetTranslation() - ballPos).Magnitude();
      float dist = CMath::Min(cameraDist, ballDist);
      if (dist < 30.f && dist < mTooCloseActorDist) {
        mTooCloseActorId = actor->GetUniqueId();
        mTooCloseActorDist = dist;
      }
    }
  }
}

const bool CBallCamera::ConstrainElevationAndDistance(float& elevation, float& distance, float dt,
                                                      CStateManager& mgr) {
  const CPlayer* player = mgr.GetPlayer();
  CVector3f ballToCam = GetTranslation() - player->GetBallPosition();
  float ballToCamMag = 0.f;
  if (ballToCam.CanBeNormalized()) {
    CVector2f flat(ballToCam.GetX(), ballToCam.GetY());
    ballToCamMag = flat.Magnitude();
  } else {
    ballToCam = -player->GetMovementDirection();
  }

  const CScriptDoor* door = TCastToConstPtr< CScriptDoor >(mgr.GetObjectById(mTooCloseActorId));
  bool doorClose = false;
  const float curDistance = distance;
  float newDistance = curDistance;
  float stretchFac = 1.f;
  float baseElevation = elevation;
  float springMul = 1.f;
  if (door) {
    if (!door->IsBallDoor()) {
      stretchFac = CMath::Limit(CMath::AbsF(mTooCloseActorDist / (3.f * curDistance)), 1.f);
      if (mTooCloseActorDist < 3.f * curDistance) {
        doorClose = true;
      }
      if (door->IsOpen()) {
        newDistance = stretchFac * (distance - mConservativeDoorCamDistance) +
                      mConservativeDoorCamDistance;
      } else {
        newDistance = stretchFac * (distance - 5.f) + 5.f;
      }
      if (mObtuseDirection) {
        newDistance = newDistance * (1.f + mSpeedFactor);
      }
      baseElevation = 0.75f;
      if (!door->IsOpen()) {
        baseElevation = 1.5f;
      }
      springMul = 4.f;
    }
  }

  distance = mBallCameraSpring.ApplyDistanceSpring(newDistance, ballToCamMag, dt * springMul);
  float elevationDelta = elevation - baseElevation;
  elevation = elevationDelta * stretchFac + baseElevation;
  return doorClose;
}

CVector3f CBallCamera::ConstrainYawAngle(const CPlayer& player, float distance, float yawSpeed,
                                         float dt, CStateManager& mgr) {
  CVector3f playerToCamFlat = GetTranslation() - player.GetTranslation();
  playerToCamFlat.SetZ(0.f);

  CVector3f lookDir = player.GetTransform().GetColumn(kDY);
  if (player.GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
    lookDir = player.GetMovementDirection();
    const CScriptDoor* door =
        TCastToConstPtr< CScriptDoor >(mgr.GetObjectById(mTooCloseActorId));
    if ((door == nullptr || (door != nullptr && !door->IsOpen())) &&
        (mState == kBCS_Boost || mState == kBCS_Chase)) {
      lookDir = player.GetLeaveMorphDirection();
    }
  }
  if (player.GetMorphballTransitionState() == CPlayer::kMS_Unmorphing) {
    lookDir = player.GetLeaveMorphDirection();
  }

  lookDir.SetZ(0.f);
  if (lookDir.CanBeNormalized()) {
    lookDir.Normalize();
  } else {
    lookDir = -playerToCamFlat;
  }

  if (playerToCamFlat.CanBeNormalized()) {
    playerToCamFlat.Normalize();
  } else {
    return -lookDir;
  }

  float dot = CVector3f::Dot(playerToCamFlat, -lookDir);
  dot = CMath::Limit(dot, 1.f);
  float angle = acosf(dot);
  if (dot >= 0.99999f) {
    return -lookDir;
  }

  float localAngle = angle / yawSpeed;
  const CRelAngle relAngle(distance * dt * CMath::Clamp(0.f, localAngle, 1.f));
  CQuaternion quat =
      CQuaternion::LookAt(CUnitVector3f(playerToCamFlat), CUnitVector3f(-lookDir), relAngle);
  CQuaternion quatCopy(quat);
  return quatCopy.Transform(playerToCamFlat);
}

CTransform4f CBallCamera::UpdateCameraPositions(float dt, const CTransform4f& oldXf,
                                                const CTransform4f& newXf) {
  CTransform4f useXf(newXf);
  if (CMath::AbsF(oldXf.GetForward().GetZ()) > 0.9f &&
      CMath::AbsF(newXf.GetForward().GetZ()) > 0.9f) {
    if (CVector3f::Dot(oldXf.GetRight(), newXf.GetRight()) > 0.999f) {
    } else {
      CRelAngle ang = CRelAngle::FromDegrees(2.f * dt);
      CQuaternion quat = CQuaternion::ClampedRotateTo(CUnitVector3f(oldXf.GetRight()),
                                                      CUnitVector3f(newXf.GetRight()), ang);
      CQuaternion quatCopy(quat);
      CVector3f useRight = quatCopy.BuildTransform4f() * oldXf.GetRight();
      if (CMath::AbsF(CVector3f::Dot(useRight, newXf.GetForward())) > 0.999f) {
      } else {
        CVector3f up = CVector3f::Cross(newXf.GetForward(), useRight).AsNormalized();
        useRight = CVector3f::Cross(up, newXf.GetForward().AsNormalized());
        useXf = CTransform4f::FromColumns(useRight, newXf.GetForward(), up, newXf.GetTranslation());
      }
    }
  }

  return useXf;
}

void CBallCamera::UpdateTransform(const CVector3f& lookDir, const CVector3f& pos, float dt,
                                  CStateManager& mgr) {
  CTransform4f xf(GetTransform());
  CVector3f useLookDir = lookDir;
  if (mOverrideLookDir) {
    if (mgr.CameraManager()->GetCameraHint(mgr)) {
      useLookDir = mgr.CameraManager()->GetCameraHint(mgr)->GetTransform().GetForward();
    }
  }

  CVector3f lookDirFlat(useLookDir);
  lookDirFlat.SetZ(0.f);
  if (!lookDirFlat.CanBeNormalized()) {
    SetTranslation(pos);
    return;
  }

  CVector3f curLookDir = GetTransform().GetForward();
  if (curLookDir.CanBeNormalized()) {
    curLookDir.Normalize();
  } else {
    SetTransform(CTransform4f::LookAt(pos, pos + useLookDir, CVector3f::Up()));
    return;
  }

  float dot = CVector3f::Dot(curLookDir, useLookDir);
  dot = CMath::Limit(dot, 1.f);

  if (fabsf(dot) >= 1.f - FLT_EPSILON) {
    SetTransform(CTransform4f::LookAt(pos, pos + useLookDir, CVector3f::Up()));
  } else {
    const float angleSpeedMul = CMath::Clamp(0.f, acosf(dot) / (1.0471976f * dt), 1.f);
    CRelAngle angleDelta(dt * (mCurAnglePerSecond * angleSpeedMul));

    float lookUpDot = CVector3f::Dot(useLookDir, CVector3f::Up());
    lookUpDot = CMath::Limit(lookUpDot, 1.f);
    const float lookUpDotAbs = fabsf(lookUpDot);
    float maxAngleDelta = (12.566371f * dt) * (1.f - lookUpDotAbs);

    if (mSplineState == kBSS_Nav) {
      maxAngleDelta = 4.1887903f * dt;
      if (angleDelta.AsRadians() > maxAngleDelta) {
        angleDelta = CRelAngle(maxAngleDelta);
      }
    }

    if (angleDelta.AsRadians() > maxAngleDelta && !mgr.GetPlayer()->IsMorphBallTransitioning() &&
        lookUpDotAbs > 0.999f) {
      angleDelta = CRelAngle(maxAngleDelta);
    }

    switch (mState) {
    case kBCS_Chase:
      if (mChaseAllowed) {
        angleDelta = CRelAngle(dt * (mChaseAnglePerSecond * angleSpeedMul));
      }
      break;
    case kBCS_Boost:
      angleDelta = CRelAngle(dt * (mBoostAnglePerSecond * angleSpeedMul));
      break;
    default:
      break;
    }

    if (mLookAtBall || mgr.CameraManager()->IsInterpolationCameraActive()) {
      mLookAtBall = false;
      CQuaternion quat = CQuaternion::LookAt(CUnitVector3f(curLookDir), CUnitVector3f(useLookDir),
                                             CRelAngle(6.2831855f));
      CQuaternion quatCopy(quat);
      SetTransform(quatCopy.BuildTransform4f() * GetTransform().GetRotation());
    } else {
      CQuaternion quat =
          CQuaternion::LookAt(CUnitVector3f(curLookDir), CUnitVector3f(useLookDir), angleDelta);
      CQuaternion quatCopy(quat);
      SetTransform(quatCopy.BuildTransform4f() * GetTransform().GetRotation());
    }
  }

  SetTranslation(pos);
}

void CBallCamera::UpdatePlayerMovement(float dt, CStateManager& mgr) {
  const CPlayer* player = mgr.GetPlayer();
  mMaxBallVel = fabsf(player->GetActualBallMaxVelocity(dt));

  CVector3f ballPos = player->GetBallPosition();
  mBallDelta = ballPos - mPrevBallPos;
  mBallDeltaFlat = mBallDelta;
  mBallDeltaFlat.SetZ(0.f);
  if (mBallDeltaFlat.CanBeNormalized()) {
    mBallVelFlat = mBallDeltaFlat.Magnitude() / dt;
  } else {
    mBallVelFlat = 0.f;
  }

  mPrevBallPos = ballPos;
  mObtuseDirection = false;

  CVector3f camToBallFlat = ballPos - GetTranslation();
  camToBallFlat.SetZ(0.f);
  if (camToBallFlat.CanBeNormalized()) {
    camToBallFlat.Normalize();
    float dot = CVector3f::Dot(camToBallFlat, player->GetMovementDirection());
    dot = CMath::Limit(dot, 1.f);
    if (fabsf(acosf(dot)) > 1.7453293f) {
      mObtuseDirection = true;
    }
  }

  mSpeedFactor = 0.f;
  float tmpVel = mBallVelFlat - 4.f;
  if (tmpVel > 0.f) {
    float speedFactor = fabsf(sinf(0.017453292f * (90.f * (tmpVel / (mMaxBallVel - 4.f)))));
    mSpeedFactor = CMath::Limit(speedFactor, 1.f);
  }

  mCurMinDistance =
      mSpeedFactor * (mMaxDistance - mTargetMinDistance) + mTargetMinDistance;
  if (mSpeedFactor > 0.5f && player->GetPlayerMovementState() == NPlayer::kMS_OnGround) {
    mSpeedingTime = dt * mSpeedFactor + mSpeedingTime;
  } else {
    mSpeedingTime = 0.f;
  }
  static const float kMaxSpeedingTime = 3.f;
  mSpeedingTime = CMath::Clamp(0.f, mSpeedingTime, kMaxSpeedingTime);
}

CVector3f CBallCamera::InterpolateCameraElevation(CVector3f camPos, float dt) {
  if (mElevation < 2.f) {
    return camPos;
  }

  CVector3f ret = camPos;
  if (!mClearLOS && mObscuringMaterial.HasMaterial(kMT_Floor)) {
    mElevInterpTimer = 1.f;
    ret.SetZ(GetTranslation().GetZ());
    mElevInterpStart = GetTranslation().GetZ();
  } else if (mElevInterpTimer > 0.f) {
    mElevInterpTimer -= dt;
    float timer = CMath::Clamp(0.f, mElevInterpTimer, 1.f);
    float delta = ret.GetZ() - mElevInterpStart;
    ret.SetZ(delta * (1.f - timer) + mElevInterpStart);
  }

  return ret;
}

const bool CBallCamera::ShouldResetSpline(CStateManager& mgr) const {
  bool ret = false;
  if (mState != kBCS_ToBall && !mgr.CameraManager()->IsInterpolationCameraActive() &&
      mgr.GetPlayer()->GetMorphBall()->GetSpiderBallState() != CMorphBall::kSBS_Active &&
      mSplineState == kBSS_Invalid) {
    const int behaviour = mBehaviour;
    if (behaviour >= kBCB_SpindleCamera + 1 || behaviour < kBCB_HintFixedPosition) {
      ret = true;
    }
  }
  return ret;
}

void CBallCamera::BuildSpline(CStateManager& mgr) {
  CVector3f ballPos = mgr.GetPlayer()->GetBallPosition();
  mSplineState = kBSS_Arc;
  mReevalSplineEnd = false;
  mCamSpline.Reset(4);
  mCamSpline.AddKnot(GetTranslation(), CVector3f(0.f, 1.f, 0.f));

  float distance = mCurMinDistance;
  float elevation = mElevation;
  ConstrainElevationAndDistance(elevation, distance, 0.f, mgr);

  const CVector3f& translation = GetTranslation();
  CVector3f halfwayPoint = GetTranslation() + 0.5f * (ballPos - GetTranslation());
  halfwayPoint.SetZ(translation.GetZ());
  CVector3f delta = translation - halfwayPoint;

  CRelAngle angle(0.7853982f);
  CQuaternion rot = CQuaternion::ZRotation(angle);
  if (CVector3f::Cross(mgr.GetPlayer()->GetMovementDirection(), GetTransform().GetColumn(kDY))
          .GetZ() >= 0.f) {
    rot = CQuaternion::ZRotation(CRelAngle(-0.7853982f));
  }

  delta = rot.Transform(delta);
  CVector3f knot1 = halfwayPoint + delta;
  const CVector3f& camPos1 = GetTranslation();
  knot1.SetZ(camPos1.GetZ());
  TUniqueId intersectId = kInvalidUniqueId;
  TEntityList nearList;
  CRayCastResult result = mgr.RayWorldIntersection(intersectId, knot1, -delta.AsNormalized(),
                                                   delta.Magnitude(), kLineOfSightFilter, nearList);
  if (result.IsValid()) {
    knot1 = result.GetPoint() + 1.5f * delta.AsNormalized();
  } else {
    knot1 = halfwayPoint + delta;
  }
  mCamSpline.AddKnot(knot1, CVector3f(0.f, 1.f, 0.f));

  CVector3f moveDir = mgr.GetPlayer()->GetMovementDirection();
  FindDesiredPosition(distance, elevation, moveDir, mgr, false);

  delta = rot.Transform(delta);
  CVector3f knot2 = halfwayPoint + delta;
  const CVector3f& camPos2 = GetTranslation();
  knot2.SetZ(camPos2.GetZ());
  result = mgr.RayWorldIntersection(intersectId, knot2, -delta.AsNormalized(), delta.Magnitude(),
                                    kLineOfSightFilter, nearList);
  if (result.IsValid()) {
    knot2 = result.GetPoint() + 2.f * delta.AsNormalized();
  } else {
    knot2 = halfwayPoint + delta;
  }
  mCamSpline.AddKnot(knot2, CVector3f(0.f, 1.f, 0.f));

  delta = rot.Transform(delta);
  CVector3f knot3 = halfwayPoint + delta;
  const CVector3f& camPos3 = GetTranslation();
  knot3.SetZ(camPos3.GetZ());
  result = mgr.RayWorldIntersection(intersectId, knot3, -delta.AsNormalized(), delta.Magnitude(),
                                    kLineOfSightFilter, nearList);
  if (result.IsValid()) {
    knot3 = result.GetPoint() + 2.f * delta.AsNormalized();
  } else {
    knot3 = halfwayPoint + delta;
  }
  mCamSpline.AddKnot(knot3, CVector3f(0.f, 1.f, 0.f));

  CMaterialList intersectMat;
  if (!SplineIntersectTest(intersectMat, mgr) && intersectMat.HasMaterial(kMT_Wall)) {
    delta = knot1 - halfwayPoint;
    result = mgr.RayWorldIntersection(intersectId, knot1, -delta.AsNormalized(), delta.Magnitude(),
                                      kLineOfSightFilter, nearList);
    if (result.IsValid() && !result.GetMaterial().HasMaterial(kMT_Pillar)) {
      CVector3f adjusted = result.GetPoint() - 1.25f * (0.3f * delta.AsNormalized());
      mCamSpline.SetKnotPosition(1, adjusted);
    }

    delta = knot2 - halfwayPoint;
    result = mgr.RayWorldIntersection(intersectId, knot2, -delta.AsNormalized(), delta.Magnitude(),
                                      kLineOfSightFilter, nearList);
    if (result.IsValid() && !result.GetMaterial().HasMaterial(kMT_Pillar)) {
      CVector3f adjusted = result.GetPoint() - 1.25f * (0.3f * delta.AsNormalized());
      mCamSpline.SetKnotPosition(2, adjusted);
    }

    mCamSpline.UpdateSplineLength();
    if (!SplineIntersectTest(intersectMat, mgr)) {
      mSplineState = kBSS_Invalid;
      return;
    }
  }

  mSplineCtrl = 0.5f;
  mSplineCtrlRange = 0.5f;
  mCamSpline.UpdateSplineLength();
  mCollisionExcludeList = CMaterialList();
}

void CBallCamera::ResetSpline(CStateManager& mgr) {
  CVector3f ballPos = mgr.GetPlayer()->GetBallPosition();
  TEntityList nearList;
  TUniqueId intersectId = kInvalidUniqueId;
  CVector3f rayDir(0.f, 0.f, -1.f);
  CRayCastResult result =
      mgr.RayWorldIntersection(intersectId, ballPos, rayDir, 20.f, kLineOfSightFilter, nearList);
  float downFactor;
  if (result.IsValid()) {
    downFactor = CMath::Clamp(0.f, result.GetTime() / 20.f, 1.f);
  } else {
    downFactor = 1.f;
  }

  const float clampedDownFactor = downFactor;

  mSplineState = kBSS_Nav;
  mReevalSplineEnd = true;
  mCamBehindFloorOrWall = false;
  mCamSpline.Reset(4);

  mCamSpline.AddKnot(GetTranslation(), CVector3f(0.f, 1.f, 0.f));

  float distance = mCurMinDistance;
  float elevation = mElevation;
  ConstrainElevationAndDistance(elevation, distance, 0.f, mgr);

  CVector3f knot1 = mSplineIntermediatePos;
  const CVector3f& camPos = GetTranslation();
  knot1.SetZ(camPos.GetZ());
  mCamSpline.AddKnot(knot1, CVector3f(0.f, 1.f, 0.f));

  CVector3f knot2 =
      knot1 + (0.5f + clampedDownFactor) * (mSplineIntermediatePos - GetTranslation());
  mCamSpline.AddKnot(knot2, CVector3f(0.f, 1.f, 0.f));

  CVector3f pt2Ball = ballPos - knot2;
  pt2Ball.SetZ(0.f);
  if (pt2Ball.CanBeNormalized()) {
    pt2Ball.Normalize();
  } else {
    pt2Ball = mgr.GetPlayer()->GetMovementDirection();
  }

  CVector3f desiredPos = FindDesiredPosition(distance, elevation, pt2Ball, mgr, false);
  mCamSpline.AddKnot(desiredPos, CVector3f(0.f, 1.f, 0.f));
  mCamSpline.UpdateSplineLength();

  CMaterialList intersectMat;
  mCamBehindFloorOrWall = false;
  mCollisionExcludeList = CMaterialList(kMT_Floor, kMT_Ceiling);
  if (!SplineIntersectTest(intersectMat, mgr) &&
      (intersectMat.HasMaterial(kMT_Floor) || intersectMat.HasMaterial(kMT_Wall))) {
    CVector3f newKnot2 = knot1;
    newKnot2.SetZ(knot2.GetZ());
    mCamSpline.SetKnotPosition(2, newKnot2);
    mCamSpline.UpdateSplineLength();
    if (!SplineIntersectTest(intersectMat, mgr) && intersectMat.HasMaterial(kMT_Floor)) {
      mCamBehindFloorOrWall = true;
      mCollisionExcludeList = CMaterialList();
    }
  }

  mSplineCtrl = 0.5f * clampedDownFactor + 2.f;
  mSplineCtrlRange = 2.5f;
}

void CBallCamera::UpdateUsingFreeLook(float dt, CStateManager& mgr) {
  if (mState == kBCS_ToBall || mState == kBCS_FromBall) {
    mSplineState = kBSS_Invalid;
    return;
  }

  if (mSplineState == kBSS_Nav) {
    switch (mBehaviour) {
    case kBCB_HintFixedPosition:
    case kBCB_HintFixedTransform:
    case kBCB_PathCameraDesiredPos:
    case kBCB_PathCamera:
    case kBCB_SpindleCamera:
      mSplineState = kBSS_Invalid;
      return;
    }
  }

  float distance = mCurMinDistance;
  float elevation = mElevation;
  ConstrainElevationAndDistance(elevation, distance, 0.f, mgr);

  CVector3f ballPos = mgr.GetPlayer()->GetBallPosition();
  CVector3f knotToBall = ballPos - mCamSpline.GetKnotPosition(2);
  knotToBall.SetZ(0.f);
  if (knotToBall.CanBeNormalized()) {
    knotToBall.Normalize();
  } else {
    knotToBall = mgr.GetPlayer()->GetMovementDirection();
  }

  CVector3f knot3 = mCamSpline.GetKnotPosition(3);

  CVector3f desiredPos = FindDesiredPosition(distance, elevation, knotToBall, mgr, false);

  if (mReevalSplineEnd) {
    mCamSpline.SetKnotPosition(3, desiredPos);
  }

  mSplineCtrl -= dt;

  float splineT = 1.f - CMath::Clamp(0.f, mSplineCtrl / mSplineCtrlRange, 1.f);

  if (mSplineState == kBSS_Nav) {
    CMaterialList intersectMat;
    if (!SplineIntersectTest(intersectMat, mgr)) {
      mCamSpline.SetKnotPosition(3, knot3);
      if (intersectMat.HasMaterial(kMT_Floor)) {
        mSplineState = kBSS_Invalid;
        return;
      }
    }
  }

  if (mSplineCtrl <= 0.f || (splineT > 0.75f && mClearLOS)) {
    if (mSplineState == kBSS_Arc && !mClearLOS) {
      CMaterialList intersectMat;
      if (!SplineIntersectTest(intersectMat, mgr)) {
        mSplineState = kBSS_Invalid;
      } else {
        CVector3f oldKnot2 = mCamSpline.GetKnotPosition(2);
        CVector3f oldKnot1 = mCamSpline.GetKnotPosition(1);
        BuildSpline(mgr);
        mCamSpline.SetKnotPosition(3, mCamSpline.GetKnotPosition(1));
        mCamSpline.SetKnotPosition(2, mCamSpline.GetKnotPosition(0));
        mCamSpline.SetKnotPosition(1, oldKnot2);
        mCamSpline.SetKnotPosition(0, oldKnot1);
        mCamSpline.UpdateSplineLength();
        mSplineCtrl =
            mSplineCtrlRange -
            mSplineCtrlRange * (mCamSpline.GetKnotT(2) / mCamSpline.GetLength());
        mSplineCtrl -= dt;
        splineT = 1.f - CMath::Clamp(0.f, mSplineCtrl / mSplineCtrlRange, 1.f);
      }
    } else {
      mSplineState = kBSS_Invalid;
    }
  }

  mCamSpline.UpdateSplineLength();
  CVector3f pos =
      mCamSpline.GetInterpolatedSplinePointByLength(splineT * mCamSpline.GetLength())
          .GetTranslation();

  if (CPhysicsActor* act = TCastToPtr< CPhysicsActor >(mgr.ObjectById(mCollisionActorId))) {
    CMaterialFilter filter = act->GetMaterialFilter();
    CMaterialList incl(filter.GetIncludeList());
    incl.Add(kMT_Wall);
    CMaterialList excl(filter.GetExcludeList());
    excl.Add(mCollisionExcludeList);
    CMaterialFilter tmpFilter = CMaterialFilter::MakeIncludeExclude(incl, excl);
    act->SetMaterialFilter(tmpFilter);
    desiredPos = MoveCollisionActor(pos, dt, mgr);
    act->SetMaterialFilter(filter);
  }

  CVector3f lookDir = mLookPos - desiredPos;
  if (mLookAtBall) {
    lookDir = ballPos - desiredPos;
  }

  if (lookDir.CanBeNormalized()) {
    lookDir.Normalize();
    UpdateTransform(lookDir, desiredPos, dt, mgr);
  }

  TeleportCamera(desiredPos, mgr);

  if (mCamBehindFloorOrWall && mSplineCtrl / mSplineCtrlRange < 0.5f) {
    mSplineState = kBSS_Invalid;
  }
}

void CBallCamera::UpdateUsingColliders(float dt, CStateManager& mgr) {
  if (mgr.GetPlayer()->GetBombJumpCounter() == 1) {
    return;
  }

  const CVector3f ballPos = mgr.GetPlayer()->GetBallPosition();
  const CPlayer* player = mgr.GetPlayer();
  if (player->GetBombJumpCounter() == 2) {
    CVector3f lookDir(mLookPos.GetX() - GetTransform().Get03(),
                      mLookPos.GetY() - GetTransform().Get13(),
                      mLookPos.GetZ() - GetTransform().Get23());
    if (mLookAtBall) {
      lookDir = CVector3f(ballPos.GetX() - GetTransform().Get03(),
                          ballPos.GetY() - GetTransform().Get13(),
                          ballPos.GetZ() - GetTransform().Get23());
    }

    if (lookDir.CanBeNormalized()) {
      lookDir.Normalize();
      UpdateTransform(lookDir, GetTranslation(), dt, mgr);
    }
    return;
  }

  if (player->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed &&
      !mAvoidGeometryFull) {
    return;
  }

  const CTransform4f oldXf = GetTransform();
  const CVector3f oldPos = GetTranslation();
  mSmallCollidersObsCount = CountObscuredColliders(mSmallColliders);
  mMediumCollidersObsCount = CountObscuredColliders(mMediumColliders);
  mLargeCollidersObsCount = CountObscuredColliders(mLargeColliders);

  CVector3f ballToCamFlat(GetTransform().Get03() - ballPos.GetX(),
                          GetTransform().Get13() - ballPos.GetY(),
                          GetTransform().Get23() - ballPos.GetZ());
  CVector3f posAtBallLevel(0.f, 0.f, ballToCamFlat.GetZ());
  ballToCamFlat[kDZ] = 0.f;
  float ballToCamFlatMag = 0.f;
  if (ballToCamFlat.CanBeNormalized()) {
    ballToCamFlatMag = ballToCamFlat.Magnitude();
  } else {
    ballToCamFlat = -player->GetMovementDirection();
  }

  posAtBallLevel = CVector3f(GetTransform().Get03() - posAtBallLevel.GetX(),
                             GetTransform().Get13() - posAtBallLevel.GetY(),
                             GetTransform().Get23() - posAtBallLevel.GetZ());
  CTransform4f ballToUnderCamLook = CTransform4f::Identity();
  if (CVector3f(posAtBallLevel - ballPos).CanBeNormalized()) {
    ballToUnderCamLook = CTransform4f::LookAt(ballPos, posAtBallLevel, CVector3f::Up());
  }

  float distance = mBallCameraSpring.ApplyDistanceSpring(mCurMinDistance, ballToCamFlatMag,
                                                             dt * (3.f + mSpeedFactor));
  CVector3f camToBall(ballPos.GetX() - GetTransform().Get03(),
                      ballPos.GetY() - GetTransform().Get13(),
                      ballPos.GetZ() - GetTransform().Get23());
  camToBall[kDZ] = 0.f;
  if (camToBall.CanBeNormalized()) {
    camToBall.Normalize();
    float dot = CVector3f::Dot(camToBall, player->GetMovementDirection());
    dot = CMath::Limit(dot, 1.f);
    if (CMath::AbsF(acosf(dot)) > (150.f * (M_PIF / 180.f))) {
      CVector3f velocity = player->GetVelocityWR();
      if (velocity.CanBeNormalized()) {
        distance = mBallCameraSpring.ApplyDistanceSpring(
            mCurMinDistance + mSpeedFactor * (mBackwardsDistance - mCurMinDistance),
            ballToCamFlatMag, 3.f * dt);
      }
    }
  }

  mCollidersAABB = CalculateCollidersBoundingBox(mLargeColliders, mgr);
  TEntityList nearList;
  mgr.BuildNearList(nearList, mCollidersAABB, kLineOfSightFilter,
                    TCastToConstPtr< CActor >(mgr.GetObjectById(mCollisionActorId)));

  if (!mClearLOS && mObscuringObjectId == kInvalidUniqueId) {
    if (mObscuredTime > 0.f || mObscuringMaterial.HasMaterial(kMT_Floor) ||
        mObscuringMaterial.HasMaterial(kMT_Wall)) {
      mColliderMag += 2.f * dt;
      if (mColliderMag < 2.f) {
        mColliderMag = 2.f;
      }
      if (mColliderMag > 2.f) {
        mColliderMag = 2.f;
      }
      UpdateCollidersDistances(mSmallColliders, 7.f * 0.33f * mColliderMag,
                               7.f * 0.33f * mColliderMag / 2.f, -M_PIF / 2.f);
      UpdateCollidersDistances(mMediumColliders, 7.f * 0.66f * mColliderMag,
                               7.f * 0.66f * mColliderMag / 2.f, -M_PIF / 2.f);
      UpdateCollidersDistances(mLargeColliders, 7.f * mColliderMag,
                               7.f * mColliderMag / 2.f, -M_PIF / 2.f);
    }
  } else {
    float targetColliderMag = 1.f;
    if (mPrevClearLOS && player->GetMoveSpeed() < 1.f) {
      targetColliderMag = 0.25f;
    }
    mColliderMag += 2.f * ((targetColliderMag - mColliderMag) * dt);
    UpdateCollidersDistances(mSmallColliders, mColliderMag * (7.f * 0.33f),
                             mColliderMag * (7.f * 0.33f), -M_PIF / 2.f);
    UpdateCollidersDistances(mMediumColliders, mColliderMag * (7.f * 0.66f),
                             mColliderMag * (7.f * 0.66f), -M_PIF / 2.f);
    UpdateCollidersDistances(mLargeColliders, mColliderMag * 7.f, mColliderMag * 7.f,
                             -M_PIF / 2.f);
  }

  float elevation = mElevation;
  bool interpolateElevation = true;
  if (ConstrainElevationAndDistance(elevation, distance, dt, mgr)) {
    interpolateElevation = false;
  }
  CVector3f desiredBallToCam(0.f, distance, elevation);
  desiredBallToCam = ballToUnderCamLook.Rotate(desiredBallToCam);

  const CScriptDoor* door = TCastToConstPtr< CScriptDoor >(mgr.GetObjectById(mTooCloseActorId));
  if (door == nullptr || (door != nullptr && !door->IsOpen())) {
    if (mState == kBCS_Boost) {
      CVector3f ballToCam(GetTranslation() - ballPos);
      if (ballToCam.CanBeNormalized()) {
        ballToCam.Normalize();
      } else {
        ballToCam = GetTransform().GetColumn(kDY);
      }
      if (CMath::AbsF(ballToCamFlatMag - mBoostElevation) < 1.f) {
        const float distance = gpTweakBall->GetBallCameraBoostDistance();
        const float yawSpeed = gpTweakBall->GetBallCameraBoostYawSpeed();
        ballToCam = ConstrainYawAngle(*player, distance, yawSpeed, dt, mgr);
      }
      ballToCam[kDZ] = 0.f;
      ballToCam.Normalize();
      ballToCam *= distance;
      ballToCam[kDZ] = 1.f;
      desiredBallToCam = ballToCam;
      interpolateElevation = false;
    }
    if (mChaseAllowed &&
        (mState == kBCS_Chase || mBehaviour == kBCB_FreezeLookPosition)) {
      CVector3f ballToCam(GetTranslation() - ballPos);
      if (ballToCam.CanBeNormalized()) {
        ballToCam.Normalize();
      } else {
        ballToCam = GetTransform().GetColumn(kDY);
      }
      if (CMath::AbsF(ballToCamFlatMag - mChaseElevation) < 3.f) {
        const float distance = gpTweakBall->GetBallCameraChaseDistance();
        const float yawSpeed = gpTweakBall->GetBallCameraChaseYawSpeed();
        ballToCam = ConstrainYawAngle(*player, distance, yawSpeed, dt, mgr);
      }
      ballToCam[kDZ] = 0.f;
      ballToCam.Normalize();
      ballToCam *= distance;
      ballToCam[kDZ] = 2.736f;
      desiredBallToCam = ballToCam;
      interpolateElevation = false;
    }
  }

  switch (mBehaviour) {
  default:
    break;
  case kBCB_HintBallToCam: {
    desiredBallToCam = mOverrideBallToCam;
    if (mObscureAvoidance) {
      CVector3f ballToCamDir = desiredBallToCam;
      if (ballToCamDir.CanBeNormalized()) {
        ballToCamDir.Normalize();
      } else {
        ballToCamDir = -player->GetMovementDirection();
      }
      TUniqueId intersectId = kInvalidUniqueId;
      const CRayCastResult& result = mgr.RayWorldIntersection(
          intersectId, ballPos, ballToCamDir, distance, kLineOfSightFilter, nearList);
      const float hitTime = result.GetTime();
      if (result.IsValid()) {
        float hitX = hitTime * ballToCamDir.GetX();
        float hitY = hitTime * ballToCamDir.GetY();
        float hitZ = hitTime * ballToCamDir.GetZ();
        desiredBallToCam = CVector3f(0.9f * hitX, 0.9f * hitY, 0.9f * hitZ);
      }
    }
    interpolateElevation = false;
    break;
  }
  }

  distance = desiredBallToCam.Magnitude();
  CVector3f desiredCamPos(ballPos.GetX() + desiredBallToCam.GetX(),
                          ballPos.GetY() + desiredBallToCam.GetY(),
                          ballPos.GetZ() + desiredBallToCam.GetZ());
  float collDist = 0.f;
  bool noCollision = !DetectCollision(ballPos,
                                      CVector3f(ballPos.GetX() + desiredBallToCam.GetX(),
                                                ballPos.GetY() + desiredBallToCam.GetY(),
                                                ballPos.GetZ() + desiredBallToCam.GetZ()),
                                      0.3f, collDist, mgr);
  if (!noCollision) {
    const float collisionDistance = collDist;
    if (collisionDistance >= 1.f) {
      const CVector3f& normalizedDir = desiredBallToCam.AsNormalized();
      desiredBallToCam = CVector3f(collisionDistance * normalizedDir.GetX(),
                                   collisionDistance * normalizedDir.GetY(),
                                   collisionDistance * normalizedDir.GetZ());
      desiredCamPos = CVector3f(ballPos.GetX() + desiredBallToCam.GetX(),
                                ballPos.GetY() + desiredBallToCam.GetY(),
                                ballPos.GetZ() + desiredBallToCam.GetZ());
    } else {
      desiredCamPos = GetTranslation();
      desiredBallToCam =
          CVector3f(desiredCamPos.GetX() - ballPos.GetX(), desiredCamPos.GetY() - ballPos.GetY(),
                    desiredCamPos.GetZ() - ballPos.GetZ());
    }
  }

  CTransform4f lookXf = CTransform4f::LookAt(desiredCamPos, mLookPos, CVector3f::Up());
  CTransform4f oldLookXf = CTransform4f::LookAt(GetTranslation(), mLookPos, CVector3f::Up());
  mNextLookXf = lookXf;
  lookXf = oldLookXf;

  CVector3f colliderPointLocal = CVector3f::Zero();
  if (mAvoidGeometryFull || !mClearLOS) {
    colliderPointLocal = AvoidGeometryFull(lookXf, nearList, dt, mgr);
  } else {
    colliderPointLocal = AvoidGeometry(lookXf, nearList, dt, mgr);
  }

  CVector3f oldBallToCamFlat(CVector3f(GetTranslation() - ballPos).DropZ(), 0.f);
  if (oldBallToCamFlat.Magnitude() < 2.f) {
    if (mClearLOS && mShortMoveCount > 2) {
      colliderPointLocal *= 1.f / float(mShortMoveCount);
    }
    if (collDist < 3.f) {
      colliderPointLocal *= 0.25f;
      if (mClearLOS && mShortMoveCount > 0) {
        colliderPointLocal *= mSpeedFactor;
      }
    }
    if (collDist < 1.f) {
      colliderPointLocal = CVector3f::Zero();
    }
  }

  CVector3f rotatedColliderPoint = lookXf.Rotate(colliderPointLocal);
  CVector3f camDelta(rotatedColliderPoint[kDX] + desiredCamPos[kDX] - ballPos[kDX],
                     rotatedColliderPoint[kDY] + desiredCamPos[kDY] - ballPos[kDY],
                     rotatedColliderPoint[kDZ] + desiredCamPos[kDZ] - ballPos[kDZ]);
  if (camDelta.CanBeNormalized()) {
    camDelta.Normalize();
  }
  float desiredX = distance * camDelta[kDX];
  float desiredY = distance * camDelta[kDY];
  float desiredZ = distance * camDelta[kDZ];
  CVector3f desiredPos(ballPos.GetX() + desiredX, ballPos.GetY() + desiredY,
                       ballPos.GetZ() + desiredZ);

  if (mBehaviour == kBCB_PathCameraDesiredPos) {
    const CPathCamera* pathCam =
        TCastToConstPtr< CPathCamera >(mgr.GetObjectById(mgr.CameraManager()->GetPathCameraId()));
    if (pathCam != nullptr) {
      desiredPos = pathCam->GetTranslation();
    }
  }

  camDelta = mDampedPos - desiredPos;
  float dampDeltaMag = camDelta.Magnitude();
  if (camDelta.CanBeNormalized()) {
    camDelta.Normalize();
  }
  float springDist = mBallCameraCentroidSpring.ApplyDistanceSpring(0.f, dampDeltaMag, dt);
  float springX = springDist * camDelta[kDX];
  float springY = springDist * camDelta[kDY];
  float springZ = springDist * camDelta[kDZ];
  mDampedPos = CVector3f(desiredPos.GetX() + springX, desiredPos.GetY() + springY,
                             desiredPos.GetZ() + springZ);

  CVector3f posDelta(oldPos.GetX() - mDampedPos.GetX(), oldPos.GetY() - mDampedPos.GetY(),
                     oldPos.GetZ() - mDampedPos.GetZ());
  float posDeltaMag = posDelta.Magnitude();
  if (posDelta.CanBeNormalized()) {
    posDelta.Normalize();
  }

  float springSpeed = 1.f;
  if (mObtuseDirection) {
    springSpeed = 3.f;
  }
  float springMag =
      mBallCameraCentroidDistanceSpring.ApplyDistanceSpring(0.f, posDeltaMag, dt * springSpeed);
  if (GetState() == kBCS_Boost) {
    springMag = mBallCameraBoostSpring.ApplyDistanceSpring(0.f, posDeltaMag, dt);
  } else if (mChaseAllowed) {
    if (mState == kBCS_Chase || mBehaviour == kBCB_FreezeLookPosition) {
      springMag = mBallCameraChaseSpring.ApplyDistanceSpring(0.f, posDeltaMag, dt);
    }
  }

  float finalX = springMag * posDelta.GetX();
  float finalY = springMag * posDelta.GetY();
  float finalZ = springMag * posDelta.GetZ();
  CVector3f finalPos(mDampedPos.GetX() + finalX, mDampedPos.GetY() + finalY,
                     mDampedPos.GetZ() + finalZ);
  if (player->GetMorphBall()->GetSpiderBallState() != CMorphBall::kSBS_Active &&
      !mNoElevationVelClamp && player->GetVelocityWR().GetZ() > 8.f) {
    CVector3f delta(finalPos.GetX() - oldPos.GetX(), finalPos.GetY() - oldPos.GetY(),
                    finalPos.GetZ() - oldPos.GetZ());
    delta[kDZ] = CMath::Limit(delta.GetZ(), 0.1f * dt);
    finalPos = CVector3f(oldPos.GetX() + delta.GetX(), oldPos.GetY() + delta.GetY(),
                         oldPos.GetZ() + delta.GetZ());
  }

  if (interpolateElevation && mState != kBCS_ToBall) {
    finalPos = InterpolateCameraElevation(finalPos, dt);
  }
  if (mNoElevationInterp) {
    finalPos[kDZ] = elevation + ballPos.GetZ();
  }

  if (oldBallToCamFlat.Magnitude() < 2.f) {
    if (finalPos.GetZ() < 2.f + ballPos.GetZ()) {
      finalPos[kDZ] = 2.f + ballPos.GetZ();
    }
    mBallCameraSpring.Reset();
  }

  finalPos = ClampElevationToWater(finalPos, mgr);
  if (oldBallToCamFlat.Magnitude() < 2.f) {
    if (mTooCloseActorId != kInvalidUniqueId && mTooCloseActorDist < 5.f) {
      door = TCastToConstPtr< CScriptDoor >(mgr.GetObjectById(TUniqueId(mTooCloseActorId)));
      if (door != nullptr && !door->IsOpen()) {
        finalPos[kDX] = GetTranslation()[kDX];
        finalPos[kDY] = GetTranslation()[kDY];
        finalPos[kDZ] = GetTranslation()[kDZ];
      }
    }
  }

  const float backupZ = finalPos.GetZ();
  finalPos = MoveCollisionActor(finalPos, dt, mgr);
  if (mClearLOS && mShortMoveCount > 0) {
    finalPos[kDZ] = backupZ;
    finalPos = MoveCollisionActor(finalPos, dt, mgr);
  }

  CVector3f lookDir(mLookPos.GetX() - finalPos.GetX(), mLookPos.GetY() - finalPos.GetY(),
                    mLookPos.GetZ() - finalPos.GetZ());
  if (mLookAtBall) {
    lookDir = ballPos - finalPos;
  }
  if (lookDir.CanBeNormalized()) {
    lookDir.Normalize();
    UpdateTransform(lookDir, finalPos, dt, mgr);
  }

  if (mClampVelTimer > 0.f) {
    mClampVelTimer -= dt;
  }
}

void CBallCamera::UpdateUsingTransitions(float dt, CStateManager& mgr) {
  if (mState == kBCS_FromBall) {
    UpdateTransitionFromBallCamera(mgr);
    return;
  }

  mLookAtBall = false;
  const CPlayer* player = mgr.GetPlayer();
  CVector3f ballPos = player->GetBallPosition();
  CVector3f eyePos = player->GetEyePosition();
  float lookZ = ballPos.GetZ() + mLookAtOffset.GetZ();
  ballPos[kDZ] = lookZ;

  CVector3f lookDir = GetTransform().GetForward();
  CTransform4f oldXf(GetTransform());

  switch (mState) {
  case kBCS_ToBall: {
    float distance = mTargetMinDistance;
    float elevation = mElevation;
    ConstrainElevationAndDistance(elevation, distance, dt, mgr);
    distance = mTargetMinDistance;

    bool nearDoor = false;
    if (CheckDoorProximity(GetTranslation(), mgr) || mShortMoveCount > 2) {
      nearDoor = true;
    }

    CVector3f toDesired =
        FindDesiredPosition(distance, elevation, mgr.GetPlayer()->GetMovementDirection(), mgr,
                            nearDoor) -
        eyePos;
    CVector3f finalPos = eyePos + toDesired * player->GetMorphBallTransitionFactor();

    if (CPhysicsActor* act = TCastToPtr< CPhysicsActor >(mgr.ObjectById(mCollisionActorId))) {
      act->SetTranslation(GetTranslation());
      finalPos = ClampElevationToWater(finalPos, mgr);
      finalPos = MoveCollisionActor(finalPos, dt, mgr);

      CVector3f camToLookDir = player->GetTransform().GetForward();
      camToLookDir = mLookPos - finalPos;
      if (camToLookDir.CanBeNormalized()) {
        camToLookDir.Normalize();
        float absDevDot = CMath::AbsF(CMath::Limit(CVector3f::Dot(lookDir, camToLookDir), 1.f));
        float halfMorphFactor = CMath::Limit(player->GetMorphBallTransitionFactor() * 0.5f, 1.f);
        CRelAngle devAngle(acosf(absDevDot) * halfMorphFactor);
        if (absDevDot < 1.f - 0.00001f) {
          CQuaternion quat = CQuaternion::LookAt(CUnitVector3f(oldXf.GetForward()),
                                                 CUnitVector3f(camToLookDir), devAngle);
          CQuaternion quatCopy(quat);
          SetTransform(quatCopy.BuildTransform4f() * oldXf.GetRotation());
        } else {
          SetTransform(CTransform4f::LookAt(CVector3f::Zero(), camToLookDir));
        }
      }
    }
    SetTransform(ValidateCameraTransform(GetTransform(), oldXf));
    SetTranslation(finalPos);
    TeleportCamera(finalPos, mgr);
    break;
  }
  case kBCS_FromBall: {
    if (close_enough(player->GetMorphBallTransitionFactor(), 1.f)) {
      SetTransform(player->GetTransform());
      SetTranslation(player->GetEyePosition());
      return;
    }

    float morphT = player->GetMorphBallTransitionFactor();
    morphT = CMath::Limit(morphT / 0.9f, 1.f);

    CVector3f finalPos = GetTranslation();
    CVector3f eyeToCam = GetTranslation() - eyePos;
    eyeToCam.SetZ(0.f);
    if (eyeToCam.CanBeNormalized()) {
      float distance = eyeToCam.Magnitude();
      float maxDist = (1.f - player->GetMorphBallTransitionFactor()) * mCurMinDistance;
      if (distance > maxDist) {
        distance = maxDist;
      }

      CVector3f playerToCamDir = GetTranslation() - player->GetTranslation();
      float yawSpeed = M_PIF;
      playerToCamDir.SetZ(0.f);
      CVector3f moveDir = player->GetMovementDirection();
      if (playerToCamDir.CanBeNormalized()) {
        playerToCamDir.Normalize();
      } else {
        playerToCamDir = -moveDir;
      }
      if (moveDir.CanBeNormalized()) {
        moveDir.Normalize();
        float devDot = CVector3f::Dot(playerToCamDir, -moveDir);
        devDot = CMath::Limit(devDot, 1.f);
        float angle = CMath::AbsF(acosf(devDot));
        angle *= morphT;
        yawSpeed = angle / dt;
      }

      CVector3f useLookDir =
          ConstrainYawAngle(*player, yawSpeed, CRelAngle::FromDegrees(10.f).AsRadians(), dt, mgr);
      useLookDir.SetZ(0.f);
      useLookDir.Normalize();

      const CVector3f& camPos = GetTranslation();
      finalPos = eyePos + distance * useLookDir;
      float heightDelta = camPos.GetZ() - eyePos.GetZ();
      finalPos.SetZ(heightDelta * morphT + eyePos.GetZ());
      finalPos = ClampElevationToWater(finalPos, mgr);
      finalPos = MoveCollisionActor(finalPos, dt, mgr);

      CVector3f lookPos = ballPos;
      lookPos.SetZ(morphT * (eyePos.GetZ() - ballPos.GetZ()) + ballPos.GetZ());
      CVector3f finalToBall = lookPos - finalPos;
      finalToBall.SetZ(0.f);
      if (finalToBall.CanBeNormalized()) {
        SetTransform(CTransform4f::LookAt(finalPos, lookPos));
      } else {
        SetTransform(mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform());
      }
    } else {
      SetTransform(mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform());
    }
    SetTranslation(finalPos);
    break;
  }
  default:
    break;
  }

  mgr.CameraManager()->FirstPersonCamera()->Reset(GetTransform(), mgr);
}

void CBallCamera::UpdateUsingSpindleCameras(float dt, CStateManager& mgr) {
  if (const CScriptSpindleCamera* cam = TCastToConstPtr< CScriptSpindleCamera >(
          mgr.ObjectById(mgr.GetCameraManager()->GetSpindleCameraId()))) {
    TeleportCamera(cam->GetTransform(), mgr);
    mLookAtBall = true;
  }
}

void CBallCamera::UpdateUsingPathCameras(float dt, CStateManager& mgr) {
  if (const CPathCamera* cam = TCastToConstPtr< CPathCamera >(
          mgr.ObjectById(mgr.GetCameraManager()->GetPathCameraId()))) {
    TeleportCamera(cam->GetTransform(), mgr);
    mLookAtBall = true;
  }
}

CVector3f CBallCamera::TweenVelocity(const CVector3f& curVel, const CVector3f& newVel, float rate,
                                     float dt) {
  CVector3f ret = curVel;
  CVector3f velDelta = newVel - curVel;
  if (velDelta.CanBeNormalized()) {
    float t = CMath::Limit(velDelta.Magnitude() / (rate * dt), 1.f);
    ret += t * (dt * (rate * velDelta.AsNormalized()));
  } else {
    ret = newVel;
  }
  return ret;
}

CVector3f CBallCamera::ComputeVelocity(CVector3f curVel, CVector3f posDelta, float dt) {
  CVector3f ret = posDelta;
  float mag = ret.Magnitude();
  if (mClampVelTimer > 0.f && ret.CanBeNormalized() && !mObtuseDirection) {
    mag = CMath::Limit(mag, mClampVelRange);
    ret = ret.AsNormalized() * mag;
  }
  return ret;
}

void CBallCamera::UpdateAnglePerSecond(float dt) {
  float delta = mTargetAnglePerSecond - mCurAnglePerSecond;
  if (fabsf(delta) >= 0.0017453292f) {
    float angle = CMath::Limit(delta / M_PIF, 1.f);
    mCurAnglePerSecond = angle * (10.471975f * dt) + mCurAnglePerSecond;
  } else {
    mCurAnglePerSecond = mTargetAnglePerSecond;
  }
}

CVector3f CBallCamera::ClampElevationToWater(CVector3f pos, CStateManager& mgr) const {
  const CScriptWater* water =
      TCastToConstPtr< CScriptWater >(mgr.GetObjectById(mgr.GetPlayer()->InFluidId()));
  CVector3f ret(pos);
  if (water) {
    const float waterZ = water->GetTriggerBoundsWR().GetMaxPoint().GetZ();
    float posZ = pos.GetZ();
    float deltaZ = posZ - waterZ;
    if (posZ >= waterZ && deltaZ <= 0.25f) {
      ret.SetZ(0.25f + waterZ);
    } else if (posZ < waterZ && deltaZ >= -0.12f) {
      ret.SetZ(waterZ - 0.12f);
    }
  }
  return ret;
}

CVector3f CBallCamera::MoveCollisionActor(const CVector3f& pos, float dt, CStateManager& mgr) {
  if (CPhysicsActor* act = TCastToPtr< CPhysicsActor >(mgr.ObjectById(mCollisionActorId))) {
    CVector3f posDelta = pos - act->GetTranslation();
    if (!posDelta.CanBeNormalized() || posDelta.Magnitude() < 0.01f) {
      act->Stop();
      return act->GetTranslation();
    }

    const CVector3f oldVel = act->GetVelocityWR();
    const CVector3f oldTranslation = act->GetTranslation();
    CVector3f newVel = posDelta / dt;
    newVel = ComputeVelocity(oldVel, newVel, dt);
    act->SetVelocityWR(newVel);
    act->SetMovable(true);
    act->AddMaterial(kMT_Solid, mgr);
    CGameCollision::Move(mgr, *act, dt, nullptr);

    CVector3f postDelta = act->GetTranslation() - pos;
    if (postDelta.CanBeNormalized() && postDelta.Magnitude() > 0.1f) {
      act->SetTranslation(oldTranslation);
      CVector3f tweenVel = TweenVelocity(oldVel, newVel, 50.f, dt);
      act->SetVelocityWR(tweenVel);
      CGameCollision::Move(mgr, *act, dt, nullptr);
      postDelta = act->GetTranslation() - pos;
      if (postDelta.Magnitude() > 0.1f) {
        mShortMoveCount += 1;
      } else {
        mShortMoveCount = 0;
      }
    } else {
      act->Stop();
      mShortMoveCount = 0;
    }

    act->SetMovable(false);
    act->RemoveMaterial(kMT_Solid, mgr);
    return act->GetTranslation();
  }

  return pos;
}

CVector3f CBallCamera::GetFixedLookTarget(const CVector3f& hintToLookDir,
                                          CStateManager& mgr) const {
  const CScriptCameraHint* hint = mgr.GetCameraManager()->GetCameraHint(mgr);
  if (hint == nullptr) {
    return hintToLookDir;
  }

  CVector3f hintDir = hint->GetTransform().GetColumn(kDY);
  CVector3f hintDirFlat(hintDir);
  hintDirFlat.SetZ(0.f);
  if (hintDir.CanBeNormalized() && hintDirFlat.CanBeNormalized()) {
    hintDir.Normalize();
    hintDirFlat.Normalize();
  } else {
    hintDir = CVector3f(0.f, 1.f, 0.f);
    hintDirFlat = CVector3f(0.f, 1.f, 0.f);
  }

  CVector3f hintToLookDirFlat(hintToLookDir);
  hintToLookDirFlat.SetZ(0.f);
  if (hintToLookDir.CanBeNormalized() && hintToLookDirFlat.CanBeNormalized()) {
    hintToLookDirFlat.Normalize();
  } else {
    hintToLookDirFlat = hintDirFlat;
  }

  float attitude;
  attitude = acosf(CMath::Limit(CVector3f::Dot(hintToLookDir, hintToLookDirFlat), 1.f));
  if (mClampAttitude) {
    float refAttitude = acosf(CMath::Limit(CVector3f::Dot(hintDir, hintDirFlat), 1.f));
    float delta = CMath::Limit(attitude - refAttitude, mAttitudeRange);
    attitude = refAttitude + delta;
  }

  if (hintToLookDir.GetZ() >= 0.f) {
    attitude = -attitude;
  }

  float azimuth;
  azimuth = acosf(CMath::Limit(CVector3f::Dot(hintToLookDirFlat, hintDirFlat), 1.f));
  if (mClampAzimuth) {
    azimuth = CMath::Limit(azimuth, mAzimuthRange);
  }

  if (hintToLookDirFlat.GetX() * hintDirFlat.GetY() -
          hintDirFlat.GetX() * hintToLookDirFlat.GetY() >=
      0.f) {
    azimuth = -azimuth;
  }

  CQuaternion azimuthQuat = CQuaternion::ZRotation(CRelAngle(azimuth));
  CVector3f azimuthLookDirFlat = azimuthQuat.Transform(hintDirFlat);
  CUnitVector3f attitudeAxis(azimuthLookDirFlat.GetY(), -azimuthLookDirFlat.GetX(), 0.f);
  attitudeAxis.Normalize();
  CQuaternion attitudeQuat = CQuaternion::AxisAngle(attitudeAxis, CRelAngle(-attitude));
  azimuthLookDirFlat = attitudeQuat.Transform(azimuthLookDirFlat);
  return azimuthLookDirFlat;
}

void CBallCamera::UpdateUsingFixedCameras(float dt, CStateManager& mgr) {
  if (const CScriptCameraHint* hint = mgr.GetCameraManager()->GetCameraHint(mgr)) {
    switch (mBehaviour) {
    case kBCB_HintFixedPosition: {
      CVector3f hintPos = hint->GetTranslation();
      CVector3f hintToLookPos = mLookPos - hintPos;
      if (hintToLookPos.CanBeNormalized()) {
        CVector3f lookDir = GetFixedLookTarget(hintToLookPos.AsNormalized(), mgr);
        hintToLookPos = lookDir;
        if ((hint->GetOverrideFlags() & 0x40) != 0) {
          mLookAtBall = true;
        }
        UpdateTransform(hintToLookPos, hintPos, dt, mgr);
      }
      break;
    }
    case kBCB_HintFixedTransform:
      const_cast< CTransform4f& >(GetTransform()) = hint->GetTransform();
      SetTransformDirty(true);
      SetTransformDirtySpare(true);
      SetPreRenderHasMoved(true);
      break;
    default:
      break;
    }
    TeleportCamera(GetTranslation(), mgr);
  }
}

void CBallCamera::UpdateLookAtPosition(float dt, CStateManager& mgr) {
  const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(GetWatchedObject()));
  if (player == nullptr) {
    return;
  }

  CVector3f ballPos = player->GetBallPosition();
  if (player->IsMorphBallTransitioning()) {
    mLookPos = ballPos;
    mLookPos.SetZ(mLookPos.GetZ() + mLookAtOffset.GetZ());
    mLookPosAhead = mLookPos;
    mFixedLookPos = mLookPos;
    return;
  }

  CVector3f moveDir = player->GetMovementDirection();
  moveDir.Normalize();

  CVector3f lookAtOffset(mSpeedFactor * mLookAtOffset.GetX(),
                         mSpeedFactor * mLookAtOffset.GetY(), mLookAtOffset.GetZ());
  if (mChaseAllowed &&
      (mState == kBCS_Chase || mBehaviour == kBCB_FreezeLookPosition)) {
    lookAtOffset =
        CVector3f(mSpeedFactor * mChaseLookAtOffset.GetX(),
                  mSpeedFactor * mChaseLookAtOffset.GetY(), mChaseLookAtOffset.GetZ());
  }

  if (mgr.GetCameraManager()->IsInterpolationCameraActive()) {
    lookAtOffset = CVector3f(0.f, 0.f, mLookAtOffset.GetZ());
  }

  CTransform4f moveRot = player->CreateTransformFromMovementDirection().GetRotation();
  if (mBallDeltaFlat.CanBeNormalized()) {
    lookAtOffset = moveRot * lookAtOffset;
  }

  CVector3f prevLook = mLookPos;
  CVector3f lookAhead = ballPos + lookAtOffset;
  mLookPosAhead = lookAhead;
  mFixedLookPos = ballPos + CVector3f(0.f, 0.f, lookAtOffset.GetZ());

  CVector3f lookDelta = prevLook - lookAhead;
  float lookDeltaMag = lookDelta.Magnitude();
  if (lookDelta.CanBeNormalized()) {
    lookDelta.Normalize();
  }

  float speedingTime = mSpeedingTime / 3.f;
  float springScale = 1.f;
  springScale += 2.f * CMath::Clamp(0.f, speedingTime, 1.f);
  float springDist =
      mBallCameraLookAtSpring.ApplyDistanceSpringNoMax(0.f, lookDeltaMag, dt * springScale);
  if (springDist > 0.0001f) {
    lookAhead += springDist * lookDelta;
  }

  lookDelta = lookAhead - prevLook;
  if (mLookAtBall) {
    mLookPos = ballPos;
    mLookPos.SetZ(mLookPos.GetZ() + mLookAtOffset.GetZ());
  } else {
    mLookPos = lookAhead;
  }

  switch (mBehaviour) {
  default:
    if (mgr.GetCameraManager()->IsInterpolationCameraActive()) {
      mLookPos = mLookPosAhead;
      mFixedLookPos = mLookPosAhead;
    }
    break;
  case kBCB_HintFixedPosition:
    mLookPos = mFixedLookPos;
    mLookPosAhead = mLookPos;
    break;
  case kBCB_HintFixedTransform:
  case kBCB_SpindleCamera:
    mLookPos = mFixedLookPos;
    mLookPosAhead = mFixedLookPos;
    break;
  }

  if (mDirectElevation) {
    mLookPos.SetZ(ballPos.GetZ() + mLookAtOffset.GetZ());
    mLookPosAhead.SetZ(mLookPos.GetZ());
    mFixedLookPos.SetZ(mLookPos.GetZ());
  }

  if (mOverrideLookDir && mgr.GetCameraManager()->GetCameraHint(mgr) != nullptr) {
    const CScriptCameraHint* hint = mgr.GetCameraManager()->GetCameraHint(mgr);
    mLookPos = GetTranslation() + 10.f * hint->GetTransform().GetForward();
    mLookPosAhead = mLookPos;
    mFixedLookPos = mLookPos;
  }
}

void CBallCamera::ActivateFailSafe(float dt, CStateManager& mgr) {
  float distance = mCurMinDistance;
  float elevation = mElevation;
  ConstrainElevationAndDistance(elevation, distance, dt, mgr);

  CVector3f desiredPos =
      FindDesiredPosition(distance, elevation, mgr.GetPlayer()->GetMovementDirection(), mgr, true);
  SetTranslation(desiredPos);
  TeleportLookAtStuff(mgr);

  CTransform4f camXf = CTransform4f::LookAt(desiredPos, mLookPos, CVector3f::Up());
  TeleportCamera(camXf, mgr);
  mgr.CameraManager()->SetPlayerCamera(mgr, GetUniqueId());

  mPendingFailsafe = false;
  mObscuredTime = 0.f;
}

void CBallCamera::CheckFailSafe(float dt, CStateManager& mgr) {
  CVector3f ballPos = mgr.Player()->GetBallPosition();
  mPrevClearLOS = mClearLOS;

  CVector3f camToBall = ballPos - GetTranslation();
  float camToBallMag = camToBall.Magnitude();
  camToBall.Normalize();

  TEntityList nearList;
  mgr.BuildNearList(nearList, GetTranslation(), camToBall, camToBallMag, kLineOfSightFilter,
                    nullptr);

  CRayCastResult result =
      mgr.RayWorldIntersection(mObscuringObjectId, GetTranslation(), camToBall, camToBallMag,
                               kLineOfSightFilter, nearList);
  if (result.IsValid()) {
    mObscuringMaterial = result.GetMaterial();
    CVector3f ballPosCopy(ballPos);
    ballPosCopy.SetZ(ballPosCopy.GetZ() + gpTweakPlayer->GetPlayerBallHalfExtent());
    CVector3f playerPos = mgr.Player()->GetTranslation();
    bool clearAbove = mgr.RayCollideWorld(GetTranslation(), ballPosCopy, nearList,
                                          kLineOfSightFilter, mgr.GetPlayer());
    bool clearBelow = mgr.RayCollideWorld(GetTranslation(), playerPos, nearList, kLineOfSightFilter,
                                          mgr.GetPlayer());
    if (!clearAbove && !clearBelow) {
      mClearLOS = false;
      if (mPrevClearLOS) {
        mSplineIntermediatePos = ballPos;
        if (ShouldResetSpline(mgr) && !mNoSpline &&
            mObscuringMaterial.HasMaterial(kMT_Floor) &&
            mgr.RayCollideWorld(ballPos, ballPos + CVector3f(0.f, 0.f, -2.5f), nearList,
                                kLineOfSightFilter, nullptr)) {
          ResetSpline(mgr);
        }
      }
    }
  } else {
    mClearLOS = true;
    mObscuringMaterial = CMaterialList(kMT_NoStepLogic);
  }

  if (!mClearLOS) {
    mObscuredTime += dt;
    if (ShouldResetSpline(mgr) && !mNoSpline &&
        mObscuringMaterial.HasMaterial(kMT_Pillar)) {
      BuildSpline(mgr);
    }
  } else {
    mObscuredTime = 0.f;
  }

  float unobscureMag = mObscuredTime / 2.f;
  mUnobscureMag = CMath::Clamp(0.f, unobscureMag, 1.f);
  if (mObscureAvoidance &&
      (mObscuredTime > 2.f ||
       (mTooCloseActorId != kInvalidUniqueId && mObscuredTime > 1.f)) &&
      !mClearLOS && mSplineState == kBSS_Invalid) {
    mPendingFailsafe = true;
  } else {
    mPendingFailsafe = false;
  }

  bool doFailSafe = mPendingFailsafe;
  float ballDist = CVector3f(GetTranslation() - ballPos).Magnitude();
  if (ballDist < 0.3f + gpTweakPlayer->GetPlayerBallHalfExtent()) {
    doFailSafe = true;
  }

  if (mNearbyDoorClosed) {
    mNearbyDoorClosed = false;
    if (result.IsValid()) {
      doFailSafe = true;
    }
  }

  if (mNearbyDoorClosing) {
    mNearbyDoorClosing = false;
    if (CheckDoorProximity(GetTranslation(), mgr)) {
      doFailSafe = true;
    }
  }

  if (doFailSafe) {
    ActivateFailSafe(dt, mgr);
  }
}

bool CBallCamera::CheckDoorProximity(const CVector3f& vec, const CStateManager& mgr) {
  TUniqueId dockId;
  TUniqueId useDockId;
  const CScriptDoor* door = TCastToConstPtr< CScriptDoor >(
      mgr.GetObjectById(mgr.GetCameraManager()->GetBallCamera()->GetTooCloseActorId()));
  if (door == nullptr || (door != nullptr && door->IsOpen())) {
    return false;
  }

  rstl::optional_object< CAABox > touchBounds = door->GetTouchBounds();
  const CVector3f boxRange(0.3f, 0.3f, 0.3f);
  CVector3f mins = vec - boxRange;
  CVector3f maxs = vec + boxRange;
  CAABox testAABB(mins, maxs);
  if (!touchBounds || !touchBounds->DoBoundsOverlap(testAABB)) {
    return false;
  }

  dockId = door->GetConnectedDockID();
  useDockId = dockId;
  const CScriptDock* dock = TCastToConstPtr< CScriptDock >(mgr.GetObjectById(useDockId));
  if (dock == nullptr) {
    return false;
  }

  if (fabsf(dock->GetPlane(mgr).GetHeight(vec)) < 1.15f) {
    return true;
  }

  return false;
}

void CBallCamera::DoorClosing(TUniqueId uid) {
  if (uid == mTooCloseActorId) {
    mNearbyDoorClosing = true;
  }
}

void CBallCamera::DoorClosed(TUniqueId uid) {
  if (uid == mTooCloseActorId) {
    mNearbyDoorClosed = true;
  }
}

void CBallCamera::Think(float dt, CStateManager& mgr) {
  const TAreaId areaId = mgr.GetNextAreaId();
  mgr.SetActorAreaId(*this, areaId);
  UpdatePlayerMovement(dt, mgr);

  CCollisionActor* colAct = TCastToPtr< CCollisionActor >(mgr.ObjectById(mCollisionActorId));
  if (colAct != nullptr) {
    mgr.SetActorAreaId(*colAct, areaId);
  }

  const int camState = mgr.GetPlayer()->GetCameraState();
  if (camState != CPlayer::kCS_Ball && camState != CPlayer::kCS_Transitioning &&
      camState != CPlayer::kCS_Two && !mForceProcessing) {
    if (colAct == nullptr) {
      return;
    }
    colAct->SetActive(false);
    return;
  }

  if (colAct != nullptr) {
    colAct->SetActive(true);
  }

  CTransform4f oldXf(GetTransform());
  if (mgr.GetPlayer()->GetBombJumpCounter() != 1) {
    UpdateLookAtPosition(dt, mgr);
  }
  CheckFailSafe(dt, mgr);
  UpdateObjectTooCloseId(mgr);
  UpdateAnglePerSecond(dt);

  switch (mState) {
  case kBCS_One:
    break;
  case kBCS_Default:
  case kBCS_Chase:
  case kBCS_Boost:
    switch (mBehaviour) {
    case kBCB_PathCamera:
      UpdateUsingPathCameras(dt, mgr);
      break;
    case kBCB_HintFixedPosition:
    case kBCB_HintFixedTransform:
      UpdateUsingFixedCameras(dt, mgr);
      break;
    case kBCB_Default:
    case kBCB_FreezeLookPosition:
    case kBCB_HintBallToCam:
    case kBCB_PathCameraDesiredPos:
      if (mSplineState != kBSS_Invalid) {
        UpdateUsingFreeLook(dt, mgr);
      } else {
        UpdateUsingColliders(dt, mgr);
      }
      break;
    case kBCB_SpindleCamera:
      UpdateUsingSpindleCameras(dt, mgr);
      break;
    case kBCB_HintInitializePosition:
    default:
      break;
    }
    break;
  case kBCS_ToBall:
  case kBCS_FromBall:
    UpdateUsingTransitions(dt, mgr);
    break;
  default:
    break;
  }

  CTransform4f copyXf(UpdateCameraPositions(dt, oldXf, GetTransform()));
  copyXf = ValidateCameraTransform(copyXf, oldXf);
  SetTransform(copyXf);
}

void CBallCamera::SetState(EBallCameraState state, CStateManager& mgr) {
  switch (state) {
  case kBCS_ToBall: {
    CTransform4f xf = mgr.CameraManager()->GetFirstPersonCamera()->GetTransform();
    SetTransform(xf);
    TeleportCamera(xf.GetTranslation(), mgr);
    InterpolateFOV(mgr.CameraManager()->GetFirstPersonCamera()->GetFov(),
                   CCameraManager::GetDefaultThirdPersonVerticalFOV(), 1.f, 0.f);
    mSplineState = kBSS_Invalid;
  }
  case kBCS_Default:
  case kBCS_Chase:
  case kBCS_Boost:
    mgr.SetGameState(CStateManager::kGS_Running);
    break;
  case kBCS_FromBall:
    mgr.CameraManager()->SetPlayerCamera(mgr, GetUniqueId());
    mgr.SetGameState(CStateManager::kGS_Running);
    InterpolateFOV(GetFov(), CCameraManager::GetDefaultFirstPersonVerticalFOV(), 1.f, 0.f);
    mSplineState = kBSS_Invalid;
    break;
  default:
    break;
  }

  mState = state;
}

void CBallCamera::ProcessInput(const CFinalInput& input, CStateManager& mgr) {
  if (input.ControllerNumber() == 0) {
    const CPlayer* pl = TCastToConstPtr< CPlayer >(mgr.GetObjectById(GetWatchedObject()));
    if (pl != nullptr && pl->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
      int state = mState;
      switch (state) {
      case kBCS_Chase:
        if (!ControlMapper::GetDigitalInput(ControlMapper::kC_ChaseCamera, input) ||
            pl->IsInFreeLook()) {
          SetState(kBCS_Default, mgr);
        }
        break;
      case kBCS_Boost:
        if (!pl->GetMorphBall()->IsBoosting()) {
          SetState(kBCS_Default, mgr);
        }
        break;
      case kBCS_Default:
        if (mChaseAllowed) {
          bool pressed = ControlMapper::GetPressInput(ControlMapper::kC_ChaseCamera, input);
          if (pressed) {
            SetState(kBCS_Chase, mgr);
          }
        }
        break;
      case kBCS_ToBall:
      case kBCS_FromBall:
      default:
        break;
      }

      if (mBoostAllowed && mState != kBCS_Boost &&
          (pl->GetMorphBall()->IsBoosting() || pl->GetMorphBall()->GetBoostChargeTimer() > 0.f)) {
        SetState(kBCS_Boost, mgr);
      }
    }
  }
}

void CBallCamera::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CGameCamera::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case kSM_Registered: {
    TUniqueId newId = mgr.AllocateUniqueId();
    mCollisionActorId = newId;
    CCollisionActor* colAct = rs_new CCollisionActor(mCollisionActorId, GetAreaId(),
                                                     kInvalidUniqueId, true, 0.3f, 1.f);
    if (colAct != nullptr) {
      CMaterialList include(kMT_Solid);
      CMaterialList exclude(kMT_Player, kMT_CameraPassthrough);
      CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(include, exclude);
      colAct->SetMaterialFilter(filter);
      colAct->MaterialList() = CMaterialList(kMT_ProjectilePassthrough, kMT_ScanPassthrough,
                                             kMT_SeeThrough, kMT_CameraPassthrough);
      colAct->SetTranslation(GetTranslation());
      mgr.AddObject(colAct);
      colAct->SetMovable(false);
      colAct->SetLastNonCollidingState(CMotionState(
          GetTranslation(), CNUQuaternion::BuildFromAxisAngle(CVector3f::Forward(), 0.f),
          CVector3f::Zero(), CAxisAngle::Identity()));
    }

    CMaterialList include;
    CMaterialList exclude(kMT_Solid, kMT_ProjectilePassthrough, kMT_Player, kMT_Character,
                          kMT_CameraPassthrough);
    CMaterialFilter selfFilter = CMaterialFilter::MakeIncludeExclude(include, exclude);
    SetMaterialFilter(selfFilter);
    RemoveMaterial(kMT_Solid, mgr);
    break;
  }
  case kSM_Deleted:
    mgr.DeleteObjectRequest(mCollisionActorId);
    mCollisionActorId = kInvalidUniqueId;
    break;
  default:
    break;
  }
}

ENTITY_ACCEPT_IMPL(CBallCamera)

void CBallCamera::OverrideCameraInfo(CStateManager& mgr) {
  const CScriptCameraHint* hint = mgr.CameraManager()->GetCameraHint(mgr);
  if (hint == nullptr) {
    return;
  }

  ResetToTweaks(mgr);
  mBehaviour = hint->GetBehaviourType();

  if ((hint->GetOverrideFlags() & 0x2) != 0) {
    mChaseAllowed = true;
  } else {
    mChaseAllowed = false;
  }
  if ((hint->GetOverrideFlags() & 0x4) != 0) {
    mBoostAllowed = true;
  } else {
    mBoostAllowed = false;
  }
  if ((hint->GetOverrideFlags() & 0x8) != 0) {
    mObscureAvoidance = true;
  } else {
    mObscureAvoidance = false;
  }
  if ((hint->GetOverrideFlags() & 0x10) != 0) {
    mVolumeCollider = true;
  } else {
    mVolumeCollider = false;
  }

  if ((hint->GetOverrideFlags() & 0x40) != 0) {
    mLookAtBall = true;
  }
  if ((hint->GetOverrideFlags() & 0x4000) != 0) {
    mNoElevationInterp = true;
  } else {
    mNoElevationInterp = false;
  }
  if ((hint->GetOverrideFlags() & 0x8000) != 0) {
    mDirectElevation = true;
  } else {
    mDirectElevation = false;
  }
  if ((hint->GetOverrideFlags() & 0x10000) != 0) {
    mOverrideLookDir = true;
  } else {
    mOverrideLookDir = false;
  }
  if ((hint->GetOverrideFlags() & 0x20000) != 0) {
    mNoElevationVelClamp = true;
  } else {
    mNoElevationVelClamp = false;
  }
  if ((hint->GetOverrideFlags() & 0x80000) != 0) {
    mNoSpline = true;
  } else {
    mNoSpline = false;
  }
  if ((hint->GetOverrideFlags() & 0x80000) != 0) {
    x18e_26_ = true;
  } else {
    x18e_26_ = false;
  }

  if ((hint->GetOverrideFlags() & 0x400000) != 0) {
    mTargetMinDistance = hint->GetInfo().GetMinDist();
  }
  if ((hint->GetOverrideFlags() & 0x800000) != 0) {
    mMaxDistance = hint->GetInfo().GetMaxDist();
  }
  if ((hint->GetOverrideFlags() & 0x1000000) != 0) {
    mBackwardsDistance = hint->GetInfo().GetBackwardsDist();
  }
  if ((hint->GetOverrideFlags() & 0x80000000) != 0) {
    mElevation = hint->GetInfo().GetElevation();
  }
  if ((hint->GetOverrideFlags() & 0x2000000) != 0) {
    SetLookAtOffset(hint->GetInfo().GetLookAtOffset());
  }
  if ((hint->GetOverrideFlags() & 0x4000000) != 0) {
    SetChaseLookAtOffset(hint->GetInfo().GetChaseLookAtOffset());
  }

  if ((hint->GetOverrideFlags() & 0x10000000) != 0) {
    mClampAttitude = true;
    mAttitudeRange = hint->GetInfo().GetAttitudeRange();
  } else {
    mClampAttitude = false;
  }
  if ((hint->GetOverrideFlags() & 0x20000000) != 0) {
    mClampAzimuth = true;
    mAzimuthRange = hint->GetInfo().GetAzimuthRange();
  } else {
    mClampAzimuth = false;
  }

  if ((hint->GetOverrideFlags() & 0x8000000) != 0) {
    InterpolateFOV(mCurrentFov, hint->GetInfo().GetFov(), 1.f, 0.f);
  }
  if ((hint->GetOverrideFlags() & 0x40000000) != 0) {
    mTargetAnglePerSecond = hint->GetInfo().GetAnglePerSecond();
  }

  if ((hint->GetOverrideFlags() & 0x200) != 0) {
    mgr.Player()->SetControlDirectionInterpolation(hint->GetInfo().GetControlInterpDur());
  } else {
    mgr.Player()->ResetControlDirectionInterpolation();
  }

  switch (hint->GetBehaviourType()) {
  case kBCB_HintBallToCam: {
    CVector3f ballToCam = hint->GetInfo().GetBallToCam();
    SetWorldOffset(ballToCam);
    TeleportLookAtStuff(mgr);

    CVector3f camPos = mgr.GetPlayer()->GetBallPosition() + ballToCam;
    if ((hint->GetOverrideFlags() & 0x1) != 0) {
      CVector2f ballToCamXY(ballToCam.GetX(), ballToCam.GetY());
      float distance = ballToCamXY.Magnitude();
      CVector2f camToBallXY(ballToCam.GetX(), ballToCam.GetY());
      CVector3f camToBall = -CVector3f(camToBallXY.GetX(), camToBallXY.GetY(), 0.f).AsNormalized();
      camPos = FindDesiredPosition(distance, ballToCam.GetZ(), camToBall, mgr, false);
    }

    CTransform4f camXf = CTransform4f::LookAt(camPos, mLookPos, CVector3f::Up());
    TeleportCamera(camXf, mgr);
    break;
  }
  case kBCB_HintFixedTransform:
    TeleportLookAtStuff(mgr);
    TeleportCamera(hint->GetTransform(), mgr);
    break;
  case kBCB_Default:
    if ((hint->GetOverrideFlags() & 0x20) != 0) {
      TeleportLookAtStuff(mgr);
      if ((hint->GetOverrideFlags() & 0x40000) != 0) {
        const CTransform4f& camXf = mgr.CameraManager()->GetCurrentCameraTransform(mgr);
        CVector3f lookDir = mgr.GetPlayer()->GetTranslation() - camXf.GetTranslation();
        lookDir[kDZ] = 0.f;
        if (lookDir.CanBeNormalized()) {
          lookDir.Normalize();
        } else {
          lookDir = mgr.GetPlayer()->GetMovementDirection();
        }
        TeleportCamera(FindDesiredTransform(lookDir, mgr), mgr);
      } else {
        CVector3f hintPos = hint->GetTranslation();
        CTransform4f camXf = CTransform4f::LookAt(hintPos, mLookPos, CVector3f::Up());
        TeleportCamera(camXf, mgr);
      }
    }
    break;
  case kBCB_HintFixedPosition: {
    TeleportLookAtStuff(mgr);
    CVector3f hintPos = hint->GetTranslation();
    CTransform4f camXf = CTransform4f::LookAt(hintPos, mLookPos, CVector3f::Up());
    TeleportCamera(camXf, mgr);
    break;
  }
  case kBCB_FreezeLookPosition:
  case kBCB_HintInitializePosition:
    if ((hint->GetOverrideFlags() & 0x20) != 0) {
      TeleportLookAtStuff(mgr);
      float distance = mCurMinDistance;
      float elevation = mElevation;
      ConstrainElevationAndDistance(elevation, distance, 0.f, mgr);
      CVector3f desiredPos = FindDesiredPosition(
          distance, elevation, mgr.GetPlayer()->GetMovementDirection(), mgr, false);
      CTransform4f camXf = CTransform4f::LookAt(desiredPos, mFixedLookPos, CVector3f::Up());
      TeleportCamera(camXf, mgr);
    }
    break;
  case kBCB_PathCameraDesiredPos:
  case kBCB_PathCamera:
  case kBCB_SpindleCamera:
  default:
    break;
  }

  if ((hint->GetOverrideFlags() & 0x20) != 0) {
    mgr.CameraManager()->SetPlayerCamera(mgr, GetUniqueId());
  }
}

bool CBallCamera::CheckFailsafeFromMorphBallState(CStateManager& mgr) {
  TUniqueId intersectId = kInvalidUniqueId;
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  rstl::reserved_vector< CRayCastResult, 6 > frontResults;
  rstl::reserved_vector< CRayCastResult, 6 > backResults;

  float curT = 0.f;
  while (curT < 6.f) {
    CVector3f posA = GetFailsafeBezierPoint(mFailsafeState->mSplinePoints, curT / 6.f);
    CVector3f posB =
        GetFailsafeBezierPoint(mFailsafeState->mSplinePoints, (1.f + curT) / 6.f);
    CVector3f delta = posB - posA;
    if (delta.Magnitude() > 0.1f) {
      frontResults.push_back(mgr.RayWorldIntersection(intersectId, posA, delta.AsNormalized(),
                                                      delta.Magnitude(), kLineOfSightFilter,
                                                      nearList));
      backResults.push_back(mgr.RayWorldIntersection(intersectId, posB, -delta.AsNormalized(),
                                                     delta.Magnitude(), kLineOfSightFilter,
                                                     nearList));
    } else {
      frontResults.push_back(CRayCastResult());
      backResults.push_back(CRayCastResult());
    }
    curT += 1.f;
  }

  for (int i = 0; i < frontResults.size(); ++i) {

    const CRayCastResult& resA = frontResults[i];
    const CRayCastResult& resB = backResults[i];
    if (resA.IsValid()) {
      CVector3f delta = resA.GetPoint() - resB.GetPoint();
      if (close_enough(delta.Magnitude(), 0.f)) {
        delta = GetFailsafeBezierPoint(mFailsafeState->mSplinePoints,
                                       (1.f + CCast::LtoF(i)) / 6.f) -
                resA.GetPoint();
      }
      if (delta.Magnitude() > 0.3f) {
        return false;
      }
    }
  }
  return true;
}

bool CBallCamera::SplineIntersectTest(CMaterialList& intersectMat, CStateManager& mgr) const {
  TUniqueId intersectId(kInvalidUniqueId);
  TEntityList nearList;
  rstl::reserved_vector< CRayCastResult, 12 > frontResults;
  rstl::reserved_vector< CRayCastResult, 12 > backResults;
  CMaterialList include(kMT_Solid, kMT_Floor, kMT_Wall);
  CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      include,
      CMaterialList(kMT_ProjectilePassthrough, kMT_Player, kMT_Character, kMT_CameraPassthrough));

  float curT = 0.f;
  while (curT < 12.f) {
    CVector3f posA = mCamSpline.GetInterpolatedSplinePointByTime(curT, 12.f);
    CVector3f posB = mCamSpline.GetInterpolatedSplinePointByTime(curT + 1.f, 12.f);
    CVector3f delta = posB - posA;
    if (delta.Magnitude() > 0.1f) {
      frontResults.push_back(mgr.RayWorldIntersection(intersectId, posA, delta.AsNormalized(),
                                                      delta.Magnitude(), filter, nearList));
      backResults.push_back(mgr.RayWorldIntersection(intersectId, posB, -delta.AsNormalized(),
                                                     delta.Magnitude(), filter, nearList));
    } else {
      frontResults.push_back(CRayCastResult());
      backResults.push_back(CRayCastResult());
    }
    curT += 1.f;
  }

  for (int i = 0; i < frontResults.size(); ++i) {
    const CRayCastResult& resA = frontResults[i];
    const CRayCastResult& resB = backResults[i];
    if (resA.IsValid()) {
      CVector3f delta = resA.GetPoint() - resB.GetPoint();
      if (close_enough(delta.Magnitude(), 0.f)) {
        delta = mCamSpline.GetInterpolatedSplinePointByTime(1.f + CCast::LtoF(i), 12.f) -
                resA.GetPoint();
      }
      if (delta.Magnitude() > 0.3f) {
        intersectMat = frontResults[i].GetMaterial();
        return false;
      }
    }
  }

  return true;
}
