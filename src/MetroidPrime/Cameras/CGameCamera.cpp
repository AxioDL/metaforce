#include "MetroidPrime/Cameras/CGameCamera.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CRayCastResult.hpp"

#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/algorithm.hpp"

#include "float.h"

#if defined(TARGET_PC)
#include "Metaforce/Display.hpp"
#endif

CGameCamera::CGameCamera(const TUniqueId uid, const bool active, const rstl::string& name,
                         const CEntityInfo& info, const CTransform4f& xf, const float fov,
                         const float nearZ, const float farZ, const float aspect,
                         const TUniqueId watchedId, const bool disableInput,
                         const int controllerIdx)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, mWatchedObject(watchedId)
, mPerspectiveMatrix(CMatrix4f::Identity())
, mOrigXf(xf)
, mCurrentFov(fov)
, mZnear(nearZ)
, mZfar(farZ)
, mAspect(aspect)
, mControllerIdx(controllerIdx)
, mPerspDirty(true)
, mDisablesInput(disableInput)
, mDelayTime(0.f)
, mPerspInterpRemTime(0.f)
, mPerspInterpDur(0.f)
, mPerspInterpStartFov(fov)
, mPerspInterpEndFov(fov) {
  SetDrawEnabled(false);
}

CGameCamera::~CGameCamera() {}

#if defined(TARGET_PC)
float CGameCamera::GetAspectRatio() const { return metaforce::AdjustDisplayAspect(mAspect); }
#endif

const CMatrix4f& CGameCamera::GetPerspectiveMatrix() const {
#if defined(TARGET_PC)
  const float aspect = GetAspectRatio();
  if (mPerspDirty || mCachedAspect != aspect) {
    mPerspectiveMatrix =
        CGraphics::CalculatePerspectiveMatrix(mCurrentFov, aspect, mZnear, mZfar);
    mCachedAspect = aspect;
    mPerspDirty = false;
  }
#else
  if (mPerspDirty == true) {
    mPerspectiveMatrix =
        CGraphics::CalculatePerspectiveMatrix(mCurrentFov, mAspect, mZnear, mZfar);
    mPerspDirty = false;
  }
#endif

  return mPerspectiveMatrix;
}

CVector3f CGameCamera::ConvertToScreenSpace(const CVector3f& vec) const {
  CVector3f rotated = GetTransform().TransposeMultiply(vec);
  if (rotated.IsNonZero()) {
    return GetPerspectiveMatrix().MultiplyOneOverW(rotated);
  }

  return CVector3f(-1.f, -1.f, 1.f);
}

float CCameraSpring::ApplyDistanceSpring(float targetX, float curX, float dt) {
  float useX = curX + mTardis * (mDx * dt);
  float accel = mK * (targetX - curX) - mK2Sqrt * mDx;
  mDx = mTardis * (accel * dt) + mDx;
  if (useX < targetX) {
    useX = targetX;
  }
  if (useX - targetX > mMax) {
    useX = targetX + mMax;
  }
  return useX;
}

float CCameraSpring::ApplyDistanceSpringNoMax(float targetX, float curX, float dt) {
  float useX = curX + mTardis * (mDx * dt);
  float accel = mK * (targetX - curX) - mK2Sqrt * mDx;
  mDx = mTardis * (accel * dt) + mDx;
  if (useX < targetX) {
    useX = targetX;
  }
  return useX;
}

void CCameraSpring::Reset() {
  mK2Sqrt = 2.f * CMath::SqrtF(mK);
  mDx = 0.f;
}

CCameraSpline::CCameraSpline(bool closedLoop) : mLength(0.f), mClosedLoop(closedLoop) {}

CCameraSpline::~CCameraSpline() {}

void CCameraSpline::Initialise(TUniqueId uid, const rstl::vector< SConnection >& connections,
                               CStateManager& mgr) {
  CalculateKnots(uid, connections, mgr);
  mLength = CalculateSplineLength();
}

