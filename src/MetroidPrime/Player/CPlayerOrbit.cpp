#include "MetroidPrime/Player/CPlayer.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/Player/CGrappleArm.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "math.h"

#include "MetroidPrime/Cameras/CCameraManager.hpp"

#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"

#include "MetroidPrime/CAxisAngle.hpp"

#include "MetroidPrime/ScriptObjects/CScriptGrapplePoint.hpp"

class CThardusRockProjectile;

#if VERSION >= VERSION_R3IJ_00

#include "MetroidPrime/CControlMapper.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "Collision/CCollidableAABox.hpp"

void CPlayer::UpdateOrbitModeTimer(float dt) {
  if (mOrbitState == kOS_NoOrbit) {
    if (mOrbitModeTimer > 0.f) {
      mOrbitModeTimer -= dt;
    } else {
      mOrbitModeTimer = 0.f;
    }
  } else {
    mOrbitModeTimer += dt;
    if (mOrbitModeTimer > gpTweakPlayer->GetOrbitModeTimer()) {
      mOrbitModeTimer = gpTweakPlayer->GetOrbitModeTimer();
    }
  }
  const float& blend = mOrbitModeTimer / gpTweakPlayer->GetOrbitModeTimer();
  const float& zero = 0.f;
  const float& one = 1.f;
  mOrbitModeBlend = CMath::FastClamp(zero, blend, one);
}

bool CPlayer::CheckPostGrapple() const {
  if (mMovementState != NPlayer::kMS_OnGround && mGrappleJumpTimeout > 0.f) {
    return true;
  }
  return false;
}

#else

#include "MetroidPrime/CControlMapper.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CPlayerCameraBob.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "Collision/CCollidableAABox.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CWallCrawlerSwarm.hpp"
#include "MetroidPrime/Enemies/CThardusRockProjectile.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGunTurret.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDoor.hpp"
#endif

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CRayCastResult.hpp"

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"

#if defined(TARGET_PC)
#include "Metaforce/Display.hpp"
#endif

static const CMaterialList kLineOfSightIncludeList = CMaterialList(kMT_Solid);
static const CMaterialList kLineOfSightExcludeList =
    CMaterialList(kMT_ProjectilePassthrough, kMT_ScanPassthrough, kMT_Player);
static const CMaterialFilter kLineOfSightFilter =
    CMaterialFilter::MakeIncludeExclude(kLineOfSightIncludeList, kLineOfSightExcludeList);
static const CMaterialList kOccluderIncludeList = CMaterialList(kMT_Solid, kMT_Occluder);
static const CMaterialList kOccluderExcludeList =
    CMaterialList(kMT_ProjectilePassthrough, kMT_ScanPassthrough, kMT_Player);
static const CMaterialFilter kOccluderFilter =
    CMaterialFilter::MakeIncludeExclude(kOccluderIncludeList, kOccluderExcludeList);
#if VERSION >= VERSION_R3IJ_00
static const CMaterialList kCharacterLineOfSightExcludeList =
    CMaterialList(kMT_ProjectilePassthrough, kMT_ScanPassthrough, kMT_Character, kMT_Player);
static const CMaterialFilter kCharacterLineOfSightFilter =
    CMaterialFilter::MakeIncludeExclude(kLineOfSightIncludeList, kCharacterLineOfSightExcludeList);
#endif
static CAABox staticBox(CVector3f(0.f, 0.f, 0.f), CVector3f(1.f, 1.f, 1.f));

static CAABox BuildNearListBox(bool cropBottom, const CTransform4f& xf, float x, float z, float y) {
  const CAABox bounds(-x, cropBottom ? 0.f : -y, -z, x, y, z);
  return bounds.GetTransformedAABox(xf);
}

bool CPlayer::ValidateOrbitTargetIdAndPointer(const TUniqueId id, CStateManager& mgr) const {
  if (id == kInvalidUniqueId) {
    return false;
  }
  return TCastToConstPtr< CActor >(mgr.GetObjectById(id)) != nullptr;
}

#if VERSION < VERSION_R3IJ_00

CPlayer::EOrbitValidationResult CPlayer::ValidateCurrentOrbitTargetId(CStateManager& mgr) {
  const CActor* const act = TCastToConstPtr< CActor >(mgr.GetObjectById(GetOrbitTargetId()));
  if (!act || !act->GetTargetable() || !act->GetActive()) {
    return kOVR_InvalidTarget;
  }
  if (!act->GetMaterialList().HasMaterial(kMT_Orbit)) {
    if (!act->GetMaterialList().HasMaterial(kMT_Scannable)) {
      return kOVR_NonTargetableTarget;
    }
    if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::kPV_Scan) {
      return kOVR_NonTargetableTarget;
    }
  }
  const EOrbitValidationResult result = ValidateOrbitTargetId(GetOrbitTargetId(), mgr);
  if (result != kOVR_OK) {
    return result;
  }
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan &&
      act->GetCurrentAreaId() != GetCurrentAreaId()) {
    return kOVR_TargetingThroughDoor;
  }
  const CScriptGrapplePoint* const point =
      TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(GetOrbitTargetId()));
  if ((mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan &&
       gpTweakPlayer->GetOrbitWhileScanning()) ||
      point || act->GetCurrentAreaId() != GetCurrentAreaId()) {
    const CVector3f eyePosition = GetEyePosition();
    TEntityList nearList;
    TUniqueId bestId = kInvalidUniqueId;
    const CVector3f eyeToOrbit = act->GetOrbitPosition(mgr) - eyePosition;
    if (eyeToOrbit.CanBeNormalized()) {
      mgr.BuildNearList(nearList, eyePosition, eyeToOrbit.AsNormalized(), eyeToOrbit.Magnitude(),
                        kOccluderFilter, act);
      for (AUTO(it, nearList.begin()); it != nearList.end();) {
        if (const CEntity* ent = mgr.GetObjectById(*it)) {
          if (ent->GetCurrentAreaId() != mgr.GetNextAreaId()) {
            const CGameArea& area = mgr.GetWorld()->GetAreaAlways(ent->GetCurrentAreaId());
            if (area.GetOcclusionState() == CGameArea::kOS_Occluded) {
              it = nearList.erase(it);
              continue;
            }
          }
        }
        ++it;
      }
      const CRayCastResult rayResult =
          mgr.RayWorldIntersection(bestId, eyePosition, eyeToOrbit.AsNormalized(),
                                   eyeToOrbit.Magnitude(), kLineOfSightFilter, nearList);
      if (rayResult.IsValid()) {
        if (TCastToConstPtr< CScriptDoor >(mgr.GetObjectById(bestId)) || point) {
          return kOVR_TargetingThroughDoor;
        }
      }
    }
  }
  const CVector3f orbitPosition = act->GetOrbitPosition(mgr);
  CVector3f eyeToOrbitFlat = orbitPosition - GetEyePosition();
  eyeToOrbitFlat.SetZ(0.f);
  if (eyeToOrbitFlat.CanBeNormalized()) {
    const float angle = acosf(CMath::Limit(
        CVector3f::Dot(eyeToOrbitFlat.AsNormalized(), GetTransform().GetForward()), 1.f));
    if (mOrbitLockEstablished) {
      if (angle >= gpTweakPlayer->GetOrbitHorizAngle()) {
        return kOVR_BrokenLookAngle;
      }
    } else {
      if (angle <= M_PIF / 180.f) {
        mOrbitLockEstablished = true;
      }
    }
  } else {
    return kOVR_BrokenLookAngle;
  }
  return kOVR_OK;
}

#endif

CPlayer::EOrbitValidationResult CPlayer::ValidateOrbitTargetId(const TUniqueId id,
                                                               CStateManager& mgr) const {
  if (id == kInvalidUniqueId) {
    return kOVR_InvalidTarget;
  }
  const CActor* const act = TCastToConstPtr< CActor >(mgr.GetObjectById(id));
  if (!act || !act->GetTargetable() || !act->GetActive()) {
    return kOVR_InvalidTarget;
  }
  if (GetStaticTimer()) {
    return kOVR_PlayerNotReadyToTarget;
  }
  const CVector3f orbitPosition = act->GetOrbitPosition(mgr);
  const CVector3f eyePosition = GetEyePosition();
  const CVector3f eyeToOrbit = orbitPosition - eyePosition;
  CVector3f eyeToOrbitFlat = eyeToOrbit;
  eyeToOrbitFlat.SetZ(0.f);
  if (eyeToOrbitFlat.CanBeNormalized() && eyeToOrbitFlat.Magnitude() > 1.f) {
#if VERSION >= VERSION_R3IJ_00
    float maxSin = 1.f;
    const float angle = static_cast< float >(
        asin(CMath::FastLimit(CMath::AbsF(eyeToOrbit.GetZ()) / eyeToOrbit.Magnitude(), maxSin)));
#else
    const float angle = static_cast< float >(
        asin(CMath::Limit(CMath::AbsF(eyeToOrbit.GetZ()) / eyeToOrbit.Magnitude(), 1.f)));
#endif
    if ((eyeToOrbit.GetZ() >= 0.f && angle >= gpTweakPlayer->GetOrbitUpperAngle()) ||
        (eyeToOrbit.GetZ() < 0.f && angle >= gpTweakPlayer->GetOrbitLowerAngle())) {
      return kOVR_ExtremeHorizonAngle;
    }
  } else {
    return kOVR_ExtremeHorizonAngle;
  }
  const uchar flags = act->GetTargetableVisorFlags();
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Combat && (flags & 1) == 0) {
    return kOVR_PlayerNotReadyToTarget;
  }
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan && (flags & 2) == 0) {
    return kOVR_PlayerNotReadyToTarget;
  }
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Thermal && (flags & 4) == 0) {
    return kOVR_PlayerNotReadyToTarget;
  }
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_XRay && (flags & 8) == 0) {
    return kOVR_PlayerNotReadyToTarget;
  }
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan &&
      act->GetCurrentAreaId() != GetCurrentAreaId()) {
    return kOVR_TargetingThroughDoor;
  }
  return kOVR_OK;
}

float CPlayer::GetOrbitMaxTargetDistance(const CStateManager& mgr) const {
  float distance = gpTweakPlayer->GetOrbitMaxTargetDistance();
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan) {
    distance = gpTweakPlayer->GetScanMaxTargetDistance();
  }
  return distance;
}

float CPlayer::GetOrbitMaxLockDistance(const CStateManager& mgr) const {
  float distance = gpTweakPlayer->GetOrbitMaxLockDistance();
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan) {
    distance = gpTweakPlayer->GetScanMaxLockDistance();
  }
  return distance;
}

#if VERSION < VERSION_R3IJ_00

