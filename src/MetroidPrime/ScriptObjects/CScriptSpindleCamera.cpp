#include "MetroidPrime/ScriptObjects/CScriptSpindleCamera.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CMaterialList.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraHint.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

extern uint LoadParameterFlags(CInputStream& in);

static const CMaterialList sIncludeMaterial = CMaterialList(kMT_Solid);
static const CMaterialList sExcludeMaterial = CMaterialList(kMT_ProjectilePassthrough, kMT_Player, kMT_Character,
                                            kMT_CameraPassthrough);
static const CMaterialFilter sMaterialFilter =
    CMaterialFilter::MakeIncludeExclude(sIncludeMaterial, sExcludeMaterial);

CSpindleCameraInterpolant::CSpindleCameraInterpolant(ESpindleInput input, uint flags, float lowOut,
                                                     float highOut, float lowIn, float highIn)
: mFlags(flags)
, mInput(input)
, mLowOut(lowOut)
, mHighOut(highOut)
, mLowIn(lowIn)
, mHighIn(highIn) {}

CSpindleCameraInterpolant LoadSpindleSegment(CInputStream& in) {
  ESpindleInput input = static_cast< ESpindleInput >(in.ReadLong());
  uint flags = LoadParameterFlags(in);
  float lowOut = in.ReadFloat();
  float highOut = in.ReadFloat();
  float lowIn = in.ReadFloat();
  float highIn = in.ReadFloat();

  switch (input) {
  case kSI_HintBallAngle:
  case kSI_HintBallRightAngle:
  case kSI_HintBallLeftAngle:
    lowIn *= 0.017453292f;
    highIn *= 0.017453292f;
    break;
  default:
    break;
  }

  return CSpindleCameraInterpolant(input, flags, lowOut, highOut, lowIn, highIn);
}

void CSpindleCameraInterpolant::ConvertToRadians() {
  mLowOut *= 0.017453292f;
  mHighOut *= 0.017453292f;
}

float CSpindleCameraInterpolant::InterpolateValue(float inVar) const {
  float ret;
  if (mInput == kSI_Constant) {
    ret = mLowOut;
  } else {
    const float reflectRange = mHighIn - mLowIn;
    if (close_enough(reflectRange, 0.f)) {
      ret = mLowOut;
    } else {
      const float outRange = mHighOut - mLowOut;
      float reflectedVar = inVar;
      if ((mFlags & 0x1) != 0 && inVar > mHighIn) {
        reflectedVar = mHighIn - (inVar - mHighIn);
      }
      if ((mFlags & 0x2) != 0 && inVar < mLowIn) {
        reflectedVar = mLowIn + (mLowIn - inVar);
      }

      float value = mLowOut + (outRange * (reflectedVar - mLowIn)) / reflectRange;
      if (mLowOut < mHighOut) {
        value = CMath::Clamp(mLowOut, value, mHighOut);
      } else {
        value = CMath::Clamp(mHighOut, value, mLowOut);
      }
      ret = value;
    }
  }
  return ret;
}

CScriptSpindleCamera::CScriptSpindleCamera(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    const bool active, int flags, float hintToCamDistMin, float hintToCamDistMax,
    float hintToCamVOffMin, float hintToCamVOffMax,
    CSpindleCameraInterpolant targetHintToCamDeltaAngleVel,
    CSpindleCameraInterpolant deltaAngleScaleWithCamDist, CSpindleCameraInterpolant hintToCamDist,
    CSpindleCameraInterpolant distOffsetFromBallDist,
    CSpindleCameraInterpolant hintBallToCamAzimuth, CSpindleCameraInterpolant unused,
    CSpindleCameraInterpolant maxHintBallToCamAzimuth, CSpindleCameraInterpolant camLookRelAzimuth,
    CSpindleCameraInterpolant lookPosZOffset, CSpindleCameraInterpolant camPosZOffset,
    CSpindleCameraInterpolant clampedAzimuthFromHintDir,
    CSpindleCameraInterpolant dampingAzimuthSpeed,
    CSpindleCameraInterpolant targetHintToCamDeltaAngleVelRange,
    CSpindleCameraInterpolant deleteHintBallDist,
    CSpindleCameraInterpolant recoverClampedAzimuthFromHintDir)
