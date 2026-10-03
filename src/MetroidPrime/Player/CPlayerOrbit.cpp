#include "MetroidPrime/Player/CPlayer.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CControlMapper.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/Player/CPlayerCameraBob.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/Player/CGrappleArm.hpp"
#include "MetroidPrime/CAxisAngle.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "Collision/CCollidableAABox.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CWallCrawlerSwarm.hpp"
#include "MetroidPrime/Enemies/CThardusRockProjectile.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGunTurret.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDoor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGrapplePoint.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CRayCastResult.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "math.h"

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
  const CVector3f eyeToOrbit = orbitPosition - GetEyePosition();
  CVector3f eyeToOrbitFlat = eyeToOrbit;
  eyeToOrbitFlat.SetZ(0.f);
  if (eyeToOrbitFlat.CanBeNormalized() && eyeToOrbitFlat.Magnitude() > 1.f) {
    const float angle = static_cast< float >(
        asin(CMath::Limit(CMath::AbsF(eyeToOrbit.GetZ()) / eyeToOrbit.Magnitude(), 1.f)));
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

void CPlayer::UpdateOrbitOrientation(CStateManager& mgr) {
  if (mMorphBallState != kMS_Unmorphed) {
    return;
  }
  switch (mOrbitState) {
  case kOS_NoOrbit:
    return;
  case kOS_OrbitPoint:
    if (mInFreeLook) {
      return;
    }
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
  if (ControlMapper::GetPressInput(ControlMapper::kC_OrbitObject, input) &&
      mOrbitNextTargetId != kInvalidUniqueId) {
    SetOrbitTargetId(mOrbitNextTargetId, mgr);
    if (ValidateAimTargetId(GetOrbitTargetId(), mgr)) {
      SetAimTargetId(GetOrbitTargetId());
    }
    SetOrbitState(kOS_OrbitObject, mgr);
    UpdateOrbitPosition(gpTweakPlayer->GetOrbitNormalDistance(mOrbitType), mgr);
  }
}

void CPlayer::ActivateOrbitSource(CStateManager& mgr) {
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
  if (ControlMapper::GetDigitalInput(ControlMapper::kC_OrbitClose, input) ||
      ControlMapper::GetDigitalInput(ControlMapper::kC_OrbitFar, input) ||
      ControlMapper::GetDigitalInput(ControlMapper::kC_OrbitObject, input)) {
    switch (mOrbitState) {
    case kOS_NoOrbit:
      if (ControlMapper::GetPressInput(ControlMapper::kC_OrbitObject, input)) {
        SetOrbitTargetId(GetOrbitNextTargetId(), mgr);
        if (mOrbitTargetId != kInvalidUniqueId) {
          if (ValidateAimTargetId(GetOrbitTargetId(), mgr)) {
            SetAimTargetId(GetOrbitTargetId());
          }
          SetOrbitState(kOS_OrbitObject, mgr);
          UpdateOrbitPosition(gpTweakPlayer->GetOrbitNormalDistance(mOrbitType), mgr);
        }
      } else {
        if (ControlMapper::GetPressInput(ControlMapper::kC_OrbitFar, input)) {
          OrbitPoint(kOT_Far, mgr);
        }
        if (ControlMapper::GetPressInput(ControlMapper::kC_OrbitClose, input)) {
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
      if (ControlMapper::GetPressInput(ControlMapper::kC_OrbitObject, input)) {
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
          if (ControlMapper::GetDigitalInput(ControlMapper::kC_OrbitClose, input)) {
            mOrbitType = kOT_Close;
            SetOrbitPosition(gpTweakPlayer->GetOrbitNormalDistance(mOrbitType), mgr);
          }
          break;
        case kOT_Close:
          if (ControlMapper::GetDigitalInput(ControlMapper::kC_OrbitFar, input) &&
              !ControlMapper::GetDigitalInput(ControlMapper::kC_OrbitClose, input)) {
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
      if (ControlMapper::GetPressInput(ControlMapper::kC_OrbitObject, input)) {
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

void CPlayer::UpdateOrbitModeTimer(float dt) {
  if (mOrbitState == kOS_NoOrbit && mOrbitModeTimer > 0.f) {
    mOrbitModeTimer -= dt;
    return;
  }
  mOrbitModeTimer = 0.f;
}

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

bool CPlayer::WithinOrbitScreenEllipse(const CVector3f& screenCoords, EPlayerZoneInfo zone) const {
  if (screenCoords.GetZ() >= 1.f) {
    return false;
  }
  const float x =
      CMath::AbsF(screenCoords.GetX() - CCast::LtoF(gpTweakPlayer->GetOrbitZoneCentreX(zone)));
  const float heYSq = CCast::LtoF(gpTweakPlayer->GetOrbitZoneHeight(zone) *
                                  gpTweakPlayer->GetOrbitZoneHeight(zone));
  const float heXSq =
      CCast::LtoF(gpTweakPlayer->GetOrbitZoneWidth(zone) * gpTweakPlayer->GetOrbitZoneWidth(zone));
  const float y =
      CMath::AbsF(screenCoords.GetY() - CCast::LtoF(gpTweakPlayer->GetOrbitZoneCentreY(zone)));
  const bool inside = x * x <= (1.f - y * y / heYSq) * heXSq;
  return inside;
}

bool CPlayer::WithinOrbitScreenBox(const CVector3f& screenCoords, EPlayerZoneInfo zone,
                                   EPlayerZoneType type) const {
  if (screenCoords.GetZ() >= 1.f) {
    return false;
  }
  switch (type) {
  case kZT_Box:
    if (CMath::AbsF(screenCoords.GetX() - CCast::LtoF(gpTweakPlayer->GetOrbitZoneCentreX(zone))) <=
            CCast::LtoF(gpTweakPlayer->GetOrbitZoneWidth(zone)) &&
        CMath::AbsF(screenCoords.GetY() - CCast::LtoF(gpTweakPlayer->GetOrbitZoneCentreY(zone))) <=
            CCast::LtoF(gpTweakPlayer->GetOrbitZoneHeight(zone)) &&
        screenCoords.GetZ() < 1.f) {
      return true;
    }
    break;
  case kZT_Ellipse:
    return WithinOrbitScreenEllipse(screenCoords, zone);
  default:
    return true;
  }
  return false;
}

void CPlayer::FindOrbitableObjects(const rstl::reserved_vector< TUniqueId, 1024 >& nearObjects,
                                   rstl::vector< TUniqueId >& listOut, EPlayerZoneInfo zone,
                                   EPlayerZoneType type, CStateManager& mgr,
                                   bool onScreenTest) const {
  const CVector3f eyePosition = GetEyePosition();
  CVector3f forward = GetTransform().GetForward();
  forward.Normalize();
  const CFirstPersonCamera* const fpCamera = mgr.GetCameraManager()->GetFirstPersonCamera();
  for (AUTO(it, nearObjects.begin()); it != nearObjects.end(); ++it) {
    const CActor* const act = TCastToConstPtr< CActor >(mgr.GetObjectById(*it));
    if (act) {
      if (act->GetUniqueId() == GetUniqueId()) {
        continue;
      }
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
      if (onScreenTest && WithinOrbitScreenBox(screenPosition, zone, type)) {
        pass = true;
      } else if (!onScreenTest && !WithinOrbitScreenBox(screenPosition, zone, type)) {
        pass = true;
      }
      if (pass) {
        const CVector3f eyeToOrbit = orbitPosition - eyePosition;
        const float distance = eyeToOrbit.Magnitude();
        if (!act->GetDoTargetDistanceTest() || distance <= GetOrbitMaxTargetDistance(mgr)) {
          if (listOut.size() != listOut.capacity()) {
            listOut.push_back(act->GetUniqueId());
          }
        }
      }
    }
  }
}

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
  const CVector3f eyePosition = GetEyePosition();
  CVector3f forward = GetTransform().GetForward();
  forward.Normalize();
  float minDistance = 10000.f;
  float minScreenDistanceSq = 10000.f;
  TUniqueId bestId = kInvalidUniqueId;
  const int viewportHalfX = CGraphics::GetViewportWidth() / 2;
  const int idealX = gpTweakPlayer->GetOrbitZoneIdealX(zone);
  const int viewportHalfY = CGraphics::GetViewportHeight() / 2;
  const int idealY = gpTweakPlayer->GetOrbitZoneIdealY(zone);
  float boxLeft = CCast::LtoF(idealX) - CCast::LtoF(viewportHalfX);
  boxLeft /= CCast::LtoF(viewportHalfX);
  float boxTop = CCast::LtoF(idealY) - CCast::LtoF(viewportHalfY);
  boxTop /= CCast::LtoF(viewportHalfY);
  const CFirstPersonCamera* const fpCamera = mgr.GetCameraManager()->GetFirstPersonCamera();
  for (AUTO(it, ids.begin()); it != ids.end(); ++it) {
    const CActor* act = static_cast< const CActor* >(mgr.GetObjectById(*it));
    if (act && act->GetUniqueId() != GetUniqueId() &&
        ValidateObjectForMode(act->GetUniqueId(), mgr)) {
      const CVector3f aimPosition = act->GetAimPosition(mgr, 0.f);
      CVector3f positionInBox = fpCamera->ConvertToScreenSpace(aimPosition);
      positionInBox.SetX(positionInBox.GetX() * CCast::LtoF(CGraphics::GetViewportWidth()) / 2.f +
                         CCast::LtoF(CGraphics::GetViewportWidth()) / 2.f);
      positionInBox.SetY(positionInBox.GetY() * CCast::LtoF(CGraphics::GetViewportHeight()) / 2.f +
                         CCast::LtoF(CGraphics::GetViewportHeight()) / 2.f);
      if (WithinOrbitScreenBox(positionInBox, zone, type)) {
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
              const float screenX = positionInBox.GetX() - boxLeft;
              const float screenY = positionInBox.GetY() - boxTop;
              const float screenXSq = screenX * screenX;
              const float screenYSq = screenY * screenY;
              minDistance = distance;
              minScreenDistanceSq = screenXSq + screenYSq;
            }
          } else if (CMath::AbsF(distance - minDistance) <
                     gpTweakPlayer->GetAimThresholdDistance()) {
            const float screenX = positionInBox.GetX() - boxLeft;
            const float screenY = positionInBox.GetY() - boxTop;
            const float screenXSq = screenX * screenX;
            const float screenYSq = screenY * screenY;
            const float screenDistanceSq = screenXSq + screenYSq;
            if (screenDistanceSq < minScreenDistanceSq) {
              TEntityList nearList;
              TUniqueId intersectId = kInvalidUniqueId;
              mgr.BuildNearList(nearList, eyePosition, eyeToAim.Normalize(), distance,
                                kOccluderFilter, act);
              const CRayCastResult result =
                  mgr.RayWorldIntersection(intersectId, eyePosition, eyeToAim.Normalize(), distance,
                                           kLineOfSightFilter, nearList);
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
  if (act->GetMaterialList().HasMaterial(kMT_Target) && id != kInvalidUniqueId &&
      ValidateObjectForMode(id, mgr)) {
    const CVector3f aimPosition = act->GetAimPosition(mgr, 0.f);
    const CVector3f eyePosition = GetEyePosition();
    CVector3f eyeToAim = aimPosition - eyePosition;
    const CVector3f screenPosition =
        mgr.GetCameraManager()->GetFirstPersonCamera()->ConvertToScreenSpace(aimPosition);
    CVector3f positionInBox = screenPosition;
    positionInBox.SetX(positionInBox.GetX() * CCast::LtoF(CGraphics::GetViewportWidth()) / 2.f +
                       CCast::LtoF(CGraphics::GetViewportWidth()) / 2.f);
    positionInBox.SetY(positionInBox.GetY() * CCast::LtoF(CGraphics::GetViewportHeight()) / 2.f +
                       CCast::LtoF(CGraphics::GetViewportHeight()) / 2.f);
    if (WithinOrbitScreenBox(positionInBox, mOrbitZoneMode, mOrbitZoneType) ||
        (mOrbitZoneMode != kZI_Targeting &&
         WithinOrbitScreenBox(positionInBox, kZI_Targeting, mOrbitZoneType))) {
      const float distance = eyeToAim.Magnitude();
      if (distance <= gpTweakPlayer->GetAimMaxDistance()) {
        TEntityList nearList;
        TUniqueId intersectId = kInvalidUniqueId;
        mgr.BuildNearList(nearList, eyePosition, eyeToAim.Normalize(), distance, kOccluderFilter,
                          nullptr);
        const CRayCastResult result = mgr.RayWorldIntersection(
            intersectId, eyePosition, eyeToAim.Normalize(), distance, kLineOfSightFilter, nearList);
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
  SetAimTargetId(kInvalidUniqueId);
  mAimTargetTimer = 0.f;
  return false;
}

void CPlayer::UpdateAimTargetTimer(float dt) {
  if (GetAimTargetId() == kInvalidUniqueId) {
    return;
  }
  if (mAimTargetTimer > 0.f) {
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
  if (gpTweakPlayer->GetAimWhenOrbitingPoint()) {
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
  } else if (mOrbitState == kOS_NoOrbit) {
    needsReset = true;
  }
  if (needsReset && !ValidateAimTargetId(GetAimTargetId(), mgr)) {
    if (act && ValidateObjectForMode(GetAimTargetId(), mgr)) {
      SetAimTargetId(kInvalidUniqueId);
    } else {
      SetAimTargetId(FindAimTargetId(mgr));
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
  float dot = CVector3f::Dot(flatForward.AsNormalized(), cameraXf.GetForward());
  dot = CMath::Limit(dot, 1.f);
  const CVector3f orbitVector(0.f, distance / dot, 0.f);
  mOrbitPoint = cameraXf.GetTranslation() + cameraXf.Rotate(orbitVector);
  mOrbitVector =
      CVector3f(0.f, distance, mOrbitPoint.GetZ() - cameraXf.GetTranslation().GetZ());
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
  switch (mOrbitState) {
  case kOS_NoOrbit:
    break;
  case kOS_OrbitPoint:
  case kOS_OrbitCarcass:
    SetOrbitPosition(distance, mgr);
    break;
  case kOS_ForcedOrbitObject:
  case kOS_Grapple:
  case kOS_OrbitObject:
    if (const CActor* act = TCastToConstPtr< CActor >(mgr.GetObjectById(GetOrbitTargetId()))) {
      if (mOrbitTargetId != kInvalidUniqueId) {
        mOrbitPoint = act->GetOrbitPosition(mgr);
      }
    }
    break;
  default:
    break;
  }
}

void CPlayer::SetOrbitTargetId(TUniqueId id, CStateManager& mgr) {
  if (id != kInvalidUniqueId) {
    const CPatterned* patterned = TCastToConstPtr< CPatterned >(mgr.GetObjectById(id));
    const CWallCrawlerSwarm* swarm = TCastToConstPtr< CWallCrawlerSwarm >(mgr.GetObjectById(id));
    const CThardusRockProjectile* rock = PATTERNED_CAST_TO(CThardusRockProjectile, const_cast< CEntity* >(mgr.GetObjectById(id)));
    const CScriptGunTurret* turret = TCastToConstPtr< CScriptGunTurret >(mgr.GetObjectById(id));
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
    CVector3f playerToPoint = mOrbitPoint - GetTranslation();
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
  float distance = gpTweakPlayer->GetOrbitMinDistance(type);
  distance *= CMath::Clamp(
      1.f, CMath::AbsF(mOrbitPoint.GetZ() - GetTranslation().GetZ()) / 20.f, maxScale);
  return distance;
}

void CPlayer::OrbitPoint(EPlayerOrbitType type, CStateManager& mgr) {
  mOrbitType = type;
  SetOrbitState(kOS_OrbitPoint, mgr);
  SetOrbitPosition(gpTweakPlayer->GetOrbitNormalDistance(mOrbitType), mgr);
}

void CPlayer::OrbitCarcass(CStateManager& mgr) {
  if (mOrbitState == kOS_OrbitObject) {
    mOrbitType = kOT_Default;
    SetOrbitState(kOS_OrbitCarcass, mgr);
  }
}

void CPlayer::PreventFallingCameraPitch() {
  mJumpCameraTimer = 0.f;
  mFallCameraTimer = 0.01f;
  mCancelCameraPitch = true;
}

bool CPlayer::CheckPostGrapple() const {
  if (mMovementState != NPlayer::kMS_OnGround &&
      (mGrappleJumpTimeout > 0.f || mJumpCameraTimer == 0.f)) {
    return true;
  }
  return false;
}

void CPlayer::TryToBreakOrbit(TUniqueId id, EOrbitBrokenType type, CStateManager& mgr) {
  if (mOrbitState == kOS_OrbitObject || mOrbitState == kOS_Grapple ||
      mOrbitState == kOS_ForcedOrbitObject) {
    if (id == mOrbitTargetId) {
      BreakOrbit(type, mgr);
    }
  }
}

void CPlayer::BreakOrbit(EOrbitBrokenType type, CStateManager& mgr) {
  mOrbitBrokenType = type;
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
}

void CPlayer::BreakGrapple(EOrbitBrokenType type, CStateManager& mgr) {
  mJumpCameraTimer = 0.f;
  mFallCameraTimer = 0.f;
  if (static_cast< int >(gpTweakPlayer->GetGrappleJumpMode()) == 2 &&
      mGrappleState == kGS_Swinging) {
    ApplyGrappleJump(mgr);
    PreventFallingCameraPitch();
  }
  BreakOrbit(type, mgr);
  mGrappleState = kGS_None;
  AddMaterial(kMT_GroundCollider, mgr);
  mGun->GrappleArm().SetAnimState(CGrappleArm::kAS_OutOfGrapple);
  if (!CheckPostGrapple() && mGrappleState != kGS_JumpOff) {
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
  const CVector3f pointToPlayer = GetTranslation() - point->GetTranslation();
  const CVector3f cross = CVector3f::Cross(pointToPlayer.AsNormalized(), swingAxis);
  CVector3f pointToPlayerFlat = pointToPlayer;
  pointToPlayerFlat.SetZ(0.f);
  float dot = 1.f;
  if (pointToPlayerFlat.CanBeNormalized() && cross.CanBeNormalized()) {
    float cosAngle =
        CMath::AbsF(CVector3f::Dot(cross.AsNormalized(), pointToPlayerFlat.AsNormalized()));
    cosAngle = CMath::Limit(cosAngle, 1.f);
    dot = cosAngle;
  }
  const CVector3f force = dot * (10000.f * (gpTweakPlayer->GetGrappleJumpForce() * cross));
  ApplyForceWR(force, CAxisAngle::Identity());
}

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

bool CPlayer::ValidateFPPosition(CVector3f position, CStateManager& mgr) {
  TEntityList nearList;
  const CMaterialFilter solidFilter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
  const CVector3f margin(1.f, 1.f, 1.f);
  mgr.BuildColliderList(nearList, *this,
                        CAABox(mFpBounds.GetMinPoint() - margin + position,
                               mFpBounds.GetMaxPoint() + margin + position));
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
  const CVector3f playerPosition = GetTranslation();
  if (const CScriptGrapplePoint* point =
          TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(GetOrbitTargetId()))) {
    const CVector3f pointPosition = point->GetTranslation();
    switch (mGrappleState) {
    case kGS_Pull: {
      const CVector3f swingLow =
          pointPosition + CVector3f(0.f, 0.f, -gpTweakPlayer->GetGrappleSwingLength());
      const CVector3f playerToPoint = pointPosition - GetTranslation();
      if (playerToPoint.CanBeNormalized()) {
        CVector3f playerToSwingLow = swingLow - playerPosition;
        if (playerToSwingLow.CanBeNormalized()) {
          const float distanceToLow = playerToSwingLow.Magnitude();
          playerToSwingLow.Normalize();
          const float timeToLow =
              CMath::Limit(distanceToLow / gpTweakPlayer->GetGrapplePullSpeedProportion(), 1.f);
          const float pullSpeed = timeToLow * (gpTweakPlayer->GetGrapplePullSpeedMax() -
                                               gpTweakPlayer->GetGrapplePullSpeedMin()) +
                                  gpTweakPlayer->GetGrapplePullSpeedMin();
          const CVector3f pullVelocity = pullSpeed * playerToSwingLow;
          SetVelocityWR(pullVelocity);
          if (distanceToLow < gpTweakPlayer->GetMaxGrappleLockedTurnAlignDistance()) {
            mGrappleState = kGS_Swinging;
            mGrappleSwingTimer = 0.25f * gpTweakPlayer->GetGrappleSwingPeriod();
            mGrappleJumpTimeout = 0.f;
            mAligningGrappleSwingTurn = point->GetGrappleParameters().GetLockSwingTurn();
          } else {
            const CMotionState& motion = PredictMotion(dt);
            CVector3f lookDirectionFlat = GetTransform().GetForward();
            CVector3f newPlayerToPoint =
                pointPosition - (GetTranslation() + motion.GetTranslation());
            lookDirectionFlat.SetZ(0.f);
            if (lookDirectionFlat.CanBeNormalized()) {
              lookDirectionFlat.Normalize();
            }
            newPlayerToPoint.SetZ(0.f);
            if (newPlayerToPoint.CanBeNormalized()) {
              newPlayerToPoint.Normalize();
              float cosAngle = CVector3f::Dot(lookDirectionFlat, newPlayerToPoint);
              cosAngle = CMath::Limit(cosAngle, 1.f);
              const double lookToPointAngle = acos(cosAngle);
              if (lookToPointAngle > 0.001f) {
                float deltaAngle = dt * gpTweakPlayer->GetGrappleLookCenterSpeed();
                if (lookToPointAngle >= deltaAngle) {
                  CVector3f leftDirection(lookDirectionFlat.GetY(), -lookDirectionFlat.GetX(), 0.f);
                  if (leftDirection.CanBeNormalized()) {
                    leftDirection.Normalize();
                  }
                  if (CVector3f::Dot(newPlayerToPoint, leftDirection) >= 0.f) {
                    deltaAngle = -deltaAngle;
                  }
                  RotateToOR(
                      CQuaternion::AxisAngle(CUnitVector3f(0.f, 0.f, 1.f, CUnitVector3f::kN_Yes),
                                             CRelAngle(deltaAngle)),
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
      const float pointToPlayerZProjection =
          CMath::Limit(CMath::AbsF(pointToPlayer.GetZ() / pointToPlayer.Magnitude()), 1.f);
      bool enableTurn = false;
      if (!point->GetGrappleParameters().GetLockSwingTurn()) {
        if (ControlMapper::GetAnalogInput(ControlMapper::kC_TurnLeft, input) > 0.05f) {
          enableTurn = true;
          turnAngleSpeed *= -ControlMapper::GetAnalogInput(ControlMapper::kC_TurnLeft, input);
        }
        if (ControlMapper::GetAnalogInput(ControlMapper::kC_TurnRight, input) > 0.05f) {
          enableTurn = true;
          turnAngleSpeed *= ControlMapper::GetAnalogInput(ControlMapper::kC_TurnRight, input);
        }
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
      swingCos = CMath::Limit(swingCos, 1.f);
      const float pullSpeed = CMath::AbsF(swingCos) * gpTweakPlayer->GetGrapplePullSpeedMin();
      CVector3f pullVector = pullSpeed * CVector3f::Cross(pointToPlayer.AsNormalized(), swingAxis);
      const float lengthError = pointToPlayer.Magnitude() - gpTweakPlayer->GetGrappleSwingLength();
      const float lengthScale =
          CMath::Limit(lengthError / gpTweakPlayer->GetGrappleSwingLength(), 1.f);
      pullVector += pointToPlayerZProjection * (-32.f * (lengthScale * pointToPlayer));
      const CVector3f backupVelocity = GetVelocityWR();
      SetVelocityWR(pullVector);
      const CTransform4f backupTransform = GetTransform();
      const CMotionState& predictedMotion = PredictMotion(dt);
      const CVector3f translation = GetTranslation();
      if (ValidateFPPosition(translation + predictedMotion.GetTranslation(), mgr)) {
        if (enableTurn) {
          CQuaternion turnRotation = CQuaternion::ZRotation(CRelAngle(turnAngleSpeed * dt));
          if (point->GetGrappleParameters().GetLockSwingTurn() &&
              mAligningGrappleSwingTurn) {
            CVector3f playerDirection = GetTransform().GetForward();
            CVector3f pointDirection = point->GetTransform().GetForward().AsNormalized();
            float playerPointProjection =
                CVector3f::Dot(playerDirection.AsNormalized(), pointDirection);
            playerPointProjection = CMath::Limit(playerPointProjection, 1.f);
            if (CMath::AbsF(playerPointProjection) == 1.f) {
              mAligningGrappleSwingTurn = false;
            }
            if (playerPointProjection < 0.f) {
              playerPointProjection = -playerPointProjection;
              pointDirection = -pointDirection;
            }
            float turnAngle = acosf(playerPointProjection);
            playerDirection.SetZ(0.f);
            turnAngle *= dt;
            turnRotation = CQuaternion::LookAt(playerDirection.AsNormalized(), pointDirection,
                                               CRelAngle(turnAngle));
          }
          if (pointToPlayer.MagSquared() > 0.2f * 0.2f) {
            const CVector3f pointAtPlayerHeight(pointPosition.GetX(), pointPosition.GetY(),
                                                playerPosition.GetZ());
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
          const CVector3f swingForward(-mGrappleSwingAxis.GetY(), mGrappleSwingAxis.GetX(),
                                       0.f);
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
    case kGS_JumpOff: {
      ApplyForceOR(CVector3f(0.f, 0.f, GetGravity() * GetMass()), CAxisAngle::Identity());
      break;
    }
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
                armToTargetFlat.AsNormalized(), lookDirection, CRelAngle(2.f * M_PIF));
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