void CPlayer::UpdateOrbitTarget(CStateManager& mgr) {
  if (!ValidateOrbitTargetIdAndPointer(GetOrbitTargetId(), mgr)) {
    SetOrbitTargetId(kInvalidUniqueId, mgr);
  }
  if (!ValidateOrbitTargetIdAndPointer(GetOrbitNextTargetId(), mgr)) {
    SetOrbitNextTargetId(kInvalidUniqueId);
  }
  CVector3f playerToPoint = mOrbitPoint - GetTranslation();
  playerToPoint.SetZ(0.f);
  const float distance = playerToPoint.Magnitude();
  switch (mOrbitState) {
  case kOS_OrbitObject: {
    const CActor* const act = static_cast< const CActor* >(mgr.GetObjectById(GetOrbitTargetId()));
    if (act && act->GetDoTargetDistanceTest() &&
        (distance >= GetOrbitMaxLockDistance(mgr) || distance < .5f)) {
      if (distance < .5f) {
        BreakOrbit(kOB_BadVerticalAngle, mgr);
      } else {
        ActivateOrbitSource(mgr);
      }
    } else {
      UpdateOrbitPosition(gpTweakPlayer->GetOrbitNormalDistance(mOrbitType), mgr);
    }
    break;
  }
  case kOS_OrbitPoint: {
    if (gpTweakPlayer->GetOrbitFixedOffset() &&
        CMath::AbsF(mOrbitVector.GetZ()) > gpTweakPlayer->GetOrbitFixedOffsetZDiff()) {
      UpdateOrbitFixedPosition();
      return;
    }
    if (distance < CalculateOrbitZBasedDistance(mOrbitType)) {
      UpdateOrbitPosition(CalculateOrbitZBasedDistance(mOrbitType), mgr);
    }
    const float maxDistance = gpTweakPlayer->GetOrbitMaxDistance(mOrbitType);
    if (distance > maxDistance) {
      UpdateOrbitPosition(maxDistance, mgr);
    }
    if (mLookButtonHeld) {
      SetOrbitPosition(gpTweakPlayer->GetOrbitNormalDistance(mOrbitType), mgr);
    }
    const CVector3f eyeToPoint = mOrbitPoint - GetEyePosition();
    const float angle = static_cast< float >(
        asin(CMath::Limit(CMath::AbsF(eyeToPoint.GetZ()) / eyeToPoint.Magnitude(), 1.f)));
    if ((eyeToPoint.GetZ() >= 0.f && angle >= gpTweakPlayer->GetOrbitUpperAngle()) ||
        (eyeToPoint.GetZ() < 0.f && angle >= gpTweakPlayer->GetOrbitLowerAngle())) {
      BreakOrbit(kOB_BadVerticalAngle, mgr);
    }
    break;
  }
  case kOS_OrbitCarcass: {
    if (mLookButtonHeld) {
      SetOrbitPosition(x340_, mgr);
    }
    if (distance < CalculateOrbitZBasedDistance(mOrbitType)) {
      UpdateOrbitPosition(CalculateOrbitZBasedDistance(mOrbitType), mgr);
      x340_ = CalculateOrbitZBasedDistance(mOrbitType);
    }
    const float maxDistance = gpTweakPlayer->GetOrbitMaxDistance(mOrbitType);
    if (distance > maxDistance) {
      UpdateOrbitPosition(maxDistance, mgr);
      x340_ = gpTweakPlayer->GetOrbitMaxDistance(mOrbitType);
    }
    break;
  }
  case kOS_NoOrbit:
    SetOrbitTargetId(kInvalidUniqueId, mgr);
    break;
  case kOS_ForcedOrbitObject:
  case kOS_Grapple:
  default:
    break;
  }
  UpdateOrbitZPosition();
}

#endif

#if VERSION >= VERSION_R3IJ_00
void CPlayer::UpdateOrbitOrientation(CStateManager& mgr, float dt)
#else
void CPlayer::UpdateOrbitOrientation(CStateManager& mgr)
#endif
{
  if (mMorphBallState != kMS_Unmorphed) {
    return;
  }
  switch (mOrbitState) {
  case kOS_NoOrbit:
#if VERSION >= VERSION_R3IJ_00
    if (mTurnToCursor) {
      CVector3f playerToCursor = mAimingCursor.GetCursorOrbitPosition(mgr) - GetTranslation();
      playerToCursor.SetZ(0.f);
      if (playerToCursor.IsMagnitudeSafe()) {
        playerToCursor.Normalize();
        float maxBlend = 1.f;
        float maxCount = 60.f;
        const float blend =
            CMath::FastClamp(0.f, mAimingCursor.GetCursorObjectCount() / maxCount, maxBlend);
        const float scale = CMath::EaseInOut(blend, CMath::kET_Sinusoidal, 0.25f, 0.75f,
                                           0.f, 1.f, 2.f);
        const CRelAngle maxAngle = CRelAngle::FromRadians(scale * (CMath::Deg2Rad(60.f) * dt));
        const CQuaternion rotation = CQuaternion::ShortestRotationArcClamped(
            GetTransform().GetForward(), playerToCursor, maxAngle);
        CTransform4f xf = rotation.BuildTransform4f() * GetTransform();
        xf.SetTranslation(GetTranslation());
        SetTransform(xf);
      }
    }
#endif
    return;
  case kOS_OrbitPoint:
#if VERSION < VERSION_R3IJ_00
    if (mInFreeLook) {
      return;
    }
#endif
  case kOS_OrbitObject:
  case kOS_OrbitCarcass:
  case kOS_ForcedOrbitObject: {
    CVector3f playerToPoint = mOrbitPoint - GetTranslation();
    if (!mOrbitLockEstablished) {
      playerToPoint = mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform().GetForward();
    }
    playerToPoint.SetZ(0.f);
    if (playerToPoint.CanBeNormalized()) {
      CTransform4f xf = CTransform4f::LookAt(CVector3f::Zero(), playerToPoint);
      xf.SetTranslation(GetTranslation());
      SetTransform(xf);
    }
    break;
  }
  case kOS_Grapple:
    return;
  default:
    break;
  }
}

void CPlayer::UpdateOrbitSelection(const CFinalInput& input, CStateManager& mgr) {
  mOrbitNextTargetId = FindOrbitTargetId(mgr);
  const CScriptGrapplePoint* const curPoint =
      TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(mOrbitTargetId));
  const CScriptGrapplePoint* const nextPoint =
      TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(mOrbitNextTargetId));
  if (curPoint || (mOrbitState == kOS_Grapple && !nextPoint)) {
    mOrbitNextTargetId = kInvalidUniqueId;
    return;
  }
#if VERSION >= VERSION_R3IJ_00
  if (ControlMapper().GetPressInput(CControlMapper::kC_OrbitObject, input,
                                    CControlMapper::kFT_Filtered) &&
      mOrbitNextTargetId != kInvalidUniqueId) {
#else
  if (ControlMapper::GetPressInput(ControlMapper::kC_OrbitObject, input) &&
      mOrbitNextTargetId != kInvalidUniqueId) {
#endif
    SetOrbitTargetId(mOrbitNextTargetId, mgr);
    if (ValidateAimTargetId(GetOrbitTargetId(), mgr)) {
      SetAimTargetId(GetOrbitTargetId());
    }
    SetOrbitState(kOS_OrbitObject, mgr);
    UpdateOrbitPosition(gpTweakPlayer->GetOrbitNormalDistance(mOrbitType), mgr);
  }
}

void CPlayer::ActivateOrbitSource(CStateManager& mgr) {
#if VERSION >= VERSION_R3IJ_00
  BreakOrbit(kOB_InvalidateTarget, mgr);
#else
  switch (mOrbitSource) {
  case 0:
  default:
    OrbitCarcass(mgr);
    break;
  case 1:
    BreakOrbit(kOB_InvalidateTarget, mgr);
    break;
  case 2:
    if (mOrbitingEnemy) {
      OrbitPoint(kOT_Far, mgr);
    } else {
      OrbitCarcass(mgr);
    }
    break;
  }
#endif
}

