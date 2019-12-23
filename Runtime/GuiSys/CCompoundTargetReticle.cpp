#include "Runtime/GuiSys/CCompoundTargetReticle.hpp"

#include <cstdlib>

#include "CTimeProvider.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/Camera/CGameCamera.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptGrapplePoint.hpp"
#include "Runtime/World/CWorld.hpp"
#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
namespace {
constexpr char skCrosshairsReticleAssetName[] = "CMDL_Crosshairs";
[[maybe_unused]] constexpr char skOrbitZoneReticleAssetName[] = "CMDL_OrbitZone";
constexpr char skSeekerAssetName[] = "CMDL_Seeker";
constexpr char skLockConfirmAssetName[] = "CMDL_LockConfirm";
constexpr char skTargetFlowerAssetName[] = "CMDL_TargetFlower";
constexpr char skMissileBracketAssetName[] = "CMDL_MissileBracket";
constexpr char skChargeGaugeAssetName[] = "CMDL_ChargeGauge";
constexpr char skChargeBeamTickAssetName[] = "CMDL_ChargeTickFirst";
constexpr char skOuterBeamIconSquareNameBase[] = "CMDL_BeamSquare";
constexpr char skInnerBeamIconName[] = "CMDL_InnerBeamIcon";
constexpr char skLockFireAssetName[] = "CMDL_LockFire";
constexpr char skLockDaggerAssetName[] = "CMDL_LockDagger0";
constexpr char skGrappleReticleAssetName[] = "CMDL_Grapple";
constexpr char skXRayRingModelName[] = "CMDL_XRayRetRing";
constexpr char skThermalReticleAssetName[] = "CMDL_ThermalRet";
[[maybe_unused]] constexpr char skOrbitPointAssetName[] = "CMDL_OrbitPoint";
// RS5
constexpr char skCombatAimingCenter[] = "CMDL_CombatAimingCenter";
constexpr char skCombatAimingArm[] = "CMDL_CombatAimingArm";
constexpr char skOrbitLockArm[] = "CMDL_OrbitLockArm";
constexpr char skOrbitLockTech[] = "CMDL_OrbitLockTech";
constexpr char skOrbitLockBrackets[] = "CMDL_OrbitLockBrackets";
constexpr char skOrbitLockBase[] = "CMDL_OrbitLockBase";
constexpr char skOffScreen[] = "CMDL_OffScreen";
constexpr char skScanReticleRing[] = "CMDL_ScanReticleRing";
constexpr char skScanReticleBracket[] = "CMDL_ScanReticleBracket";
constexpr char skScanReticleProgress[] = "CMDL_ScanReticleProgress";

float offshoot_func(float f1, float f2, float f3) { return (f1 * 0.5f) + std::sin((f3 - 0.5f) * f2); }

float calculate_premultiplied_overshoot_offset(float f1) { return 2.f * (M_PIF - std::asin(1.f / f1)); }
} // Anonymous namespace

const CTargetReticleRenderState CTargetReticleRenderState::skZeroRenderState(kInvalidUniqueId, 1.f, zeus::skZero3f, 0.f,
                                                                             1.f, true);

CCompoundTargetReticle::SOuterItemInfo::SOuterItemInfo(std::string_view res) : x0_model(g_SimplePool->GetObj(res)) {}

CCompoundTargetReticle::CCompoundTargetReticle(const CStateManager& mgr)
: x0_leadingOrientation(mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTransform().buildMatrix3f())
, x10_laggingOrientation(mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTransform().buildMatrix3f())
, x2c_overshootOffsetHalf(0.5f * g_tweakTargeting->GetChargeGaugeOvershootOffset())
, x30_premultOvershootOffset(
      calculate_premultiplied_overshoot_offset(g_tweakTargeting->GetChargeGaugeOvershootOffset()))
, x34_crosshairs(g_SimplePool->GetObj(skCrosshairsReticleAssetName))
, x40_seeker(g_SimplePool->GetObj(skSeekerAssetName))
, x4c_lockConfirm(g_SimplePool->GetObj(skLockConfirmAssetName))
, x58_targetFlower(g_SimplePool->GetObj(skTargetFlowerAssetName))
, x64_missileBracket(g_SimplePool->GetObj(skMissileBracketAssetName))
, x70_innerBeamIcon(g_SimplePool->GetObj(skInnerBeamIconName))
, x7c_lockFire(g_SimplePool->GetObj(skLockFireAssetName))
, x88_lockDagger(g_SimplePool->GetObj(skLockDaggerAssetName))
, x94_grapple(g_SimplePool->GetObj(skGrappleReticleAssetName))
, xa0_chargeTickFirst(g_SimplePool->GetObj(skChargeBeamTickAssetName))
, xac_xrayRetRing(g_SimplePool->GetObj(skXRayRingModelName))
, xb8_thermalReticle(g_SimplePool->GetObj(skThermalReticleAssetName))
, xc4_chargeGauge(skChargeGaugeAssetName)
, xf4_targetPos(CalculateOrbitZoneReticlePosition(mgr, false))
, x100_laggingTargetPos(CalculateOrbitZoneReticlePosition(mgr, true))
, x208_lockonTimer(g_tweakTargeting->GetLockonDuration())
, x38c_combatAimingCenter(g_SimplePool->GetObj(skCombatAimingCenter))
, x398_combatAimingArm(g_SimplePool->GetObj(skCombatAimingArm))
, x3a4_orbitLockArm(g_SimplePool->GetObj(skOrbitLockArm))
, x3b0_orbitLockTech(g_SimplePool->GetObj(skOrbitLockTech))
, x3bc_orbitLockBrackets(g_SimplePool->GetObj(skOrbitLockBrackets))
, x3c8_orbitLockBase(g_SimplePool->GetObj(skOrbitLockBase))
, x3d4_offScreen(g_SimplePool->GetObj(skOffScreen))
, x3e0_scanReticleRing(g_SimplePool->GetObj(skScanReticleRing))
, x3ec_scanReticleBracket(g_SimplePool->GetObj(skScanReticleBracket))
, x3f8_scanReticleProgress(g_SimplePool->GetObj(skScanReticleProgress)) {
  xe0_outerBeamIconSquares.reserve(9);
  for (u32 i = 0; i < 9; ++i)
    xe0_outerBeamIconSquares.emplace_back(fmt::format(fmt("{}{}"), skOuterBeamIconSquareNameBase, i));
  x34_crosshairs.Lock();
  std::fill(std::begin(x444_), std::end(x444_), kInvalidUniqueId);
}

CCompoundTargetReticle::SScanReticuleRenderer::SScanReticuleRenderer() {
  CGraphics::CommitResources([this](boo::IGraphicsDataFactory::Context& ctx) {
    for (int i = 0; i < 2; ++i) {
      m_lineRenderers[i].emplace(ctx, CLineRenderer::EPrimitiveMode::Lines, 8, nullptr, true, true);
      for (int j = 0; j < 4; ++j)
        m_stripRenderers[i][j].emplace(ctx, CLineRenderer::EPrimitiveMode::LineStrip, 4, nullptr, true, true);
    }
    return true;
  } BooTrace);
}

CCompoundTargetReticle::EReticleState CCompoundTargetReticle::GetDesiredReticleState(const CStateManager& mgr) const {
  switch (mgr.GetPlayerState()->GetCurrentVisor()) {
  case CPlayerState::EPlayerVisor::Scan:
    return EReticleState::Scan;
  case CPlayerState::EPlayerVisor::XRay:
    return EReticleState::XRay;
  case CPlayerState::EPlayerVisor::Combat:
  default:
    return EReticleState::Combat;
  case CPlayerState::EPlayerVisor::Thermal:
    return EReticleState::Thermal;
  }
}

