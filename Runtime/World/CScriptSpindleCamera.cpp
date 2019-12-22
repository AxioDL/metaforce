#include "Runtime/World/CScriptSpindleCamera.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Camera/CBallCamera.hpp"
#include "Runtime/Camera/CCameraManager.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptCameraHint.hpp"
#include "Runtime/World/ScriptLoader.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

SSpindleProperty::SSpindleProperty(CInputStream& in) {
  x4_input = ESpindleInput(in.readUint32Big());
  x0_flags = ScriptLoader::LoadParameterFlags(in);
  x8_lowOut = in.readFloatBig();
  xc_highOut = in.readFloatBig();
  x10_lowIn = in.readFloatBig();
  x14_highIn = in.readFloatBig();
  switch (x4_input) {
  case ESpindleInput::HintBallAngle:
  case ESpindleInput::HintBallRightAngle:
  case ESpindleInput::HintBallLeftAngle:
    x10_lowIn = zeus::degToRad(x10_lowIn);
    x14_highIn = zeus::degToRad(x14_highIn);
    break;
  default:
    break;
  }
}

CScriptSpindleCamera::CScriptSpindleCamera(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                           const zeus::CTransform& xf, bool active, u32 flags, float hintToCamDistMin,
                                           float hintToCamDistMax, float hintToCamVOffMin, float hintToCamVOffMax,
                                           const SSpindleProperty& targetHintToCamDeltaAngleVel,
                                           const SSpindleProperty& deltaAngleScaleWithCamDist,
                                           const SSpindleProperty& hintToCamDist,
                                           const SSpindleProperty& distOffsetFromBallDist,
                                           const SSpindleProperty& hintBallToCamAzimuth,
                                           const SSpindleProperty& unused,
                                           const SSpindleProperty& maxHintBallToCamAzimuth,
                                           const SSpindleProperty& camLookRelAzimuth,
                                           const SSpindleProperty& lookPosZOffset,
                                           const SSpindleProperty& camPosZOffset,
                                           const SSpindleProperty& clampedAzimuthFromHintDir,
                                           const SSpindleProperty& dampingAzimuthSpeed,
                                           const SSpindleProperty& targetHintToCamDeltaAngleVelRange,
                                           const SSpindleProperty& deleteHintBallDist,
                                           const SSpindleProperty& recoverClampedAzimuthFromHintDir)
: CGameCamera(uid, active, name, info, xf, CCameraManager::ThirdPersonFOV(), CCameraManager::NearPlane(),
              CCameraManager::FarPlane(), CCameraManager::Aspect(), kInvalidUniqueId, false, 0)
, x188_flags(flags)
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
, x330_lookDir(xf.basis[1]) {}

void CScriptSpindleCamera::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptSpindleCamera::ProcessInput(const CFinalInput& input, CStateManager& mgr) {
  // Empty
}

void CScriptSpindleCamera::Reset(const zeus::CTransform& xf, CStateManager& mgr) {
  const CScriptCameraHint* hint = mgr.GetCameraManager()->GetCameraHint(mgr);
  if (!GetActive() || hint == nullptr)
    return;

  x33c_24_inResetThink = true;
  mgr.GetCameraManager()->GetBallCamera()->UpdateLookAtPosition(0.01f, mgr);
  Think(0.01f, mgr);
  x33c_24_inResetThink = false;
}

void CScriptSpindleCamera::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  CGameCamera::AcceptScriptMsg(msg, sender, mgr);
}