void CPlayer::UpdateOrbitInput(const CFinalInput& input, CStateManager& mgr) {
  if (mMorphBallState != kMS_Unmorphed) {
    return;
  }
  if (mOrbitPreventionTimer > 0.f) {
    return;
  }
  UpdateOrbitableObjects(mgr);
  if (mOrbitState == kOS_NoOrbit) {
    SetOrbitNextTargetId(FindOrbitTargetId(mgr));
  }
#if VERSION >= VERSION_R3IJ_00
  if (ControlMapper().GetDigitalInput(CControlMapper::kC_OrbitClose, input,
                                      CControlMapper::kFT_Filtered) ||
      ControlMapper().GetDigitalInput(CControlMapper::kC_OrbitFar, input,
                                      CControlMapper::kFT_Filtered) ||
      ControlMapper().GetDigitalInput(CControlMapper::kC_OrbitObject, input,
                                      CControlMapper::kFT_Filtered)) {
#else
  if (ControlMapper::GetDigitalInput(ControlMapper::kC_OrbitClose, input) ||
      ControlMapper::GetDigitalInput(ControlMapper::kC_OrbitFar, input) ||
      ControlMapper::GetDigitalInput(ControlMapper::kC_OrbitObject, input)) {
#endif
    switch (mOrbitState) {
    case kOS_NoOrbit:
#if VERSION >= VERSION_R3IJ_00
      if (ControlMapper().GetPressInput(CControlMapper::kC_OrbitObject, input,
                                        CControlMapper::kFT_Filtered)) {
#else
      if (ControlMapper::GetPressInput(ControlMapper::kC_OrbitObject, input)) {
#endif
        SetOrbitTargetId(GetOrbitNextTargetId(), mgr);
        if (mOrbitTargetId != kInvalidUniqueId) {
          if (ValidateAimTargetId(GetOrbitTargetId(), mgr)) {
            SetAimTargetId(GetOrbitTargetId());
          }
          SetOrbitState(kOS_OrbitObject, mgr);
          UpdateOrbitPosition(gpTweakPlayer->GetOrbitNormalDistance(mOrbitType), mgr);
        }
      } else {
#if VERSION >= VERSION_R3IJ_00
        if (ControlMapper().GetPressInput(CControlMapper::kC_OrbitFar, input,
                                          CControlMapper::kFT_Filtered)) {
#else
        if (ControlMapper::GetPressInput(ControlMapper::kC_OrbitFar, input)) {
#endif
          OrbitPoint(kOT_Far, mgr);
        }
#if VERSION >= VERSION_R3IJ_00
        if (ControlMapper().GetPressInput(CControlMapper::kC_OrbitClose, input,
                                          CControlMapper::kFT_Filtered)) {
#else
        if (ControlMapper::GetPressInput(ControlMapper::kC_OrbitClose, input)) {
#endif
          OrbitPoint(kOT_Close, mgr);
        }
      }
      break;
    case kOS_Grapple:
      if (mOrbitTargetId == kInvalidUniqueId) {
        BreakGrapple(kOB_StopOrbit, mgr);
      }
      break;
    case kOS_OrbitObject:
      if (TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(GetOrbitTargetId()))) {
        if (ValidateCurrentOrbitTargetId(mgr) == kOVR_OK) {
          UpdateOrbitPosition(gpTweakPlayer->GetOrbitNormalDistance(mOrbitType), mgr);
        } else {
          BreakGrapple(kOB_InvalidateTarget, mgr);
        }
      } else {
        const EOrbitValidationResult result = ValidateCurrentOrbitTargetId(mgr);
        if (result == kOVR_OK) {
          UpdateOrbitPosition(gpTweakPlayer->GetOrbitNormalDistance(mOrbitType), mgr);
        } else if (result == kOVR_BrokenLookAngle) {
          OrbitPoint(kOT_Far, mgr);
        } else if (result == kOVR_ExtremeHorizonAngle) {
          BreakOrbit(kOB_BadVerticalAngle, mgr);
        } else {
          ActivateOrbitSource(mgr);
        }
      }
      UpdateOrbitSelection(input, mgr);
      break;
    case kOS_OrbitPoint:
#if VERSION >= VERSION_R3IJ_00
      if (ControlMapper().GetPressInput(CControlMapper::kC_OrbitObject, input,
                                        CControlMapper::kFT_Filtered)) {
#else
      if (ControlMapper::GetPressInput(ControlMapper::kC_OrbitObject, input)) {
#endif
        SetOrbitTargetId(FindOrbitTargetId(mgr), mgr);
        if (mOrbitTargetId != kInvalidUniqueId) {
          if (ValidateAimTargetId(GetOrbitTargetId(), mgr)) {
            SetAimTargetId(GetOrbitTargetId());
          }
          SetOrbitState(kOS_OrbitObject, mgr);
          UpdateOrbitPosition(gpTweakPlayer->GetOrbitNormalDistance(mOrbitType), mgr);
        }
      } else {
        switch (mOrbitType) {
        case kOT_Default:
          break;
        case kOT_Far:
#if VERSION >= VERSION_R3IJ_00
          if (ControlMapper().GetDigitalInput(CControlMapper::kC_OrbitClose, input,
                                              CControlMapper::kFT_Filtered)) {
#else
          if (ControlMapper::GetDigitalInput(ControlMapper::kC_OrbitClose, input)) {
#endif
            mOrbitType = kOT_Close;
            SetOrbitPosition(gpTweakPlayer->GetOrbitNormalDistance(mOrbitType), mgr);
          }
          break;
        case kOT_Close:
#if VERSION >= VERSION_R3IJ_00
          if (ControlMapper().GetDigitalInput(CControlMapper::kC_OrbitFar, input,
                                              CControlMapper::kFT_Filtered) &&
              !ControlMapper().GetDigitalInput(CControlMapper::kC_OrbitClose, input,
                                               CControlMapper::kFT_Filtered)) {
#else
          if (ControlMapper::GetDigitalInput(ControlMapper::kC_OrbitFar, input) &&
              !ControlMapper::GetDigitalInput(ControlMapper::kC_OrbitClose, input)) {
#endif
            mOrbitType = kOT_Far;
            SetOrbitPosition(gpTweakPlayer->GetOrbitNormalDistance(mOrbitType), mgr);
          }
          break;
        default:
          break;
        }
      }
      UpdateOrbitPosition(gpTweakPlayer->GetOrbitNormalDistance(mOrbitType), mgr);
      break;
    case kOS_OrbitCarcass:
#if VERSION >= VERSION_R3IJ_00
      if (ControlMapper().GetPressInput(CControlMapper::kC_OrbitObject, input,
                                        CControlMapper::kFT_Filtered)) {
#else
      if (ControlMapper::GetPressInput(ControlMapper::kC_OrbitObject, input)) {
#endif
        SetOrbitTargetId(FindOrbitTargetId(mgr), mgr);
        if (mOrbitTargetId != kInvalidUniqueId) {
          if (ValidateAimTargetId(GetOrbitTargetId(), mgr)) {
            SetAimTargetId(GetOrbitTargetId());
          }
          SetOrbitState(kOS_OrbitObject, mgr);
          UpdateOrbitPosition(gpTweakPlayer->GetOrbitNormalDistance(mOrbitType), mgr);
        }
      }
      UpdateOrbitSelection(input, mgr);
      break;
    case kOS_ForcedOrbitObject:
      UpdateOrbitPosition(gpTweakPlayer->GetOrbitNormalDistance(mOrbitType), mgr);
      UpdateOrbitSelection(input, mgr);
      break;
    }
    if (mOrbitState == kOS_Grapple) {
      mOrbitNextTargetId = FindOrbitTargetId(mgr);
      if (mOrbitNextTargetId == mOrbitTargetId) {
        mOrbitNextTargetId = kInvalidUniqueId;
      }
    }
  } else {
    switch (mOrbitState) {
    case kOS_NoOrbit:
      break;
    case kOS_OrbitObject:
      if (TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(GetOrbitTargetId()))) {
        BreakGrapple(kOB_Default, mgr);
      } else {
        BreakOrbit(kOB_StopOrbit, mgr);
      }
      break;
    case kOS_Grapple:
      if (!gpTweakPlayer->GetOrbitReleaseBreaksGrapple()) {
        mOrbitNextTargetId = FindOrbitTargetId(mgr);
        if (mOrbitNextTargetId == mOrbitTargetId) {
          mOrbitNextTargetId = kInvalidUniqueId;
        }
      } else {
        BreakGrapple(kOB_StopOrbit, mgr);
      }
      break;
    case kOS_ForcedOrbitObject:
      UpdateOrbitPosition(gpTweakPlayer->GetOrbitNormalDistance(mOrbitType), mgr);
      UpdateOrbitSelection(input, mgr);
      break;
    default:
      BreakOrbit(kOB_StopOrbit, mgr);
      break;
    }
  }
}

void CPlayer::UpdateOrbitZone(CStateManager& mgr) {
  if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::kPV_Scan) {
    mOrbitZoneType = kZT_Ellipse;
    x338_ = 1;
    mOrbitZoneMode = kZI_Targeting;
  } else {
    mOrbitZoneType = kZT_Box;
    x338_ = 2;
    mOrbitZoneMode = kZI_Scan;
  }
}

#if VERSION < VERSION_R3IJ_00

void CPlayer::UpdateOrbitModeTimer(float dt) {
  if (mOrbitState == kOS_NoOrbit && mOrbitModeTimer > 0.f) {
    mOrbitModeTimer -= dt;
    return;
  }
  mOrbitModeTimer = 0.f;
}

#endif

void CPlayer::UpdateOrbitPreventionTimer(float dt) {
  if (mOrbitPreventionTimer > 0.f) {
    mOrbitPreventionTimer -= dt;
  }
}

void CPlayer::AddOrbitDisableSource(CStateManager& mgr, TUniqueId id) {
  if (mOrbitDisableList.size() >= 5) {
    return;
  }
  for (AUTO(it, mOrbitDisableList.begin()); it != mOrbitDisableList.end(); ++it) {
    if (*it == id) {
      return;
    }
  }
  mOrbitDisableList.push_back(id);
  SetAimTargetId(kInvalidUniqueId);
  const TUniqueId orbitTarget = GetOrbitTargetId();
  if (!TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(orbitTarget))) {
    SetOrbitTargetId(kInvalidUniqueId, mgr);
  }
}

void CPlayer::RemoveOrbitDisableSource(TUniqueId id) {
  for (AUTO(it, mOrbitDisableList.begin()); it != mOrbitDisableList.end(); ++it) {
    if (*it == id) {
      mOrbitDisableList.erase(it);
      return;
    }
  }
}

bool CPlayer::CheckOrbitDisableSourceList() const { return !mOrbitDisableList.empty(); }

bool CPlayer::CheckOrbitDisableSourceList(const CStateManager& mgr) {
  for (AUTO(it, mOrbitDisableList.begin()); it != mOrbitDisableList.end();) {
    if (!mgr.GetObjectById(*it)) {
      mOrbitDisableList.erase(it);
      it = mOrbitDisableList.begin();
    } else {
      ++it;
    }
  }
  return !mOrbitDisableList.empty();
}

#if VERSION >= VERSION_R3IJ_00
bool CPlayer::WithinOrbitScreenEllipse(const CVector3f& screenCoords, EPlayerZoneInfo zone,
                                     const CStateManager& mgr) const {
#else
bool CPlayer::WithinOrbitScreenEllipse(const CVector3f& screenCoords, EPlayerZoneInfo zone) const {
#endif
  if (screenCoords.GetZ() >= 1.f) {
    return false;
  }
#if VERSION >= VERSION_R3IJ_00
  const float centerX = CCast::LtoF(gpTweakPlayer->GetOrbitZoneCentreX(zone, mgr));
#if defined(TARGET_PC)
  const float uiX = metaforce::ScreenToUiX(screenCoords.GetX(), CGraphics::GetViewportWidth());
  const float x = CMath::AbsF(uiX - centerX);
#else
  const float x = CMath::AbsF(screenCoords.GetX() - centerX);
#endif
  const float centerY = CCast::LtoF(gpTweakPlayer->GetOrbitZoneCentreY(zone, mgr));
  const float y = CMath::AbsF(screenCoords.GetY() - centerY);
  const float xSq = x * x;
  const float ySq = y * y;
  const float heXSq =
      CCast::LtoF(gpTweakPlayer->GetOrbitZoneWidth(zone) * gpTweakPlayer->GetOrbitZoneWidth(zone));
  const float heYSq =
      CCast::LtoF(gpTweakPlayer->GetOrbitZoneHeight(zone) * gpTweakPlayer->GetOrbitZoneHeight(zone));
  return xSq <= heXSq * (1.f - ySq / heYSq);
#else
#if defined(TARGET_PC)
  const float uiX = metaforce::ScreenToUiX(screenCoords.GetX(), CGraphics::GetViewportWidth());
  const float x = CMath::AbsF(uiX - CCast::LtoF(gpTweakPlayer->GetOrbitZoneCentreX(zone)));
#else
  const float x =
      CMath::AbsF(screenCoords.GetX() - CCast::LtoF(gpTweakPlayer->GetOrbitZoneCentreX(zone)));
#endif
  const float heYSq = CCast::LtoF(gpTweakPlayer->GetOrbitZoneHeight(zone) *
                                  gpTweakPlayer->GetOrbitZoneHeight(zone));
  const float heXSq =
      CCast::LtoF(gpTweakPlayer->GetOrbitZoneWidth(zone) * gpTweakPlayer->GetOrbitZoneWidth(zone));
  const float y =
      CMath::AbsF(screenCoords.GetY() - CCast::LtoF(gpTweakPlayer->GetOrbitZoneCentreY(zone)));
  const bool inside = x * x <= (1.f - y * y / heYSq) * heXSq;
  return inside;
#endif
}

#if VERSION >= VERSION_R3IJ_00
bool CPlayer::WithinOrbitScreenBox(const CVector3f& screenCoords, EPlayerZoneInfo zone,
                                 EPlayerZoneType type, const CStateManager& mgr) const {
#else
bool CPlayer::WithinOrbitScreenBox(const CVector3f& screenCoords, EPlayerZoneInfo zone,
                                 EPlayerZoneType type) const {
#endif
  if (screenCoords.GetZ() >= 1.f) {
    return false;
  }
  switch (type) {
  case kZT_Box: {
#if defined(TARGET_PC)
    const float x = metaforce::ScreenToUiX(screenCoords.GetX(), CGraphics::GetViewportWidth());
#else
    const float x = screenCoords.GetX();
#endif
#if VERSION >= VERSION_R3IJ_00
    const float distanceX =
        CMath::AbsF(x - CCast::LtoF(gpTweakPlayer->GetOrbitZoneCentreX(zone, mgr)));
#else
    const float distanceX = CMath::AbsF(x - CCast::LtoF(gpTweakPlayer->GetOrbitZoneCentreX(zone)));
#endif
    if (distanceX <= CCast::LtoF(gpTweakPlayer->GetOrbitZoneWidth(zone))) {
      const float y = screenCoords.GetY();
#if VERSION >= VERSION_R3IJ_00
      const float distanceY =
          CMath::AbsF(y - CCast::LtoF(gpTweakPlayer->GetOrbitZoneCentreY(zone, mgr)));
#else
      const float distanceY = CMath::AbsF(y - CCast::LtoF(gpTweakPlayer->GetOrbitZoneCentreY(zone)));
#endif
      if (distanceY <= CCast::LtoF(gpTweakPlayer->GetOrbitZoneHeight(zone)) &&
          screenCoords.GetZ() < 1.f) {
        return true;
      }
    }
    break;
  }
  case kZT_Ellipse:
#if VERSION >= VERSION_R3IJ_00
    return WithinOrbitScreenEllipse(screenCoords, zone, mgr);
#else
    return WithinOrbitScreenEllipse(screenCoords, zone);
#endif
  default:
    return true;
  }
  return false;
}

void CPlayer::FindOrbitableObjects(const rstl::reserved_vector< TUniqueId, 1024 >& nearObjects,
                                   rstl::vector< TUniqueId >& listOut, EPlayerZoneInfo zone,
                                   EPlayerZoneType type, CStateManager& mgr,
                                   bool onScreenTest) const {
  const CVector3f position = GetTranslation();
  const CVector3f eyePosition = GetEyePosition();
  CVector3f forward = GetTransform().GetForward();
  forward.Normalize();
  const CFirstPersonCamera* const fpCamera = mgr.GetCameraManager()->GetFirstPersonCamera();
  for (AUTO(it, nearObjects.begin()); it != nearObjects.end(); ++it) {
    const CActor* const act = TCastToConstPtr< CActor >(mgr.GetObjectById(*it));
    if (act && act->GetUniqueId() != GetUniqueId()) {
      if (ValidateOrbitTargetId(act->GetUniqueId(), mgr) != kOVR_OK) {
        continue;
      }
      const CVector3f orbitPosition = act->GetOrbitPosition(mgr);
      CVector3f screenPosition = fpCamera->ConvertToScreenSpace(orbitPosition);
      screenPosition.SetX(screenPosition.GetX() * CCast::LtoF(CGraphics::GetViewportWidth()) / 2.f +
                          CCast::LtoF(CGraphics::GetViewportWidth()) / 2.f);
      screenPosition.SetY(screenPosition.GetY() * CCast::LtoF(CGraphics::GetViewportHeight()) /
                              2.f +
                          CCast::LtoF(CGraphics::GetViewportHeight()) / 2.f);
      bool pass = false;
#if VERSION >= VERSION_R3IJ_00
      if (onScreenTest && WithinOrbitScreenBox(screenPosition, zone, type, mgr)) {
        pass = true;
      } else if (!onScreenTest && !WithinOrbitScreenBox(screenPosition, zone, type, mgr)) {
        pass = true;
      }
#else
      if (onScreenTest && WithinOrbitScreenBox(screenPosition, zone, type)) {
        pass = true;
      } else if (!onScreenTest && !WithinOrbitScreenBox(screenPosition, zone, type)) {
        pass = true;
      }
#endif
      if (pass) {
        const CVector3f eyeToOrbit = orbitPosition - eyePosition;
        const float distance = eyeToOrbit.Magnitude();
        if (!act->GetDoTargetDistanceTest() || distance <= GetOrbitMaxTargetDistance(mgr)) {
          if (listOut.size() != listOut.capacity()) {
#if VERSION >= VERSION_R3IJ_00
            listOut.push_back_unsafe(act->GetUniqueId());
#else
            listOut.push_back(act->GetUniqueId());
#endif
          }
        }
      }
    }
  }
}

#if VERSION < VERSION_R3IJ_00

TUniqueId CPlayer::FindBestOrbitableObject(const rstl::vector< TUniqueId >& ids,
                                           EPlayerZoneInfo zone, CStateManager& mgr) const {
  const CVector3f eyePosition = GetEyePosition();
  CVector3f forward = GetTransform().GetForward();
  forward.Normalize();
  float minDistance = 10000.f;
  float minScreenDistanceSq = 10000.f;
  TUniqueId bestId = kInvalidUniqueId;
  const int viewportHalfX = CGraphics::GetViewportWidth() / 2;
  const int viewportHalfY = CGraphics::GetViewportHeight() / 2;
  float boxLeft = CCast::LtoF(gpTweakPlayer->GetOrbitZoneIdealX(zone)) - CCast::LtoF(viewportHalfX);
  boxLeft /= CCast::LtoF(viewportHalfX);
  float boxTop = CCast::LtoF(gpTweakPlayer->GetOrbitZoneIdealY(zone)) - CCast::LtoF(viewportHalfY);
  boxTop /= CCast::LtoF(viewportHalfY);
  const CFirstPersonCamera* const fpCamera = mgr.GetCameraManager()->GetFirstPersonCamera();
  for (AUTO(it, ids.begin()); it != ids.end(); ++it) {
    const CActor* const act = TCastToConstPtr< CActor >(mgr.GetObjectById(*it));
    if (act) {
      const CVector3f orbitPosition = act->GetOrbitPosition(mgr);
      CVector3f eyeToOrbit = orbitPosition - eyePosition;
      const float distance = eyeToOrbit.Magnitude();
      const CVector3f screenPosition = fpCamera->ConvertToScreenSpace(orbitPosition);
      if (screenPosition.GetZ() >= 0.f) {
        if (*it != mOrbitTargetId) {
          const CScriptGrapplePoint* const point = TCastToConstPtr< CScriptGrapplePoint >(act);
          if (point && point->GetUniqueId() != mOrbitTargetId) {
            if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_GrappleBeam) &&
                distance < minDistance && distance < gpTweakPlayer->GetOrbitDistanceMax()) {
              TUniqueId intersectId = kInvalidUniqueId;
              TEntityList nearList;
              mgr.BuildNearList(nearList, eyePosition, eyeToOrbit.Normalize(), distance,
                                kOccluderFilter, act);
              const CRayCastResult result =
                  mgr.RayWorldIntersection(intersectId, eyePosition, eyeToOrbit.Normalize(),
                                           distance, kLineOfSightFilter, nearList);
              if (result.IsInvalid()) {
                if (point->GetGrappleParameters().GetLockSwingTurn()) {
                  CVector3f pointToPlayer = GetTranslation() - point->GetTranslation();
                  if (pointToPlayer.CanBeNormalized()) {
                    const CVector3f pointForward =
                        point->GetTransform().GetForward().AsNormalized();
                    pointToPlayer.SetZ(0.f);
                    if (CMath::AbsF(CVector3f::Dot(pointForward, pointToPlayer.AsNormalized())) <=
                        0.70710677f) {
                      continue;
                    }
                  }
                }
                bestId = act->GetUniqueId();
                const float screenX = screenPosition.GetX() - boxLeft;
                const float screenYSq =
                    (screenPosition.GetY() - boxTop) * (screenPosition.GetY() - boxTop);
                const float screenXSq = screenX * screenX;
                minDistance = distance;
                minScreenDistanceSq = screenXSq + screenYSq;
              }
            }
            continue;
          }
          if (minDistance - distance > gpTweakPlayer->GetOrbitDistanceThreshold() &&
              mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::kPV_Scan) {
            TEntityList nearList;
            TUniqueId intersectId = kInvalidUniqueId;
            mgr.BuildNearList(nearList, eyePosition, eyeToOrbit.Normalize(), distance,
                              kOccluderFilter, act);
            for (AUTO(nearIt, nearList.begin()); nearIt != nearList.end();) {
              if (const CEntity* ent = mgr.GetObjectById(*nearIt)) {
                const TAreaId areaId = ent->GetCurrentAreaId();
                if (areaId == kInvalidAreaId ||
                    (areaId != kInvalidAreaId && areaId != mgr.GetNextAreaId() &&
                     mgr.GetWorld()->GetAreaAlways(areaId).GetOcclusionState() ==
                         CGameArea::kOS_Occluded)) {
                  nearIt = nearList.erase(nearIt);
                  continue;
                }
              }
              ++nearIt;
            }
            const CRayCastResult result =
                mgr.RayWorldIntersection(intersectId, eyePosition, eyeToOrbit.Normalize(), distance,
                                         kLineOfSightFilter, nearList);
            if (result.IsInvalid()) {
              bestId = act->GetUniqueId();
              const float screenX = screenPosition.GetX() - boxLeft;
              const float screenYSq =
                  (screenPosition.GetY() - boxTop) * (screenPosition.GetY() - boxTop);
              const float screenXSq = screenX * screenX;
              minDistance = distance;
              minScreenDistanceSq = screenXSq + screenYSq;
            }
            continue;
          }
          if (CMath::AbsF(distance - minDistance) < gpTweakPlayer->GetOrbitDistanceThreshold() ||
              mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan) {
            const float screenX = screenPosition.GetX() - boxLeft;
            const float screenYSq =
                (screenPosition.GetY() - boxTop) * (screenPosition.GetY() - boxTop);
            const float screenXSq = screenX * screenX;
            const float screenDistanceSq = screenXSq + screenYSq;
            if (screenDistanceSq < minScreenDistanceSq) {
              TUniqueId intersectId = kInvalidUniqueId;
              TEntityList nearList;
              mgr.BuildNearList(nearList, eyePosition, eyeToOrbit.Normalize(), distance,
                                kOccluderFilter, act);
              for (AUTO(nearIt, nearList.begin()); nearIt != nearList.end();) {
                if (const CEntity* ent = mgr.GetObjectById(*nearIt)) {
                  const TAreaId areaId = ent->GetCurrentAreaId();
                  if (areaId == kInvalidAreaId ||
                      (areaId != kInvalidAreaId && areaId != mgr.GetNextAreaId() &&
                       mgr.GetWorld()->GetAreaAlways(areaId).GetOcclusionState() ==
                           CGameArea::kOS_Occluded)) {
                    nearIt = nearList.erase(nearIt);
                    continue;
                  }
                }
                ++nearIt;
              }
              const CRayCastResult result =
                  mgr.RayWorldIntersection(intersectId, eyePosition, eyeToOrbit.Normalize(),
                                           distance, kLineOfSightFilter, nearList);
              if (result.IsInvalid()) {
                bestId = act->GetUniqueId();
                minScreenDistanceSq = screenDistanceSq;
                minDistance = distance;
              }
            }
          }
        }
      }
    }
  }
  return bestId;
}

#endif

void CPlayer::UpdateOrbitableObjects(CStateManager& mgr) {
  mOnScreenOrbitObjects.clear();
  mNearbyOrbitObjects.clear();
  mOffScreenOrbitObjects.clear();
  if (CheckOrbitDisableSourceList(mgr)) {
    return;
  }
  const CTransform4f& cameraXf = GetFirstPersonCameraTransform(mgr);
  float distance = GetOrbitMaxTargetDistance(mgr);
  if (mExtendTargetDistance) {
    distance *= 5.f;
  }
  const CAABox nearBounds = BuildNearListBox(true, cameraXf, gpTweakPlayer->GetOrbitNearX(),
                                             gpTweakPlayer->GetOrbitNearZ(), distance);
  staticBox = nearBounds;
  CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Orbit));
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan) {
    filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Scannable));
  }
  TEntityList nearList;
  mgr.BuildNearList(nearList, nearBounds, filter, nullptr);
  FindOrbitableObjects(nearList, mNearbyOrbitObjects, mOrbitZoneMode, kZT_Always, mgr,
                       true);
  FindOrbitableObjects(nearList, mOnScreenOrbitObjects, mOrbitZoneMode, mOrbitZoneType, mgr,
                       true);
  FindOrbitableObjects(nearList, mOffScreenOrbitObjects, mOrbitZoneMode, mOrbitZoneType,
                       mgr, false);
}