void CCompoundTargetReticle::Update(float dt, const CStateManager& mgr) {
  float angle = x10_laggingOrientation.angleFrom(x0_leadingOrientation).asDegrees();
  float t;
  if (angle < 0.1f || angle > 45.f)
    t = 1.f;
  else
    t = std::min(1.f, g_tweakTargeting->GetAngularLagSpeed() * dt / angle);
  x10_laggingOrientation =
      t == 1.f ? x0_leadingOrientation : zeus::CQuaternion::slerp(x10_laggingOrientation, x0_leadingOrientation, t);
  xf4_targetPos = CalculateOrbitZoneReticlePosition(mgr, false);
  x100_laggingTargetPos = CalculateOrbitZoneReticlePosition(mgr, true);
  UpdateNewGroup4(dt, mgr);
  UpdateNewGroup5(dt, mgr);
  UpdateNewGroup6(dt, mgr);
  UpdateNextLockOnGroupRS5(dt, mgr);
  UpdateCurrLockOnGroupRS5(dt, mgr);
  UpdateNewGroup3(dt, mgr);
  UpdateCurrLockOnGroup(dt, mgr);
  UpdateNextLockOnGroup(dt, mgr);
  UpdateOrbitZoneGroup(dt, mgr);
  EReticleState desiredState = GetDesiredReticleState(mgr);
  if (desiredState != x20_prevState && x20_prevState == x24_nextState) {
    x24_nextState = desiredState;
    x28_noDrawTicks = 2;
  }
  if (x20_prevState != x24_nextState && x28_noDrawTicks <= 0) {
    x20_prevState = x24_nextState;
    float combat = false;
    float scan = false;
    float xray = false;
    float thermal = false;
    switch (x24_nextState) {
    case EReticleState::Combat:
      combat = true;
      break;
    case EReticleState::Scan:
      scan = true;
      break;
    case EReticleState::XRay:
      xray = true;
      break;
    case EReticleState::Thermal:
      thermal = true;
      break;
    default:
      break;
    }
    if (combat)
      x40_seeker.Lock();
    else
      x40_seeker.Unlock();
    if (combat)
      x4c_lockConfirm.Lock();
    else
      x4c_lockConfirm.Unlock();
    if (combat)
      x58_targetFlower.Lock();
    else
      x58_targetFlower.Unlock();
    if (combat)
      x64_missileBracket.Lock();
    else
      x64_missileBracket.Unlock();
    if (combat)
      x70_innerBeamIcon.Lock();
    else
      x70_innerBeamIcon.Unlock();
    if (combat)
      x7c_lockFire.Lock();
    else
      x7c_lockFire.Unlock();
    if (combat)
      x88_lockDagger.Lock();
    else
      x88_lockDagger.Unlock();
    if (combat)
      xa0_chargeTickFirst.Lock();
    else
      xa0_chargeTickFirst.Unlock();
    if (xray)
      xac_xrayRetRing.Lock();
    else
      xac_xrayRetRing.Unlock();
    if (thermal)
      xb8_thermalReticle.Lock();
    else
      xb8_thermalReticle.Unlock();
    if (combat)
      xc4_chargeGauge.x0_model.Lock();
    else
      xc4_chargeGauge.x0_model.Unlock();
    if (!scan)
      x94_grapple.Lock();
    else
      x94_grapple.Unlock();
    for (SOuterItemInfo& info : xe0_outerBeamIconSquares) {
      if (combat)
        info.x0_model.Lock();
      else
        info.x0_model.Unlock();
    }
  }
  CPlayerGun* gun = mgr.GetPlayer().GetPlayerGun();
  bool fullyCharged = (gun->IsCharging() ? gun->GetChargeBeamFactor() : 0.f) >= 1.f;
  if (fullyCharged != x21a_fullyCharged)
    x21a_fullyCharged = fullyCharged;
  if (x21a_fullyCharged)
    x214_fullChargeFadeTimer = std::min(dt / g_tweakTargeting->GetFullChargeFadeDuration() + x214_fullChargeFadeTimer,
                                        g_tweakTargeting->GetFullChargeFadeDuration());
  else
    x214_fullChargeFadeTimer =
        std::max(x214_fullChargeFadeTimer - dt / g_tweakTargeting->GetFullChargeFadeDuration(), 0.f);
  bool missileActive = gun->GetMissleMode() == CPlayerGun::EMissleMode::Active;
  if (missileActive != x1f4_missileActive) {
    if (x1f8_missileBracketTimer != 0.f)
      x1f8_missileBracketTimer = FLT_EPSILON - x1f8_missileBracketTimer;
    else
      x1f8_missileBracketTimer = FLT_EPSILON;
    x1f4_missileActive = missileActive;
  }
  CPlayerState::EBeamId beam = gun->GetCurrentBeam();
  if (beam != x200_beam) {
    x204_chargeGaugeOvershootTimer = g_tweakTargeting->GetChargeGaugeOvershootDuration();
    for (int i = 0; i < 9; ++i) {
      zeus::CRelAngle baseAngle = g_tweakTargeting->GetOuterBeamSquareAngles(int(beam))[i];
      baseAngle.makeRel();
      SOuterItemInfo& icon = xe0_outerBeamIconSquares[i];
      zeus::CRelAngle offshootAngleDelta = baseAngle.asRadians() - icon.x10_rotAng;
      if ((i & 0x1) == 1)
        offshootAngleDelta =
            (baseAngle > 0.f) ? zeus::CRelAngle(-2.f * M_PIF - baseAngle) : zeus::CRelAngle(2.f * M_PIF + baseAngle);
      icon.xc_offshootBaseAngle = icon.x10_rotAng;
      icon.x18_offshootAngleDelta = offshootAngleDelta;
      icon.x14_baseAngle = baseAngle;
    }
    zeus::CRelAngle baseAngle = g_tweakTargeting->GetChargeGaugeAngle(int(beam));
    baseAngle.makeRel();
    float offshootAngleDelta = baseAngle.asRadians() - xc4_chargeGauge.x10_rotAng;
    if ((rand() & 0x1) == 1)
      offshootAngleDelta =
          (offshootAngleDelta > 0.f) ? -2.f * M_PIF - offshootAngleDelta : 2.f * M_PIF + offshootAngleDelta;
    xc4_chargeGauge.xc_offshootBaseAngle = xc4_chargeGauge.x10_rotAng;
    xc4_chargeGauge.x18_offshootAngleDelta = offshootAngleDelta;
    xc4_chargeGauge.x14_baseAngle = baseAngle;
    x200_beam = beam;
    x208_lockonTimer = 0.f;
  }
  if (gun->GetLastFireButtonStates() & 0x1) {
    if (!x218_beamShot)
      x210_lockFireTimer = g_tweakTargeting->GetLockFireDuration();
    x218_beamShot = true;
  } else {
    x218_beamShot = false;
  }
  if (gun->GetLastFireButtonStates() & 0x2) {
    if (!x219_missileShot)
      x1fc_missileBracketScaleTimer = g_tweakTargeting->GetMissileBracketScaleDuration();
    x219_missileShot = true;
  } else {
    x219_missileShot = false;
  }
  if (TCastToConstPtr<CScriptGrapplePoint> point = mgr.GetObjectById(xf2_nextTargetId)) {
    if (point->GetUniqueId() != x1dc_grapplePoint0) {
      float tmp;
      if (point->GetUniqueId() == x1de_grapplePoint1)
        tmp = std::max(FLT_EPSILON, x1e4_grapplePoint1T);
      else
        tmp = FLT_EPSILON;
      x1de_grapplePoint1 = x1dc_grapplePoint0;
      x1e4_grapplePoint1T = x1e0_grapplePoint0T;
      x1e0_grapplePoint0T = tmp;
      x1dc_grapplePoint0 = point->GetUniqueId();
    }
  } else if (x1dc_grapplePoint0 != kInvalidUniqueId) {
    x1de_grapplePoint1 = x1dc_grapplePoint0;
    x1e4_grapplePoint1T = x1e0_grapplePoint0T;
    x1e0_grapplePoint0T = 0.f;
    x1dc_grapplePoint0 = kInvalidUniqueId;
  }
  if (x1e0_grapplePoint0T > 0.f)
    x1e0_grapplePoint0T = std::min(dt / 0.5f + x1e0_grapplePoint0T, 1.f);
  if (x1e4_grapplePoint1T > 0.f) {
    x1e4_grapplePoint1T = std::max(0.f, x1e4_grapplePoint1T - dt / 0.5f);
    if (x1e4_grapplePoint1T == 0.f)
      x1de_grapplePoint1 = kInvalidUniqueId;
  }
  x1f0_xrayRetAngle =
      zeus::CRelAngle(zeus::degToRad(g_tweakTargeting->GetXRayRetAngleSpeed() * dt) + x1f0_xrayRetAngle).asRel();
  x1ec_seekerAngle =
      zeus::CRelAngle(zeus::degToRad(g_tweakTargeting->GetSeekerAngleSpeed() * dt) + x1ec_seekerAngle).asRel();
}

void CTargetReticleRenderState::InterpolateWithClamp(const CTargetReticleRenderState& a, CTargetReticleRenderState& out,
                                                     const CTargetReticleRenderState& b, float t) {
  t = zeus::clamp(0.f, t, 1.f);
  float omt = 1.f - t;
  out.x4_radiusWorld = omt * a.x4_radiusWorld + t * b.x4_radiusWorld;
  out.x14_factor = omt * a.x14_factor + t * b.x14_factor;
  out.x18_minVpClampScale = omt * a.x18_minVpClampScale + t * b.x18_minVpClampScale;
  out.x8_positionWorld = zeus::CVector3f::lerp(a.x8_positionWorld, b.x8_positionWorld, t);

  if (t == 1.f)
    out.x0_target = b.x0_target;
  else if (t == 0.f)
    out.x0_target = a.x0_target;
  else
    out.x0_target = kInvalidUniqueId;
}

static bool IsDamageOrbit(CPlayer::EPlayerOrbitRequest req) {
  switch (req) {
  case CPlayer::EPlayerOrbitRequest::Five:
  case CPlayer::EPlayerOrbitRequest::ActivateOrbitSource:
  case CPlayer::EPlayerOrbitRequest::ProjectileCollide:
  case CPlayer::EPlayerOrbitRequest::Freeze:
  case CPlayer::EPlayerOrbitRequest::DamageOnGrapple:
    return true;
  default:
    return false;
  }
}