// TODO: non-matching
void CCameraSpline::CalculateKnots(TUniqueId uid, const rstl::vector< SConnection >& connections,
                                   CStateManager& mgr) {
  const SConnection* lastConn = nullptr;

  for (rstl::vector< SConnection >::const_iterator it = connections.begin();
       it != connections.end(); ++it) {
    if (it->mState == kSS_CameraPath && it->mMsg == kSM_Follow) {
      lastConn = &*it;
    }
  }

  if (lastConn != nullptr) {
    CScriptCameraWaypoint* waypoint =
        TCastToPtr< CScriptCameraWaypoint >(mgr.ObjectById(mgr.GetIdForScript(lastConn->mObjId)));
    int size = 0;

    mWpTracker.clear();
    mWpTracker.reserve(4);

    while (waypoint != nullptr) {
      if (rstl::find(mWpTracker.begin(), mWpTracker.end(), waypoint->GetUniqueId()) !=
          mWpTracker.end()) {
        break;
      }

      // what is going on here?
      if (mWpTracker.size() == mWpTracker.capacity()) {
        mWpTracker.reserve(mWpTracker.size());
      }
      mWpTracker.push_back(waypoint->GetUniqueId());
      size += 1;

      waypoint = TCastToPtr< CScriptCameraWaypoint >(mgr.ObjectById(waypoint->NextWaypoint(mgr)));
    }

    Reset(size);
    mWpTracker = rstl::vector< TUniqueId >();

    waypoint =
        TCastToPtr< CScriptCameraWaypoint >(mgr.ObjectById(mgr.GetIdForScript(lastConn->mObjId)));
    while (waypoint != nullptr) {
      if (rstl::find(mWpTracker.begin(), mWpTracker.end(), waypoint->GetUniqueId()) !=
          mWpTracker.end()) {
        return;
      }

      AddKnot(waypoint->GetTranslation(), waypoint->GetTransform().GetForward());

      waypoint = TCastToPtr< CScriptCameraWaypoint >(mgr.ObjectById(waypoint->NextWaypoint(mgr)));
    }
  }
}

float CCameraSpline::GetKnotT(int idx) const {
  if (idx < mPositions.size()) {
    return mT[idx];
  }
  return 0.f;
}

CVector3f CCameraSpline::GetKnotPosition(int idx) const {
  if (idx < mPositions.size()) {
    return mPositions[idx];
  }
  return CVector3f::Zero();
}

void CCameraSpline::SetKnotPosition(int idx, CVector3f pos) {
  if (idx >= mPositions.size()) {
    return;
  }
  mPositions[idx] = pos;
}

void CCameraSpline::AddKnot(CVector3f pos, CVector3f dir) {
  mPositions.push_back(pos);
  mDirections.push_back(dir);
}

void CCameraSpline::Reset(int size) {
  mPositions.clear();
  mT.clear();
  mDirections.clear();
  if (static_cast< uint >(size) == 0) {
    return;
  }
  mPositions.reserve(size);
  mT.reserve(size);
  mDirections.reserve(size);
}

bool CCameraSpline::GetSurroundingPoints(int idx, rstl::reserved_vector< CVector3f, 4 >& positions,
                                         rstl::reserved_vector< CVector3f, 4 >& directions) {
  const int size = mPositions.size();
  if (size <= 3 || idx < 0 || idx >= size) {
    return false;
  }

  if (idx > 0) {
    positions.push_back(mPositions[idx - 1]);
    directions.push_back(mDirections[idx - 1]);
  } else {
    if (mClosedLoop) {
      positions.push_back(mPositions[size - 1]);
      directions.push_back(mDirections[size - 1]);
    } else {
      positions.push_back(mPositions[0] - (mPositions[1] - mPositions[0]));
      directions.push_back(mDirections[0]);
    }
  }

  positions.push_back(mPositions[idx]);
  directions.push_back(mDirections[idx]);

  ++idx;
  if (idx >= size) {
    if (mClosedLoop) {
      positions.push_back(mPositions[idx - size]);
      directions.push_back(mDirections[idx - size]);
    } else {
      positions.push_back(mPositions[size - 1] -
                          (mPositions[size - 2] - mPositions[size - 1]));
      directions.push_back(mDirections[size - 1]);
    }
  } else {
    positions.push_back(mPositions[idx]);
    directions.push_back(mDirections[idx]);
  }

  ++idx;
  if (idx >= size) {
    if (mClosedLoop) {
      positions.push_back(mPositions[idx - size]);
      directions.push_back(mDirections[idx - size]);
    } else {
      positions.push_back(mPositions[size - 1] -
                          (mPositions[size - 2] - mPositions[size - 1]));
      directions.push_back(mDirections[size - 1]);
    }
  } else {
    positions.push_back(mPositions[idx]);
    directions.push_back(mDirections[idx]);
  }

  return true;
}