TUniqueId CPlayer::FindOrbitTargetId(CStateManager& mgr) {
  return FindBestOrbitableObject(mOnScreenOrbitObjects, mOrbitZoneMode, mgr);
}

TUniqueId CPlayer::CheckEnemiesAgainstOrbitZone(const rstl::reserved_vector< TUniqueId, 1024 >& ids,
                                                EPlayerZoneInfo zone, EPlayerZoneType type,
                                                CStateManager& mgr) const {
  const CVector3f position = GetTranslation();
  const CVector3f eyePosition = GetEyePosition();
  CVector3f forward = GetTransform().GetForward();
  forward.Normalize();
  float minDistance = 10000.f;
  float minScreenDistanceSq = 10000.f;
  TUniqueId bestId = kInvalidUniqueId;
#if VERSION >= VERSION_R3IJ_00
  const int viewportWidth = CGraphics::GetViewportWidth();
  const float idealX = static_cast< float >(gpTweakPlayer->GetOrbitZoneCentreX(zone, mgr));
  float boxLeft = idealX - static_cast< float >(viewportWidth / 2);
  boxLeft /= static_cast< float >(CGraphics::GetViewportWidth() / 2);
  const int viewportHeight = CGraphics::GetViewportHeight();
  const float idealY = static_cast< float >(gpTweakPlayer->GetOrbitZoneCentreY(zone, mgr));
  float boxTop = idealY - static_cast< float >(viewportHeight / 2);
  boxTop /= static_cast< float >(CGraphics::GetViewportHeight() / 2);
#else
  const int viewportHalfX = CGraphics::GetViewportWidth() / 2;
  const int idealX = gpTweakPlayer->GetOrbitZoneIdealX(zone);
  const int viewportHalfY = CGraphics::GetViewportHeight() / 2;
  const int idealY = gpTweakPlayer->GetOrbitZoneIdealY(zone);
  float boxLeft = CCast::LtoF(idealX) - CCast::LtoF(viewportHalfX);
  boxLeft /= CCast::LtoF(viewportHalfX);
  float boxTop = CCast::LtoF(idealY) - CCast::LtoF(viewportHalfY);
  boxTop /= CCast::LtoF(viewportHalfY);
#endif
  const CFirstPersonCamera* const fpCamera = mgr.GetCameraManager()->GetFirstPersonCamera();
  for (AUTO(it, ids.begin()); it != ids.end(); ++it) {
    const CActor* act = static_cast< const CActor* >(mgr.GetObjectById(*it));
    if (act && act->GetUniqueId() != GetUniqueId()) {
      if (ValidateObjectForMode(act->GetUniqueId(), mgr)) {
        const CVector3f aimPosition = act->GetAimPosition(mgr, 0.f);
        CVector3f positionInBox = fpCamera->ConvertToScreenSpace(aimPosition);
        positionInBox.SetX(positionInBox.GetX() *
                               static_cast< float >(CGraphics::GetViewportWidth()) / 2.f +
                           static_cast< float >(CGraphics::GetViewportWidth()) / 2.f);
        positionInBox.SetY(positionInBox.GetY() *
                               static_cast< float >(CGraphics::GetViewportHeight()) / 2.f +
                           static_cast< float >(CGraphics::GetViewportHeight()) / 2.f);
#if VERSION >= VERSION_R3IJ_00
        if (WithinOrbitScreenBox(positionInBox, zone, type, mgr)) {
#else
        if (WithinOrbitScreenBox(positionInBox, zone, type)) {
#endif
          CVector3f eyeToAim = aimPosition - eyePosition;
          const float distance = eyeToAim.Magnitude();
          if (distance <= gpTweakPlayer->GetAimMaxDistance()) {
            if (minDistance - distance > gpTweakPlayer->GetAimThresholdDistance()) {
              TEntityList nearList;
              TUniqueId intersectId = kInvalidUniqueId;
              mgr.BuildNearList(nearList, eyePosition, eyeToAim.Normalize(), distance,
                                kOccluderFilter, act);
              const CRayCastResult result =
                  mgr.RayWorldIntersection(intersectId, eyePosition, eyeToAim.Normalize(), distance,
                                           kLineOfSightFilter, nearList);
              if (result.IsInvalid()) {
                bestId = act->GetUniqueId();
                minDistance = distance;
                minScreenDistanceSq =
                    (positionInBox.GetX() - boxLeft) * (positionInBox.GetX() - boxLeft) +
                    (positionInBox.GetY() - boxTop) * (positionInBox.GetY() - boxTop);
              }
            } else if (CMath::AbsF(distance - minDistance) <
                       gpTweakPlayer->GetAimThresholdDistance()) {
              const float screenDistanceSq =
                  (positionInBox.GetX() - boxLeft) * (positionInBox.GetX() - boxLeft) +
                  (positionInBox.GetY() - boxTop) * (positionInBox.GetY() - boxTop);
              if (screenDistanceSq < minScreenDistanceSq) {
                TEntityList nearList;
                TUniqueId intersectId = kInvalidUniqueId;
                mgr.BuildNearList(nearList, eyePosition, eyeToAim.Normalize(), distance,
                                  kOccluderFilter, act);
                const CRayCastResult result =
                    mgr.RayWorldIntersection(intersectId, eyePosition, eyeToAim.Normalize(),
                                             distance, kLineOfSightFilter, nearList);
                if (result.IsInvalid()) {
                  bestId = act->GetUniqueId();
                  minScreenDistanceSq = screenDistanceSq;
                  minDistance = distance;
                }
              }
            }
          }
        }
      }
    }
  }
  return bestId;
}

TUniqueId CPlayer::FindAimTargetId(CStateManager& mgr) {
  const CTransform4f& cameraXf = GetFirstPersonCameraTransform(mgr);
  float distance = gpTweakPlayer->GetAimMaxDistance();
  if (mExtendTargetDistance) {
    distance *= 5.f;
  }
  const CAABox bounds = BuildNearListBox(true, cameraXf, gpTweakPlayer->GetAimBoxWidth(),
                                         gpTweakPlayer->GetAimBoxHeight(), distance);
  const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Target));
  TEntityList nearList;
  mgr.BuildNearList(nearList, bounds, filter, this);
  return CheckEnemiesAgainstOrbitZone(nearList, kZI_Targeting, kZT_Ellipse, mgr);
}