void CCompoundTargetReticle::UpdateNextLockOnGroupRS5(float dt, const class CStateManager& mgr) {
  CPlayer& player = mgr.GetPlayer();
  TUniqueId nextTargetId = player.GetOrbitNextTargetId();

  x2a4_uiPositionOffset = g_tweakTargeting->x2e0_uiPositionOffset;
  x290_uiScale = zeus::CVector3f(g_tweakTargeting->x2cc_uiScale);
  if (nextTargetId != kInvalidUniqueId)
    x29c_nextTargetOrbitLockColor = g_tweakTargeting->x2d0_orbitLockArmColor;

  if (auto act = TCastToConstPtr<CActor>(mgr.GetObjectById(nextTargetId)))
    x2b4_nextTargetOrbitPosition = act->GetOrbitPosition(mgr);
  else
    x2b4_nextTargetOrbitPosition = x2c0_orbitLockArmNextOrigin;

  const zeus::CTransform cameraTransform = mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTransform();
  const zeus::CVector3f cameraOrigin = cameraTransform.origin;
  const zeus::CVector3f cameraFrontVector = cameraTransform.frontVector().normalized();
  const zeus::CVector3f nextTargetCameraOffset = (x2b4_nextTargetOrbitPosition - cameraOrigin).normalized();
  const float dot = 1.25f / cameraFrontVector.dot(nextTargetCameraOffset);
  const zeus::CVector3f final = cameraOrigin + (nextTargetCameraOffset * dot);
  x2d8_orbitLockArm1Origin = final;

  if (nextTargetId != kInvalidUniqueId && x27c_nextTargetId != nextTargetId) {
    if (x2a0_nextTargetOrbitLockAlpha == 0.f) {
      x2e4_orbitLockArmFadeOrigin = final;
    } else {
      x2e4_orbitLockArmFadeOrigin = x2c0_orbitLockArmNextOrigin;
      x2cc_orbitLockArmFading = true;
      x2d0_orbitLockArmFadeAmt = g_tweakTargeting->x2f0_;
      x2d4_orbitLockArmFadeDt = 0.f;
    }
  }

  float alphaMax = g_tweakTargeting->x2d4_;
  float alphaChg = dt * (alphaMax / g_tweakTargeting->x2ec_);
  if (nextTargetId == kInvalidUniqueId) {
    x2a0_nextTargetOrbitLockAlpha = std::clamp(x2a0_nextTargetOrbitLockAlpha - alphaChg, 0.f, alphaMax);
  } else {
    x2a0_nextTargetOrbitLockAlpha = std::clamp(x2a0_nextTargetOrbitLockAlpha + alphaChg, 0.f, alphaMax);
  }

  if (!x2cc_orbitLockArmFading || x2d0_orbitLockArmFadeAmt <= 0.f) {
    x280_nextTargetOrbitLockOrigin = x2d8_orbitLockArm1Origin;
    x2c0_orbitLockArmNextOrigin = x2b4_nextTargetOrbitPosition;
  } else {
    float f3 = x2d0_orbitLockArmFadeAmt;
    float pt = std::max(0.f, x2d4_orbitLockArmFadeDt + dt);
    float f0 = (pt - f3) >= 0.f ? f3 : pt;
    x2d4_orbitLockArmFadeDt = f0;
    if (x2d0_orbitLockArmFadeAmt <= f0) {
      x2cc_orbitLockArmFading = false;
      x280_nextTargetOrbitLockOrigin = x2d8_orbitLockArm1Origin;
      x2c0_orbitLockArmNextOrigin = x2b4_nextTargetOrbitPosition;
    } else {
      const zeus::CVector3f origin = (x2e4_orbitLockArmFadeOrigin - cameraOrigin).normalized();
      float dot2 = 1.25f / origin.dot(cameraFrontVector);
      const zeus::CVector3f vec = cameraOrigin + (origin * dot2);

      float div = x2d4_orbitLockArmFadeDt / x2d0_orbitLockArmFadeAmt;
      float oneMinusDiv = 1.f - div;
      const zeus::CVector3f vec2 = (vec * oneMinusDiv) + (x2d8_orbitLockArm1Origin * div);
      x280_nextTargetOrbitLockOrigin = vec2;

      const zeus::CVector3f vec3 = (vec2 - cameraOrigin).normalized();
      float mag = (x2b4_nextTargetOrbitPosition - cameraOrigin).magnitude();
      x2c0_orbitLockArmNextOrigin = cameraOrigin + (vec3 * mag);
    }
  }

  x27c_nextTargetId = nextTargetId;
  x28c_orbitLockArmRotation += zeus::degToRad(dt * g_tweakTargeting->x2f4_orbitLockArmRotSpeed);
}

void CCompoundTargetReticle::UpdateCurrLockOnGroupRS5(float dt, const class CStateManager& mgr) {
  CPlayer& player = mgr.GetPlayer();
  TUniqueId targetId = player.GetOrbitTargetId();
  bool bVar1 = false;
  if (targetId != kInvalidUniqueId) {
    TCastToConstPtr<CScriptGrapplePoint> prevPoint = mgr.GetObjectById(x27c_nextTargetId);
    TCastToConstPtr<CScriptGrapplePoint> currPoint = mgr.GetObjectById(targetId);
    bVar1 = currPoint.operator bool();
    if (!prevPoint) {
      x27c_nextTargetId = kInvalidUniqueId;
      x2a0_nextTargetOrbitLockAlpha = 0.f;
    }
  }

  zeus::CTransform cameraTransform = mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTransform();
  zeus::CQuaternion cameraRotation = cameraTransform.getRotation().buildMatrix3f();
  zeus::CVector3f translate = cameraRotation.transform(zeus::CVector3f(0.f, 1.25f, 0.f));
  x2f2_orbitLockOrigin = cameraTransform.origin + translate;

  bool bVar11 = false;
  auto uVar14 = ((u32)kInvalidUniqueId.id) - ((u32)targetId.id) | ((u32)targetId.id) - ((u32)kInvalidUniqueId.id);
  if (((s32)uVar14) > -1 && player.x1188_) { // < 0
    bVar11 = true;
  }

  if (targetId == kInvalidUniqueId || targetId != x2f0_) {
    if (targetId == kInvalidUniqueId && x2f0_ != kInvalidUniqueId) {
      if (bVar11) {
        TCastToConstPtr<CScriptGrapplePoint> point = mgr.GetObjectById(x2f0_);
        if (x20_prevState == EReticleState::Combat && !point.operator bool()) {
          x350_nextGrappleOrbitLockAlpha = 1.f;
        }
      }
    } else {
      x324_ = 0.f;
    }
  } else {
    x324_ += dt;
  }

  x300_orbitLockArmColor = g_tweakTargeting->x2d8_;
  zeus::CColor _1d4 = g_tweakTargeting->x304_;
  if (IsGrappleTarget(targetId, mgr)) {
    x300_orbitLockArmColor = g_tweakTargeting->x320_;
    _1d4 = g_tweakTargeting->x320_;
  }

  if (((s32)uVar14) < 0 || !bVar11) {
    x37c_ = std::max(0.f, g_tweakTargeting->x318_);
    x380_ = std::max(0.f, g_tweakTargeting->x31c_);
  } else {
    x380_ = std::max(0.f, g_tweakTargeting->x318_);
    x37c_ = std::max(0.f, g_tweakTargeting->x31c_);
  }

  float fVar4 = 1.f;
  if (player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed) {
    fVar4 = dt * (g_tweakTargeting->x354_nextGrappleOrbitLockAlphaMax / g_tweakTargeting->x358_);
  }
  x350_nextGrappleOrbitLockAlpha =
      std::clamp(x350_nextGrappleOrbitLockAlpha - fVar4, 0.f, g_tweakTargeting->x354_nextGrappleOrbitLockAlphaMax);

  float dVar31 = x37c_ / g_tweakTargeting->x318_;
  float dVar30 = x380_ / g_tweakTargeting->x31c_;
  x36c_lerped_color_ = zeus::CColor::lerp(g_tweakTargeting->x30c_, _1d4, dVar31);

  x378_ = dVar30;
  x370_ = ((1.f - dVar31) * g_tweakTargeting->x310_) + (dVar31 * g_tweakTargeting->x308_);
  x374_ = /*((1.f - dVar30) * 0.f) +*/ (dVar30 * g_tweakTargeting->x314_);

  float clamp = std::clamp((x324_ - g_tweakTargeting->x330_) / g_tweakTargeting->x334_, 0.f, 1.f);
  float negativeClamp = g_tweakTargeting->x338_ * (1.f - clamp);
  x328_orbitLockBaseScale = (x290_uiScale * clamp) + negativeClamp;
  x308_orbitLockBaseColor = g_tweakTargeting->x328_;
  x30c_orbitLockBaseAlpha = g_tweakTargeting->x32c_;

  float f31 = std::clamp((x324_ - g_tweakTargeting->x33c_) / g_tweakTargeting->x340_, 0.f, 1.f);
  x334_ = f31 * g_tweakTargeting->x32c_;

  const zeus::CVector3f& playerRotation = player.GetTransform().frontVector().normalized();
  float f = 1.f / (2.f * M_PI);
  float degrees = 360.f * (f * zeus::CVector3f::getAngleDiff(playerRotation, zeus::skForward));
  float radians = zeus::degToRad(playerRotation.x() > 0.f ? 360.f - degrees : degrees);
  float playerAngle = radians - std::trunc(radians * f) * M_2_PI;
  if (playerAngle < 0.f)
    playerAngle += M_2_PI;

  if (f31 < 1.f) {
    x338_orbitLockBracketsRotation = 0.f;
  } else {
    x338_orbitLockBracketsRotation = x338_orbitLockBracketsRotation + (playerAngle - x33c_prevPlayerAngle);
  }
  x33c_prevPlayerAngle = playerAngle;

  float f32 = ((x324_ - 0.f) / g_tweakTargeting->x348_) * 0.5f;
  x340_orbitLockTechScrollTime = std::clamp(f32, 0.f, 0.496f);
  if (f32 < 1.f) {
    x344_orbitLockTechRotation = 0.f;
  } else {
    x344_orbitLockTechRotation += zeus::degToRad(g_tweakTargeting->x34c_ * dt);
  }
  x348_playerAngle = playerAngle;

  if (x20_prevState != EReticleState::Combat && x20_prevState != EReticleState::Unspecified) {
    bVar1 = true;
    x374_ = 0.f;
  }
  if (bVar1) {
    x304_orbitLockArmAlpha = 0.f;
  } else {
    float g = g_tweakTargeting->x2dc_;
    float f3 = 1.f;
    if (0.f < g_tweakTargeting->x2fc_) {
      f3 = (dt * (g / g_tweakTargeting->x2fc_));
    }
    if (x324_ < g_tweakTargeting->x2f8_) {
      x304_orbitLockArmAlpha = g;
    } else {
      float f4 = g_tweakTargeting->x300_;
      if (f4 <= 0.f) {
        f4 = x304_orbitLockArmAlpha - f3;
      }
      if (g <= 0.f) {
        g = f4;
      }
      x304_orbitLockArmAlpha = g;
    }
  }
  x34c_ = bVar11;
  x2f0_ = targetId;
}