float CCameraSpline::ClampLength(const CVector3f pos, bool collide, const CMaterialFilter filter,
                                 const CStateManager& mgr) const {
  if (mPositions.empty()) {
    return 0.f;
  }
  if (mClosedLoop) {
    return 0.f;
  }

  const CVector3f first = mPositions.front();
  const CVector3f last = mPositions.back();
  CVector3f deltaA = pos - first;
  CVector3f deltaB = pos - last;
  const float magA = deltaA.Magnitude();
  const float magB = deltaB.Magnitude();
  if (!deltaA.CanBeNormalized()) {
    return 0.f;
  }
  if (!deltaB.CanBeNormalized()) {
    return mLength;
  }

  if (collide) {
    const CRayCastResult collideA =
        mgr.RayStaticIntersection(first, deltaA.AsNormalized(), deltaA.Magnitude(), filter);
    const CRayCastResult collideB =
        mgr.RayStaticIntersection(last, deltaB.AsNormalized(), deltaB.Magnitude(), filter);
    if (collideA.IsValid()) {
      return mLength;
    }
    if (collideB.IsValid()) {
      return 0.f;
    }
  }

  return magA < magB ? 0.f : mLength;
}

float CCameraSpline::CalculateSplineLength() {
  float ret = 0.f;
  mT.clear();
  if (mPositions.size() > 0) {
    CVector3f prevPoint = mPositions[0];
    const float tDiv = 1.f / float(mPositions.size() - 1);
    for (int i = 0; i < mPositions.size() - 1; ++i) {
      const float baseT = i * tDiv;
      float subT = 0.f;
      mT.push_back(ret);
      while (subT <= tDiv) {
        subT = tDiv / 32.f + subT;
        const CVector3f nextPoint = GetInterpolatedSplinePointByTime(baseT + subT, 1.f);
        const CVector3f delta = nextPoint - prevPoint;
        if (delta.CanBeNormalized()) {
          ret += delta.Magnitude();
          prevPoint = nextPoint;
        }
      }
    }

    mT.push_back(ret);
    if (mClosedLoop) {
      const CVector3f delta = mPositions[0] - mPositions[mPositions.size() - 1];
      if (delta.CanBeNormalized()) {
        ret += delta.Magnitude();
      }
    }

    return ret;
  }
  return 0.f;
}

float CCameraSpline::ValidateLength(float t) const {
  if (mClosedLoop) {
    while (t >= mLength) {
      t -= mLength;
    }
    while (t < 0.f) {
      t += mLength;
    }
  } else {
    t = CMath::Clamp(0.f, t, mLength);
  }
  return t;
}

CVector3f CCameraSpline::GetInterpolatedSplinePointByTime(float time, float range) const {
  if (mPositions.size() > 0) {
    const int size = mPositions.size();
    const float rangeFac = range / float(size - 1);
    int baseIdx = int(time / rangeFac);
    int useIdx = baseIdx;
    if (baseIdx >= size) {
      useIdx = size - 1;
    }

    const float t = (time - float(useIdx) * rangeFac) / rangeFac;
    rstl::reserved_vector< CVector3f, 4 > positions;
    rstl::reserved_vector< CVector3f, 4 > directions;
    if (const_cast< CCameraSpline* >(this)->GetSurroundingPoints(useIdx, positions, directions)) {
      const CVector3f& p3 = positions[3];
      return CMath::GetCatmullRomSplinePoint(positions[0], positions[1], positions[2], p3, t);
    }
  }

  return CVector3f::Zero();
}