bool CPlayer::ValidateObjectForMode(const TUniqueId id, CStateManager& mgr) const {
  const CActor* act = TCastToConstPtr< CActor >(mgr.GetObjectById(id));
  if (!act || id == kInvalidUniqueId) {
    return false;
  }
  if (TCastToConstPtr< CScriptDoor >(mgr.GetObjectById(id))) {
    return true;
  }
  if (GetCombatMode()) {
    if (act->GetHealthInfo(mgr)) {
      if (act->GetHealthInfo(mgr)->GetHP() > 0.f) {
        return true;
      }
    } else {
      if (act->GetMaterialList().HasMaterial(kMT_Projectile) ||
          act->GetMaterialList().HasMaterial(kMT_Scannable)) {
        return true;
      }
      if (const CScriptGrapplePoint* point =
              TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(id))) {
        const CVector3f playerToPoint = point->GetTranslation() - GetTranslation();
        if (playerToPoint.CanBeNormalized() &&
            playerToPoint.Magnitude() < gpTweakPlayer->GetOrbitDistanceMax()) {
          return true;
        }
      }
    }
  }
  if (GetExplorationMode()) {
    if (!act->GetHealthInfo(mgr)) {
      if (const CScriptGrapplePoint* point =
              TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(id))) {
        const CVector3f playerToPoint = point->GetTranslation() - GetTranslation();
        if (playerToPoint.CanBeNormalized() &&
            playerToPoint.Magnitude() < gpTweakPlayer->GetOrbitDistanceMax()) {
          return true;
        }
      } else {
        return true;
      }
    } else {
      return true;
    }
  }
  return false;
}