void CCompoundTargetReticle::UpdateNewGroup3(float dt, const CStateManager& mgr) {
  bool bVar5;
  CPlayer& player = mgr.GetPlayer();
  std::shared_ptr<CPlayerState> playerState = mgr.GetPlayerState();
  if (playerState->GetCurrentVisor() == CPlayerState::EPlayerVisor::Scan) {
    TUniqueId id = kInvalidUniqueId;
    if (x440_ != 0) {
      id = *x444_.begin();
    }
    TCastToConstPtr<CActor> checkActor = mgr.GetObjectById(id);
    zeus::CVector3f vec = player.GetUnknownPlayerState2().Getx7c();
    zeus::CVector3f vec2 = zeus::skZero3f;

    if (checkActor.operator bool()) {
      bool bVar10 = false;
      zeus::CAABox total;

      auto iter = x444_.begin();
      // TODO x440_ tracks count?
      while (iter != x444_.end()) {
        TCastToConstPtr<CActor> actor = mgr.GetObjectById(*iter);
        if (actor.operator bool()) {
          const CModelData* modelData = actor->GetModelData();
          bool bVar6 = false;
          if (modelData != nullptr) {
            bVar5 = false;
            if (!modelData->HasAnimData() && !modelData->GetNormalModel().IsLocked()) { // Locked??
              bVar5 = true;
            }
            if (!bVar5) {
              bVar6 = true;
            }
          }
          if (bVar6) {
            bVar6 = false;
            if (!modelData->HasAnimData() && !modelData->GetNormalModel().IsLocked()) {
              bVar6 = true;
            }
            if (!bVar6) {
              if (!modelData->HasAnimData()) {
                const zeus::CAABox box = modelData->GetBounds();
                const zeus::CVector3f center = box.center();
                const zeus::CVector3f mul = box.extents() * modelData->GetScale();
                total.accumulateBounds(zeus::CAABox(center - mul, center + mul));
                bVar10 = true;
              } else {
                const zeus::CAABox box = modelData->GetAnimationData()->GetBoundingBox();
                zeus::CTransform transform = player.GetTransform();
                const zeus::CVector3f min = transform * (box.min * modelData->GetScale());
                const zeus::CVector3f max = transform * (box.max * modelData->GetScale());
                total.accumulateBounds(zeus::CAABox(min, max));
                bVar10 = true;
              }
            }
          }
        }
      }
      if (bVar10) {
        const zeus::CVector3f _1b8 = total.center();
        vec = _1b8;
        const CGameCamera* camera = mgr.GetCameraManager()->GetCurrentCamera(mgr);
        const zeus::CAABox transformed = total.getTransformedAABox(camera->GetTransform());
        const zeus::CVector3f _1c4 = transformed.max - transformed.min;
        vec2 = _1c4;
        const zeus::CVector3f _320 = camera->ConvertToScreenSpace(_1b8);
        if (_320.z() > 1.f) {
          vec = player.GetUnknownPlayerState2().Getx7c();
          vec2 = zeus::skOne3f;
        }
      }
    }

    x41c_ = vec;
    x434_ = vec2;
  }
}

void CCompoundTargetReticle::UpdateNewGroup4(float dt, const CStateManager& mgr) {
  CPlayer& player = mgr.GetPlayer();
  x228_ = player.GetUnknownPlayerState2().Getx7c();

  TUniqueId targetId = mgr.GetPlayer().GetOrbitTargetId();
  TCastToConstPtr<CActor> actor = mgr.GetObjectById(targetId);
  float d = dt / 1.f;
  if (actor.operator bool()) {
    x21c_ = std::min(1.f, x21c_ + d);
    x234_ = CalculatePositionWorld(*actor, mgr);
  } else {
    x21c_ = std::max(0.f, x21c_ - d);
  }

  x224_ = std::max(0.f, x224_ - d);
}

void CCompoundTargetReticle::UpdateNewGroup5(float dt, const CStateManager& mgr) {
  const CPlayer& player = mgr.GetPlayer();
  x240_ = player.GetUnknownPlayerState2().Getx7c();
  x250_ = g_tweakTargeting->x220_scanTargetClampMax;
  x268_ = g_tweakTargeting->x23c_;

  const zeus::CVector3f v1 = player.GetUnknownPlayerState2().Getx7c() - player.GetTransform().origin;
  float mag = v1.magnitude();
}

void CCompoundTargetReticle::UpdateNewGroup6(float dt, const CStateManager& mgr) {

}

void CCompoundTargetReticle::UpdateCurrLockOnGroup(float dt, const CStateManager& mgr) {
  TUniqueId targetId = mgr.GetPlayer().GetOrbitTargetId();
  if (targetId != xf0_targetId) {
    if (targetId != kInvalidUniqueId) {
      if (TCastToConstPtr<CScriptGrapplePoint> point = mgr.GetObjectById(targetId))
        CSfxManager::SfxStart(SFXui_lockon_grapple, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
      else
        CSfxManager::SfxStart(SFXui_lockon_poi, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    }

    if (targetId == kInvalidUniqueId) {
      x12c_currGroupA = x10c_currGroupInterp;
      x14c_currGroupB.SetFactor(0.f);
      x16c_currGroupDur =
          IsDamageOrbit(mgr.GetPlayer().GetOrbitRequest()) ? 0.65f : g_tweakTargeting->GetCurrLockOnEnterDuration();
    } else {
      x12c_currGroupA = x10c_currGroupInterp;
      if (xf0_targetId == kInvalidUniqueId)
        x12c_currGroupA.SetTargetId(targetId);
      x14c_currGroupB = CTargetReticleRenderState(
          targetId, 1.f, zeus::skZero3f, 1.f,
          IsGrappleTarget(targetId, mgr) ? g_tweakTargeting->GetGrappleMinClampScale() : 1.f, false);
      x16c_currGroupDur = xf0_targetId == kInvalidUniqueId ? g_tweakTargeting->GetCurrLockOnExitDuration()
                                                           : g_tweakTargeting->GetCurrLockOnSwitchDuration();
    }
    x170_currGroupTimer = x16c_currGroupDur;
    xf0_targetId = targetId;
  }
  if (x170_currGroupTimer > 0.f) {
    UpdateTargetParameters(x12c_currGroupA, mgr);
    UpdateTargetParameters(x14c_currGroupB, mgr);
    x170_currGroupTimer = std::max(0.f, x170_currGroupTimer - dt);
    CTargetReticleRenderState::InterpolateWithClamp(x12c_currGroupA, x10c_currGroupInterp, x14c_currGroupB,
                                                    1.f - x170_currGroupTimer / x16c_currGroupDur);
  } else {
    UpdateTargetParameters(x10c_currGroupInterp, mgr);
  }
  if (x1f8_missileBracketTimer != 0.f && x1f8_missileBracketTimer < g_tweakTargeting->GetMissileBracketDuration()) {
    if (x1f8_missileBracketTimer < 0.f)
      x1f8_missileBracketTimer = std::min(0.f, x1f8_missileBracketTimer + dt);
    else
      x1f8_missileBracketTimer = std::min(g_tweakTargeting->GetMissileBracketDuration(), x1f8_missileBracketTimer + dt);
  }
  if (x204_chargeGaugeOvershootTimer > 0.f) {
    x204_chargeGaugeOvershootTimer = std::max(0.f, x204_chargeGaugeOvershootTimer - dt);
    if (x204_chargeGaugeOvershootTimer == 0.f) {
      for (int i = 0; i < 9; ++i)
        xe0_outerBeamIconSquares[i].x10_rotAng = xe0_outerBeamIconSquares[i].x14_baseAngle;
      xc4_chargeGauge.x10_rotAng = xc4_chargeGauge.x14_baseAngle;
      x208_lockonTimer = FLT_EPSILON;
    } else {
      float offshoot =
          offshoot_func(x2c_overshootOffsetHalf, x30_premultOvershootOffset,
                        1.f - x204_chargeGaugeOvershootTimer / g_tweakTargeting->GetChargeGaugeOvershootDuration());
      for (int i = 0; i < 9; ++i) {
        SOuterItemInfo& item = xe0_outerBeamIconSquares[i];
        item.x10_rotAng = zeus::CRelAngle(item.x18_offshootAngleDelta * offshoot + item.xc_offshootBaseAngle).asRel();
      }
      xc4_chargeGauge.x10_rotAng =
          zeus::CRelAngle(xc4_chargeGauge.x18_offshootAngleDelta * offshoot + xc4_chargeGauge.xc_offshootBaseAngle)
              .asRel();
    }
  }
  if (x208_lockonTimer > 0.f && x208_lockonTimer < g_tweakTargeting->GetLockonDuration())
    x208_lockonTimer = std::min(g_tweakTargeting->GetLockonDuration(), x208_lockonTimer + dt);
  if (x210_lockFireTimer > 0.f)
    x210_lockFireTimer = std::max(0.f, x210_lockFireTimer - dt);
  if (x1fc_missileBracketScaleTimer > 0.f)
    x1fc_missileBracketScaleTimer = std::max(0.f, x1fc_missileBracketScaleTimer - dt);
  if (x20_prevState == EReticleState::Scan && x456_ != kInvalidUniqueId)
    x404_ = std::min(1.f, x404_ + dt / g_tweakTargeting->x360_);
  else
    x404_ = std::max(0.f, x404_ - dt / g_tweakTargeting->x360_);
  if (mgr.GetPlayer().GetScanningObjectId() == kInvalidUniqueId)
    x214_ = std::min(1.f, x214_ + dt * 4.f);
  else
    x214_ = std::max(0.f, x214_ - dt * 4.f);
}

void CCompoundTargetReticle::UpdateNextLockOnGroup(float dt, const CStateManager& mgr) {
  TUniqueId nextTargetId = mgr.GetPlayer().GetOrbitNextTargetId();
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Scan &&
      mgr.GetPlayer().GetOrbitTargetId() != kInvalidUniqueId)
    nextTargetId = mgr.GetPlayer().GetOrbitTargetId();
  if (nextTargetId != xf2_nextTargetId) {
    if (nextTargetId == kInvalidUniqueId) {
      x194_nextGroupA = x174_nextGroupInterp;
      x1b4_nextGroupB = CTargetReticleRenderState(
          kInvalidUniqueId, 1.f,
          (x20_prevState == EReticleState::XRay || x20_prevState == EReticleState::Thermal) ? x100_laggingTargetPos
                                                                                            : xf4_targetPos,
          0.f, 1.f, true);
      x1d4_nextGroupDur = x1d8_nextGroupTimer = g_tweakTargeting->GetNextLockOnExitDuration();
      xf2_nextTargetId = nextTargetId;
    } else {
      x194_nextGroupA = x174_nextGroupInterp;
      x1b4_nextGroupB = CTargetReticleRenderState(
          nextTargetId, 1.f, zeus::skZero3f, 1.f,
          IsGrappleTarget(nextTargetId, mgr) ? g_tweakTargeting->GetGrappleMinClampScale() : 1.f, true);
      x1d4_nextGroupDur = x1d8_nextGroupTimer = xf2_nextTargetId == kInvalidUniqueId
                                                    ? g_tweakTargeting->GetNextLockOnEnterDuration()
                                                    : g_tweakTargeting->GetNextLockOnSwitchDuration();
      xf2_nextTargetId = nextTargetId;
    }
  }
  if (x1d8_nextGroupTimer > 0.f) {
    UpdateTargetParameters(x194_nextGroupA, mgr);
    UpdateTargetParameters(x1b4_nextGroupB, mgr);
    x1d8_nextGroupTimer = std::max(0.f, x1d8_nextGroupTimer - dt);
    CTargetReticleRenderState::InterpolateWithClamp(x194_nextGroupA, x174_nextGroupInterp, x1b4_nextGroupB,
                                                    1.f - x1d8_nextGroupTimer / x1d4_nextGroupDur);
  } else {
    UpdateTargetParameters(x174_nextGroupInterp, mgr);
  }
}

void CCompoundTargetReticle::UpdateOrbitZoneGroup(float dt, const CStateManager& mgr) {
  if (xf0_targetId == kInvalidUniqueId && xf2_nextTargetId != kInvalidUniqueId)
    x20c_ = std::min(1.f, 2.f * dt + x20c_);
  else
    x20c_ = std::max(0.f, x20c_ - 2.f * dt);
  if (mgr.GetPlayer().IsShowingCrosshairs() &&
      mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::Scan)
    x1e8_crosshairsScale = std::min(1.f, dt / g_tweakTargeting->GetCrosshairsScaleDuration() + x1e8_crosshairsScale);
  else
    x1e8_crosshairsScale = std::max(0.f, x1e8_crosshairsScale - dt / g_tweakTargeting->GetCrosshairsScaleDuration());
}

void CCompoundTargetReticle::Draw(const CStateManager& mgr, bool hideLockon) const {
  if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed &&
      !mgr.GetCameraManager()->IsInCinematicCamera()) {
    SCOPED_GRAPHICS_DEBUG_GROUP("CCompoundTargetReticle::Draw", zeus::skCyan);
    zeus::CTransform camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
    CGraphics::SetViewPointMatrix(camXf);
    CGraphics::SetCullMode(ERglCullMode::None);
    if (!hideLockon) {
      DrawNextLockOnGroupRS5(camXf.basis, mgr);
      DrawCurrLockOnGroupRS5(camXf.basis, mgr);
      //      DrawCurrLockOnGroup(camXf.basis, mgr);
      //      DrawNextLockOnGroup(camXf.basis, mgr);
      DrawOrbitZoneGroup(camXf.basis, mgr);
    }
    DrawGrappleGroup(camXf.basis, mgr, hideLockon);
    CGraphics::SetCullMode(ERglCullMode::Front);
  }
  if (x28_noDrawTicks > 0)
    --x28_noDrawTicks;
}