// TODO: non-matching
CTransform4f CCameraSpline::GetInterpolatedSplinePointByLength(float pos) const {
  if (mPositions.size() > 0) {
    int baseIdx = 0;
    int i = 0;
    for (i = 1; i < mPositions.size(); ++i) {
      if (mT[i] > pos) {
        baseIdx = i - 1;
        break;
      }
    }

    if (i == mPositions.size()) {
      baseIdx = i - 1;
    }
    if (pos < 0.f) {
      baseIdx = 0;
    }

    float range = mLength;
    if (pos >= range) {
      if (mClosedLoop) {
        baseIdx = 0;
        pos -= range;
      } else {
        baseIdx = mPositions.size() - 2;
        pos = range;
      }
    }

    if (baseIdx == mPositions.size() - 1) {
      if (mClosedLoop) {
        range -= mT[baseIdx];
      } else {
        range -= mT[mPositions.size() - 2];
      }
    } else {
      range = mT[baseIdx + 1] - mT[baseIdx];
    }

    float tRaw = (pos - mT[baseIdx]) / range;
    float t = CMath::Clamp(0.f, tRaw, 1.f);

    rstl::reserved_vector< CVector3f, 4 > positions;
    rstl::reserved_vector< CVector3f, 4 > directions;
    if (const_cast< CCameraSpline* >(this)->GetSurroundingPoints(baseIdx, positions, directions)) {
      const CVector3f& dirB = directions[2];
      const CVector3f& dirA = directions[1];
      float dot = CVector3f::Dot(dirA, dirB);
      dot = CMath::Limit(dot, 1.f);

      if (dot >= 0.99999f) {
        CTransform4f ret = CTransform4f::LookAt(CVector3f::Zero(), dirB);
        const CVector3f& p3 = positions[3];
        ret.SetTranslation(
            CMath::GetCatmullRomSplinePoint(positions[0], positions[1], positions[2], p3, t));
        return ret;
      }

      CRelAngle angle(acosf(dot) * t);
      CQuaternion quat = CQuaternion::LookAt(CUnitVector3f(dirA), CUnitVector3f(dirB), angle);
      CVector3f dir = quat.Transform(dirA);
      CTransform4f ret = CTransform4f::LookAt(CVector3f::Zero(), dir);
      const CVector3f& p3 = positions[3];
      ret.SetTranslation(
          CMath::GetCatmullRomSplinePoint(positions[0], positions[1], positions[2], p3, t));
      return ret;
    }
  }

  return CTransform4f::Identity();
}

