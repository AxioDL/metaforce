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

CGameCamera::CGameCamera(const TUniqueId uid, const bool active, const rstl::string& name,
                         const CEntityInfo& info, const CTransform4f& xf, const float fov,
                         const float nearZ, const float farZ, const float aspect,
                         const TUniqueId watchedId, const bool disableInput,
                         const int controllerIdx)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_watchedObject(watchedId)
, xec_perspectiveMatrix(CMatrix4f::Identity())
, x12c_origXf(xf)
, x15c_currentFov(fov)
, x160_znear(nearZ)
, x164_zfar(farZ)
, x168_aspect(aspect)
, x16c_controllerIdx(controllerIdx)
, x170_24_perspDirty(true)
, x170_25_disablesInput(disableInput)
, x174_delayTime(0.f)
, x178_perspInterpRemTime(0.f)
, x17c_perspInterpDur(0.f)
, x180_perspInterpStartFov(fov)
, x184_perspInterpEndFov(fov) {
  SetDrawEnabled(false);
}

CGameCamera::~CGameCamera() {}

const CMatrix4f& CGameCamera::GetPerspectiveMatrix() const {
  if (x170_24_perspDirty == true) {
    xec_perspectiveMatrix =
        CGraphics::CalculatePerspectiveMatrix(x15c_currentFov, x168_aspect, x160_znear, x164_zfar);
    x170_24_perspDirty = false;
  }

  return xec_perspectiveMatrix;
}

CVector3f CGameCamera::ConvertToScreenSpace(const CVector3f& vec) const {
  CVector3f rotated = GetTransform().TransposeMultiply(vec);
  if (rotated.IsNonZero()) {
    return GetPerspectiveMatrix().MultiplyOneOverW(rotated);
  }

  return CVector3f(-1.f, -1.f, 1.f);
}

float CCameraSpring::ApplyDistanceSpring(float targetX, float curX, float dt) {
  float useX = curX + xc_tardis * (x10_dx * dt);
  float accel = x0_k * (targetX - curX) - x4_k2Sqrt * x10_dx;
  x10_dx = xc_tardis * (accel * dt) + x10_dx;
  if (useX < targetX) {
    useX = targetX;
  }
  if (useX - targetX > x8_max) {
    useX = targetX + x8_max;
  }
  return useX;
}

float CCameraSpring::ApplyDistanceSpringNoMax(float targetX, float curX, float dt) {
  float useX = curX + xc_tardis * (x10_dx * dt);
  float accel = x0_k * (targetX - curX) - x4_k2Sqrt * x10_dx;
  x10_dx = xc_tardis * (accel * dt) + x10_dx;
  if (useX < targetX) {
    useX = targetX;
  }
  return useX;
}

void CCameraSpring::Reset() {
  x4_k2Sqrt = 2.f * CMath::SqrtF(x0_k);
  x10_dx = 0.f;
}

CCameraSpline::CCameraSpline(bool closedLoop) : x44_length(0.f), x48_closedLoop(closedLoop) {}

CCameraSpline::~CCameraSpline() {}

void CCameraSpline::Initialise(TUniqueId uid, const rstl::vector< SConnection >& connections,
                               CStateManager& mgr) {
  CalculateKnots(uid, connections, mgr);
  x44_length = CalculateSplineLength();
}

