#include "MetroidPrime/Cameras/CBallCamera.hpp"

#include "Collision/CCollidableSphere.hpp"
#include "Collision/CCollisionInfo.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "WorldFormat/CMetroidAreaCollider.hpp"

bool CBallCamera::CheckTransitionLineOfSight(const CVector3f& eyePos, const CVector3f& behindPos,
                                             float& eyeToOccDist, float colRadius,
                                             const CStateManager& mgr) {
  CVector3f delta = behindPos - eyePos;
  float deltaMag = delta.Magnitude();
  CVector3f deltaNorm = delta * (1.f / deltaMag);
  bool clear = true;

  if (deltaMag > FLT_EPSILON * 10.f) {
    float margin = 2.f * colRadius;
    CAABox aabb = CAABox::MakeMaxInvertedBox();
    aabb.AccumulateBounds(eyePos);
    aabb.AccumulateBounds(behindPos);
    aabb = CAABox(aabb.GetMinPoint() - CVector3f(margin, margin, margin),
                  aabb.GetMaxPoint() + CVector3f(margin, margin, margin));
    TEntityList nearList;
    mgr.BuildColliderList(nearList, *mgr.GetPlayer(), aabb);
    CAreaCollisionCache cache(aabb);
    CGameCollision::BuildAreaCollisionCache(mgr, cache);
    if (cache.HasCacheOverflowed()) {
      clear = false;
    }
    if (clear) {
      TUniqueId intersectId = kInvalidUniqueId;
      CCollisionInfo info;
      double distance = deltaMag;
      if (CGameCollision::DetectCollision_Cached_Moving(
              mgr, cache,
              CCollidableSphere(CSphere(CVector3f::Zero(), colRadius), CMaterialList(kMT_Solid)),
              CTransform4f::Translate(eyePos),
              CMaterialFilter::MakeIncludeExclude(
                  CMaterialList(kMT_Solid), CMaterialList(kMT_ProjectilePassthrough, kMT_Player,
                                                          kMT_Character, kMT_CameraPassthrough)),
              nearList, deltaNorm, intersectId, info, distance)) {
        eyeToOccDist = static_cast< float >(distance);
        clear = false;
      }
    }
  }
  return !clear;
}

CVector3f CBallCamera::GetFailsafeBezierPoint(const rstl::vector< CVector3f >& points, float t) {
  t *= points.size() - 3;
  int baseIdx = 0;
  while (t > 1.f) {
    t -= 1.f;
    ++baseIdx;
  }
  const CVector3f& end = points[baseIdx + 3];
  return CMath::GetBezierPoint(points[baseIdx], points[baseIdx + 1], points[baseIdx + 2], end, t);
}

bool CBallCamera::TransitionFromMorphBallState(CStateManager& mgr) {
  mFailsafeState->mPlayerXf = mgr.GetPlayer()->GetTransform();
  mFailsafeState->mCamXf = GetTransform();
  mFailsafeState->mLookPos = mLookPos;
  mFailsafeState->mPlayerPos = mFailsafeState->mPlayerXf.GetTranslation();
  CVector3f camPos = mFailsafeState->mCamXf.GetTranslation();
  const CVector3f eyePos = mgr.GetPlayer()->GetEyePosition();
  float lookDist = (mFailsafeState->mLookPos - camPos).Magnitude();
  CVector3f behindPos = (0.6f * -lookDist) * mFailsafeState->mPlayerXf.GetForward() + eyePos;
  float eyeToOccDist;
  if (CheckTransitionLineOfSight(eyePos, behindPos, eyeToOccDist, 0.6f, mgr)) {
    mFailsafeState->mBehindPos =
        -eyeToOccDist * mFailsafeState->mPlayerXf.GetForward() + eyePos;
  } else {
    eyeToOccDist = lookDist;
    mFailsafeState->mBehindPos = behindPos;
  }
  mFailsafeState->mSplinePoints.clear();
  mFailsafeState->mSplinePoints.reserve(4);
  mFailsafeState->mSplinePoints.push_back(camPos);
  mFailsafeState->mSplinePoints.push_back(mFailsafeState->mBehindPos);
  mFailsafeState->mSplinePoints.push_back(mFailsafeState->mBehindPos);
  mFailsafeState->mSplinePoints.push_back(eyePos);
  return CheckFailsafeFromMorphBallState(mgr);
}

bool CBallCamera::UpdateTransitionFromBallCamera(CStateManager& mgr) {
  const CPlayer& player = *mgr.GetPlayer();
  float morphFactor = player.GetMorphBallTransitionFactor();
  const CVector3f eyePos = player.GetEyePosition();
  CVector3f playerPos = player.GetTranslation();
  CVector3f delta = playerPos - mFailsafeState->mPlayerPos;
  mFailsafeState->mSplinePoints[1] += delta;
  mFailsafeState->mSplinePoints[2] += delta;
  mFailsafeState->mSplinePoints[3] += delta;
  CVector3f splinePoint = GetFailsafeBezierPoint(mFailsafeState->mSplinePoints, morphFactor);
  float zDelta = splinePoint.GetZ() - eyePos.GetZ();
  zDelta *= CMath::Clamp(0.f, 1.f - 1.5f * morphFactor, 1.f);
  splinePoint.SetZ(zDelta + eyePos.GetZ());
  CVector3f deltaFlat = eyePos - splinePoint;
  deltaFlat.SetZ(0.f);
  if (deltaFlat.Magnitude() > FLT_EPSILON * 10000.f) {
    SetTransform(CTransform4f::LookAt(splinePoint, eyePos, CVector3f::Up()));
  } else {
    SetTransform(mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform());
    SetTranslation(splinePoint);
  }
  mgr.CameraManager()->FirstPersonCamera()->Reset(GetTransform(), mgr);
  mFailsafeState->mPlayerPos = playerPos;
  return false;
}

CBallCamera::SFailsafeState::SFailsafeState()
: mPlayerXf(CTransform4f::Identity())
, mCamXf(CTransform4f::Identity())
, mLookPos(CVector3f::Zero())
, mBehindPos(CVector3f::Zero())
, x78_(CVector3f::Zero())
, mPlayerPos(CVector3f::Zero()) {}

CBallCamera::SUnknown::SUnknown() {}