void CCompoundTargetReticle::DrawNextLockOnGroupRS5(const zeus::CMatrix3f& rot, const CStateManager& mgr) const {
  if (x2a0_nextTargetOrbitLockAlpha > 0) {
    for (int i = 0; i < 3; i++) {
      const zeus::CTransform origin = zeus::CTransform(rot, x280_nextTargetOrbitLockOrigin);
      const zeus::CTransform transform =
          zeus::CMatrix3f::RotateY(x28c_orbitLockArmRotation + zeus::degToRad(120.f * i)) *
          zeus::CMatrix3f(x290_uiScale);
      CGraphics::SetModelMatrix(origin * transform * zeus::CTransform::Translate(x2a4_uiPositionOffset));

      zeus::CColor color = x29c_nextTargetOrbitLockColor;
      color.a() *= std::max(0.25f, x2a0_nextTargetOrbitLockAlpha);
      CModelFlags flags(5, 0, 0, color);
      x3a4_orbitLockArm->Draw(flags);
    }
  }

  if (x350_nextGrappleOrbitLockAlpha > 0) {
    for (int i = 0; i < 3; i++) {
      const zeus::CTransform origin = zeus::CTransform(rot, x2f2_orbitLockOrigin);
      const zeus::CTransform transform =
          zeus::CMatrix3f::RotateY(x28c_orbitLockArmRotation + zeus::degToRad(120.f * i)) *
          zeus::CMatrix3f(x290_uiScale);
      CGraphics::SetModelMatrix(origin * transform * zeus::CTransform::Translate(x2a4_uiPositionOffset));

      zeus::CColor color = g_tweakTargeting->x2d0_orbitLockArmColor;
      color.a() *= std::max(0.25f, x350_nextGrappleOrbitLockAlpha);
      CModelFlags flags(5, 0, 0, color);
      x3a4_orbitLockArm->Draw(flags);
    }
  }
}

void CCompoundTargetReticle::DrawCurrLockOnGroupRS5(const zeus::CMatrix3f& rot, const CStateManager& mgr) const {
  if (x2f0_ != kInvalidUniqueId && x304_orbitLockArmAlpha > 0) {
    for (int i = 0; i < 3; i++) {
      const zeus::CTransform origin(rot, x2f2_orbitLockOrigin);
      const zeus::CMatrix3f rotation = zeus::CMatrix3f::RotateY(zeus::degToRad(120.f * i));
      const zeus::CTransform transform(rotation * zeus::CMatrix3f(x290_uiScale));
      CGraphics::SetModelMatrix(origin * transform * zeus::CTransform::Translate(x2a4_uiPositionOffset));

      zeus::CColor color = x300_orbitLockArmColor;
      color.a() *= x304_orbitLockArmAlpha;
      CModelFlags flags(5, 0, 0, color);
      x3a4_orbitLockArm->Draw(flags);

      const zeus::CTransform baseTransform(rotation * zeus::CMatrix3f(x328_orbitLockBaseScale));
      CGraphics::SetModelMatrix(origin * baseTransform);

      zeus::CColor baseColor = x308_orbitLockBaseColor;
      baseColor.a() *= std::max(0.25f, x30c_orbitLockBaseAlpha);
      CModelFlags baseFlags(5, 0, 0, baseColor);
      x3c8_orbitLockBase->Draw(baseFlags);
    }

    {
      const zeus::CTransform origin(rot, x2f2_orbitLockOrigin);
      const zeus::CTransform transform(zeus::CMatrix3f::RotateY(x338_orbitLockBracketsRotation) *
                                       zeus::CMatrix3f(x290_uiScale));
      CGraphics::SetModelMatrix(origin * transform);
      CGraphics::SetAlphaCompare(ERglAlphaFunc::Always, 0, ERglAlphaOp::And, ERglAlphaFunc::Always, 0);

      zeus::CColor bracketsColor = x308_orbitLockBaseColor;
      bracketsColor.a() *= std::max(0.25f, x334_);
      CModelFlags bracketsFlags(5, 0, 0, bracketsColor);
      x3bc_orbitLockBrackets->Draw(bracketsFlags);
    }

    if (x340_orbitLockTechScrollTime > 0) {
      const zeus::CTransform origin(rot, x2f2_orbitLockOrigin);
      const zeus::CTransform transform(zeus::CMatrix3f::RotateY(x344_orbitLockTechRotation) *
                                       zeus::CMatrix3f(x290_uiScale));
      CTimeProvider prov(x340_orbitLockTechScrollTime);
      CGraphics::SetModelMatrix(origin * transform);
      CGraphics::SetAlphaCompare(ERglAlphaFunc::Always, 0, ERglAlphaOp::And, ERglAlphaFunc::Always, 0);

      zeus::CColor techColor = x308_orbitLockBaseColor;
      techColor.a() *= std::max(0.25f, x30c_orbitLockBaseAlpha);
      CModelFlags techFlags(5, 0, 0, techColor);
      x3b0_orbitLockTech->Draw(techFlags);
    }
  }
}

void CCompoundTargetReticle::DrawGrapplePoint(const CScriptGrapplePoint& point, float t, const CStateManager& mgr,
                                              const zeus::CMatrix3f& rot, bool zEqual) const {
  zeus::CVector3f orbitPos = point.GetOrbitPosition(mgr);
  zeus::CColor color;
  if (point.GetGrappleParameters().GetLockSwingTurn())
    color = g_tweakTargeting->GetLockedGrapplePointSelectColor();
  else
    color = g_tweakTargeting->GetGrapplePointSelectColor();
  color = zeus::CColor::lerp(g_tweakTargeting->GetGrapplePointColor(), color, t);
  zeus::CMatrix3f scale(
      CalculateClampedScale(orbitPos, 1.f, g_tweakTargeting->GetGrappleClampMin(),
                            g_tweakTargeting->GetGrappleClampMax(), mgr) *
      ((1.f - t) * g_tweakTargeting->GetGrappleScale() + t * g_tweakTargeting->GetGrappleSelectScale()));
  zeus::CTransform modelXf(rot * scale, orbitPos);
  CGraphics::SetModelMatrix(modelXf);
  CModelFlags flags(7, 0, 0, color);
  x94_grapple->Draw(flags);
}