// TODO: non-matching
float CCameraSpline::FindClosestLengthOnSpline(float time, CVector3f p) const {
  float ret = -1.f;
  float minLenDelta = 10000.f;
  float minMag = 10000.f;

  int iterations = mPositions.size() - 1;
  if (mClosedLoop) {
    iterations += 1;
  }

  for (int i = 0; i < iterations; ++i) {
    const float thisPosX = mPositions[i].GetX();
    const float thisPosY = mPositions[i].GetY();
    const float thisPosZ = mPositions[i].GetZ();

    float nextPosX, nextPosY, nextPosZ;
    if (!mClosedLoop) {
      nextPosX = mPositions[i + 1].GetX();
      nextPosY = mPositions[i + 1].GetY();
      nextPosZ = mPositions[i + 1].GetZ();
    } else if (i == mPositions.size() - 1) {
      nextPosX = mPositions[0].GetX();
      nextPosY = mPositions[0].GetY();
      nextPosZ = mPositions[0].GetZ();
    } else {
      nextPosX = mPositions[i + 1].GetX();
      nextPosY = mPositions[i + 1].GetY();
      nextPosZ = mPositions[i + 1].GetZ();
    }

    const float deltaX = nextPosX - thisPosX;
    const float deltaY = nextPosY - thisPosY;
    const float deltaZ = nextPosZ - thisPosZ;
    const float revDeltaX = -deltaX;
    const float revDeltaY = -deltaY;
    const float revDeltaZ = -deltaZ;

    CVector3f forwardDir(deltaX, deltaY, deltaZ);
    CVector3f backwardDir(revDeltaX, revDeltaY, revDeltaZ);

    if (i != 0) {
      forwardDir = CVector3f((thisPosX - mPositions[i - 1].GetX()) + deltaX,
                             (thisPosY - mPositions[i - 1].GetY()) + deltaY,
                             (thisPosZ - mPositions[i - 1].GetZ()) + deltaZ);
    } else {
      CVector3f extrap = mPositions[0] - (mPositions[1] - mPositions[0]);
      if (mClosedLoop) {
        extrap = mPositions[mPositions.size() - 1];
      }
      forwardDir =
          CVector3f((thisPosX - extrap.GetX()) + deltaX, (thisPosY - extrap.GetY()) + deltaY,
                    (thisPosZ - extrap.GetZ()) + deltaZ);
    }
    forwardDir.Normalize();

    if (i < mPositions.size() - 2) {
      backwardDir = CVector3f((nextPosX - mPositions[i + 2].GetX()) + revDeltaX,
                              (nextPosY - mPositions[i + 2].GetY()) + revDeltaY,
                              (nextPosZ - mPositions[i + 2].GetZ()) + revDeltaZ);
    } else {
      CVector3f extrap;
      if (mClosedLoop) {
        extrap = (i == iterations - 1) ? mPositions[1] : mPositions[0];
      } else {
        extrap = mPositions[i + 1] + (mPositions[i + 1] - mPositions[i]);
      }
      backwardDir =
          CVector3f((nextPosX - extrap.GetX()) + revDeltaX, (nextPosY - extrap.GetY()) + revDeltaY,
                    (nextPosZ - extrap.GetZ()) + revDeltaZ);
    }
    backwardDir.Normalize();

    const CVector3f thisToPoint(p.GetX() - thisPosX, p.GetY() - thisPosY, p.GetZ() - thisPosZ);
    const float projA = CVector3f::Dot(thisToPoint, forwardDir);
    const CVector3f delta(deltaX, deltaY, deltaZ);
    const float dotA = CVector3f::Dot(forwardDir, delta.AsNormalized());

    const CVector3f nextToPoint(p.GetX() - nextPosX, p.GetY() - nextPosY, p.GetZ() - nextPosZ);
    const float projB = CVector3f::Dot(nextToPoint, backwardDir);
    const CVector3f revDelta(revDeltaX, revDeltaY, revDeltaZ);
    const float dotB = CVector3f::Dot(backwardDir, revDelta.AsNormalized());

    float t = (projA / dotA) / ((projA / dotA) + (projB / dotB));

    if (!mClosedLoop) {
      if (i == 0 && t < 0.f) {
        t = 0.f;
      }
      if (i == mPositions.size() - 2 && t > 1.f) {
        t = 1.f;
      }
    }

    if (t >= 0.f && t <= 1.f) {
      float tLen;
      if (i == mPositions.size() - 1) {
        tLen = mLength - mT[i];
      } else {
        tLen = mT[i + 1] - mT[i];
      }

      const float lenT = t * tLen + mT[i];
      const CVector3f pointDelta = p - GetInterpolatedSplinePointByLength(lenT).GetTranslation();
      float mag = 0.f;
      if (pointDelta.CanBeNormalized()) {
        mag = pointDelta.Magnitude();
      }

      float lenDelta = CMath::AbsF(lenT - time);
      if (mClosedLoop) {
        const float altDelta = mLength - lenDelta;
        if (lenDelta > altDelta) {
          lenDelta = altDelta;
        }
      }

      if (close_enough(CMath::AbsF(mag - minMag), 0.f)) {
        if (lenDelta < minLenDelta) {
          ret = lenT;
          minLenDelta = lenDelta;
        }
      } else if (mag < minMag) {
        ret = lenT;
        minLenDelta = lenDelta;
        minMag = mag;
      }
    }
  }

  if (ret < 0.f) {
    ret = 0.f;
  }
  return ret;
}