// TODO: non-matching
void CCameraSpline::CalculateKnots(TUniqueId uid, const rstl::vector< SConnection >& connections,
                                   CStateManager& mgr) {
  const SConnection* lastConn = nullptr;

  for (rstl::vector< SConnection >::const_iterator it = connections.begin();
       it != connections.end(); ++it) {
    if (it->x0_state == kSS_CameraPath && it->x4_msg == kSM_Follow) {
      lastConn = &*it;
    }
  }

  if (lastConn != nullptr) {
    CScriptCameraWaypoint* waypoint =
        TCastToPtr< CScriptCameraWaypoint >(mgr.ObjectById(mgr.GetIdForScript(lastConn->x8_objId)));
    int size = 0;

    x14_wpTracker.clear();
    x14_wpTracker.reserve(4);

    while (waypoint != nullptr) {
      if (rstl::find(x14_wpTracker.begin(), x14_wpTracker.end(), waypoint->GetUniqueId()) !=
          x14_wpTracker.end()) {
        break;
      }

      // what is going on here?
      if (x14_wpTracker.size() == x14_wpTracker.capacity()) {
        x14_wpTracker.reserve(x14_wpTracker.size());
      }
      x14_wpTracker.push_back(waypoint->GetUniqueId());
      size += 1;

      waypoint = TCastToPtr< CScriptCameraWaypoint >(mgr.ObjectById(waypoint->NextWaypoint(mgr)));
    }

    Reset(size);
    x14_wpTracker = rstl::vector< TUniqueId >();

    waypoint =
        TCastToPtr< CScriptCameraWaypoint >(mgr.ObjectById(mgr.GetIdForScript(lastConn->x8_objId)));
    while (waypoint != nullptr) {
      if (rstl::find(x14_wpTracker.begin(), x14_wpTracker.end(), waypoint->GetUniqueId()) !=
          x14_wpTracker.end()) {
        return;
      }

      AddKnot(waypoint->GetTranslation(), waypoint->GetTransform().GetForward());

      waypoint = TCastToPtr< CScriptCameraWaypoint >(mgr.ObjectById(waypoint->NextWaypoint(mgr)));
    }
  }
}

float CCameraSpline::GetKnotT(int idx) const {
  if (idx < x4_positions.size()) {
    return x24_t[idx];
  }
  return 0.f;
}

CVector3f CCameraSpline::GetKnotPosition(int idx) const {
  if (idx < x4_positions.size()) {
    return x4_positions[idx];
  }
  return CVector3f::Zero();
}

void CCameraSpline::SetKnotPosition(int idx, CVector3f pos) {
  if (idx >= x4_positions.size()) {
    return;
  }
  x4_positions[idx] = pos;
}

void CCameraSpline::AddKnot(CVector3f pos, CVector3f dir) {
  x4_positions.push_back(pos);
  x34_directions.push_back(dir);
}

void CCameraSpline::Reset(int size) {
  x4_positions.clear();
  x24_t.clear();
  x34_directions.clear();
  if (static_cast< uint >(size) == 0) {
    return;
  }
  x4_positions.reserve(size);
  x24_t.reserve(size);
  x34_directions.reserve(size);
}

bool CCameraSpline::GetSurroundingPoints(int idx, rstl::reserved_vector< CVector3f, 4 >& positions,
                                         rstl::reserved_vector< CVector3f, 4 >& directions) {
  const int size = x4_positions.size();
  if (size <= 3 || idx < 0 || idx >= size) {
    return false;
  }

  if (idx > 0) {
    positions.push_back(x4_positions[idx - 1]);
    directions.push_back(x34_directions[idx - 1]);
  } else {
    if (x48_closedLoop) {
      positions.push_back(x4_positions[size - 1]);
      directions.push_back(x34_directions[size - 1]);
    } else {
      positions.push_back(x4_positions[0] - (x4_positions[1] - x4_positions[0]));
      directions.push_back(x34_directions[0]);
    }
  }

  positions.push_back(x4_positions[idx]);
  directions.push_back(x34_directions[idx]);

  ++idx;
  if (idx >= size) {
    if (x48_closedLoop) {
      positions.push_back(x4_positions[idx - size]);
      directions.push_back(x34_directions[idx - size]);
    } else {
      positions.push_back(x4_positions[size - 1] -
                          (x4_positions[size - 2] - x4_positions[size - 1]));
      directions.push_back(x34_directions[size - 1]);
    }
  } else {
    positions.push_back(x4_positions[idx]);
    directions.push_back(x34_directions[idx]);
  }

  ++idx;
  if (idx >= size) {
    if (x48_closedLoop) {
      positions.push_back(x4_positions[idx - size]);
      directions.push_back(x34_directions[idx - size]);
    } else {
      positions.push_back(x4_positions[size - 1] -
                          (x4_positions[size - 2] - x4_positions[size - 1]));
      directions.push_back(x34_directions[size - 1]);
    }
  } else {
    positions.push_back(x4_positions[idx]);
    directions.push_back(x34_directions[idx]);
  }

  return true;
}

