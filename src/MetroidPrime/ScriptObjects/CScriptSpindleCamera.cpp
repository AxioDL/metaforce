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
: x0_flags(flags)
, x4_input(input)
, x8_lowOut(lowOut)
, xc_highOut(highOut)
, x10_lowIn(lowIn)
, x14_highIn(highIn) {}

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
  x8_lowOut *= 0.017453292f;
  xc_highOut *= 0.017453292f;
}

float CSpindleCameraInterpolant::InterpolateValue(float inVar) const {
  float ret;
  if (x4_input == kSI_Constant) {
    ret = x8_lowOut;
  } else {
    const float reflectRange = x14_highIn - x10_lowIn;
    if (close_enough(reflectRange, 0.f)) {
      ret = x8_lowOut;
    } else {
      const float outRange = xc_highOut - x8_lowOut;
      float reflectedVar = inVar;
      if ((x0_flags & 0x1) != 0 && inVar > x14_highIn) {
        reflectedVar = x14_highIn - (inVar - x14_highIn);
      }
      if ((x0_flags & 0x2) != 0 && inVar < x10_lowIn) {
        reflectedVar = x10_lowIn + (x10_lowIn - inVar);
      }

      float value = x8_lowOut + (outRange * (reflectedVar - x10_lowIn)) / reflectRange;
      if (x8_lowOut < xc_highOut) {
        value = CMath::Clamp(x8_lowOut, value, xc_highOut);
      } else {
        value = CMath::Clamp(xc_highOut, value, x8_lowOut);
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
, x188_flags(flags)
, x18c_inVars()
, x1b0_hintToCamDistMin(hintToCamDistMin)
, x1b4_hintToCamDistMax(hintToCamDistMax)
, x1b8_hintToCamVOffMin(hintToCamVOffMin)
, x1bc_hintToCamVOffMax(hintToCamVOffMax)
, x1c0_targetHintToCamDeltaAngleVel(targetHintToCamDeltaAngleVel)
, x1d8_deltaAngleScaleWithCamDist(deltaAngleScaleWithCamDist)
, x1f0_hintToCamDist(hintToCamDist)
, x208_distOffsetFromBallDist(distOffsetFromBallDist)
, x220_hintBallToCamAzimuth(hintBallToCamAzimuth)
, x238_unused(unused)
, x250_maxHintBallToCamAzimuth(maxHintBallToCamAzimuth)
, x268_camLookRelAzimuth(camLookRelAzimuth)
, x280_lookPosZOffset(lookPosZOffset)
, x298_camPosZOffset(camPosZOffset)
, x2b0_clampedAzimuthFromHintDir(clampedAzimuthFromHintDir)
, x2c8_dampingAzimuthSpeed(dampingAzimuthSpeed)
, x2e0_targetHintToCamDeltaAngleVelRange(targetHintToCamDeltaAngleVelRange)
, x2f8_deleteHintBallDist(deleteHintBallDist)
, x310_recoverClampedAzimuthFromHintDir(recoverClampedAzimuthFromHintDir)
, x328_maxAzimuthInterpTimer(0.f)
, x32c_24_outsideClampedAzimuth(false)
, x330_lookDir(xf.GetForward()) {}

CScriptSpindleCamera::~CScriptSpindleCamera() {}

void CScriptSpindleCamera::Reset(const CTransform4f&, CStateManager& mgr) {
  const CScriptCameraHint* hint = mgr.GetCameraManager()->GetCameraHint(mgr);
  if (!GetActive() || hint == nullptr) {
    return;
  }

  x33c_24_inResetThink = true;
  mgr.GetCameraManager()->BallCamera()->UpdateLookAtPosition(0.01f, mgr);
  Think(0.01f, mgr);
  x33c_24_inResetThink = false;
}

float CScriptSpindleCamera::GetInterpolant(const CSpindleCameraInterpolant& seg) const {
  return x18c_inVars[static_cast< int >(seg.x4_input)];
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

  x18c_inVars.clear();
  x18c_inVars.push_back(0.f);
  x18c_inVars.push_back(hintToBallDist);
  x18c_inVars.push_back(CMath::AbsF(hintToBallVOff));

  float hintBallDot = CMath::Limit(CVector3f::Dot(hintToBallDir, hintDir), 1.f);
  const float hintBallAngle = CMath::AbsF(acos(hintBallDot));
  x18c_inVars.push_back(hintBallAngle);

  const float hintBallCross =
      hintToBallDir.GetX() * hintDir.GetY() - hintDir.GetX() * hintToBallDir.GetY();
  if (hintBallCross >= 0.f) {
    x18c_inVars.push_back(hintBallAngle);
    x18c_inVars.push_back(2.f * M_PIF - hintBallAngle);
  } else {
    x18c_inVars.push_back(2.f * M_PIF - hintBallAngle);
    x18c_inVars.push_back(hintBallAngle);
  }

  CVector3f hintDelta = hint->GetTranslation() - hint->GetOriginalTransform().GetTranslation();
  const float hintDeltaVOff = CMath::AbsF(hintDelta.GetZ());
  hintDelta[kDZ] = 0.f;
  float hintDeltaDist = 0.f;
  if (hintDelta.CanBeNormalized()) {
    hintDeltaDist = hintDelta.Magnitude();
  }
  x18c_inVars.push_back(hintDeltaDist);
  x18c_inVars.push_back(hintDeltaVOff);

  if ((x188_flags & 0x2000) != 0 &&
      hintToBallDist > x2f8_deleteHintBallDist.InterpolateValue(GetInterpolant(x2f8_deleteHintBallDist))) {
    if (hint->GetDelegatedCameraId() == GetUniqueId()) {
      mgr.CameraManager()->ReallyRemoveCameraHint(hint->GetUniqueId(), mgr);
    }
  } else {
    if ((x188_flags & 0x800) == 0) {
      hintToBallDir = hintDir;
    }

    if ((x188_flags & 0x20) != 0) {
      if (!x32c_24_outsideClampedAzimuth) {
        if (hintBallAngle >
            x2b0_clampedAzimuthFromHintDir.InterpolateValue(GetInterpolant(x2b0_clampedAzimuthFromHintDir))) {
          x330_lookDir = hintToBallDir;
          x32c_24_outsideClampedAzimuth = true;
        }
      } else {
        const float hintCamCross =
            hintToCamDir.GetX() * hintDir.GetY() - hintDir.GetX() * hintToCamDir.GetY();
        if ((hintBallAngle < x310_recoverClampedAzimuthFromHintDir.InterpolateValue(
                                 GetInterpolant(x310_recoverClampedAzimuthFromHintDir)) &&
             hintBallCross * hintCamCross < 0.f) ||
            hintBallAngle <=
                x2b0_clampedAzimuthFromHintDir.InterpolateValue(GetInterpolant(x2b0_clampedAzimuthFromHintDir))) {
          x32c_24_outsideClampedAzimuth = false;
        } else {
          hintToBallDir = x330_lookDir;
        }
      }
    }

    float newHintToCamDist = x1f0_hintToCamDist.InterpolateValue(GetInterpolant(x1f0_hintToCamDist));
    if ((x188_flags & 0x40) != 0) {
      newHintToCamDist = hintToBallDist + x208_distOffsetFromBallDist.InterpolateValue(
                                              GetInterpolant(x208_distOffsetFromBallDist));
    }
    newHintToCamDist = CMath::Clamp(x1b0_hintToCamDistMin, newHintToCamDist, x1b4_hintToCamDistMax);

    CVector3f newCamPos = GetTranslation();
    float hintToCamDist = hintToCamDir.Magnitude();
    if (hintToCamDir.CanBeNormalized()) {
      hintToCamDir.Normalize();
    } else {
      hintToCamDir = hintDir;
      hintToCamDist = x1f0_hintToCamDist.InterpolateValue(GetInterpolant(x1f0_hintToCamDist));
    }

    float hintBallToCamTargetAzimuth =
        x220_hintBallToCamAzimuth.InterpolateValue(GetInterpolant(x220_hintBallToCamAzimuth));
    if ((x188_flags & 0x4000) == 0 && CVector3f::Cross(hintToCamDir, hintToBallDir).GetZ() >= 0.f) {
      hintBallToCamTargetAzimuth = -hintBallToCamTargetAzimuth;
    }

    CQuaternion hintBallToCamTargetAzimuthQuat = CQuaternion::ZRotation(hintBallToCamTargetAzimuth);
    const CVector3f targetHintToCam = hintBallToCamTargetAzimuthQuat.Transform(hintToBallDir);

    CVector3f newHintToCamDir = hintToCamDir;

    float hintToCamDot = CMath::Limit(CVector3f::Dot(hintToCamDir, targetHintToCam), 1.f);
    const float hintToCamDeltaAngleRange = CMath::AbsF(acosf(hintToCamDot));

    float hintToCamDeltaAngleSpeedFactor =
        CMath::Limit(hintToCamDeltaAngleRange /
                         x2c8_dampingAzimuthSpeed.InterpolateValue(GetInterpolant(x2c8_dampingAzimuthSpeed)),
                     1.f);

    float targetHintToCamDeltaAngleVel =
        x1c0_targetHintToCamDeltaAngleVel.InterpolateValue(GetInterpolant(x1c0_targetHintToCamDeltaAngleVel));
    if ((x188_flags & 0x100) == 0) {
      targetHintToCamDeltaAngleVel = CMath::Limit(
          x1d8_deltaAngleScaleWithCamDist.InterpolateValue(GetInterpolant(x1d8_deltaAngleScaleWithCamDist)) /
              hintToCamDist,
          targetHintToCamDeltaAngleVel);
    }

    if ((CVector3f::Cross(hintToBallDir, hintToCamDir).GetZ() >= 0.f &&
         CVector3f::Cross(targetHintToCam, hintToCamDir).GetZ() < 0.f) ||
        (CVector3f::Cross(hintToBallDir, hintToCamDir).GetZ() < 0.f &&
         CVector3f::Cross(targetHintToCam, hintToCamDir).GetZ() >= 0.f)) {
      const float targetHintToCamDeltaAngleVelRange =
          x2e0_targetHintToCamDeltaAngleVelRange.InterpolateValue(
              GetInterpolant(x2e0_targetHintToCamDeltaAngleVelRange));
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
    if ((x188_flags & 0x10) != 0) {
      if (CMath::AbsF(hintBallToCamAzimuth) <
              x220_hintBallToCamAzimuth.InterpolateValue(GetInterpolant(x220_hintBallToCamAzimuth)) ||
          (x188_flags & 0x8) != 0 || x33c_24_inResetThink) {
        newHintToCamDir = targetHintToCam;
      }
    }

    const float maxHintBallToCamAzimuth =
        x250_maxHintBallToCamAzimuth.InterpolateValue(GetInterpolant(x250_maxHintBallToCamAzimuth));

    if (CMath::AbsF(hintBallToCamAzimuth) > maxHintBallToCamAzimuth) {
      x328_maxAzimuthInterpTimer += dt;
      if (x328_maxAzimuthInterpTimer < 3.f) {
        const float ballToCamAzimuthInterp = CMath::Limit(x328_maxAzimuthInterpTimer / 3.f, 1.f);
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
      x328_maxAzimuthInterpTimer = 0.f;
    }

    if ((x188_flags & 0x20) != 0) {
      CVector3f hintDir2 = hint->GetTransform().GetForward();
      hintDir2.SetZ(0.f);
      if (hintDir2.CanBeNormalized()) {
        hintDir2.Normalize();

        float hintCamDot = CMath::Limit(CVector3f::Dot(hintDir2, newHintToCamDir), 1.f);
        float hintCamAzimuth = CMath::Limit(
            CMath::AbsF(acosf(hintCamDot)),
            x2b0_clampedAzimuthFromHintDir.InterpolateValue(GetInterpolant(x2b0_clampedAzimuthFromHintDir)));

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
    if ((x188_flags & 0x80) != 0) {
      newCamPos[kDZ] = ballPos.GetZ() + x298_camPosZOffset.InterpolateValue(GetInterpolant(x298_camPosZOffset));
    } else {
      newCamPos[kDZ] = hintPos.GetZ() + x298_camPosZOffset.InterpolateValue(GetInterpolant(x298_camPosZOffset));
    }

    float hintToCamVOff = newCamPos.GetZ() - hintPos.GetZ();
    hintToCamVOff = CMath::Clamp(x1b8_hintToCamVOffMin, hintToCamVOff, x1bc_hintToCamVOffMax);
    newCamPos[kDZ] = hintToCamVOff + hintPos.GetZ();

    if ((x188_flags & 0x200) != 0) {
      lookAheadPos[kDZ] =
          ballPos.GetZ() + x280_lookPosZOffset.InterpolateValue(GetInterpolant(x280_lookPosZOffset));
    } else {
      lookAheadPos[kDZ] =
          hintPos.GetZ() + x280_lookPosZOffset.InterpolateValue(GetInterpolant(x280_lookPosZOffset));
    }

    CVector3f newLookDelta = lookAheadPos - newCamPos;
    CVector3f newLookDirFlat = newLookDelta;
    newLookDirFlat[kDZ] = 0.f;

    if (newLookDirFlat.CanBeNormalized()) {
      const float newLookDistFlat = newLookDirFlat.Magnitude();
      newLookDirFlat.Normalize();

      float camLookRelAzimuth = -x268_camLookRelAzimuth.InterpolateValue(GetInterpolant(x268_camLookRelAzimuth));
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

      if ((x188_flags & 0x1000) != 0) {
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

    if ((x188_flags & 0x1) != 0) {
      newLookDelta = CVector3f(hintPos.GetX() - newCamPos.GetX(), hintPos.GetY() - newCamPos.GetY(),
                               newLookDelta.GetZ());
    }

    if ((x188_flags & 0x2) != 0) {
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