float SSpindleProperty::GetValue(float inVar) const {
  if (x4_input == ESpindleInput::Constant)
    return x8_lowOut;

  float reflectRange = x14_highIn - x10_lowIn;
  if (zeus::close_enough(reflectRange, 0.f))
    return x8_lowOut;

  float reflectedVar = inVar;
  if (x0_flags & 0x1 && inVar > x14_highIn)
    reflectedVar = x14_highIn - (inVar - x14_highIn);
  if (x0_flags & 0x2 && inVar < x10_lowIn)
    reflectedVar = x10_lowIn + (x10_lowIn - inVar);

  float outRange = xc_highOut - x8_lowOut;
  float res = (reflectedVar - x10_lowIn) * outRange / reflectRange + x8_lowOut;
  if (x8_lowOut < xc_highOut)
    return zeus::clamp(x8_lowOut, res, xc_highOut);
  else
    return zeus::clamp(xc_highOut, res, x8_lowOut);
}

void CScriptSpindleCamera::Think(float dt, CStateManager& mgr) {
  const CScriptCameraHint* hint = mgr.GetCameraManager()->GetCameraHint(mgr);
  if (!GetActive() || hint == nullptr)
    return;

  zeus::CVector3f hintPos = hint->GetTranslation();
  zeus::CVector2f lookAheadPos = mgr.GetCameraManager()->GetBallCamera()->GetLookPosAhead().toVec2f();
  zeus::CVector3f hintToCamDir = GetTranslation() - hintPos;
  hintToCamDir.z() = 0.f;
  zeus::CVector3f ballPos = mgr.GetPlayer().GetBallPosition();
  zeus::CVector3f hintToBallDir = ballPos - hintPos;
  float hintToBallVOff = hintToBallDir.z();
  hintToBallDir.z() = 0.f;
  zeus::CVector3f hintDir = hint->GetTransform().basis[1];
  hintDir.z() = 0.f;
  if (hintDir.canBeNormalized())
    hintDir.normalize();
  else
    hintDir = zeus::skForward;
  float hintToBallDist = 0.f;
  if (hintToBallDir.canBeNormalized()) {
    hintToBallDist = hintToBallDir.magnitude();
    hintToBallDir.normalize();
  } else {
    hintToBallDir = hintDir;
  }

  x18c_inVars.clear();
  x18c_inVars.push_back(0.f); // Zero
  x18c_inVars.push_back(hintToBallDist); // HintToBallDist
  x18c_inVars.push_back(std::fabs(hintToBallVOff)); // HintToBallVOff
  float hintBallAngle = std::fabs(std::acos(zeus::clamp(-1.f, hintToBallDir.dot(hintDir), 1.f)));
  x18c_inVars.push_back(hintBallAngle); // HintBallAngle
  float hintBallCross = hintToBallDir.toVec2f().cross(hintDir.toVec2f());
  if (hintBallCross >= 0.f) {
    x18c_inVars.push_back(hintBallAngle); // HintBallRightAngle
    x18c_inVars.push_back(2.f * M_PIF - hintBallAngle); // HintBallLeftAngle
  } else {
    x18c_inVars.push_back(2.f * M_PIF - hintBallAngle); // HintBallRightAngle
    x18c_inVars.push_back(hintBallAngle); // HintBallLeftAngle
  }
  zeus::CVector3f hintDelta = hint->GetTranslation() - hint->GetOriginalTransform().origin;
  float hintDeltaVOff = std::fabs(hintDelta.z());
  hintDelta.z() = 0.f;
  x18c_inVars.push_back(hintDelta.canBeNormalized() ? hintDelta.magnitude() : 0.f); // HintDeltaDist
  x18c_inVars.push_back(hintDeltaVOff); // HintDeltaVOff

  if ((x188_flags & 0x2000) && hintToBallDist >
      x2f8_deleteHintBallDist.GetValue(GetInVar(x2f8_deleteHintBallDist))) {

    if (hint->GetDelegatedCamera() == GetUniqueId())
      mgr.GetCameraManager()->DeleteCameraHint(hint->GetUniqueId(), mgr);

  } else {

    if (!(x188_flags & 0x800))
      hintToBallDir = hintDir;
    if (x188_flags & 0x20) {
      if (!x32c_outsideClampedAzimuth) {
        if (hintBallAngle > x2b0_clampedAzimuthFromHintDir.GetValue(GetInVar(x2b0_clampedAzimuthFromHintDir))) {
          x330_lookDir = hintToBallDir;
          x32c_outsideClampedAzimuth = true;
        }
      } else {
        float hintCamCross = hintToCamDir.toVec2f().cross(hintDir.toVec2f());
        if ((hintBallAngle < x310_recoverClampedAzimuthFromHintDir.
            GetValue(GetInVar(x310_recoverClampedAzimuthFromHintDir)) && hintBallCross * hintCamCross < 0.f) ||
            hintBallAngle <= x2b0_clampedAzimuthFromHintDir.GetValue(GetInVar(x2b0_clampedAzimuthFromHintDir))) {
          x32c_outsideClampedAzimuth = false;
        } else {
          hintToBallDir = x330_lookDir;
        }
      }
    }

    float newHintToCamDist = x1f0_hintToCamDist.GetValue(GetInVar(x1f0_hintToCamDist));
    if (x188_flags & 0x40)
      newHintToCamDist = hintToBallDist +
        x208_distOffsetFromBallDist.GetValue(GetInVar(x208_distOffsetFromBallDist));
    newHintToCamDist = zeus::clamp(x1b0_hintToCamDistMin, newHintToCamDist, x1b4_hintToCamDistMax);
    zeus::CVector3f newCamPos = GetTranslation();
    float hintToCamDist = hintToCamDir.magnitude();
    if (hintToCamDir.canBeNormalized()) {
      hintToCamDir.normalize();
    } else {
      hintToCamDir = hintDir;
      hintToCamDist = x1f0_hintToCamDist.GetValue(GetInVar(x1f0_hintToCamDist));
    }

    float hintBallToCamTargetAzimuth =
      x220_hintBallToCamAzimuth.GetValue(GetInVar(x220_hintBallToCamAzimuth));
    if (!(x188_flags & 0x4000) && hintToCamDir.cross(hintToBallDir).z() >= 0.f)
      hintBallToCamTargetAzimuth = -hintBallToCamTargetAzimuth;
    zeus::CQuaternion hintBallToCamTargetAzimuthQuat;
    hintBallToCamTargetAzimuthQuat.rotateZ(hintBallToCamTargetAzimuth);
    zeus::CVector3f targetHintToCam = hintBallToCamTargetAzimuthQuat.transform(hintToBallDir);
    zeus::CVector3f newHintToCamDir = hintToCamDir;
    float hintToCamDeltaAngleRange = std::fabs(std::acos(zeus::clamp(-1.f, hintToCamDir.dot(targetHintToCam), 1.f)));
    float hintToCamDeltaAngleSpeedFactor =
      zeus::clamp(-1.f, hintToCamDeltaAngleRange /
      x2c8_dampingAzimuthSpeed.GetValue(GetInVar(x2c8_dampingAzimuthSpeed)), 1.f);
    float targetHintToCamDeltaAngleVel =
      x1c0_targetHintToCamDeltaAngleVel.GetValue(GetInVar(x1c0_targetHintToCamDeltaAngleVel));
    if (x188_flags & 0x100)
      targetHintToCamDeltaAngleVel = zeus::clamp(-targetHintToCamDeltaAngleVel,
        x1d8_deltaAngleScaleWithCamDist.GetValue(GetInVar(x1d8_deltaAngleScaleWithCamDist)) / hintToCamDist,
        targetHintToCamDeltaAngleVel);
    if ((hintToBallDir.cross(hintToCamDir).z() >= 0.f && targetHintToCam.cross(hintToCamDir).z() < 0.f) ||
        (hintToBallDir.cross(hintToCamDir).z() < 0.f && targetHintToCam.cross(hintToCamDir).z() >= 0.f)) {
      float targetHintToCamDeltaAngleVelRange =
        x2e0_targetHintToCamDeltaAngleVelRange.GetValue(GetInVar(x2e0_targetHintToCamDeltaAngleVelRange));
      targetHintToCamDeltaAngleVel = zeus::clamp(-targetHintToCamDeltaAngleVelRange,
        targetHintToCamDeltaAngleVel, targetHintToCamDeltaAngleVelRange);
    }

    zeus::CVector3f camToBall = ballPos - GetTranslation();
    camToBall.z() = 0.f;
    float targetHintToCamDeltaAngle = targetHintToCamDeltaAngleVel * dt * hintToCamDeltaAngleSpeedFactor;
    float camToBallDist = 0.f;
    if (camToBall.canBeNormalized())
      camToBallDist = camToBall.magnitude();
    targetHintToCamDeltaAngle *= (1.f - zeus::clamp(0.f, (camToBallDist - 2.f) * 0.5f, 1.f)) * 10.f + 1.f;
    targetHintToCamDeltaAngle = zeus::clamp(-hintToCamDeltaAngleRange,
      targetHintToCamDeltaAngle, hintToCamDeltaAngleRange);
    if (std::fabs(zeus::clamp(-1.f, hintToCamDir.dot(targetHintToCam), 1.f)) < 0.99999f)
      newHintToCamDir = zeus::CQuaternion::lookAt(hintToCamDir, targetHintToCam,
        targetHintToCamDeltaAngle).transform(hintToCamDir);
    float hintBallToCamAzimuth = std::acos(zeus::clamp(-1.f, hintToBallDir.dot(newHintToCamDir), 1.f));
    if (x188_flags & 0x10) {
      if (std::fabs(hintBallToCamAzimuth) <
          x220_hintBallToCamAzimuth.GetValue(GetInVar(x220_hintBallToCamAzimuth)) ||
          (x188_flags & 0x8) || x33c_24_inResetThink)
        newHintToCamDir = targetHintToCam;
    }

    float maxHintBallToCamAzimuth = x250_maxHintBallToCamAzimuth.GetValue(GetInVar(x250_maxHintBallToCamAzimuth));
    if (std::fabs(hintBallToCamAzimuth) > maxHintBallToCamAzimuth) {
      x328_maxAzimuthInterpTimer += dt;
      if (x328_maxAzimuthInterpTimer < 3.f) {
        float ballToCamAzimuthInterp = zeus::clamp(-1.f, x328_maxAzimuthInterpTimer / 3.f, 1.f);
        float hintBallToCamAzimuthDelta = std::fabs(maxHintBallToCamAzimuth - hintBallToCamAzimuth);
        if (hintToBallDir.cross(newHintToCamDir).z() > 0.f)
          hintBallToCamAzimuthDelta = -hintBallToCamAzimuthDelta;
        zeus::CQuaternion hintBallToCamAzimuthQuat;
        hintBallToCamAzimuthQuat.rotateZ(hintBallToCamAzimuthDelta * ballToCamAzimuthInterp);
        newHintToCamDir = hintBallToCamAzimuthQuat.transform(newHintToCamDir);
      } else {
        zeus::CQuaternion hintBallToCamAzimuthQuat;
        if (hintBallToCamTargetAzimuth > 0.f)
          hintBallToCamAzimuthQuat.rotateZ(maxHintBallToCamAzimuth);
        else
          hintBallToCamAzimuthQuat.rotateZ(-maxHintBallToCamAzimuth);
        newHintToCamDir = hintBallToCamAzimuthQuat.transform(hintToBallDir);
      }
    } else {
      x328_maxAzimuthInterpTimer = 0.f;
    }

    if (x188_flags & 0x20) {
      zeus::CVector3f hintDir2 = hint->GetTransform().basis[1];
      hintDir2.z() = 0.f;
      if (hintDir2.canBeNormalized()) {
        hintDir2.normalize();
        float hintCamAzimuth = std::fabs(std::acos(zeus::clamp(-1.f, hintDir2.dot(newHintToCamDir), 1.f)));
        float hintCamAzimuthRange =
          x2b0_clampedAzimuthFromHintDir.GetValue(GetInVar(x2b0_clampedAzimuthFromHintDir));
        hintCamAzimuth = zeus::clamp(-hintCamAzimuthRange, hintCamAzimuth, hintCamAzimuthRange);
        if (hintDir2.cross(newHintToCamDir).z() < 0.f)
          hintCamAzimuth = -hintCamAzimuth;
        zeus::CQuaternion hintCamAzimuthQuat;
        hintCamAzimuthQuat.rotateZ(hintCamAzimuth);
        newHintToCamDir = hintCamAzimuthQuat.transform(hintDir2);
      }
    }

    newCamPos = hintPos + newHintToCamDir * newHintToCamDist;
    if (x188_flags & 0x80)
      newCamPos.z() = ballPos.z() + x298_camPosZOffset.GetValue(GetInVar(x298_camPosZOffset));
    else
      newCamPos.z() = hintPos.z() + x298_camPosZOffset.GetValue(GetInVar(x298_camPosZOffset));
    newCamPos.z() = zeus::clamp(x1b8_hintToCamVOffMin, newCamPos.z() - hintPos.z(), x1bc_hintToCamVOffMax) +
      hintPos.z();

    float lookPosZ;
    if (x188_flags & 0x200)
      lookPosZ = ballPos.z() + x280_lookPosZOffset.GetValue(GetInVar(x280_lookPosZOffset));
    else
      lookPosZ = hintPos.z() + x280_lookPosZOffset.GetValue(GetInVar(x280_lookPosZOffset));

    zeus::CVector3f newLookDelta(lookAheadPos - newCamPos.toVec2f(), lookPosZ - newCamPos.z());
    zeus::CVector3f newLookDirFlat = newLookDelta;
    newLookDirFlat.z() = 0.f;
    if (newLookDirFlat.canBeNormalized()) {
      float newLookDistFlat = newLookDirFlat.magnitude();
      newLookDirFlat.normalize();
      float camLookRelAzimuth = -x268_camLookRelAzimuth.GetValue(GetInVar(x268_camLookRelAzimuth));
      zeus::CVector3f newHintToCamDirFlat = newCamPos - hintPos;
      newHintToCamDirFlat.z() = 0.f;
      if (newHintToCamDirFlat.canBeNormalized())
        newHintToCamDirFlat.normalize();
      else
        newHintToCamDirFlat = zeus::skForward;
      if (newHintToCamDirFlat.cross(hintToBallDir).z() >= 0.f)
        camLookRelAzimuth = -camLookRelAzimuth;
      if (x188_flags & 0x1000)
        camLookRelAzimuth *= zeus::clamp(-1.f, std::acos(std::fabs(
          zeus::clamp(-1.f, hintToBallDir.dot(newHintToCamDirFlat), 1.f))) / zeus::degToRad(10.f), 1.f);
      zeus::CQuaternion azimuthQuat;
      azimuthQuat.rotateZ(camLookRelAzimuth);
      lookAheadPos = azimuthQuat.transform(newLookDirFlat).toVec2f() * std::cos(camLookRelAzimuth) * newLookDistFlat +
        newCamPos.toVec2f();
    }
    newLookDelta = zeus::CVector3f(lookAheadPos, lookPosZ) - newCamPos;
    if (x188_flags & 0x1)
      newLookDelta = zeus::CVector3f(hintPos.toVec2f() - newCamPos.toVec2f(), newLookDelta.z());
    if (x188_flags & 0x2)
      newLookDelta = lookAheadPos - hintPos.toVec2f();
    if (newLookDelta.canBeNormalized())
      SetTransform(zeus::lookAt(newCamPos, newCamPos + newLookDelta.normalized()));
  }
}

void CScriptSpindleCamera::Render(const CStateManager&) const {
  // Empty
}

} // namespace urde