float CCameraSpline::ClampLength(const CVector3f pos, bool collide, const CMaterialFilter filter,
                                 const CStateManager& mgr) const {
  if (x4_positions.empty()) {
    return 0.f;
  }
  if (x48_closedLoop) {
    return 0.f;
  }

  const CVector3f first = x4_positions.front();
  const CVector3f last = x4_positions.back();
  CVector3f deltaA = pos - first;
  CVector3f deltaB = pos - last;
  const float magA = deltaA.Magnitude();
  const float magB = deltaB.Magnitude();
  if (!deltaA.CanBeNormalized()) {
    return 0.f;
  }
  if (!deltaB.CanBeNormalized()) {
    return x44_length;
  }

  if (collide) {
    const CRayCastResult collideA =
        mgr.RayStaticIntersection(first, deltaA.AsNormalized(), deltaA.Magnitude(), filter);
    const CRayCastResult collideB =
        mgr.RayStaticIntersection(last, deltaB.AsNormalized(), deltaB.Magnitude(), filter);
    if (collideA.IsValid()) {
      return x44_length;
    }
    if (collideB.IsValid()) {
      return 0.f;
    }
  }

  return magA < magB ? 0.f : x44_length;
}

float CCameraSpline::CalculateSplineLength() {
  float ret = 0.f;
  x24_t.clear();
  if (x4_positions.size() > 0) {
    CVector3f prevPoint = x4_positions[0];
    const float tDiv = 1.f / float(x4_positions.size() - 1);
    for (int i = 0; i < x4_positions.size() - 1; ++i) {
      const float baseT = i * tDiv;
      float subT = 0.f;
      x24_t.push_back(ret);
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

    x24_t.push_back(ret);
    if (x48_closedLoop) {
      const CVector3f delta = x4_positions[0] - x4_positions[x4_positions.size() - 1];
      if (delta.CanBeNormalized()) {
        ret += delta.Magnitude();
      }
    }

    return ret;
  }
  return 0.f;
}

float CCameraSpline::ValidateLength(float t) const {
  if (x48_closedLoop) {
    while (t >= x44_length) {
      t -= x44_length;
    }
    while (t < 0.f) {
      t += x44_length;
    }
  } else {
    t = CMath::Clamp(0.f, t, x44_length);
  }
  return t;
}

CVector3f CCameraSpline::GetInterpolatedSplinePointByTime(float time, float range) const {
  if (x4_positions.size() > 0) {
    const int size = x4_positions.size();
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
  if (x4_positions.size() > 0) {
    int baseIdx = 0;
    int i = 0;
    for (i = 1; i < x4_positions.size(); ++i) {
      if (x24_t[i] > pos) {
        baseIdx = i - 1;
        break;
      }
    }

    if (i == x4_positions.size()) {
      baseIdx = i - 1;
    }
    if (pos < 0.f) {
      baseIdx = 0;
    }

    float range = x44_length;
    if (pos >= range) {
      if (x48_closedLoop) {
        baseIdx = 0;
        pos -= range;
      } else {
        baseIdx = x4_positions.size() - 2;
        pos = range;
      }
    }

    if (baseIdx == x4_positions.size() - 1) {
      if (x48_closedLoop) {
        range -= x24_t[baseIdx];
      } else {
        range -= x24_t[x4_positions.size() - 2];
      }
    } else {
      range = x24_t[baseIdx + 1] - x24_t[baseIdx];
    }

    float tRaw = (pos - x24_t[baseIdx]) / range;
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

  int iterations = x4_positions.size() - 1;
  if (x48_closedLoop) {
    iterations += 1;
  }

  for (int i = 0; i < iterations; ++i) {
    const float thisPosX = x4_positions[i].GetX();
    const float thisPosY = x4_positions[i].GetY();
    const float thisPosZ = x4_positions[i].GetZ();

    float nextPosX, nextPosY, nextPosZ;
    if (!x48_closedLoop) {
      nextPosX = x4_positions[i + 1].GetX();
      nextPosY = x4_positions[i + 1].GetY();
      nextPosZ = x4_positions[i + 1].GetZ();
    } else if (i == x4_positions.size() - 1) {
      nextPosX = x4_positions[0].GetX();
      nextPosY = x4_positions[0].GetY();
      nextPosZ = x4_positions[0].GetZ();
    } else {
      nextPosX = x4_positions[i + 1].GetX();
      nextPosY = x4_positions[i + 1].GetY();
      nextPosZ = x4_positions[i + 1].GetZ();
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
      forwardDir = CVector3f((thisPosX - x4_positions[i - 1].GetX()) + deltaX,
                             (thisPosY - x4_positions[i - 1].GetY()) + deltaY,
                             (thisPosZ - x4_positions[i - 1].GetZ()) + deltaZ);
    } else {
      CVector3f extrap = x4_positions[0] - (x4_positions[1] - x4_positions[0]);
      if (x48_closedLoop) {
        extrap = x4_positions[x4_positions.size() - 1];
      }
      forwardDir =
          CVector3f((thisPosX - extrap.GetX()) + deltaX, (thisPosY - extrap.GetY()) + deltaY,
                    (thisPosZ - extrap.GetZ()) + deltaZ);
    }
    forwardDir.Normalize();

    if (i < x4_positions.size() - 2) {
      backwardDir = CVector3f((nextPosX - x4_positions[i + 2].GetX()) + revDeltaX,
                              (nextPosY - x4_positions[i + 2].GetY()) + revDeltaY,
                              (nextPosZ - x4_positions[i + 2].GetZ()) + revDeltaZ);
    } else {
      CVector3f extrap;
      if (x48_closedLoop) {
        extrap = (i == iterations - 1) ? x4_positions[1] : x4_positions[0];
      } else {
        extrap = x4_positions[i + 1] + (x4_positions[i + 1] - x4_positions[i]);
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

    if (!x48_closedLoop) {
      if (i == 0 && t < 0.f) {
        t = 0.f;
      }
      if (i == x4_positions.size() - 2 && t > 1.f) {
        t = 1.f;
      }
    }

    if (t >= 0.f && t <= 1.f) {
      float tLen;
      if (i == x4_positions.size() - 1) {
        tLen = x44_length - x24_t[i];
      } else {
        tLen = x24_t[i + 1] - x24_t[i];
      }

      const float lenT = t * tLen + x24_t[i];
      const CVector3f pointDelta = p - GetInterpolatedSplinePointByLength(lenT).GetTranslation();
      float mag = 0.f;
      if (pointDelta.CanBeNormalized()) {
        mag = pointDelta.Magnitude();
      }

      float lenDelta = CMath::AbsF(lenT - time);
      if (x48_closedLoop) {
        const float altDelta = x44_length - lenDelta;
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
  if (x178_perspInterpRemTime > 0.f) {
    x15c_currentFov = x184_perspInterpEndFov;
    x170_24_perspDirty = true;
  }
  x178_perspInterpRemTime = 0.f;
  x174_delayTime = 0.f;
}

void CGameCamera::InterpolateFOV(float start, float fov, float time, float delayTime) {
  if (time <= 0.f) {
    x15c_currentFov = fov;
    x170_24_perspDirty = true;
    x184_perspInterpEndFov = fov;
    x178_perspInterpRemTime = 0.f;
    x174_delayTime = 0.f;
    return;
  }

  if (delayTime < 0.f) {
    x174_delayTime = 0.f;
  } else {
    x174_delayTime = delayTime;
  }
  x17c_perspInterpDur = time;
  x178_perspInterpRemTime = time;
  x180_perspInterpStartFov = start;
  x184_perspInterpEndFov = fov;
  x15c_currentFov = start;
  x170_24_perspDirty = true;
}

void CGameCamera::UpdatePerspective(float dt) {
  if (x174_delayTime > 0.f) {
    x174_delayTime -= dt;
    return;
  }

  if (x178_perspInterpRemTime > 0.f) {
    x178_perspInterpRemTime -= dt;
    const float remTime = x178_perspInterpRemTime;
    if (remTime <= 0.f) {
      x15c_currentFov = x184_perspInterpEndFov;
      x170_24_perspDirty = true;
    } else {
      const float fovDelta = x180_perspInterpStartFov - x184_perspInterpEndFov;
      const float t = remTime / x17c_perspInterpDur;
      x15c_currentFov = fovDelta * CMath::Clamp(0.f, t, 1.f) + x184_perspInterpEndFov;
      x170_24_perspDirty = true;
    }
  }
}