void CCompoundTargetReticle::DrawGrappleGroup(const zeus::CMatrix3f& rot, const CStateManager& mgr,
                                              bool hideLockon) const {
  if (x28_noDrawTicks > 0)
    return;

  if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::GrappleBeam) && x94_grapple.IsLoaded() &&
      x20_prevState != EReticleState::Scan) {
    if (hideLockon) {
      for (const CEntity* ent : mgr.GetAllObjectList()) {
        if (TCastToConstPtr<CScriptGrapplePoint> point = ent) {
          if (point->GetActive()) {
            if (point->GetAreaIdAlways() != kInvalidAreaId) {
              const CGameArea* area = mgr.GetWorld()->GetAreaAlways(point->GetAreaIdAlways());
              CGameArea::EOcclusionState occState =
                  area->IsPostConstructed() ? area->GetOcclusionState() : CGameArea::EOcclusionState::Occluded;
              if (occState != CGameArea::EOcclusionState::Visible)
                continue;
            }
            float t = 0.f;
            if (point->GetUniqueId() == x1dc_grapplePoint0)
              t = x1e0_grapplePoint0T;
            else if (point->GetUniqueId() == x1de_grapplePoint1)
              t = x1e4_grapplePoint1T;
            if (std::fabs(t) < 0.00001f)
              DrawGrapplePoint(*point, t, mgr, rot, true);
          }
        }
      }
    } else {
      TCastToConstPtr<CScriptGrapplePoint> point0 = mgr.GetObjectById(x1dc_grapplePoint0);
      TCastToConstPtr<CScriptGrapplePoint> point1 = mgr.GetObjectById(x1de_grapplePoint1);
      for (int i = 0; i < 2; ++i) {
        const CScriptGrapplePoint* point = i == 0 ? point0.GetPtr() : point1.GetPtr();
        float t = i == 0 ? x1e0_grapplePoint0T : x1e4_grapplePoint1T;
        if (point)
          DrawGrapplePoint(*point, t, mgr, rot, false);
      }
    }
  }
}

void CCompoundTargetReticle::DrawCurrLockOnGroup(const zeus::CMatrix3f& rot, const CStateManager& mgr) const {
  if (x28_noDrawTicks > 0)
    return;
  if (x1e0_grapplePoint0T + x1e4_grapplePoint1T > 0 || x10c_currGroupInterp.GetFactor() == 0.f)
    return;

  float lockBreakAlpha = x10c_currGroupInterp.GetFactor();
  float visorFactor = mgr.GetPlayerState()->GetVisorTransitionFactor();
  bool lockConfirm = false;
  bool lockReticule = false;
  switch (x20_prevState) {
  case EReticleState::Combat:
    lockConfirm = true;
    lockReticule = true;
    break;
  case EReticleState::Scan:
    lockConfirm = true;
    break;
  default:
    break;
  }

  zeus::CMatrix3f lockBreakXf;
  zeus::CColor lockBreakColor = zeus::skClear;
  if (IsDamageOrbit(mgr.GetPlayer().GetOrbitRequest()) && x14c_currGroupB.GetFactor() == 0.f) {
    zeus::CMatrix3f lockBreakRM;
    for (int i = 0; i < 4; ++i) {
      int a = rand() % 9;
      auto b = std::div(a, 3);
      lockBreakRM[b.rem][b.quot] += rand() / float(RAND_MAX) - 0.5f;
    }
    lockBreakXf = lockBreakRM.transposed();
    if (x10c_currGroupInterp.GetFactor() > 0.8f)
      lockBreakColor = zeus::CColor(1.f, (x10c_currGroupInterp.GetFactor() - 0.8f) * 0.3f / 0.2f);
    lockBreakAlpha = x10c_currGroupInterp.GetFactor() > 0.75f
                         ? 1.f
                         : std::max(0.f, (x10c_currGroupInterp.GetFactor() - 0.55f) / 0.2f);
  }

  if (lockConfirm && x4c_lockConfirm.IsLoaded()) {
    zeus::CMatrix3f scale(CalculateClampedScale(x10c_currGroupInterp.GetTargetPositionWorld(),
                                                x10c_currGroupInterp.GetRadiusWorld(),
                                                x10c_currGroupInterp.GetMinViewportClampScale() *
                                                    g_tweakTargeting->GetLockConfirmClampMin(),
                                                g_tweakTargeting->GetLockConfirmClampMax(), mgr) *
                          g_tweakTargeting->GetLockConfirmScale() / x10c_currGroupInterp.GetFactor());
    zeus::CTransform modelXf(lockBreakXf * (rot * zeus::CMatrix3f::RotateY(x1ec_seekerAngle) * scale),
                             x10c_currGroupInterp.GetTargetPositionWorld());
    CGraphics::SetModelMatrix(modelXf);
    zeus::CColor color = g_tweakTargeting->GetLockConfirmColor();
    color.a() *= lockBreakAlpha;
    CModelFlags flags(7, 0, 0, lockBreakColor + color);
    x4c_lockConfirm->Draw(flags);
  }

  if (lockReticule) {
    if (x58_targetFlower.IsLoaded()) {
      zeus::CMatrix3f scale(CalculateClampedScale(x10c_currGroupInterp.GetTargetPositionWorld(),
                                                  x10c_currGroupInterp.GetRadiusWorld(),
                                                  x10c_currGroupInterp.GetMinViewportClampScale() *
                                                      g_tweakTargeting->GetTargetFlowerClampMin(),
                                                  g_tweakTargeting->GetTargetFlowerClampMax(), mgr) *
                            g_tweakTargeting->GetTargetFlowerScale() / lockBreakAlpha);
      zeus::CTransform modelXf(lockBreakXf * (rot * zeus::CMatrix3f::RotateY(x1f0_xrayRetAngle) * scale),
                               x10c_currGroupInterp.GetTargetPositionWorld());
      CGraphics::SetModelMatrix(modelXf);
      zeus::CColor color = g_tweakTargeting->GetTargetFlowerColor();
      color.a() *= lockBreakAlpha * visorFactor;
      CModelFlags flags(7, 0, 0, lockBreakColor + color);
      x58_targetFlower->Draw(flags);
    }
    if (x1f8_missileBracketTimer != 0.f && x64_missileBracket.IsLoaded()) {
      float t = std::fabs((x1fc_missileBracketScaleTimer - 0.5f * g_tweakTargeting->GetMissileBracketScaleDuration()) /
                          0.5f * g_tweakTargeting->GetMissileBracketScaleDuration());
      float tscale = ((1.f - t) * g_tweakTargeting->GetMissileBracketScaleEnd() +
                      t * g_tweakTargeting->GetMissileBracketScaleStart());
      zeus::CMatrix3f scale(CalculateClampedScale(x10c_currGroupInterp.GetTargetPositionWorld(),
                                                  x10c_currGroupInterp.GetRadiusWorld(),
                                                  x10c_currGroupInterp.GetMinViewportClampScale() *
                                                      g_tweakTargeting->GetMissileBracketClampMin(),
                                                  g_tweakTargeting->GetMissileBracketClampMax(), mgr) *
                            std::fabs(x1f8_missileBracketTimer) / g_tweakTargeting->GetMissileBracketDuration() *
                            tscale / x10c_currGroupInterp.GetFactor());
      for (int i = 0; i < 4; ++i) {
        zeus::CTransform modelXf(
            lockBreakXf * rot * zeus::CMatrix3f(zeus::CVector3f{i < 2 ? 1.f : -1.f, 1.f, i & 0x1 ? 1.f : -1.f}) * scale,
            x10c_currGroupInterp.GetTargetPositionWorld());
        CGraphics::SetModelMatrix(modelXf);
        zeus::CColor color = g_tweakTargeting->GetMissileBracketColor();
        color.a() *= lockBreakAlpha * visorFactor;
        CModelFlags flags(7, 0, 0, lockBreakColor + color);
        x64_missileBracket->Draw(flags);
      }
    }
    zeus::CMatrix3f scale(CalculateClampedScale(x10c_currGroupInterp.GetTargetPositionWorld(),
                                                x10c_currGroupInterp.GetRadiusWorld(),
                                                x10c_currGroupInterp.GetMinViewportClampScale() *
                                                    g_tweakTargeting->GetChargeGaugeClampMin(),
                                                g_tweakTargeting->GetChargeGaugeClampMax(), mgr) *
                          1.f / x10c_currGroupInterp.GetFactor() * g_tweakTargeting->GetOuterBeamSquaresScale());
    zeus::CMatrix3f outerBeamXf = rot * scale;
    for (int i = 0; i < 9; ++i) {
      const SOuterItemInfo& info = xe0_outerBeamIconSquares[i];
      if (info.x0_model.IsLoaded()) {
        zeus::CTransform modelXf(lockBreakXf * outerBeamXf * zeus::CMatrix3f::RotateY(info.x10_rotAng),
                                 x10c_currGroupInterp.GetTargetPositionWorld());
        CGraphics::SetModelMatrix(modelXf);
        zeus::CColor color = g_tweakTargeting->GetOuterBeamSquareColor();
        color.a() *= lockBreakAlpha * visorFactor;
        CModelFlags flags(7, 0, 0, lockBreakColor + color);
        info.x0_model->Draw(flags);
      }
    }
    if (xc4_chargeGauge.x0_model.IsLoaded()) {
      zeus::CMatrix3f scale(CalculateClampedScale(x10c_currGroupInterp.GetTargetPositionWorld(),
                                                  x10c_currGroupInterp.GetRadiusWorld(),
                                                  x10c_currGroupInterp.GetMinViewportClampScale() *
                                                      g_tweakTargeting->GetChargeGaugeClampMin(),
                                                  g_tweakTargeting->GetChargeGaugeClampMax(), mgr) *
                            g_tweakTargeting->GetChargeGaugeScale() / x10c_currGroupInterp.GetFactor());
      zeus::CMatrix3f chargeGaugeXf = rot * scale * zeus::CMatrix3f::RotateY(xc4_chargeGauge.x10_rotAng);
      float pulseT = std::fabs(std::fmod(CGraphics::GetSecondsMod900(), g_tweakTargeting->GetChargeGaugePulsePeriod()));
      zeus::CColor gaugeColor =
          zeus::CColor::lerp(g_tweakTargeting->GetChargeGaugeNonFullColor(),
                             zeus::CColor::lerp(g_tweakTargeting->GetChargeGaugePulseColorHigh(),
                                                g_tweakTargeting->GetChargeGaugePulseColorLow(),
                                                pulseT < 0.5f * g_tweakTargeting->GetChargeGaugePulsePeriod()
                                                    ? pulseT / (0.5f * g_tweakTargeting->GetChargeGaugePulsePeriod())
                                                    : (g_tweakTargeting->GetChargeGaugePulsePeriod() - pulseT) /
                                                          (0.5f * g_tweakTargeting->GetChargeGaugePulsePeriod())),
                             x214_fullChargeFadeTimer / g_tweakTargeting->GetFullChargeFadeDuration());
      zeus::CTransform modelXf(lockBreakXf * chargeGaugeXf, x10c_currGroupInterp.GetTargetPositionWorld());
      CGraphics::SetModelMatrix(modelXf);
      zeus::CColor color = gaugeColor;
      color.a() *= lockBreakAlpha * visorFactor;
      CModelFlags flags(7, 0, 0, lockBreakColor + color);
      xc4_chargeGauge.x0_model->Draw(flags);

      if (xa0_chargeTickFirst.IsLoaded()) {
        const CPlayerGun* gun = mgr.GetPlayer().GetPlayerGun();
        int numTicks =
            int(g_tweakTargeting->GetChargeTickCount() * (gun->IsCharging() ? gun->GetChargeBeamFactor() : 0.f));
        for (int i = 0; i < numTicks; ++i) {
          CModelFlags flags(7, 0, 0, lockBreakColor + color);
          xa0_chargeTickFirst->Draw(flags);
          modelXf.rotateLocalY(g_tweakTargeting->GetChargeTickAnglePitch());
          CGraphics::SetModelMatrix(modelXf);
        }
      }
    }
    if (x208_lockonTimer > 0.f && x70_innerBeamIcon.IsLoaded()) {
      const zeus::CColor* iconColor;
      switch (x200_beam) {
      case CPlayerState::EBeamId::Power:
        iconColor = &g_tweakTargeting->GetInnerBeamColorPower();
        break;
      case CPlayerState::EBeamId::Ice:
        iconColor = &g_tweakTargeting->GetInnerBeamColorIce();
        break;
      case CPlayerState::EBeamId::Wave:
        iconColor = &g_tweakTargeting->GetInnerBeamColorWave();
        break;
      default:
        iconColor = &g_tweakTargeting->GetInnerBeamColorPlasma();
        break;
      }
      zeus::CMatrix3f scale(
          CalculateClampedScale(x10c_currGroupInterp.GetTargetPositionWorld(), x10c_currGroupInterp.GetRadiusWorld(),
                                x10c_currGroupInterp.GetMinViewportClampScale() *
                                    g_tweakTargeting->GetInnerBeamClampMin(),
                                g_tweakTargeting->GetInnerBeamClampMax(), mgr) *
          g_tweakTargeting->GetInnerBeamScale() * (x208_lockonTimer / g_tweakTargeting->GetLockonDuration()) /
          x10c_currGroupInterp.GetFactor());
      zeus::CTransform modelXf(lockBreakXf * rot * scale, x10c_currGroupInterp.GetTargetPositionWorld());
      CGraphics::SetModelMatrix(modelXf);
      zeus::CColor color = *iconColor;
      color.a() *= lockBreakAlpha * visorFactor;
      CModelFlags flags(7, 0, 0, lockBreakColor + color);
      x70_innerBeamIcon->Draw(flags);
    }
    if (x210_lockFireTimer > 0.f && x7c_lockFire.IsLoaded()) {
      zeus::CMatrix3f scale(CalculateClampedScale(x10c_currGroupInterp.GetTargetPositionWorld(),
                                                  x10c_currGroupInterp.GetRadiusWorld(),
                                                  x10c_currGroupInterp.GetMinViewportClampScale() *
                                                      g_tweakTargeting->GetLockFireClampMin(),
                                                  g_tweakTargeting->GetLockFireClampMax(), mgr) *
                            g_tweakTargeting->GetLockFireScale() / x10c_currGroupInterp.GetFactor());
      zeus::CTransform modelXf(lockBreakXf * rot * scale * zeus::CMatrix3f::RotateY(x1f0_xrayRetAngle),
                               x10c_currGroupInterp.GetTargetPositionWorld());
      CGraphics::SetModelMatrix(modelXf);
      zeus::CColor color = g_tweakTargeting->GetLockFireColor();
      color.a() *= visorFactor * lockBreakAlpha * (x210_lockFireTimer / g_tweakTargeting->GetLockFireDuration());
      CModelFlags flags(7, 0, 0, lockBreakColor + color);
      x7c_lockFire->Draw(flags);
    }
    if (x208_lockonTimer > 0.f && x88_lockDagger.IsLoaded()) {
      float t = std::fabs((x210_lockFireTimer - 0.5f * g_tweakTargeting->GetLockFireDuration()) / 0.5f *
                          g_tweakTargeting->GetLockFireDuration());
      float tscale =
          ((1.f - t) * g_tweakTargeting->GetLockDaggerScaleEnd() + t * g_tweakTargeting->GetLockDaggerScaleStart());
      zeus::CMatrix3f scale(
          CalculateClampedScale(x10c_currGroupInterp.GetTargetPositionWorld(), x10c_currGroupInterp.GetRadiusWorld(),
                                x10c_currGroupInterp.GetMinViewportClampScale() *
                                    g_tweakTargeting->GetLockDaggerClampMin(),
                                g_tweakTargeting->GetLockDaggerClampMax(), mgr) *
          tscale * (x208_lockonTimer / g_tweakTargeting->GetLockonDuration()) / x10c_currGroupInterp.GetFactor());
      zeus::CMatrix3f lockDaggerXf = rot * scale;
      for (int i = 0; i < 3; ++i) {
        float ang;
        switch (i) {
        case 0:
          ang = g_tweakTargeting->GetLockDaggerAngle0();
          break;
        case 1:
          ang = g_tweakTargeting->GetLockDaggerAngle1();
          break;
        default:
          ang = g_tweakTargeting->GetLockDaggerAngle2();
          break;
        }
        zeus::CTransform modelXf(lockBreakXf * lockDaggerXf * zeus::CMatrix3f::RotateY(ang),
                                 x10c_currGroupInterp.GetTargetPositionWorld());
        CGraphics::SetModelMatrix(modelXf);
        zeus::CColor color = g_tweakTargeting->GetLockDaggerColor();
        color.a() *= visorFactor * lockBreakAlpha;
        CModelFlags flags(7, 0, 0, lockBreakColor + color);
        x88_lockDagger->Draw(flags);
      }
    }
  }
}