bool CPlayer::ValidateAimTargetId(const TUniqueId id, CStateManager& mgr) {
  if (id == kInvalidUniqueId) {
    mAimTargetAverage.clear();
    mAimTargetTimer = 0.f;
    return false;
  }
  const CActor* act = TCastToConstPtr< CActor >(mgr.GetObjectById(id));
  if (!act || !act->GetMaterialList().HasMaterial(kMT_Target) || !act->GetTargetable()) {
    return false;
  }
  if (mOrbitState == kOS_OrbitObject || mOrbitState == kOS_ForcedOrbitObject) {
    if (ValidateOrbitTargetId(GetOrbitTargetId(), mgr) != kOVR_OK) {
      SetAimTargetId(kInvalidUniqueId);
      mAimTargetTimer = 0.f;
      return false;
    }
    return true;
  }
  if (act->GetMaterialList().HasMaterial(kMT_Target) && id != kInvalidUniqueId) {
    if (ValidateObjectForMode(id, mgr)) {
      const CVector3f aimPosition = act->GetAimPosition(mgr, 0.f);
      const CVector3f eyePosition = GetEyePosition();
      CVector3f eyeToAim = aimPosition - eyePosition;
      CVector3f positionInBox =
          mgr.GetCameraManager()->GetFirstPersonCamera()->ConvertToScreenSpace(aimPosition);
      positionInBox.SetX(positionInBox.GetX() *
                             static_cast< float >(CGraphics::GetViewportWidth()) / 2.f +
                         static_cast< float >(CGraphics::GetViewportWidth()) / 2.f);
      positionInBox.SetY(positionInBox.GetY() *
                             static_cast< float >(CGraphics::GetViewportHeight()) / 2.f +
                         static_cast< float >(CGraphics::GetViewportHeight()) / 2.f);
#if VERSION >= VERSION_R3IJ_00
      if (WithinOrbitScreenBox(positionInBox, mOrbitZoneMode, mOrbitZoneType, mgr) ||
          (mOrbitZoneMode != kZI_Targeting &&
           WithinOrbitScreenBox(positionInBox, kZI_Targeting, mOrbitZoneType, mgr))) {
#else
      if (WithinOrbitScreenBox(positionInBox, mOrbitZoneMode, mOrbitZoneType) ||
          (mOrbitZoneMode != kZI_Targeting &&
           WithinOrbitScreenBox(positionInBox, kZI_Targeting, mOrbitZoneType))) {
#endif
        const float distance = eyeToAim.Magnitude();
        if (distance <= gpTweakPlayer->GetAimMaxDistance()) {
          TEntityList nearList;
          TUniqueId intersectId = kInvalidUniqueId;
          mgr.BuildNearList(nearList, eyePosition, eyeToAim.Normalize(), distance, kOccluderFilter,
                            nullptr);
          const CRayCastResult result =
              mgr.RayWorldIntersection(intersectId, eyePosition, eyeToAim.Normalize(), distance,
                                       kLineOfSightFilter, nearList);
          if (result.IsInvalid()) {
            mAimTargetTimer = gpTweakPlayer->GetAimTargetTimer();
            return true;
          }
        }
      }
      if (mAimTargetTimer > 0.f) {
        return true;
      }
    }
  }
  SetAimTargetId(kInvalidUniqueId);
  mAimTargetTimer = 0.f;
  return false;
}

void CPlayer::UpdateAimTargetTimer(float dt) {
  if (GetAimTargetId() != kInvalidUniqueId && mAimTargetTimer > 0.f) {
    mAimTargetTimer -= dt;
  }
}

void CPlayer::UpdateAimTarget(CStateManager& mgr) {
  if (!ValidateAimTargetId(GetAimTargetId(), mgr)) {
    SetAimTargetId(kInvalidUniqueId);
  }
  if (!GetCombatMode()) {
    SetAimTargetId(kInvalidUniqueId);
    mAimTargetTimer = 0.f;
    return;
  }
  if (!gkAutoAim && gkAutoAimAtOrbitedObject) {
    SetAimTargetId(kInvalidUniqueId);
    mAimTargetTimer = 0.f;
    if (mOrbitState == kOS_OrbitObject || mOrbitState == kOS_ForcedOrbitObject) {
      if (ValidateOrbitTargetId(GetOrbitTargetId(), mgr) == kOVR_OK) {
        SetAimTargetId(GetOrbitTargetId());
      }
    }
    return;
  }
  bool needsReset = false;
  const CActor* act = TCastToConstPtr< CActor >(mgr.GetObjectById(GetAimTargetId()));
  if (act && !act->GetMaterialList().HasMaterial(kMT_Target)) {
    act = nullptr;
  }
#if VERSION < VERSION_R3IJ_00
  if (gpTweakPlayer->GetAimWhenOrbitingPoint()) {
#endif
    switch (mOrbitState) {
    case kOS_OrbitObject:
    case kOS_ForcedOrbitObject:
      if (ValidateOrbitTargetId(GetOrbitTargetId(), mgr) == kOVR_OK) {
        SetAimTargetId(GetOrbitTargetId());
      } else {
        needsReset = true;
      }
      break;
    default:
      needsReset = true;
      break;
    }
#if VERSION < VERSION_R3IJ_00
  } else if (mOrbitState == kOS_NoOrbit) {
    needsReset = true;
  }
#endif
  if (needsReset) {
    if (!ValidateAimTargetId(GetAimTargetId(), mgr)) {
      if (act && ValidateObjectForMode(GetAimTargetId(), mgr)) {
        SetAimTargetId(kInvalidUniqueId);
      } else {
        SetAimTargetId(FindAimTargetId(mgr));
      }
    }
  }
}

void CPlayer::SetOrbitPosition(float distance, CStateManager& mgr) {
  CTransform4f cameraXf = GetFirstPersonCameraTransform(mgr);
  if (mOrbitState == kOS_OrbitPoint && mOrbitBrokenType == kOB_BadVerticalAngle) {
    cameraXf = GetTransform();
  }
  CVector3f flatForward = cameraXf.GetForward();
  flatForward.SetZ(0.f);
#if VERSION >= VERSION_R3IJ_00
  float maxDot = 1.f;
#endif
  float dot = CVector3f::Dot(flatForward.AsNormalized(), cameraXf.GetForward());
#if VERSION >= VERSION_R3IJ_00
  dot = CMath::FastLimit(dot, maxDot);
#else
  dot = CMath::Limit(dot, 1.f);
#endif
  const CVector3f orbitVector(0.f, distance / dot, 0.f);
  mOrbitPoint = cameraXf.GetTranslation() + cameraXf.Rotate(orbitVector);
  mOrbitVector = CVector3f(0.f, distance, mOrbitPoint.GetZ() - cameraXf.GetTranslation().GetZ());
}

void CPlayer::UpdateOrbitFixedPosition() {
  const CVector3f eyePosition = GetEyePosition();
  mOrbitPoint = eyePosition + GetTransform().Rotate(mOrbitVector);
}

void CPlayer::UpdateOrbitZPosition() {
  switch (mOrbitState) {
  case kOS_OrbitPoint:
    if (CMath::AbsF(mOrbitVector.GetZ()) < gpTweakPlayer->GetOrbitZRange()) {
      mOrbitPoint.SetZ(mOrbitVector[kDZ] + (GetTranslation().GetZ() + GetEyeHeight()));
    }
    break;
  default:
    break;
  }
}

void CPlayer::UpdateOrbitPosition(float distance, CStateManager& mgr) {
  switch (GetOrbitState()) {
  case kOS_NoOrbit:
    break;
  case kOS_OrbitPoint:
  case kOS_OrbitCarcass:
    SetOrbitPosition(distance, mgr);
    break;
  case kOS_ForcedOrbitObject:
  case kOS_Grapple:
  case kOS_OrbitObject: {
    const CActor* act = TCastToConstPtr< CActor >(mgr.GetObjectById(GetOrbitTargetId()));
    if (!act || GetOrbitTargetId() == kInvalidUniqueId) {
      break;
    }
    mOrbitPoint = act->GetOrbitPosition(mgr);
    break;
  }
  default:
    break;
  }
}

void CPlayer::SetOrbitTargetId(TUniqueId id, CStateManager& mgr) {
  if (id != kInvalidUniqueId) {
    const CPatterned* const patterned = TCastToConstPtr< CPatterned >(mgr.GetObjectById(id));
    const CWallCrawlerSwarm* const swarm =
        TCastToConstPtr< CWallCrawlerSwarm >(mgr.GetObjectById(id));
#ifdef HAS_TYPES_MATCH
    const CThardusRockProjectile* const rock =
        TCastToConstPtr< CThardusRockProjectile >(mgr.GetObjectById(id));
#else
    const CThardusRockProjectile* const rock =
        PATTERNED_CAST_TO(CThardusRockProjectile, const_cast< CEntity* >(mgr.GetObjectById(id)));
#endif
    const CScriptGunTurret* const turret = TCastToConstPtr< CScriptGunTurret >(mgr.GetObjectById(id));
    if (patterned || swarm || rock || turret) {
      mOrbitingEnemy = true;
    } else {
      mOrbitingEnemy = false;
    }
  }
  mOrbitTargetId = id;
  if (mOrbitTargetId == kInvalidUniqueId) {
    mOrbitLockEstablished = false;
  }
}

void CPlayer::SetOrbitState(EPlayerOrbitState state, CStateManager& mgr) {
  mOrbitState = state;
  CFirstPersonCamera* camera = mgr.CameraManager()->FirstPersonCamera();
  switch (mOrbitState) {
  case kOS_OrbitObject:
    camera->SetLockCamera(false);
    break;
  case kOS_OrbitCarcass: {
    camera->SetLockCamera(true);
    CVector3f playerToPoint = mOrbitPoint - GetTransform().GetTranslation();
    playerToPoint.SetZ(0.f);
    if (playerToPoint.CanBeNormalized()) {
      x340_ = playerToPoint.Magnitude();
    } else {
      x340_ = 0.f;
    }
    SetOrbitTargetId(kInvalidUniqueId, mgr);
    SetOrbitNextTargetId(kInvalidUniqueId);
    break;
  }
  case kOS_NoOrbit:
    mOrbitModeTimer = gpTweakPlayer->GetOrbitModeTimer();
    mOrbitModeTimer = 0.28f;
    SetOrbitTargetId(kInvalidUniqueId, mgr);
    SetOrbitNextTargetId(kInvalidUniqueId);
    break;
  case kOS_OrbitPoint:
    SetOrbitTargetId(kInvalidUniqueId, mgr);
    SetOrbitNextTargetId(kInvalidUniqueId);
    break;
  default:
    break;
  }
}

CVector3f CPlayer::GetHUDOrbitTargetPosition() const {
  return mOrbitPoint + mCameraBob->GetCameraBobTransformation().GetTranslation();
}

float CPlayer::CalculateOrbitZBasedDistance(EPlayerOrbitType type) {
  static const float maxScale = 4.f;
#if VERSION >= VERSION_R3IJ_00
  const float scale = CMath::AbsF(mOrbitPoint.GetZ() - GetTranslation().GetZ()) / 20.f;
  return gpTweakPlayer->GetOrbitMinDistance(type) * CMath::FastClamp(1.f, scale, maxScale);
#else
  float distance = gpTweakPlayer->GetOrbitMinDistance(type);
  distance *=
      CMath::Clamp(1.f, CMath::AbsF(mOrbitPoint.GetZ() - GetTranslation().GetZ()) / 20.f, maxScale);
  return distance;
#endif
}

void CPlayer::OrbitPoint(EPlayerOrbitType type, CStateManager& mgr) {
  mOrbitType = type;
  SetOrbitState(kOS_OrbitPoint, mgr);
  SetOrbitPosition(gpTweakPlayer->GetOrbitNormalDistance(mOrbitType), mgr);
}

#if VERSION < VERSION_R3IJ_00

void CPlayer::OrbitCarcass(CStateManager& mgr) {
  if (mOrbitState == kOS_OrbitObject) {
    mOrbitType = kOT_Default;
    SetOrbitState(kOS_OrbitCarcass, mgr);
  }
}

#endif

void CPlayer::PreventFallingCameraPitch() {
  mJumpCameraTimer = 0.f;
  mFallCameraTimer = 0.01f;
  mCancelCameraPitch = true;
}

#if VERSION < VERSION_R3IJ_00

bool CPlayer::CheckPostGrapple() const {
  if (mMovementState != NPlayer::kMS_OnGround &&
      (mGrappleJumpTimeout > 0.f || mJumpCameraTimer == 0.f)) {
    return true;
  }
  return false;
}

#endif

void CPlayer::TryToBreakOrbit(TUniqueId id, EOrbitBrokenType type, CStateManager& mgr) {
  if (GetOrbitState() == kOS_OrbitObject || GetOrbitState() == kOS_Grapple ||
      GetOrbitState() == kOS_ForcedOrbitObject) {
    if (id == GetOrbitTargetId()) {
      BreakOrbit(type, mgr);
    }
  }
}

void CPlayer::BreakOrbit(EOrbitBrokenType type, CStateManager& mgr) {
  mOrbitBrokenType = type;
#if VERSION >= VERSION_R3IJ_00
  SetOrbitState(kOS_NoOrbit, mgr);
#else
  switch (type) {
  case kOB_ActivateOrbitSource:
    ActivateOrbitSource(mgr);
    break;
  case kOB_BadVerticalAngle:
    SetOrbitState(kOS_OrbitPoint, mgr);
    mOrbitPoint = GetTranslation() + gpTweakPlayer->GetOrbitNormalDistance(mOrbitType) *
                                             GetTransform().GetForward();
    break;
  default:
    SetOrbitState(kOS_NoOrbit, mgr);
    break;
  }
#endif
}

void CPlayer::BreakGrapple(EOrbitBrokenType type, CStateManager& mgr) {
  mJumpCameraTimer = 0.f;
  mFallCameraTimer = 0.f;
#if VERSION >= VERSION_R3IJ_00
  if (mGrappleState == kGS_Swinging) {
#else
  if (static_cast< int >(gpTweakPlayer->GetGrappleJumpMode()) == 2 &&
      mGrappleState == kGS_Swinging) {
#endif
    ApplyGrappleJump(mgr);
    PreventFallingCameraPitch();
#if VERSION >= VERSION_R3IJ_00
    mGrappleJumpTimeout = gpTweakPlayer->GetGrappleReleaseTime();
#endif
  }
  BreakOrbit(type, mgr);
  mGrappleState = kGS_None;
  AddMaterial(kMT_GroundCollider, mgr);
  mGun->GrappleArm().SetAnimState(CGrappleArm::kAS_OutOfGrapple);
#if VERSION >= VERSION_R3IJ_00
  if (!CheckPostGrapple()) {
#else
  if (!CheckPostGrapple() && mGrappleState != kGS_JumpOff) {
#endif
    DrawGun(mgr);
  }
}

void CPlayer::BeginGrapple(CVector3f& direction, CStateManager& mgr) {
  direction.SetZ(0.f);
  if (direction.CanBeNormalized()) {
    mGrappleSwingAxis.SetX(direction.GetY());
    mGrappleSwingAxis.SetY(-direction.GetX());
    mGrappleSwingAxis.Normalize();
    mGrappleSwingTimer = 0.f;
    SetOrbitState(kOS_Grapple, mgr);
    mGrappleState = kGS_Pull;
    RemoveMaterial(kMT_GroundCollider, mgr);
  }
}

void CPlayer::ApplyGrappleJump(CStateManager& mgr) {
  const CScriptGrapplePoint* point =
      TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(GetOrbitTargetId()));
  if (!point) {
    return;
  }
  CVector3f swingAxis = mGrappleSwingAxis;
  if (mGrappleSwingTimer < 0.5f * gpTweakPlayer->GetGrappleSwingPeriod()) {
    swingAxis *= -1.f;
  }
  const CVector3f pointToPlayer =
      GetTransform().GetTranslation() - point->GetTransform().GetTranslation();
  const CVector3f cross = CVector3f::Cross(pointToPlayer.AsNormalized(), swingAxis);
  CVector3f pointToPlayerFlat = pointToPlayer;
  pointToPlayerFlat.SetZ(0.f);
  float dot = 1.f;
  if (pointToPlayerFlat.CanBeNormalized() && cross.CanBeNormalized()) {
    float maxCosAngle = 1.f;
    float cosAngle =
        CMath::AbsF(CVector3f::Dot(cross.AsNormalized(), pointToPlayerFlat.AsNormalized()));
#if VERSION >= VERSION_R3IJ_00
    cosAngle = CMath::FastLimit(cosAngle, maxCosAngle);
#else
    cosAngle = CMath::Limit(cosAngle, 1.f);
#endif
    dot = cosAngle;
  }
  const CVector3f force = dot * (10000.f * (gpTweakPlayer->GetGrappleJumpForce() * cross));
  ApplyForceWR(force, CAxisAngle::Identity());
}

#if VERSION >= VERSION_R3IJ_00

void CPlayer::UpdateGrappleState(CStateManager& mgr, float dt) {
  if (!mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_GrappleBeam)) {
    return;
  }
  const float& zero = 0.f;
  const float& time = mGrappleJumpTimeout - dt;
  mGrappleJumpTimeout = CMath::FastMax(zero, time);
  if (mMorphBallState == kMS_Morphed ||
      mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan ||
      mgr.GetPlayerState()->GetTransitioningVisor() == CPlayerState::kPV_Scan) {
    return;
  }
  if (GetOrbitTargetId() == kInvalidUniqueId) {
    mGrappleState = kGS_None;
    AddMaterial(kMT_GroundCollider, mgr);
    return;
  }

  const CScriptGrapplePoint* point =
      TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(GetOrbitTargetId()));
  if (point) {
    const CVector3f position = GetTranslation();
    const CVector3f eyePosition = GetEyePosition();
    CVector3f playerToPoint = point->GetTransform().GetTranslation() - eyePosition;
    CVector3f playerToPointFlat = playerToPoint;
    playerToPointFlat.SetZ(0.f);
    if (playerToPoint.CanBeNormalized() && playerToPointFlat.CanBeNormalized() &&
        playerToPointFlat.Magnitude() > 2.f) {
      if (mOrbitState == kOS_OrbitObject && playerToPoint.CanBeNormalized()) {
        const CRayCastResult result =
            mgr.RayStaticIntersection(eyePosition, playerToPoint.AsNormalized(),
                                      playerToPoint.Magnitude(), kLineOfSightFilter);
        if (result.IsInvalid()) {
          HolsterGun(mgr);
          switch (mGrappleState) {
          case kGS_Swinging:
          case kGS_Firing:
            switch (mGun->GrappleArm().GetAnimState()) {
            case CGrappleArm::kAS_IntoGrappleIdle:
              mGun->GrappleArm().SetAnimState(CGrappleArm::kAS_FireGrapple);
              break;
            case CGrappleArm::kAS_Connected:
              BeginGrapple(playerToPoint, mgr);
              break;
            default:
              break;
            }
            break;
          case kGS_None:
            mGrappleState = kGS_Firing;
            mGun->GrappleArm().Activate(true);
            break;
          default:
            break;
          }
        }
      }
    }
  }

  if (mOrbitState == kOS_Grapple) {
    if (!point) {
      BreakGrapple(kOB_Default, mgr);
      return;
    }
    const CVector3f position = GetTranslation();
    const CVector3f eyePosition = GetEyePosition();
    const CVector3f playerToPoint = point->GetTransform().GetTranslation() - eyePosition;
    if (playerToPoint.CanBeNormalized()) {
      const CRayCastResult result = mgr.RayStaticIntersection(
          eyePosition, playerToPoint.AsNormalized(), playerToPoint.Magnitude(), kLineOfSightFilter);
      if (result.IsValid()) {
        BreakGrapple(kOB_LostGrappleLineOfSight, mgr);
      }
    }
  }
}