void CGameCamera::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case kSM_AddSplashInhabitant:
    mgr.CameraManager()->SetInsideFluid(true, uid);
    return;
  case kSM_RemoveSplashInhabitant:
    mgr.CameraManager()->SetInsideFluid(false, kInvalidUniqueId);
    return;
  default:
    CActor::AcceptScriptMsg(msg, uid, mgr);
    return;
  }
}

void CGameCamera::SetActive(const bool active) {
  CActor::SetActive(active);
  SetDrawEnabled(false);
}

ENTITY_ACCEPT_IMPL(CGameCamera)

CTransform4f CGameCamera::ValidateCameraTransform(const CTransform4f& newXf,
                                                  const CTransform4f& oldXf) {
  CTransform4f xfCpy(newXf);
  if (!close_enough(newXf.GetRight().Magnitude(), 1.f, FLT_EPSILON * 1000.f) ||
      !close_enough(newXf.GetForward().Magnitude(), 1.f, FLT_EPSILON * 1000.f) ||
      !close_enough(newXf.GetUp().Magnitude(), 1.f, FLT_EPSILON * 1000.f)) {
    xfCpy.Orthonormalize();
  }

  float dot = CVector3f::Dot(newXf.GetForward(), CVector3f::Up());
  dot = CMath::Limit(dot, 1.f);
  if (CMath::AbsF(dot) > 0.999f) {
    xfCpy = oldXf;
  }

  if (xfCpy.GetUp()[kDZ] < -0.2f) {
    CQuaternion quat =
        CQuaternion::AxisAngle(CUnitVector3f(xfCpy.GetForward()), CRelAngle::FromRadians(M_PIF));
    xfCpy = quat.BuildTransform4f() * xfCpy;
  }

  if (!close_enough(xfCpy.GetRight()[kDZ], 0.f) && !close_enough(xfCpy.GetUp()[kDZ], 0.f)) {
    CVector3f forward = xfCpy.GetForward();
    forward[kDZ] = 0.f;
    if (forward.CanBeNormalized()) {
      xfCpy = CTransform4f::LookAt(CUnitVector3f(CVector3f::Zero()), xfCpy.GetForward(),
                                   CVector3f::Up());
    } else {
      xfCpy = oldXf;
    }
  }

  xfCpy.SetTranslation(newXf.GetTranslation());
  return xfCpy;
}

void CGameCamera::SkipFovInterpolation() {
  if (mPerspInterpRemTime > 0.f) {
    mCurrentFov = mPerspInterpEndFov;
    mPerspDirty = true;
  }
  mPerspInterpRemTime = 0.f;
  mDelayTime = 0.f;
}

void CGameCamera::InterpolateFOV(float start, float fov, float time, float delayTime) {
  if (time <= 0.f) {
    mCurrentFov = fov;
    mPerspDirty = true;
    mPerspInterpEndFov = fov;
    mPerspInterpRemTime = 0.f;
    mDelayTime = 0.f;
    return;
  }

  if (delayTime < 0.f) {
    mDelayTime = 0.f;
  } else {
    mDelayTime = delayTime;
  }
  mPerspInterpDur = time;
  mPerspInterpRemTime = time;
  mPerspInterpStartFov = start;
  mPerspInterpEndFov = fov;
  mCurrentFov = start;
  mPerspDirty = true;
}

void CGameCamera::UpdatePerspective(float dt) {
  if (mDelayTime > 0.f) {
    mDelayTime -= dt;
    return;
  }

  if (mPerspInterpRemTime > 0.f) {
    mPerspInterpRemTime -= dt;
    const float remTime = mPerspInterpRemTime;
    if (remTime <= 0.f) {
      mCurrentFov = mPerspInterpEndFov;
      mPerspDirty = true;
    } else {
      const float fovDelta = mPerspInterpStartFov - mPerspInterpEndFov;
      const float t = remTime / mPerspInterpDur;
      mCurrentFov = fovDelta * CMath::Clamp(0.f, t, 1.f) + mPerspInterpEndFov;
      mPerspDirty = true;
    }
  }
}