void CCompoundTargetReticle::DrawNextLockOnGroup(const zeus::CMatrix3f& rot, const CStateManager& mgr) const {
  if (x28_noDrawTicks > 0)
    return;

  zeus::CVector3f position = x174_nextGroupInterp.GetTargetPositionWorld();
  float visorFactor = mgr.GetPlayerState()->GetVisorTransitionFactor();

  bool scanRet = false;
  bool xrayRet = false;
  bool thermalRet = false;
  switch (x20_prevState) {
  case EReticleState::Scan:
    scanRet = true;
    break;
  case EReticleState::XRay:
    xrayRet = true;
    break;
  case EReticleState::Thermal:
    thermalRet = true;
    break;
  default:
    break;
  }

  if (!xrayRet && x174_nextGroupInterp.GetFactor() > 0.f && x40_seeker.IsLoaded()) {
    zeus::CMatrix3f scale(
        CalculateClampedScale(position, x174_nextGroupInterp.GetRadiusWorld(),
                              x174_nextGroupInterp.GetMinViewportClampScale() * g_tweakTargeting->GetSeekerClampMin(),
                              g_tweakTargeting->GetSeekerClampMax(), mgr) *
        g_tweakTargeting->GetSeekerScale());
    zeus::CTransform modelXf(rot * zeus::CMatrix3f::RotateY(x1ec_seekerAngle) * scale,
                             x174_nextGroupInterp.GetTargetPositionWorld());
    CGraphics::SetModelMatrix(modelXf);
    zeus::CColor color = g_tweakTargeting->GetSeekerColor();
    color.a() *= x174_nextGroupInterp.GetFactor();
    CModelFlags flags(7, 0, 0, color);
    x40_seeker->Draw(flags);
  }

  if (xrayRet && xac_xrayRetRing.IsLoaded()) {
    zeus::CMatrix3f scale(
        CalculateClampedScale(position, x174_nextGroupInterp.GetRadiusWorld(),
                              x174_nextGroupInterp.GetMinViewportClampScale() * g_tweakTargeting->GetReticuleClampMin(),
                              g_tweakTargeting->GetReticuleClampMax(), mgr) *
        g_tweakTargeting->GetReticuleScale());
    zeus::CTransform modelXf(rot * scale * zeus::CMatrix3f::RotateY(x1f0_xrayRetAngle),
                             x174_nextGroupInterp.GetTargetPositionWorld());
    CGraphics::SetModelMatrix(modelXf);
    zeus::CColor color = g_tweakTargeting->GetXRayRetRingColor();
    color.a() *= visorFactor;
    CModelFlags flags(7, 0, 0, color);
    xac_xrayRetRing->Draw(flags);
  }

  if (thermalRet && xb8_thermalReticle.IsLoaded()) {
    zeus::CMatrix3f scale(
        CalculateClampedScale(position, x174_nextGroupInterp.GetRadiusWorld(),
                              x174_nextGroupInterp.GetMinViewportClampScale() * g_tweakTargeting->GetReticuleClampMin(),
                              g_tweakTargeting->GetReticuleClampMax(), mgr) *
        g_tweakTargeting->GetReticuleScale());
    zeus::CTransform modelXf(rot * scale, x174_nextGroupInterp.GetTargetPositionWorld());
    CGraphics::SetModelMatrix(modelXf);
    zeus::CColor color = g_tweakTargeting->GetThermalReticuleColor();
    color.a() *= visorFactor;
    CModelFlags flags(7, 0, 0, color);
    xb8_thermalReticle->Draw(flags);
  }

  if (scanRet && visorFactor > 0.f) {
    float factor = visorFactor * x174_nextGroupInterp.GetFactor();
    zeus::CMatrix3f scale(CalculateClampedScale(position, x174_nextGroupInterp.GetRadiusWorld(),
                                                x174_nextGroupInterp.GetMinViewportClampScale() *
                                                    g_tweakTargeting->GetScanTargetClampMin(),
                                                g_tweakTargeting->GetScanTargetClampMax(), mgr) *
                          (1.f / factor));
    zeus::CTransform modelXf(rot * scale, x174_nextGroupInterp.GetTargetPositionWorld());
    CGraphics::SetModelMatrix(modelXf);
    // compare, GX_LESS, no update
    float alpha = 0.5f * factor;
    zeus::CColor color = g_tweakGuiColors->GetScanReticuleColor();
    color.a() *= alpha;
    for (int i = 0; i < 2; ++i) {
      float lineWidth = i ? 2.5f : 1.f;
      auto& rend = *m_scanRetRenderer.m_lineRenderers[i];
      rend.Reset();
      rend.AddVertex({-0.5f, 0.f, 0.f}, color, lineWidth);
      rend.AddVertex({-20.5f, 0.f, 0.f}, color, lineWidth);
      rend.AddVertex({0.5f, 0.f, 0.f}, color, lineWidth);
      rend.AddVertex({20.5f, 0.f, 0.f}, color, lineWidth);
      rend.AddVertex({0.f, 0.f, -0.5f}, color, lineWidth);
      rend.AddVertex({0.f, 0.f, -20.5f}, color, lineWidth);
      rend.AddVertex({0.f, 0.f, 0.5f}, color, lineWidth);
      rend.AddVertex({0.f, 0.f, 20.5f}, color, lineWidth);
      rend.Render();

      for (int j = 0; j < 4; ++j) {
        float xSign = j < 2 ? -1.f : 1.f;
        float zSign = (j & 0x1) ? -1.f : 1.f;
        // begin line strip
        auto& rend = *m_scanRetRenderer.m_stripRenderers[i][j];
        rend.Reset();
        rend.AddVertex({0.5f * xSign, 0.f, 0.1f * zSign}, color, lineWidth);
        rend.AddVertex({0.5f * xSign, 0.f, 0.35f * zSign}, color, lineWidth);
        rend.AddVertex({0.35f * xSign, 0.f, 0.5f * zSign}, color, lineWidth);
        rend.AddVertex({0.1f * xSign, 0.f, 0.5f * zSign}, color, lineWidth);
        rend.Render();
      }
    }
  }
}