#else

void CPlayer::UpdateGrappleState(const CFinalInput& input, CStateManager& mgr) {
  if (!mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_GrappleBeam) ||
      mMorphBallState == kMS_Morphed ||
      mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan ||
      mgr.GetPlayerState()->GetTransitioningVisor() == CPlayerState::kPV_Scan) {
    return;
  }
  if (GetOrbitTargetId() == kInvalidUniqueId) {
    mGrappleState = kGS_None;
    AddMaterial(kMT_GroundCollider, mgr);
    return;
  }
  const CScriptGrapplePoint* point =
      TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(GetOrbitTargetId()));
  if (point) {
    const CVector3f eyePosition = GetEyePosition();
    CVector3f playerToPoint = point->GetTranslation() - eyePosition;
    CVector3f playerToPointFlat = playerToPoint;
    playerToPointFlat.SetZ(0.f);
    if (playerToPoint.CanBeNormalized() && playerToPointFlat.CanBeNormalized() &&
        playerToPointFlat.Magnitude() > 2.f) {
      switch (mOrbitState) {
      case kOS_Grapple:
        switch (static_cast< int >(gpTweakPlayer->GetGrappleJumpMode())) {
        case 0:
        case 1:
          if (ControlMapper::GetPressInput(ControlMapper::kC_FireOrBomb, input)) {
            if (const CScriptGrapplePoint* nextPoint = TCastToConstPtr< CScriptGrapplePoint >(
                    mgr.GetObjectById(mOrbitNextTargetId))) {
              playerToPoint = nextPoint->GetTranslation() - eyePosition;
              playerToPoint.SetZ(0.f);
              if (playerToPoint.CanBeNormalized()) {
                mGun->GrappleArm().GrappleBeamDisconnected();
                mGrappleSwingAxis.SetX(playerToPoint.GetY());
                mGrappleSwingAxis.SetY(-playerToPoint.GetX());
                mGrappleSwingAxis.Normalize();
                mGrappleSwingTimer = 0.f;
                SetOrbitTargetId(mOrbitNextTargetId, mgr);
                mGrappleState = kGS_Pull;
                SetOrbitNextTargetId(kInvalidUniqueId);
                mGun->GrappleArm().GrappleBeamConnected();
              }
            } else {
              if (static_cast< int >(gpTweakPlayer->GetGrappleJumpMode()) == 0 &&
                  mGrappleJumpTimeout <= 0.f) {
                ApplyGrappleJump(mgr);
              }
              BreakGrapple(kOB_StopOrbit, mgr);
            }
          }
          break;
        case 2:
          break;
        default:
          break;
        }
        break;
      case kOS_OrbitObject:
        if (playerToPoint.CanBeNormalized()) {
          const CRayCastResult result =
              mgr.RayStaticIntersection(eyePosition, playerToPoint.AsNormalized(),
                                        playerToPoint.Magnitude(), kLineOfSightFilter);
          if (result.IsInvalid()) {
            HolsterGun(mgr);
            switch (mGrappleState) {
            case kGS_Firing:
            case kGS_Swinging:
              switch (static_cast< int >(gpTweakPlayer->GetGrappleJumpMode())) {
              case 0:
                switch (mGun->GrappleArm().GetAnimState()) {
                case CGrappleArm::kAS_IntoGrappleIdle:
                  if (ControlMapper::GetDigitalInput(ControlMapper::kC_FireOrBomb, input)) {
                    mGun->GrappleArm().SetAnimState(CGrappleArm::kAS_FireGrapple);
                  }
                  break;
                case CGrappleArm::kAS_Connected:
                  BeginGrapple(playerToPoint, mgr);
                  break;
                default:
                  break;
                }
                break;
              case 1:
                if (ControlMapper::GetDigitalInput(ControlMapper::kC_FireOrBomb, input)) {
                  switch (mGun->GrappleArm().GetAnimState()) {
                  case CGrappleArm::kAS_IntoGrappleIdle:
                    mGun->GrappleArm().SetAnimState(CGrappleArm::kAS_FireGrapple);
                    break;
                  case CGrappleArm::kAS_Connected:
                    BeginGrapple(playerToPoint, mgr);
                    break;
                  default:
                    break;
                  }
                }
                break;
              case 2:
                switch (mGun->GrappleArm().GetAnimState()) {
                case CGrappleArm::kAS_IntoGrappleIdle:
                  mGun->GrappleArm().SetAnimState(CGrappleArm::kAS_FireGrapple);
                  break;
                case CGrappleArm::kAS_Connected:
                  BeginGrapple(playerToPoint, mgr);
                  break;
                default:
                  break;
                }
                break;
              default:
                break;
              }
              break;
            case kGS_None:
              mGrappleState = kGS_Firing;
              mGun->GrappleArm().Activate(true);
              break;
            default:
              break;
            }
          }
        }
        break;
      default:
        break;
      }
    }
  }
  const int jumpMode = static_cast< int >(gpTweakPlayer->GetGrappleJumpMode());
  switch (mOrbitState) {
  case kOS_Grapple: {
    if (!point) {
      BreakGrapple(kOB_Default, mgr);
      return;
    }
    switch (jumpMode) {
    case 0:
    case 2:
      switch (mGrappleState) {
      case kGS_JumpOff:
        mGrappleJumpTimeout -= input.Time();
        if (mGrappleJumpTimeout <= 0.f) {
          BreakGrapple(kOB_StopOrbit, mgr);
          SetMoveState(NPlayer::kMS_ApplyJump, mgr);
          ComputeMovement(input, mgr, input.Time());
          PreventFallingCameraPitch();
        }
        break;
      default:
        break;
      }
      break;
    case 1:
      switch (mGrappleState) {
      case kGS_Swinging:
        if (!ControlMapper::GetDigitalInput(ControlMapper::kC_FireOrBomb, input) &&
            mGrappleJumpTimeout <= 0.f) {
          mGrappleJumpTimeout = gpTweakPlayer->GetGrappleReleaseTime();
          mGrappleState = kGS_JumpOff;
          ApplyGrappleJump(mgr);
        }
        break;
      case kGS_JumpOff:
        mGrappleJumpTimeout -= input.Time();
        if (mGrappleJumpTimeout <= 0.f) {
          SetMoveState(NPlayer::kMS_ApplyJump, mgr);
          ComputeMovement(input, mgr, input.Time());
          BreakGrapple(kOB_StopOrbit, mgr);
          PreventFallingCameraPitch();
        }
        break;
      case kGS_Firing:
      case kGS_Pull:
        if (!ControlMapper::GetDigitalInput(ControlMapper::kC_FireOrBomb, input)) {
          BreakGrapple(kOB_StopOrbit, mgr);
        }
        break;
      default:
        break;
      }
      break;
    default:
      break;
    }
    const CVector3f eyePosition = GetEyePosition();
    const CVector3f playerToPoint = point->GetTranslation() - eyePosition;
    if (playerToPoint.CanBeNormalized()) {
      const CRayCastResult result = mgr.RayStaticIntersection(
          eyePosition, playerToPoint.AsNormalized(), playerToPoint.Magnitude(), kLineOfSightFilter);
      if (result.IsValid()) {
        BreakGrapple(kOB_LostGrappleLineOfSight, mgr);
      }
    }
    break;
  }
  case kOS_OrbitObject:
    if (mGun->GrappleArm().BeamActive() && jumpMode == 1 &&
        !ControlMapper::GetDigitalInput(ControlMapper::kC_FireOrBomb, input)) {
      BreakGrapple(kOB_StopOrbit, mgr);
    }
    break;
  default:
    break;
  }
}

#endif

bool CPlayer::ValidateFPPosition(CVector3f position, CStateManager& mgr) {
  TEntityList nearList;
  const CMaterialFilter solidFilter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
  mgr.BuildColliderList(nearList, *this,
                        CAABox(mFpBounds.GetMinPoint() - CVector3f(1.f, 1.f, 1.f) + position,
                               mFpBounds.GetMaxPoint() + CVector3f(1.f, 1.f, 1.f) + position));
  const CAABox& baseBounds = GetBaseBoundingBox();
  const CCollidableAABox collisionBounds(
      CAABox(baseBounds.GetMinPoint() + position, baseBounds.GetMaxPoint() + position),
      CMaterialList());
  if (!CGameCollision::DetectCollisionBoolean(mgr, collisionBounds, CTransform4f::Identity(),
                                              solidFilter, nearList)) {
    return true;
  }
  return false;
}