: CGameCamera(uid, active, name, info, xf, CCameraManager::GetDefaultThirdPersonVerticalFOV(),
              CCameraManager::GetDefaultFirstPersonNearClipDistance(),
              CCameraManager::GetDefaultFirstPersonFarClipDistance(),
              CCameraManager::GetDefaultAspectRatio(), kInvalidUniqueId, false, 0)
, mFlags(flags)
, mInVars()
, mHintToCamDistMin(hintToCamDistMin)
, mHintToCamDistMax(hintToCamDistMax)
, mHintToCamVOffMin(hintToCamVOffMin)
, mHintToCamVOffMax(hintToCamVOffMax)
, mTargetHintToCamDeltaAngleVel(targetHintToCamDeltaAngleVel)
, mDeltaAngleScaleWithCamDist(deltaAngleScaleWithCamDist)
, mHintToCamDist(hintToCamDist)
, mDistOffsetFromBallDist(distOffsetFromBallDist)
, mHintBallToCamAzimuth(hintBallToCamAzimuth)
, mUnused(unused)
, mMaxHintBallToCamAzimuth(maxHintBallToCamAzimuth)
, mCamLookRelAzimuth(camLookRelAzimuth)
, mLookPosZOffset(lookPosZOffset)
, mCamPosZOffset(camPosZOffset)
, mClampedAzimuthFromHintDir(clampedAzimuthFromHintDir)
, mDampingAzimuthSpeed(dampingAzimuthSpeed)
, mTargetHintToCamDeltaAngleVelRange(targetHintToCamDeltaAngleVelRange)
, mDeleteHintBallDist(deleteHintBallDist)
, mRecoverClampedAzimuthFromHintDir(recoverClampedAzimuthFromHintDir)
, mMaxAzimuthInterpTimer(0.f)
, mOutsideClampedAzimuth(false)
, mLookDir(xf.GetForward()) {}

CScriptSpindleCamera::~CScriptSpindleCamera() {}

void CScriptSpindleCamera::Reset(const CTransform4f&, CStateManager& mgr) {
  const CScriptCameraHint* hint = mgr.GetCameraManager()->GetCameraHint(mgr);
  if (!GetActive() || hint == nullptr) {
    return;
  }

  mInResetThink = true;
  mgr.GetCameraManager()->BallCamera()->UpdateLookAtPosition(0.01f, mgr);
  Think(0.01f, mgr);
  mInResetThink = false;
}

float CScriptSpindleCamera::GetInterpolant(const CSpindleCameraInterpolant& seg) const {
  return mInVars[static_cast< int >(seg.mInput)];
}

