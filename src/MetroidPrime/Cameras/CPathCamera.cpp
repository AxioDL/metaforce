#include "MetroidPrime/Cameras/CPathCamera.hpp"

#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDoor.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "rstl/math.hpp"

static const CMaterialList kLineOfSightIncludeList = CMaterialList(kMT_Solid);
static const CMaterialList kLineOfSightExcludeList = CMaterialList(kMT_ProjectilePassthrough);
static const CMaterialFilter kLineOfSightFilter =
    CMaterialFilter::MakeIncludeExclude(kLineOfSightIncludeList, kLineOfSightExcludeList);
CPathCamera::CPathCamera(const TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                         const CTransform4f& xf, const bool active, const float lengthExtent,
                         const float filterMag, const float filterProportion,
                         const float minEaseDist, const float maxEaseDist, const uint flags,
                         const EInitialSplinePosition initPos)
: CGameCamera(uid, active, name, info, xf, CCameraManager::GetDefaultThirdPersonVerticalFOV(),
              CCameraManager::GetDefaultFirstPersonNearClipDistance(),
              CCameraManager::GetDefaultFirstPersonFarClipDistance(),
              CCameraManager::GetDefaultAspectRatio(), kInvalidUniqueId, false, 0)
, mSpline(flags & 1)
, mPos(0.f)
, mTime(0.f)
, mLengthExtent(lengthExtent)
, mFilterMag(filterMag)
, mFilterProportions(filterProportion)
, mInitialPosition(initPos)
, mFlags(flags)
, mMinEaseDist(minEaseDist)
, mMaxEaseDist(maxEaseDist) {}

CPathCamera::~CPathCamera() {}

void CPathCamera::Reset(const CTransform4f&, CStateManager& mgr) {
  const CPlayer& player = *mgr.GetPlayer();
  CVector3f playerPos = player.GetTranslation();
  playerPos.SetZ(playerPos.GetZ() + gpTweakPlayer->GetPlayerBallHalfExtent());
  const float closestLength = mSpline.FindClosestLengthOnSpline(0.f, playerPos);
  const float negLength = rstl::max_val(0.f, closestLength - mLengthExtent);
  const CVector3f negPoint = mSpline.GetInterpolatedSplinePointByLength(negLength).GetTranslation();
  const float posLength = rstl::min_val(closestLength + mLengthExtent, mSpline.GetLength());
  const CVector3f posPoint = mSpline.GetInterpolatedSplinePointByLength(posLength).GetTranslation();

  CTransform4f camXf = mgr.GetCameraManager()->GetBallCamera()->GetTransform();
  if (mgr.GetPlayer()->GetMorphballTransitionState() != CPlayer::kMS_Morphed) {
    camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
  }

  bool negative = false;
  const CVector3f camPos = camXf.GetTranslation();
  if (mInitialPosition == kISP_BallCamBasis) {
    const CVector3f toPlayer = playerPos - negPoint;
    if (toPlayer.CanBeNormalized()) {
      if (CVector3f::Dot(camXf.GetForward(), toPlayer.AsNormalized()) > 0.f) {
        negative = true;
      }
    }
  } else {
    negative = mInitialPosition == kISP_Negative;
  }

  const CVector3f toNeg = negPoint - camPos;
  mgr.RayStaticIntersection(camPos, toNeg.AsNormalized(), toNeg.Magnitude(), kLineOfSightFilter);
  const CVector3f toPos = posPoint - camPos;
  mgr.RayStaticIntersection(camPos, toPos.AsNormalized(), toPos.Magnitude(), kLineOfSightFilter);

  CVector3f viewPoint = CVector3f::Zero();
  if (negative) {
    mPos = negLength;
    viewPoint = negPoint;
  } else {
    mPos = posLength;
    viewPoint = posPoint;
  }

  if (mInitialPosition == kISP_ClampBasis) {
    if (mSpline.ClampLength(playerPos, false, kLineOfSightFilter, mgr) <= negLength) {
      mPos = negLength;
      viewPoint = negPoint;
    } else {
      mPos = posLength;
      viewPoint = posPoint;
    }
  }

  const CVector3f lookPos = mgr.GetCameraManager()->GetBallCamera()->GetFixedLookPos();
  SetTransform(CTransform4f::LookAt(viewPoint, lookPos));
}