void CPlayer::ApplyGrappleForces(const CFinalInput& input, CStateManager& mgr, float dt) {
  CVector3f playerPosition = GetTranslation();
  if (const CScriptGrapplePoint* point =
          TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(GetOrbitTargetId()))) {
    const CGrappleParameters& parameters = point->GetGrappleParameters();
    const CVector3f pointPosition = point->GetTranslation();
    switch (GetGrappleState()) {
    case kGS_Pull: {
      const CVector3f swingLow =
          pointPosition + CVector3f(0.f, 0.f, -gpTweakPlayer->GetGrappleSwingLength());
      const CVector3f playerToPoint = pointPosition - GetTranslation();
      if (playerToPoint.CanBeNormalized()) {
        CVector3f playerToSwingLow = swingLow - playerPosition;
        if (playerToSwingLow.CanBeNormalized()) {
          const float distanceToLow = playerToSwingLow.Magnitude();
          playerToSwingLow.Normalize();
#if VERSION >= VERSION_R3IJ_00
          const float timeToLow =
              CMath::FastLimit(distanceToLow / gpTweakPlayer->GetGrapplePullSpeedProportion(), 1.f);
#else
          const float timeToLow =
              CMath::Limit(distanceToLow / gpTweakPlayer->GetGrapplePullSpeedProportion(), 1.f);
#endif
          const float pullSpeed = timeToLow * (gpTweakPlayer->GetGrapplePullSpeedMax() -
                                               gpTweakPlayer->GetGrapplePullSpeedMin()) +
                                  gpTweakPlayer->GetGrapplePullSpeedMin();
          const CVector3f pullVelocity = pullSpeed * playerToSwingLow;
          SetVelocityWR(pullVelocity);
          if (distanceToLow < gpTweakPlayer->GetMaxGrappleLockedTurnAlignDistance()) {
            mGrappleState = kGS_Swinging;
            mGrappleSwingTimer = 0.25f * gpTweakPlayer->GetGrappleSwingPeriod();
            mGrappleJumpTimeout = 0.f;
            mAligningGrappleSwingTurn = parameters.GetLockSwingTurn();
          } else {
            const CMotionState& motion = PredictMotion(dt);
            CVector3f lookDirectionFlat = GetTransform().GetForward();
            CVector3f newPlayerToPoint =
                pointPosition - (GetTranslation() + motion.GetTranslation());
            lookDirectionFlat[kDZ] = 0.f;
            if (lookDirectionFlat.CanBeNormalized()) {
              lookDirectionFlat.Normalize();
            }
            newPlayerToPoint[kDZ] = 0.f;
            if (newPlayerToPoint.CanBeNormalized()) {
              newPlayerToPoint.Normalize();
              float cosAngle = CVector3f::Dot(lookDirectionFlat, newPlayerToPoint);
#if VERSION >= VERSION_R3IJ_00
              cosAngle = CMath::FastLimit(cosAngle, 1.f);
              const double lookToPointAngle = acosf(cosAngle);
#else
              cosAngle = CMath::Limit(cosAngle, 1.f);
              const double lookToPointAngle = acos(cosAngle);
#endif
              if (lookToPointAngle > 0.001f) {
                float deltaAngle = dt * gpTweakPlayer->GetGrappleLookCenterSpeed();
                if (lookToPointAngle >= deltaAngle) {
                  CVector3f leftDirection(lookDirectionFlat[1], -lookDirectionFlat[0], 0.f);
                  if (leftDirection.CanBeNormalized()) {
                    leftDirection.Normalize();
                  }
                  if (CVector3f::Dot(newPlayerToPoint, leftDirection) >= 0.f) {
                    deltaAngle = -deltaAngle;
                  }
                  RotateToOR(
                      CQuaternion::AxisAngle(CUnitVector3f(0.f, 0.f, 1.f, CUnitVector3f::kN_Yes),
                                             CRelAngle::FromRadians(deltaAngle)),
                      dt);
                } else if (fabs(lookToPointAngle - M_PI) > 0.001f) {
                  RotateToOR(CQuaternion::ShortestRotationArc(lookDirectionFlat, newPlayerToPoint),
                             dt);
                }
              } else {
                SetAngularVelocityWR(CAxisAngle::Identity());
                SetTorqueWR(CAxisAngle::Identity());
              }
            }
          }
        } else {
          mGrappleState = kGS_Swinging;
          mGrappleSwingTimer = 0.25f * gpTweakPlayer->GetGrappleSwingPeriod();
          mGrappleJumpTimeout = 0.f;
        }
      }
      break;
    }
    case kGS_Swinging: {
      float turnAngleSpeed = (M_PIF / 180.f) * gpTweakPlayer->GetMaxGrappleTurnSpeed();
      if (gpTweakPlayer->GetInvertGrappleTurn()) {
        turnAngleSpeed *= -1.f;
      }
      const CVector3f pointToPlayer = playerPosition - pointPosition;
#if VERSION >= VERSION_R3IJ_00
      const float pointToPlayerZProjection =
          CMath::FastLimit(CMath::AbsF(pointToPlayer.GetZ() / pointToPlayer.Magnitude()), 1.f);
#else
      const float pointToPlayerZProjection =
          CMath::Limit(CMath::AbsF(pointToPlayer.GetZ() / pointToPlayer.Magnitude()), 1.f);
#endif
      bool enableTurn = false;
      if (!parameters.GetLockSwingTurn()) {
#if VERSION >= VERSION_R3IJ_00
        if (ControlMapper().GetAnalogInput(CControlMapper::kC_StrafeLeft, input,
                                           CControlMapper::kFT_Filtered) > 0.05f) {
          enableTurn = true;
          turnAngleSpeed *= -ControlMapper().GetAnalogInput(CControlMapper::kC_StrafeLeft, input,
                                                            CControlMapper::kFT_Filtered);
        }
        if (ControlMapper().GetAnalogInput(CControlMapper::kC_StrafeRight, input,
                                           CControlMapper::kFT_Filtered) > 0.05f) {
          enableTurn = true;
          turnAngleSpeed *= ControlMapper().GetAnalogInput(CControlMapper::kC_StrafeRight, input,
                                                           CControlMapper::kFT_Filtered);
        }
#else
        if (ControlMapper::GetAnalogInput(ControlMapper::kC_TurnLeft, input) > 0.05f) {
          enableTurn = true;
          turnAngleSpeed *= -ControlMapper::GetAnalogInput(ControlMapper::kC_TurnLeft, input);
        }
        if (ControlMapper::GetAnalogInput(ControlMapper::kC_TurnRight, input) > 0.05f) {
          enableTurn = true;
          turnAngleSpeed *= ControlMapper::GetAnalogInput(ControlMapper::kC_TurnRight, input);
        }
#endif
      } else if (mAligningGrappleSwingTurn) {
        enableTurn = true;
      }
      mGrappleSwingTimer += dt;
      if (mGrappleSwingTimer > gpTweakPlayer->GetGrappleSwingPeriod()) {
        mGrappleSwingTimer -= gpTweakPlayer->GetGrappleSwingPeriod();
      }
      CVector3f swingAxis = mGrappleSwingAxis;
      if (mGrappleSwingTimer < 0.5f * gpTweakPlayer->GetGrappleSwingPeriod()) {
        swingAxis *= -1.f;
      }
      float swingCos =
          cosf(2.f * M_PIF * (mGrappleSwingTimer / gpTweakPlayer->GetGrappleSwingPeriod()) +
               M_PIF / 2.f);
#if VERSION >= VERSION_R3IJ_00
      swingCos = CMath::FastLimit(swingCos, 1.f);
#else
      swingCos = CMath::Limit(swingCos, 1.f);
#endif
      const float pullSpeed = CMath::AbsF(swingCos) * gpTweakPlayer->GetGrapplePullSpeedMin();
      CVector3f pullVector = pullSpeed * CVector3f::Cross(pointToPlayer.AsNormalized(), swingAxis);
      const float lengthError = pointToPlayer.Magnitude() - gpTweakPlayer->GetGrappleSwingLength();
#if VERSION >= VERSION_R3IJ_00
      const float lengthScale =
          CMath::FastLimit(lengthError / gpTweakPlayer->GetGrappleSwingLength(), 1.f);
#else
      const float lengthScale =
          CMath::Limit(lengthError / gpTweakPlayer->GetGrappleSwingLength(), 1.f);
#endif
      pullVector += pointToPlayerZProjection * (-32.f * (pointToPlayer * lengthScale));
      const CVector3f backupVelocity = GetVelocityWR();
      SetVelocityWR(pullVector);
      const CTransform4f backupTransform = GetTransform();
      const CMotionState& predictedMotion = PredictMotion(dt);
      const CVector3f translation = GetTranslation();
      if (ValidateFPPosition(translation + predictedMotion.GetTranslation(), mgr)) {
        if (enableTurn) {
          CQuaternion turnRotation =
              CQuaternion::ZRotation(CRelAngle::FromRadians(turnAngleSpeed * dt));
          if (parameters.GetLockSwingTurn() && mAligningGrappleSwingTurn) {
            CVector3f playerDirection = GetTransform().GetForward();
            CVector3f pointDirection = point->GetTransform().GetForward().AsNormalized();
            float playerPointProjection =
                CVector3f::Dot(playerDirection.AsNormalized(), pointDirection);
#if VERSION >= VERSION_R3IJ_00
            playerPointProjection = CMath::FastLimit(playerPointProjection, 1.f);
#else
            playerPointProjection = CMath::Limit(playerPointProjection, 1.f);
#endif
            if (CMath::AbsF(playerPointProjection) == 1.f) {
              mAligningGrappleSwingTurn = false;
            }
            if (playerPointProjection < 0.f) {
              pointDirection = -pointDirection;
              playerPointProjection = -playerPointProjection;
            }
            float turnAngle = acosf(playerPointProjection);
            playerDirection[kDZ] = 0.f;
            turnAngle *= dt;
            turnRotation = CQuaternion::LookAt(playerDirection.AsNormalized(), pointDirection,
                                               CRelAngle::FromRadians(turnAngle));
          }
          if (pointToPlayer.MagSquared() > 0.2f * 0.2f) {
            CVector3f pointAtPlayerHeight = pointPosition;
            pointAtPlayerHeight[kDZ] = playerPosition[kDZ];
            const CVector3f pointToPlayerFlat = playerPosition - pointAtPlayerHeight;
            const CVector3f playerToGrapplePlane =
                pointAtPlayerHeight + turnRotation.Transform(pointToPlayerFlat) - playerPosition;
            if (playerToGrapplePlane.CanBeNormalized()) {
              pullVector += (1.f / dt) * playerToGrapplePlane;
            }
          }
          const CVector3f backupSwingAxis = mGrappleSwingAxis;
          mGrappleSwingAxis = turnRotation.Transform(mGrappleSwingAxis);
          mGrappleSwingAxis.Normalize();
          const CVector3f swingForward(-mGrappleSwingAxis[kDY], mGrappleSwingAxis[kDX], 0.f);
          SetTransform(CTransform4f::FromColumns(mGrappleSwingAxis, swingForward,
                                                 CVector3f(0.f, 0.f, 1.f), GetTranslation()));
          SetVelocityWR(pullVector);
          if (!ValidateFPPosition(GetTranslation(), mgr)) {
            mGrappleSwingAxis = backupSwingAxis;
            SetTransform(backupTransform);
            SetVelocityWR(backupVelocity);
          }
        }
      } else {
        BreakGrapple(kOB_InvalidateTarget, mgr);
      }
      break;
    }
#if VERSION < VERSION_R3IJ_00
    case kGS_JumpOff: {
      ApplyForceOR(CVector3f(0.f, 0.f, GetGravity() * GetMass()), CAxisAngle::Identity());
      break;
    }
#endif
    default:
      break;
    }
  }
  SetAngularVelocityOR(
      CAxisAngle(CVector3f(0.f, 0.f, 0.9f * GetAngularVelocityOR().GetVector().GetZ())));
}

void CPlayer::UpdateGrappleArmTransform(const CVector3f& offset, CStateManager& mgr, float dt) {
  CTransform4f armXf = GetTransform();
  const CVector3f armPosition = GetTransform().Rotate(offset) + GetTranslation();
  armXf.SetTranslation(armPosition);
  if (mMorphBallState != kMS_Unmorphed) {
    mGun->GrappleArm().SetTransform(armXf);
  } else if (!mGun->GrappleArm().IsArmMoving()) {
    CVector3f lookDirection = GetTransform().GetForward();
    CVector3f armToTarget = mGun->GrappleArm().GetTransform().GetForward();
    if (lookDirection.CanBeNormalized()) {
      lookDirection.Normalize();
      if (mGrappleState != kGS_None) {
        if (const CActor* target =
                TCastToConstPtr< CActor >(mgr.GetObjectById(GetOrbitTargetId()))) {
          armToTarget = target->GetTranslation() - armPosition;
          CVector3f armToTargetFlat = armToTarget;
          armToTargetFlat.SetZ(0.f);
          if (armToTarget.CanBeNormalized()) {
            armToTarget.Normalize();
          }
          if (armToTargetFlat.CanBeNormalized() && mGrappleState != kGS_Firing) {
            const CQuaternion adjustment = CQuaternion::LookAt(
                armToTargetFlat.AsNormalized(), lookDirection, CRelAngle::FromRadians(2.f * M_PIF));
            armToTarget = adjustment.Transform(armToTarget);
            if (mGrappleSwingTimer >= 0.25f * gpTweakPlayer->GetGrappleSwingPeriod() &&
                mGrappleSwingTimer < 0.75f * gpTweakPlayer->GetGrappleSwingPeriod()) {
              armToTarget = mGun->GrappleArm().GetTransform().GetForward();
            }
          }
        }
      }
      armXf = CTransform4f::LookAt(CVector3f::Zero(), armToTarget, CVector3f::Up());
      armXf.SetTranslation(armPosition);
      mGun->GrappleArm().SetTransform(armXf);
    }
  }
}