void CScriptSpindleCamera::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  const CBallCamera* ballCam = mgr.CameraManager()->GetBallCamera();
  const CScriptCameraHint* hint = mgr.CameraManager()->GetCameraHint(mgr);
  if (hint == nullptr) {
    return;
  }

  CVector3f hintPos = hint->GetTranslation();
  CVector3f lookAheadPos = ballCam->GetLookPosAhead();

  CVector3f hintToCamDir = GetTranslation() - hintPos;
  hintToCamDir[kDZ] = 0.f;

  const CVector3f ballPos = mgr.GetPlayer()->GetBallPosition();
  CVector3f hintToBallDir = ballPos - hintPos;
  const float hintToBallVOff = hintToBallDir.GetZ();
  hintToBallDir[kDZ] = 0.f;

  float hintToBallDist = 0.f;

  CVector3f hintDir = hint->GetTransform().GetForward();
  hintDir[kDZ] = 0.f;
  if (hintDir.CanBeNormalized()) {
    hintDir.Normalize();
  } else {
    hintDir = CVector3f(0.f, 1.f, 0.f);
  }

  if (hintToBallDir.CanBeNormalized()) {
    hintToBallDist = hintToBallDir.Magnitude();
    hintToBallDir.Normalize();
  } else {
    hintToBallDir = hintDir;
  }

  mInVars.clear();
  mInVars.push_back(0.f);
  mInVars.push_back(hintToBallDist);
  mInVars.push_back(CMath::AbsF(hintToBallVOff));

  float hintBallDot = CMath::Limit(CVector3f::Dot(hintToBallDir, hintDir), 1.f);
  const float hintBallAngle = CMath::AbsF(acos(hintBallDot));
  mInVars.push_back(hintBallAngle);

  const float hintBallCross =
      hintToBallDir.GetX() * hintDir.GetY() - hintDir.GetX() * hintToBallDir.GetY();
  if (hintBallCross >= 0.f) {
    mInVars.push_back(hintBallAngle);
    mInVars.push_back(2.f * M_PIF - hintBallAngle);
  } else {
    mInVars.push_back(2.f * M_PIF - hintBallAngle);
    mInVars.push_back(hintBallAngle);
  }

  CVector3f hintDelta = hint->GetTranslation() - hint->GetOriginalTransform().GetTranslation();
  const float hintDeltaVOff = CMath::AbsF(hintDelta.GetZ());
  hintDelta[kDZ] = 0.f;
  float hintDeltaDist = 0.f;
  if (hintDelta.CanBeNormalized()) {
    hintDeltaDist = hintDelta.Magnitude();
  }
  mInVars.push_back(hintDeltaDist);
  mInVars.push_back(hintDeltaVOff);

  if ((mFlags & 0x2000) != 0 &&
      hintToBallDist > mDeleteHintBallDist.InterpolateValue(GetInterpolant(mDeleteHintBallDist))) {
    if (hint->GetDelegatedCameraId() == GetUniqueId()) {
      mgr.CameraManager()->ReallyRemoveCameraHint(hint->GetUniqueId(), mgr);
    }
  } else {
    if ((mFlags & 0x800) == 0) {
      hintToBallDir = hintDir;
    }

    if ((mFlags & 0x20) != 0) {
      if (!mOutsideClampedAzimuth) {
        if (hintBallAngle >
            mClampedAzimuthFromHintDir.InterpolateValue(GetInterpolant(mClampedAzimuthFromHintDir))) {
          mLookDir = hintToBallDir;
          mOutsideClampedAzimuth = true;
        }
      } else {
        const float hintCamCross =
            hintToCamDir.GetX() * hintDir.GetY() - hintDir.GetX() * hintToCamDir.GetY();
        if ((hintBallAngle < mRecoverClampedAzimuthFromHintDir.InterpolateValue(
                                 GetInterpolant(mRecoverClampedAzimuthFromHintDir)) &&
             hintBallCross * hintCamCross < 0.f) ||
            hintBallAngle <=
                mClampedAzimuthFromHintDir.InterpolateValue(GetInterpolant(mClampedAzimuthFromHintDir))) {
          mOutsideClampedAzimuth = false;
        } else {
          hintToBallDir = mLookDir;
        }
      }
    }

    float newHintToCamDist = mHintToCamDist.InterpolateValue(GetInterpolant(mHintToCamDist));
    if ((mFlags & 0x40) != 0) {
      newHintToCamDist = hintToBallDist + mDistOffsetFromBallDist.InterpolateValue(
                                              GetInterpolant(mDistOffsetFromBallDist));
    }
    newHintToCamDist = CMath::Clamp(mHintToCamDistMin, newHintToCamDist, mHintToCamDistMax);

    CVector3f newCamPos = GetTranslation();
    float hintToCamDist = hintToCamDir.Magnitude();
    if (hintToCamDir.CanBeNormalized()) {
      hintToCamDir.Normalize();
    } else {
      hintToCamDir = hintDir;
      hintToCamDist = mHintToCamDist.InterpolateValue(GetInterpolant(mHintToCamDist));
    }

    float hintBallToCamTargetAzimuth =
        mHintBallToCamAzimuth.InterpolateValue(GetInterpolant(mHintBallToCamAzimuth));
    if ((mFlags & 0x4000) == 0 && CVector3f::Cross(hintToCamDir, hintToBallDir).GetZ() >= 0.f) {
      hintBallToCamTargetAzimuth = -hintBallToCamTargetAzimuth;
    }

    CQuaternion hintBallToCamTargetAzimuthQuat = CQuaternion::ZRotation(hintBallToCamTargetAzimuth);
    const CVector3f targetHintToCam = hintBallToCamTargetAzimuthQuat.Transform(hintToBallDir);

    CVector3f newHintToCamDir = hintToCamDir;

    float hintToCamDot = CMath::Limit(CVector3f::Dot(hintToCamDir, targetHintToCam), 1.f);
    const float hintToCamDeltaAngleRange = CMath::AbsF(acosf(hintToCamDot));

    float hintToCamDeltaAngleSpeedFactor =
        CMath::Limit(hintToCamDeltaAngleRange /
                         mDampingAzimuthSpeed.InterpolateValue(GetInterpolant(mDampingAzimuthSpeed)),
                     1.f);

    float targetHintToCamDeltaAngleVel =
        mTargetHintToCamDeltaAngleVel.InterpolateValue(GetInterpolant(mTargetHintToCamDeltaAngleVel));
    if ((mFlags & 0x100) == 0) {
      targetHintToCamDeltaAngleVel = CMath::Limit(
          mDeltaAngleScaleWithCamDist.InterpolateValue(GetInterpolant(mDeltaAngleScaleWithCamDist)) /
              hintToCamDist,
          targetHintToCamDeltaAngleVel);
    }

    if ((CVector3f::Cross(hintToBallDir, hintToCamDir).GetZ() >= 0.f &&
         CVector3f::Cross(targetHintToCam, hintToCamDir).GetZ() < 0.f) ||
        (CVector3f::Cross(hintToBallDir, hintToCamDir).GetZ() < 0.f &&
         CVector3f::Cross(targetHintToCam, hintToCamDir).GetZ() >= 0.f)) {
      const float targetHintToCamDeltaAngleVelRange =
          mTargetHintToCamDeltaAngleVelRange.InterpolateValue(
              GetInterpolant(mTargetHintToCamDeltaAngleVelRange));
      targetHintToCamDeltaAngleVel =
          CMath::Limit(targetHintToCamDeltaAngleVel, targetHintToCamDeltaAngleVelRange);
    }

    CVector3f camToBall = ballPos - GetTranslation();
    camToBall[kDZ] = 0.f;

    float targetHintToCamDeltaAngle =
        targetHintToCamDeltaAngleVel * dt * hintToCamDeltaAngleSpeedFactor;
    float camToBallDist = 0.f;
    if (camToBall.CanBeNormalized()) {
      camToBallDist = camToBall.Magnitude();
    }

    targetHintToCamDeltaAngle *=
        (1.f - CMath::Clamp(0.f, (camToBallDist - 2.f) * 0.5f, 1.f)) * 10.f + 1.f;
    targetHintToCamDeltaAngle = CMath::Limit(targetHintToCamDeltaAngle, hintToCamDeltaAngleRange);

    float hintToCamDot2 = CMath::Limit(CVector3f::Dot(hintToCamDir, targetHintToCam), 1.f);
    if (CMath::AbsF(hintToCamDot2) < 0.99999f) {
      hintBallToCamTargetAzimuthQuat =
          CQuaternion::LookAt(hintToCamDir, targetHintToCam, targetHintToCamDeltaAngle);
      newHintToCamDir = hintBallToCamTargetAzimuthQuat.Transform(hintToCamDir);
    }

    float hintBallToCamDot = CMath::Limit(CVector3f::Dot(hintToBallDir, newHintToCamDir), 1.f);
    const float hintBallToCamAzimuth = acosf(hintBallToCamDot);
    if ((mFlags & 0x10) != 0) {
      if (CMath::AbsF(hintBallToCamAzimuth) <
              mHintBallToCamAzimuth.InterpolateValue(GetInterpolant(mHintBallToCamAzimuth)) ||
          (mFlags & 0x8) != 0 || mInResetThink) {
        newHintToCamDir = targetHintToCam;
      }
    }

    const float maxHintBallToCamAzimuth =
        mMaxHintBallToCamAzimuth.InterpolateValue(GetInterpolant(mMaxHintBallToCamAzimuth));

    if (CMath::AbsF(hintBallToCamAzimuth) > maxHintBallToCamAzimuth) {
      mMaxAzimuthInterpTimer += dt;
      if (mMaxAzimuthInterpTimer < 3.f) {
        const float ballToCamAzimuthInterp = CMath::Limit(mMaxAzimuthInterpTimer / 3.f, 1.f);
        float hintBallToCamAzimuthDelta =
            CMath::AbsF(maxHintBallToCamAzimuth - hintBallToCamAzimuth);
        if (CVector3f::Cross(hintToBallDir, newHintToCamDir).GetZ() > 0.f) {
          hintBallToCamAzimuthDelta = -hintBallToCamAzimuthDelta;
        }

        hintBallToCamTargetAzimuthQuat =
            CQuaternion::ZRotation(hintBallToCamAzimuthDelta * ballToCamAzimuthInterp);
        newHintToCamDir = hintBallToCamTargetAzimuthQuat.Transform(newHintToCamDir);
      } else {
        if (hintBallToCamTargetAzimuth > 0.f) {
          hintBallToCamTargetAzimuthQuat = CQuaternion::ZRotation(maxHintBallToCamAzimuth);
        } else {
          hintBallToCamTargetAzimuthQuat = CQuaternion::ZRotation(-maxHintBallToCamAzimuth);
        }
        newHintToCamDir = hintBallToCamTargetAzimuthQuat.Transform(hintToBallDir);
      }
    } else {
      mMaxAzimuthInterpTimer = 0.f;
    }

    if ((mFlags & 0x20) != 0) {
      CVector3f hintDir2 = hint->GetTransform().GetForward();
      hintDir2.SetZ(0.f);
      if (hintDir2.CanBeNormalized()) {
        hintDir2.Normalize();

        float hintCamDot = CMath::Limit(CVector3f::Dot(hintDir2, newHintToCamDir), 1.f);
        float hintCamAzimuth = CMath::Limit(
            CMath::AbsF(acosf(hintCamDot)),
            mClampedAzimuthFromHintDir.InterpolateValue(GetInterpolant(mClampedAzimuthFromHintDir)));

        const float hintDirCamCross = CVector3f::Cross(hintDir2, newHintToCamDir).GetZ();
        if (hintDirCamCross < 0.f) {
          hintCamAzimuth = -hintCamAzimuth;
        }

        hintBallToCamTargetAzimuthQuat = CQuaternion::ZRotation(hintCamAzimuth);
        newHintToCamDir = hintBallToCamTargetAzimuthQuat.Transform(hintDir2);
      }
    }

    newHintToCamDir *= newHintToCamDist;
    newCamPos = hintPos + newHintToCamDir;
    if ((mFlags & 0x80) != 0) {
      newCamPos[kDZ] = ballPos.GetZ() + mCamPosZOffset.InterpolateValue(GetInterpolant(mCamPosZOffset));
    } else {
      newCamPos[kDZ] = hintPos.GetZ() + mCamPosZOffset.InterpolateValue(GetInterpolant(mCamPosZOffset));
    }

    float hintToCamVOff = newCamPos.GetZ() - hintPos.GetZ();
    hintToCamVOff = CMath::Clamp(mHintToCamVOffMin, hintToCamVOff, mHintToCamVOffMax);
    newCamPos[kDZ] = hintToCamVOff + hintPos.GetZ();

    if ((mFlags & 0x200) != 0) {
      lookAheadPos[kDZ] =
          ballPos.GetZ() + mLookPosZOffset.InterpolateValue(GetInterpolant(mLookPosZOffset));
    } else {
      lookAheadPos[kDZ] =
          hintPos.GetZ() + mLookPosZOffset.InterpolateValue(GetInterpolant(mLookPosZOffset));
    }

    CVector3f newLookDelta = lookAheadPos - newCamPos;
    CVector3f newLookDirFlat = newLookDelta;
    newLookDirFlat[kDZ] = 0.f;

    if (newLookDirFlat.CanBeNormalized()) {
      const float newLookDistFlat = newLookDirFlat.Magnitude();
      newLookDirFlat.Normalize();

      float camLookRelAzimuth = -mCamLookRelAzimuth.InterpolateValue(GetInterpolant(mCamLookRelAzimuth));
      CVector3f newHintToCamDirFlat = newCamPos - hintPos;
      newHintToCamDirFlat[kDZ] = 0.f;

      if (newHintToCamDirFlat.CanBeNormalized()) {
        newHintToCamDirFlat.Normalize();
      } else {
        newHintToCamDirFlat = CVector3f(0.f, 1.f, 0.f);
      }

      const float newHintBallCross = CVector3f::Cross(newHintToCamDirFlat, hintToBallDir).GetZ();
      if (newHintBallCross >= 0.f) {
        camLookRelAzimuth = -camLookRelAzimuth;
      }

      if ((mFlags & 0x1000) != 0) {
        float hintLookDot = CMath::Limit(CVector3f::Dot(hintToBallDir, newHintToCamDirFlat), 1.f);
        float hintLookScale =
            CMath::Limit(static_cast< float >(acosf(CMath::AbsF(hintLookDot))) / 0.17453292f, 1.f);
        camLookRelAzimuth *= hintLookScale;
      }

      CQuaternion camLookRelAzimuthQuat = CQuaternion::ZRotation(camLookRelAzimuth);
      CVector3f azimuthVec = camLookRelAzimuthQuat.Transform(newLookDirFlat);
      const float azimuthCos = cosf(camLookRelAzimuth);
      float dx = azimuthCos * azimuthVec.GetX();
      float dy = azimuthCos * azimuthVec.GetY();
      dx *= newLookDistFlat;
      dy *= newLookDistFlat;
      lookAheadPos.SetX(newCamPos.GetX() + dx);
      lookAheadPos.SetY(newCamPos.GetY() + dy);
    }

    newLookDelta = lookAheadPos - newCamPos;

    if ((mFlags & 0x1) != 0) {
      newLookDelta = CVector3f(hintPos.GetX() - newCamPos.GetX(), hintPos.GetY() - newCamPos.GetY(),
                               newLookDelta.GetZ());
    }

    if ((mFlags & 0x2) != 0) {
      newLookDelta = lookAheadPos - hintPos;
    }

    if (newLookDelta.CanBeNormalized()) {
      SetTransform(CTransform4f::LookAt(newCamPos, newCamPos + newLookDelta.AsNormalized()));
    }
  }
}

void CScriptSpindleCamera::ProcessInput(const CFinalInput&, CStateManager&) {}

ENTITY_ACCEPT_IMPL(CScriptSpindleCamera)

void CScriptSpindleCamera::Render(const CStateManager&) const {}

void CScriptSpindleCamera::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                           CStateManager& mgr) {
  CGameCamera::AcceptScriptMsg(msg, uid, mgr);
}