CTransform4f CPathCamera::MoveAlongSpline(float dt, CStateManager& mgr) {
  CTransform4f ret = GetTransform();
  const CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
  const float closestLength = mSpline.FindClosestLengthOnSpline(mTime, playerPos);
  mTime = closestLength;
  float extent = mLengthExtent;
  if (mFlags & 8) {
    float distance = 0.f;
    CVector3f toPlayer =
        playerPos - mSpline.GetInterpolatedSplinePointByLength(closestLength).GetTranslation();
    toPlayer.SetZ(0.f);
    if (toPlayer.CanBeNormalized()) {
      distance = toPlayer.Magnitude();
    }
    const float eased = (distance - mMinEaseDist) / (mMaxEaseDist - mMinEaseDist);
    const float clamped = CMath::Clamp(0.f, eased, 1.f);
    float radians = 0.017453292f;
    radians *= 90.f * clamped;
    extent *= 1.f - CMath::Limit(CMath::AbsF(sinf(radians)), 1.f);
  }

  float newPos;
  if (mSpline.IsClosedLoop()) {
    const float posLength = mSpline.ValidateLength(closestLength + extent);
    const float negLength = mSpline.ValidateLength(closestLength - extent);
    const float distance = CMath::AbsF(mPos - closestLength);
    const float remaining = mSpline.GetLength() - distance;
    if (mPos > closestLength) {
      if (distance <= remaining) {
        newPos = posLength;
      } else {
        newPos = negLength;
      }
    } else {
      newPos = distance <= remaining ? negLength : posLength;
    }
  } else {
    if (mPos > closestLength) {
      newPos = mSpline.ValidateLength(closestLength + extent);
    } else {
      newPos = mSpline.ValidateLength(closestLength - extent);
    }
  }

  if (mFlags & 2) {
    mPos = newPos;
    ret = mSpline.GetInterpolatedSplinePointByLength(mPos);
  } else {
    if (mSpline.IsClosedLoop()) {
      float distance = CMath::AbsF(newPos - mPos);
      const float remainingDistance = mSpline.GetLength() - distance;
      float minDistance = distance;
      if (distance > remainingDistance) {
        minDistance = remainingDistance;
      }
      float step = CMath::Limit(minDistance / mFilterProportions, 1.f);
      step *= mFilterMag * dt;
      const float absDelta = CMath::AbsF(mPos - newPos);
      const float remaining = mSpline.GetLength() - absDelta;
      if (mPos > newPos) {
        if (absDelta <= remaining) {
          step *= -1.f;
        }
      } else {
        if (absDelta > remaining) {
          step *= -1.f;
        }
      }
      mPos = mSpline.ValidateLength(mPos + step);
    } else {
      const float bias = CMath::Limit((newPos - mPos) / mFilterProportions, 1.f);
      const float step = bias * (mFilterMag * dt);
      mPos = mSpline.ValidateLength(mPos + step);
    }
    ret = mSpline.GetInterpolatedSplinePointByLength(mPos);
  }
  return ret;
}

void CPathCamera::AvoidDoorCollisions(CStateManager& mgr) {

  if (const CScriptDoor* door = TCastToConstPtr< CScriptDoor >(
          mgr.GetObjectById(mgr.GetCameraManager()->GetBallCamera()->GetTooCloseActorId()))) {
    if (!door->IsOpen() && CBallCamera::CheckDoorProximity(GetTranslation(), mgr)) {
      float tmp = mTime + mLengthExtent;
      if (mPos > mTime) {
        tmp = mTime - mLengthExtent;
      }

      mPos = tmp;
      const CVector3f tmpVec = mSpline.GetInterpolatedSplinePointByLength(tmp).GetTranslation();
      SetTranslation(tmpVec);
    }
  }
}

void CPathCamera::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  if (mgr.GetCameraManager()->GetPathCameraId() != GetUniqueId()) {
    return;
  }

  CTransform4f xf = GetTransform();
  if (mSpline.GetSize() <= 0) {
    return;
  }

  CVector3f ballLook = mgr.GetCameraManager()->GetBallCamera()->GetLookAtPosition();
  if (mFlags & 0x10) {
    if (const CScriptCameraHint* hint = mgr.GetCameraManager()->GetCameraHint(mgr)) {
      ballLook.SetZ(hint->GetTranslation().GetZ());
    }
  }

  const CVector3f toCamera = GetTranslation() - ballLook;
  const CVector3f velocity = mgr.GetPlayer()->GetVelocityWR();
  if (!velocity.CanBeNormalized() && toCamera.CanBeNormalized()) {
    if (mFlags & 4) {
      SetTransform(mSpline.GetInterpolatedSplinePointByLength(mPos));
    } else {
      SetTransform(CTransform4f::LookAt(GetTranslation(), ballLook));
    }
    return;
  }

  xf = MoveAlongSpline(dt, mgr);
  SetTranslation(xf.GetTranslation());
  if (mFlags & 0x20) {
    AvoidDoorCollisions(mgr);
  }

  CVector3f toLook = ballLook - GetTranslation();
  toLook.SetZ(0.f);
  if (toLook.CanBeNormalized()) {
    SetTransform(CTransform4f::LookAt(GetTranslation(), ballLook));
  }
  if (mFlags & 4) {
    SetTransform(xf);
  }
}

void CPathCamera::ProcessInput(const CFinalInput&, CStateManager& mgr) {
  // Empty
}

ENTITY_ACCEPT_IMPL(CPathCamera)

void CPathCamera::Render(const CStateManager& mgr) const {}

void CPathCamera::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CGameCamera::AcceptScriptMsg(msg, uid, mgr);

  if (!GetActive() || msg != kSM_InitializedInArea) {
    return;
  }
  mSpline.Initialise(GetUniqueId(), GetConnectionList(), mgr);
}