void CCompoundTargetReticle::DrawOrbitZoneGroup(const zeus::CMatrix3f& rot, const CStateManager& mgr) const {
  if (x28_noDrawTicks > 0)
    return;

  if (x1e8_crosshairsScale > 0.f && x34_crosshairs.IsLoaded()) {
    CGraphics::SetModelMatrix(zeus::CTransform(rot, xf4_targetPos) * zeus::CTransform::Scale(x1e8_crosshairsScale));
    zeus::CColor color = g_tweakTargeting->GetCrosshairsColor();
    color.a() *= x1e8_crosshairsScale;
    CModelFlags flags(7, 0, 0, color);
    x34_crosshairs->Draw(flags);
  }
}

void CCompoundTargetReticle::UpdateTargetParameters(CTargetReticleRenderState& state, const CStateManager& mgr) {
  if (auto act = TCastToConstPtr<CActor>(mgr.GetAllObjectList().GetObjectById(state.GetTargetId()))) {
    state.SetRadiusWorld(CalculateRadiusWorld(*act, mgr));
    state.SetTargetPositionWorld(CalculatePositionWorld(*act, mgr));
  } else if (state.GetIsOrbitZoneIdlePosition()) {
    state.SetRadiusWorld(1.f);
    state.SetTargetPositionWorld((x20_prevState == EReticleState::XRay || x20_prevState == EReticleState::Thermal)
                                     ? x100_laggingTargetPos
                                     : xf4_targetPos);
  }
}

float CCompoundTargetReticle::CalculateRadiusWorld(const CActor& act, const CStateManager& mgr) const {
  auto tb = act.GetTouchBounds();
  zeus::CAABox aabb = tb ? *tb : zeus::CAABox(act.GetAimPosition(mgr, 0.f), act.GetAimPosition(mgr, 0.f));

  float radius;
  zeus::CVector3f delta = aabb.max - aabb.min;
  switch (g_tweakTargeting->GetTargetRadiusMode()) {
  case 0: {
    radius = std::min(delta.x(), std::min(delta.y(), delta.z())) * 0.5f;
    break;
  }
  case 1: {
    radius = std::max(delta.x(), std::max(delta.y(), delta.z())) * 0.5f;
    break;
  }
  default: {
    radius = (delta.x() + delta.y() + delta.z()) / 6.f;
    break;
  }
  }

  return radius > 0.f ? radius : 1.f;
}

zeus::CVector3f CCompoundTargetReticle::CalculatePositionWorld(const CActor& act, const CStateManager& mgr) const {
  if (x20_prevState == EReticleState::Scan)
    return act.GetOrbitPosition(mgr);
  return act.GetAimPosition(mgr, 0.f);
}

zeus::CVector3f CCompoundTargetReticle::CalculateOrbitZoneReticlePosition(const CStateManager& mgr, bool lag) const {
  const CGameCamera* curCam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  float distMul =
      224.f / float(g_tweakPlayer->GetOrbitScreenBoxHalfExtentY(0)) / std::tan(zeus::degToRad(0.5f * curCam->GetFov()));
  zeus::CTransform camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
  zeus::CVector3f lookDir = camXf.basis[1];
  if (lag)
    lookDir = x10_laggingOrientation.transform(lookDir);
  return lookDir * distMul + camXf.origin;
}

bool CCompoundTargetReticle::IsGrappleTarget(TUniqueId uid, const CStateManager& mgr) const {
  const TCastToConstPtr<CScriptGrapplePoint>& ptr = TCastToConstPtr<CScriptGrapplePoint>(mgr.GetObjectById(uid));
  return ptr.operator bool() && ptr->GetActive();
}

float CCompoundTargetReticle::CalculateClampedScale(const zeus::CVector3f& pos, float scale, float clampMin,
                                                    float clampMax, const CStateManager& mgr) {
  const CGameCamera* cam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
  zeus::CVector3f viewPos = cam->GetTransform().transposeRotate(pos - cam->GetTransform().origin);
  viewPos = zeus::CVector3f(viewPos.x(), viewPos.z(), -viewPos.y());
  float realX = cam->GetPerspectiveMatrix().multiplyOneOverW(viewPos).x();
  float offsetX = cam->GetPerspectiveMatrix().multiplyOneOverW(viewPos + zeus::CVector3f(scale, 0.f, 0.f)).x();
  float unclampedX = (offsetX - realX) * 640;
  return zeus::clamp(clampMin, unclampedX, clampMax) / unclampedX * scale;
}

void CCompoundTargetReticle::Touch() {
  if (x34_crosshairs.IsLoaded())
    x34_crosshairs->Touch(0);
  if (x40_seeker.IsLoaded())
    x40_seeker->Touch(0);
  if (x4c_lockConfirm.IsLoaded())
    x4c_lockConfirm->Touch(0);
  if (x58_targetFlower.IsLoaded())
    x58_targetFlower->Touch(0);
  if (x64_missileBracket.IsLoaded())
    x64_missileBracket->Touch(0);
  if (x70_innerBeamIcon.IsLoaded())
    x70_innerBeamIcon->Touch(0);
  if (x7c_lockFire.IsLoaded())
    x7c_lockFire->Touch(0);
  if (x88_lockDagger.IsLoaded())
    x88_lockDagger->Touch(0);
  if (x94_grapple.IsLoaded())
    x94_grapple->Touch(0);
  if (xa0_chargeTickFirst.IsLoaded())
    xa0_chargeTickFirst->Touch(0);
  if (xac_xrayRetRing.IsLoaded())
    xac_xrayRetRing->Touch(0);
  if (xb8_thermalReticle.IsLoaded())
    xb8_thermalReticle->Touch(0);
  if (xc4_chargeGauge.x0_model.IsLoaded())
    xc4_chargeGauge.x0_model->Touch(0);
  if (x3e0_scanReticleRing.IsLoaded())
    x3e0_scanReticleRing->Touch(0);
  if (x3ec_scanReticleBracket.IsLoaded())
    x3ec_scanReticleBracket->Touch(0);
  if (x3f8_scanReticleProgress.IsLoaded())
    x3f8_scanReticleProgress->Touch(0);
  for (SOuterItemInfo& info : xe0_outerBeamIconSquares)
    if (info.x0_model.IsLoaded())
      info.x0_model->Touch(0);
}

} // namespace urde
