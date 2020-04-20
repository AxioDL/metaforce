#include "Runtime/GuiSys/CCompoundTargetReticle.hpp"

#include <cstdlib>

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

float offshoot_func(float f1, float f2, float f3) { return (f1 * 0.5f) + std::sin((f3 - 0.5f) * f2); }

float calculate_premultiplied_overshoot_offset(float f1) { return 2.f * (M_PIF - std::asin(1.f / f1)); }
} // Anonymous namespace

constexpr CTargetReticleRenderState CTargetReticleRenderState::skZeroRenderState(kInvalidUniqueId, 1.f, zeus::skZero3f,
                                                                                 0.f, 1.f, true);

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
, x208_lockonTimer(g_tweakTargeting->GetLockonDuration()) {
  xe0_outerBeamIconSquares.reserve(9);
  for (size_t i = 0; i < xe0_outerBeamIconSquares.capacity(); ++i) {
    xe0_outerBeamIconSquares.emplace_back(fmt::format(FMT_STRING("{}{}"), skOuterBeamIconSquareNameBase, i));
  }
  x34_crosshairs.Lock();
}

CCompoundTargetReticle::SScanReticuleRenderer::SScanReticuleRenderer() {
  CGraphics::CommitResources([this](boo::IGraphicsDataFactory::Context& ctx) {
    for (size_t i = 0; i < m_lineRenderers.size(); ++i) {
      m_lineRenderers[i].emplace(ctx, CLineRenderer::EPrimitiveMode::Lines, 8, nullptr, true, true);
      for (auto& stripRenderer : m_stripRenderers[i]) {
        stripRenderer.emplace(ctx, CLineRenderer::EPrimitiveMode::LineStrip, 4, nullptr, true, true);
      }
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
  const float angle = x10_laggingOrientation.angleFrom(x0_leadingOrientation).asDegrees();
  float t;
  if (angle < 0.1f || angle > 45.f) {
    t = 1.f;
  } else {
    t = std::min(1.f, g_tweakTargeting->GetAngularLagSpeed() * dt / angle);
  }
  x10_laggingOrientation =
      t == 1.f ? x0_leadingOrientation : zeus::CQuaternion::slerp(x10_laggingOrientation, x0_leadingOrientation, t);
  xf4_targetPos = CalculateOrbitZoneReticlePosition(mgr, false);
  x100_laggingTargetPos = CalculateOrbitZoneReticlePosition(mgr, true);
  UpdateCurrLockOnGroup(dt, mgr);
  UpdateNextLockOnGroup(dt, mgr);
  UpdateOrbitZoneGroup(dt, mgr);
  const EReticleState desiredState = GetDesiredReticleState(mgr);
  if (desiredState != x20_prevState && x20_prevState == x24_nextState) {
    x24_nextState = desiredState;
    x28_noDrawTicks = 2;
  }
  if (x20_prevState != x24_nextState && x28_noDrawTicks <= 0) {
    x20_prevState = x24_nextState;
    bool combat = false;
    bool scan = false;
    bool xray = false;
    bool thermal = false;
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
    if (combat) {
      x40_seeker.Lock();
    } else {
      x40_seeker.Unlock();
    }
    if (combat) {
      x4c_lockConfirm.Lock();
    } else {
      x4c_lockConfirm.Unlock();
    }
    if (combat) {
      x58_targetFlower.Lock();
    } else {
      x58_targetFlower.Unlock();
    }
    if (combat) {
      x64_missileBracket.Lock();
    } else {
      x64_missileBracket.Unlock();
    }
    if (combat) {
      x70_innerBeamIcon.Lock();
    } else {
      x70_innerBeamIcon.Unlock();
    }
    if (combat) {
      x7c_lockFire.Lock();
    } else {
      x7c_lockFire.Unlock();
    }
    if (combat) {
      x88_lockDagger.Lock();
    } else {
      x88_lockDagger.Unlock();
    }
    if (combat) {
      xa0_chargeTickFirst.Lock();
    } else {
      xa0_chargeTickFirst.Unlock();
    }
    if (xray) {
      xac_xrayRetRing.Lock();
    } else {
      xac_xrayRetRing.Unlock();
    }
    if (thermal) {
      xb8_thermalReticle.Lock();
    } else {
      xb8_thermalReticle.Unlock();
    }
    if (combat) {
      xc4_chargeGauge.x0_model.Lock();
    } else {
      xc4_chargeGauge.x0_model.Unlock();
    }
    if (!scan) {
      x94_grapple.Lock();
    } else {
      x94_grapple.Unlock();
    }
    for (SOuterItemInfo& info : xe0_outerBeamIconSquares) {
      if (combat) {
        info.x0_model.Lock();
      } else {
        info.x0_model.Unlock();
      }
    }
  }

  const CPlayerGun* gun = mgr.GetPlayer().GetPlayerGun();
  const bool fullyCharged = (gun->IsCharging() ? gun->GetChargeBeamFactor() : 0.f) >= 1.f;
  if (fullyCharged != x21a_fullyCharged) {
    x21a_fullyCharged = fullyCharged;
  }
  if (x21a_fullyCharged) {
    x214_fullChargeFadeTimer = std::min(dt / g_tweakTargeting->GetFullChargeFadeDuration() + x214_fullChargeFadeTimer,
                                        g_tweakTargeting->GetFullChargeFadeDuration());
  } else {
    x214_fullChargeFadeTimer =
        std::max(x214_fullChargeFadeTimer - dt / g_tweakTargeting->GetFullChargeFadeDuration(), 0.f);
  }

  const bool missileActive = gun->GetMissleMode() == CPlayerGun::EMissileMode::Active;
  if (missileActive != x1f4_missileActive) {
    if (x1f8_missileBracketTimer != 0.f) {
      x1f8_missileBracketTimer = FLT_EPSILON - x1f8_missileBracketTimer;
    } else {
      x1f8_missileBracketTimer = FLT_EPSILON;
    }
    x1f4_missileActive = missileActive;
  }

  const CPlayerState::EBeamId beam = gun->GetCurrentBeam();
  if (beam != x200_beam) {
    x204_chargeGaugeOvershootTimer = g_tweakTargeting->GetChargeGaugeOvershootDuration();
    for (size_t i = 0; i < xe0_outerBeamIconSquares.size(); ++i) {
      zeus::CRelAngle baseAngle = g_tweakTargeting->GetOuterBeamSquareAngles(int(beam))[i];
      baseAngle.makeRel();
      SOuterItemInfo& icon = xe0_outerBeamIconSquares[i];
      zeus::CRelAngle offshootAngleDelta = baseAngle.asRadians() - icon.x10_rotAng;
      if ((i & 0x1) == 1) {
        offshootAngleDelta =
            (baseAngle > 0.f) ? zeus::CRelAngle(-2.f * M_PIF - baseAngle) : zeus::CRelAngle(2.f * M_PIF + baseAngle);
      }
      icon.xc_offshootBaseAngle = icon.x10_rotAng;
      icon.x18_offshootAngleDelta = offshootAngleDelta;
      icon.x14_baseAngle = baseAngle;
    }
    zeus::CRelAngle baseAngle = g_tweakTargeting->GetChargeGaugeAngle(int(beam));
    baseAngle.makeRel();
    float offshootAngleDelta = baseAngle.asRadians() - xc4_chargeGauge.x10_rotAng;
    if ((rand() & 0x1) == 1) {
      offshootAngleDelta =
          (offshootAngleDelta > 0.f) ? -2.f * M_PIF - offshootAngleDelta : 2.f * M_PIF + offshootAngleDelta;
    }
    xc4_chargeGauge.xc_offshootBaseAngle = xc4_chargeGauge.x10_rotAng;
    xc4_chargeGauge.x18_offshootAngleDelta = offshootAngleDelta;
    xc4_chargeGauge.x14_baseAngle = baseAngle;
    x200_beam = beam;
    x208_lockonTimer = 0.f;
  }

  if ((gun->GetLastFireButtonStates() & 0x1) != 0) {
    if (!x218_beamShot) {
      x210_lockFireTimer = g_tweakTargeting->GetLockFireDuration();
    }
    x218_beamShot = true;
  } else {
    x218_beamShot = false;
  }

  if ((gun->GetLastFireButtonStates() & 0x2) != 0) {
    if (!x219_missileShot) {
      x1fc_missileBracketScaleTimer = g_tweakTargeting->GetMissileBracketScaleDuration();
    }
    x219_missileShot = true;
  } else {
    x219_missileShot = false;
  }

  if (const TCastToConstPtr<CScriptGrapplePoint> point = mgr.GetObjectById(xf2_nextTargetId)) {
    if (point->GetUniqueId() != x1dc_grapplePoint0) {
      float tmp;
      if (point->GetUniqueId() == x1de_grapplePoint1) {
        tmp = std::max(FLT_EPSILON, x1e4_grapplePoint1T);
      } else {
        tmp = FLT_EPSILON;
      }
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

  if (x1e0_grapplePoint0T > 0.f) {
    x1e0_grapplePoint0T = std::min(dt / 0.5f + x1e0_grapplePoint0T, 1.f);
  }

  if (x1e4_grapplePoint1T > 0.f) {
    x1e4_grapplePoint1T = std::max(0.f, x1e4_grapplePoint1T - dt / 0.5f);
    if (x1e4_grapplePoint1T == 0.f) {
      x1de_grapplePoint1 = kInvalidUniqueId;
    }
  }

  x1f0_xrayRetAngle =
    zeus::CRelAngle(zeus::degToRad(g_tweakTargeting->GetXRayRetAngleSpeed() * dt) + x1f0_xrayRetAngle).asRel();
  x1ec_seekerAngle = zeus::CRelAngle(zeus::degToRad(g_tweakTargeting->GetSeekerAngleSpeed() * dt) + x1ec_seekerAngle).
    asRel();
}

void CTargetReticleRenderState::InterpolateWithClamp(const CTargetReticleRenderState& a, CTargetReticleRenderState& out,
                                                     const CTargetReticleRenderState& b, float t) {
  t = zeus::clamp(0.f, t, 1.f);
  const float omt = 1.f - t;
  out.x4_radiusWorld = omt * a.x4_radiusWorld + t * b.x4_radiusWorld;
  out.x14_factor = omt * a.x14_factor + t * b.x14_factor;
  out.x18_minVpClampScale = omt * a.x18_minVpClampScale + t * b.x18_minVpClampScale;
  out.x8_positionWorld = zeus::CVector3f::lerp(a.x8_positionWorld, b.x8_positionWorld, t);

  if (t == 1.f) {
    out.x0_target = b.x0_target;
  } else if (t == 0.f) {
    out.x0_target = a.x0_target;
  } else {
    out.x0_target = kInvalidUniqueId;
  }
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

void CCompoundTargetReticle::UpdateCurrLockOnGroup(float dt, const CStateManager& mgr) {
  const TUniqueId targetId = mgr.GetPlayer().GetOrbitTargetId();
  if (targetId != xf0_targetId) {
    if (targetId != kInvalidUniqueId) {
      if (const TCastToConstPtr<CScriptGrapplePoint> point = mgr.GetObjectById(targetId)) {
        CSfxManager::SfxStart(SFXui_lockon_grapple, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
      } else {
        CSfxManager::SfxStart(SFXui_lockon_poi, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
      }
    }

    if (targetId == kInvalidUniqueId) {
      x12c_currGroupA = x10c_currGroupInterp;
      x14c_currGroupB.SetFactor(0.f);
      x16c_currGroupDur =
          IsDamageOrbit(mgr.GetPlayer().GetOrbitRequest()) ? 0.65f : g_tweakTargeting->GetCurrLockOnEnterDuration();
    } else {
      x12c_currGroupA = x10c_currGroupInterp;
      if (xf0_targetId == kInvalidUniqueId) {
        x12c_currGroupA.SetTargetId(targetId);
      }
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
    if (x1f8_missileBracketTimer < 0.f) {
      x1f8_missileBracketTimer = std::min(0.f, x1f8_missileBracketTimer + dt);
    } else {
      x1f8_missileBracketTimer = std::min(g_tweakTargeting->GetMissileBracketDuration(), x1f8_missileBracketTimer + dt);
    }
  }

  if (x204_chargeGaugeOvershootTimer > 0.f) {
    x204_chargeGaugeOvershootTimer = std::max(0.f, x204_chargeGaugeOvershootTimer - dt);
    if (x204_chargeGaugeOvershootTimer == 0.f) {
      for (auto& iconSquare : xe0_outerBeamIconSquares) {
        iconSquare.x10_rotAng = iconSquare.x14_baseAngle;
      }
      xc4_chargeGauge.x10_rotAng = xc4_chargeGauge.x14_baseAngle;
      x208_lockonTimer = FLT_EPSILON;
    } else {
      const float offshoot =
          offshoot_func(x2c_overshootOffsetHalf, x30_premultOvershootOffset,
                        1.f - x204_chargeGaugeOvershootTimer / g_tweakTargeting->GetChargeGaugeOvershootDuration());
      for (auto& item : xe0_outerBeamIconSquares) {
        item.x10_rotAng = zeus::CRelAngle(item.x18_offshootAngleDelta * offshoot + item.xc_offshootBaseAngle).asRel();
      }
      xc4_chargeGauge.x10_rotAng =
        zeus::CRelAngle(xc4_chargeGauge.x18_offshootAngleDelta * offshoot + xc4_chargeGauge.xc_offshootBaseAngle).
          asRel();
    }
  }

  if (x208_lockonTimer > 0.f && x208_lockonTimer < g_tweakTargeting->GetLockonDuration()) {
    x208_lockonTimer = std::min(g_tweakTargeting->GetLockonDuration(), x208_lockonTimer + dt);
  }
  if (x210_lockFireTimer > 0.f) {
    x210_lockFireTimer = std::max(0.f, x210_lockFireTimer - dt);
  }
  if (x1fc_missileBracketScaleTimer > 0.f) {
    x1fc_missileBracketScaleTimer = std::max(0.f, x1fc_missileBracketScaleTimer - dt);
  }
}

void CCompoundTargetReticle::UpdateNextLockOnGroup(float dt, const CStateManager& mgr) {
  TUniqueId nextTargetId = mgr.GetPlayer().GetOrbitNextTargetId();
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Scan &&
      mgr.GetPlayer().GetOrbitTargetId() != kInvalidUniqueId) {
    nextTargetId = mgr.GetPlayer().GetOrbitTargetId();
  }

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
  if (xf0_targetId == kInvalidUniqueId && xf2_nextTargetId != kInvalidUniqueId) {
    x20c_ = std::min(1.f, 2.f * dt + x20c_);
  } else {
    x20c_ = std::max(0.f, x20c_ - 2.f * dt);
  }

  if (mgr.GetPlayer().IsShowingCrosshairs() &&
      mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::Scan) {
    x1e8_crosshairsScale = std::min(1.f, dt / g_tweakTargeting->GetCrosshairsScaleDuration() + x1e8_crosshairsScale);
  } else {
    x1e8_crosshairsScale = std::max(0.f, x1e8_crosshairsScale - dt / g_tweakTargeting->GetCrosshairsScaleDuration());
  }
}

void CCompoundTargetReticle::Draw(const CStateManager& mgr, bool hideLockon) {
  if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed &&
      !mgr.GetCameraManager()->IsInCinematicCamera()) {
    SCOPED_GRAPHICS_DEBUG_GROUP("CCompoundTargetReticle::Draw", zeus::skCyan);
    const zeus::CTransform camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
    CGraphics::SetViewPointMatrix(camXf);
    if (!hideLockon) {
      DrawCurrLockOnGroup(camXf.basis, mgr);
      DrawNextLockOnGroup(camXf.basis, mgr);
      DrawOrbitZoneGroup(camXf.basis, mgr);
    }
    DrawGrappleGroup(camXf.basis, mgr, hideLockon);
  }

  if (x28_noDrawTicks > 0) {
    --x28_noDrawTicks;
  }
}

void CCompoundTargetReticle::DrawGrapplePoint(const CScriptGrapplePoint& point, float t, const CStateManager& mgr,
                                              const zeus::CMatrix3f& rot, bool zEqual) const {
  zeus::CVector3f orbitPos = point.GetOrbitPosition(mgr);
  zeus::CColor color;

  if (point.GetGrappleParameters().GetLockSwingTurn()) {
    color = g_tweakTargeting->GetLockedGrapplePointSelectColor();
  } else {
    color = g_tweakTargeting->GetGrapplePointSelectColor();
  }

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
  if (x28_noDrawTicks > 0) {
    return;
  }

  if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::GrappleBeam) && x94_grapple.IsLoaded() &&
      x20_prevState != EReticleState::Scan) {
    if (hideLockon) {
      for (const CEntity* ent : mgr.GetAllObjectList()) {
        if (TCastToConstPtr<CScriptGrapplePoint> point = ent) {
          if (point->GetActive()) {
            if (point->GetAreaIdAlways() != kInvalidAreaId) {
              const CGameArea* area = mgr.GetWorld()->GetAreaAlways(point->GetAreaIdAlways());
              const auto occState =
                  area->IsPostConstructed() ? area->GetOcclusionState() : CGameArea::EOcclusionState::Occluded;
              if (occState != CGameArea::EOcclusionState::Visible) {
                continue;
              }
            }
            float t = 0.f;
            if (point->GetUniqueId() == x1dc_grapplePoint0) {
              t = x1e0_grapplePoint0T;
            } else if (point->GetUniqueId() == x1de_grapplePoint1) {
              t = x1e4_grapplePoint1T;
            }
            if (std::fabs(t) < 0.00001f) {
              DrawGrapplePoint(*point, t, mgr, rot, true);
            }
          }
        }
      }
    } else {
      const TCastToConstPtr<CScriptGrapplePoint> point0 = mgr.GetObjectById(x1dc_grapplePoint0);
      const TCastToConstPtr<CScriptGrapplePoint> point1 = mgr.GetObjectById(x1de_grapplePoint1);
      for (int i = 0; i < 2; ++i) {
        const CScriptGrapplePoint* point = i == 0 ? point0.GetPtr() : point1.GetPtr();
        const float t = i == 0 ? x1e0_grapplePoint0T : x1e4_grapplePoint1T;
        if (point) {
          DrawGrapplePoint(*point, t, mgr, rot, false);
        }
      }
    }
  }
}

void CCompoundTargetReticle::DrawCurrLockOnGroup(const zeus::CMatrix3f& rot, const CStateManager& mgr) const {
  if (x28_noDrawTicks > 0) {
    return;
  }
  if (x1e0_grapplePoint0T + x1e4_grapplePoint1T > 0 || x10c_currGroupInterp.GetFactor() == 0.f) {
    return;
  }

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
      const int a = rand() % 9;
      const auto b = std::div(a, 3);
      lockBreakRM[b.rem][b.quot] += rand() / float(RAND_MAX) - 0.5f;
    }
    lockBreakXf = lockBreakRM.transposed();
    if (x10c_currGroupInterp.GetFactor() > 0.8f) {
      lockBreakColor = zeus::CColor(1.f, (x10c_currGroupInterp.GetFactor() - 0.8f) * 0.3f / 0.2f);
    }
    lockBreakAlpha = x10c_currGroupInterp.GetFactor() > 0.75f
                         ? 1.f
                         : std::max(0.f, (x10c_currGroupInterp.GetFactor() - 0.55f) / 0.2f);
  }

  if (lockConfirm && x4c_lockConfirm.IsLoaded()) {
    const zeus::CMatrix3f scale(CalculateClampedScale(x10c_currGroupInterp.GetTargetPositionWorld(),
                                                      x10c_currGroupInterp.GetRadiusWorld(),
                                                      x10c_currGroupInterp.GetMinViewportClampScale() *
                                                          g_tweakTargeting->GetLockConfirmClampMin(),
                                                      g_tweakTargeting->GetLockConfirmClampMax(), mgr) *
                                g_tweakTargeting->GetLockConfirmScale() / x10c_currGroupInterp.GetFactor());
    const zeus::CTransform modelXf(lockBreakXf * (rot * zeus::CMatrix3f::RotateY(x1ec_seekerAngle) * scale),
                                   x10c_currGroupInterp.GetTargetPositionWorld());
    CGraphics::SetModelMatrix(modelXf);
    zeus::CColor color = g_tweakTargeting->GetLockConfirmColor();
    color.a() *= lockBreakAlpha;
    const CModelFlags flags(7, 0, 0, lockBreakColor + color);
    x4c_lockConfirm->Draw(flags);
  }

  if (lockReticule) {
    if (x58_targetFlower.IsLoaded()) {
      const zeus::CMatrix3f scale(CalculateClampedScale(x10c_currGroupInterp.GetTargetPositionWorld(),
                                                        x10c_currGroupInterp.GetRadiusWorld(),
                                                        x10c_currGroupInterp.GetMinViewportClampScale() *
                                                            g_tweakTargeting->GetTargetFlowerClampMin(),
                                                        g_tweakTargeting->GetTargetFlowerClampMax(), mgr) *
                                  g_tweakTargeting->GetTargetFlowerScale() / lockBreakAlpha);
      const zeus::CTransform modelXf(lockBreakXf * (rot * zeus::CMatrix3f::RotateY(x1f0_xrayRetAngle) * scale),
                                     x10c_currGroupInterp.GetTargetPositionWorld());
      CGraphics::SetModelMatrix(modelXf);
      zeus::CColor color = g_tweakTargeting->GetTargetFlowerColor();
      color.a() *= lockBreakAlpha * visorFactor;
      const CModelFlags flags(7, 0, 0, lockBreakColor + color);
      x58_targetFlower->Draw(flags);
    }

    if (x1f8_missileBracketTimer != 0.f && x64_missileBracket.IsLoaded()) {
      const float t =
          std::fabs((x1fc_missileBracketScaleTimer - 0.5f * g_tweakTargeting->GetMissileBracketScaleDuration()) / 0.5f *
                    g_tweakTargeting->GetMissileBracketScaleDuration());
      const float tscale = ((1.f - t) * g_tweakTargeting->GetMissileBracketScaleEnd() +
                            t * g_tweakTargeting->GetMissileBracketScaleStart());
      const zeus::CMatrix3f scale(CalculateClampedScale(x10c_currGroupInterp.GetTargetPositionWorld(),
                                                        x10c_currGroupInterp.GetRadiusWorld(),
                                                        x10c_currGroupInterp.GetMinViewportClampScale() *
                                                            g_tweakTargeting->GetMissileBracketClampMin(),
                                                        g_tweakTargeting->GetMissileBracketClampMax(), mgr) *
                                  std::fabs(x1f8_missileBracketTimer) / g_tweakTargeting->GetMissileBracketDuration() *
                                  tscale / x10c_currGroupInterp.GetFactor());
      for (int i = 0; i < 4; ++i) {
        const zeus::CTransform modelXf(
            lockBreakXf * rot * zeus::CMatrix3f(zeus::CVector3f{i < 2 ? 1.f : -1.f, 1.f, i & 0x1 ? 1.f : -1.f}) * scale,
            x10c_currGroupInterp.GetTargetPositionWorld());
        CGraphics::SetModelMatrix(modelXf);
        zeus::CColor color = g_tweakTargeting->GetMissileBracketColor();
        color.a() *= lockBreakAlpha * visorFactor;
        const CModelFlags flags(7, 0, 0, lockBreakColor + color);
        x64_missileBracket->Draw(flags);
      }
    }

    const zeus::CMatrix3f scale(CalculateClampedScale(x10c_currGroupInterp.GetTargetPositionWorld(),
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
      const zeus::CMatrix3f scale(CalculateClampedScale(x10c_currGroupInterp.GetTargetPositionWorld(),
                                                        x10c_currGroupInterp.GetRadiusWorld(),
                                                        x10c_currGroupInterp.GetMinViewportClampScale() *
                                                            g_tweakTargeting->GetChargeGaugeClampMin(),
                                                        g_tweakTargeting->GetChargeGaugeClampMax(), mgr) *
                                  g_tweakTargeting->GetChargeGaugeScale() / x10c_currGroupInterp.GetFactor());
      const zeus::CMatrix3f chargeGaugeXf = rot * scale * zeus::CMatrix3f::RotateY(xc4_chargeGauge.x10_rotAng);
      const float pulseT =
          std::fabs(std::fmod(CGraphics::GetSecondsMod900(), g_tweakTargeting->GetChargeGaugePulsePeriod()));
      const zeus::CColor gaugeColor =
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
      const CModelFlags flags(7, 0, 0, lockBreakColor + color);
      xc4_chargeGauge.x0_model->Draw(flags);

      if (xa0_chargeTickFirst.IsLoaded()) {
        const CPlayerGun* gun = mgr.GetPlayer().GetPlayerGun();
        const int numTicks =
            int(g_tweakTargeting->GetChargeTickCount() * (gun->IsCharging() ? gun->GetChargeBeamFactor() : 0.f));
        for (int i = 0; i < numTicks; ++i) {
          const CModelFlags tickFlags(7, 0, 0, lockBreakColor + color);
          xa0_chargeTickFirst->Draw(tickFlags);
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

      const zeus::CMatrix3f scale(
          CalculateClampedScale(x10c_currGroupInterp.GetTargetPositionWorld(), x10c_currGroupInterp.GetRadiusWorld(),
                                x10c_currGroupInterp.GetMinViewportClampScale() *
                                    g_tweakTargeting->GetInnerBeamClampMin(),
                                g_tweakTargeting->GetInnerBeamClampMax(), mgr) *
          g_tweakTargeting->GetInnerBeamScale() * (x208_lockonTimer / g_tweakTargeting->GetLockonDuration()) /
          x10c_currGroupInterp.GetFactor());
      const zeus::CTransform modelXf(lockBreakXf * rot * scale, x10c_currGroupInterp.GetTargetPositionWorld());
      CGraphics::SetModelMatrix(modelXf);
      zeus::CColor color = *iconColor;
      color.a() *= lockBreakAlpha * visorFactor;
      const CModelFlags flags(7, 0, 0, lockBreakColor + color);
      x70_innerBeamIcon->Draw(flags);
    }

    if (x210_lockFireTimer > 0.f && x7c_lockFire.IsLoaded()) {
      const zeus::CMatrix3f scale(CalculateClampedScale(x10c_currGroupInterp.GetTargetPositionWorld(),
                                                        x10c_currGroupInterp.GetRadiusWorld(),
                                                        x10c_currGroupInterp.GetMinViewportClampScale() *
                                                            g_tweakTargeting->GetLockFireClampMin(),
                                                        g_tweakTargeting->GetLockFireClampMax(), mgr) *
                                  g_tweakTargeting->GetLockFireScale() / x10c_currGroupInterp.GetFactor());
      const zeus::CTransform modelXf(lockBreakXf * rot * scale * zeus::CMatrix3f::RotateY(x1f0_xrayRetAngle),
                                     x10c_currGroupInterp.GetTargetPositionWorld());
      CGraphics::SetModelMatrix(modelXf);
      zeus::CColor color = g_tweakTargeting->GetLockFireColor();
      color.a() *= visorFactor * lockBreakAlpha * (x210_lockFireTimer / g_tweakTargeting->GetLockFireDuration());
      const CModelFlags flags(7, 0, 0, lockBreakColor + color);
      x7c_lockFire->Draw(flags);
    }

    if (x208_lockonTimer > 0.f && x88_lockDagger.IsLoaded()) {
      const float t = std::fabs((x210_lockFireTimer - 0.5f * g_tweakTargeting->GetLockFireDuration()) / 0.5f *
                                g_tweakTargeting->GetLockFireDuration());
      const float tscale =
          ((1.f - t) * g_tweakTargeting->GetLockDaggerScaleEnd() + t * g_tweakTargeting->GetLockDaggerScaleStart());
      const zeus::CMatrix3f scale(
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
        const zeus::CTransform modelXf(lockBreakXf * lockDaggerXf * zeus::CMatrix3f::RotateY(ang),
                                       x10c_currGroupInterp.GetTargetPositionWorld());
        CGraphics::SetModelMatrix(modelXf);
        zeus::CColor color = g_tweakTargeting->GetLockDaggerColor();
        color.a() *= visorFactor * lockBreakAlpha;
        const CModelFlags flags(7, 0, 0, lockBreakColor + color);
        x88_lockDagger->Draw(flags);
      }
    }
  }
}

void CCompoundTargetReticle::DrawNextLockOnGroup(const zeus::CMatrix3f& rot, const CStateManager& mgr) {
  if (x28_noDrawTicks > 0) {
    return;
  }

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
    const zeus::CMatrix3f scale(
        CalculateClampedScale(position, x174_nextGroupInterp.GetRadiusWorld(),
                              x174_nextGroupInterp.GetMinViewportClampScale() * g_tweakTargeting->GetSeekerClampMin(),
                              g_tweakTargeting->GetSeekerClampMax(), mgr) *
        g_tweakTargeting->GetSeekerScale());
    const zeus::CTransform modelXf(rot * zeus::CMatrix3f::RotateY(x1ec_seekerAngle) * scale,
                                   x174_nextGroupInterp.GetTargetPositionWorld());
    CGraphics::SetModelMatrix(modelXf);
    zeus::CColor color = g_tweakTargeting->GetSeekerColor();
    color.a() *= x174_nextGroupInterp.GetFactor();
    const CModelFlags flags(7, 0, 0, color);
    x40_seeker->Draw(flags);
  }

  if (xrayRet && xac_xrayRetRing.IsLoaded()) {
    const zeus::CMatrix3f scale(
        CalculateClampedScale(position, x174_nextGroupInterp.GetRadiusWorld(),
                              x174_nextGroupInterp.GetMinViewportClampScale() * g_tweakTargeting->GetReticuleClampMin(),
                              g_tweakTargeting->GetReticuleClampMax(), mgr) *
        g_tweakTargeting->GetReticuleScale());
    const zeus::CTransform modelXf(rot * scale * zeus::CMatrix3f::RotateY(x1f0_xrayRetAngle),
                                   x174_nextGroupInterp.GetTargetPositionWorld());
    CGraphics::SetModelMatrix(modelXf);
    zeus::CColor color = g_tweakTargeting->GetXRayRetRingColor();
    color.a() *= visorFactor;
    const CModelFlags flags(7, 0, 0, color);
    xac_xrayRetRing->Draw(flags);
  }

  if (thermalRet && xb8_thermalReticle.IsLoaded()) {
    const zeus::CMatrix3f scale(
        CalculateClampedScale(position, x174_nextGroupInterp.GetRadiusWorld(),
                              x174_nextGroupInterp.GetMinViewportClampScale() * g_tweakTargeting->GetReticuleClampMin(),
                              g_tweakTargeting->GetReticuleClampMax(), mgr) *
        g_tweakTargeting->GetReticuleScale());
    const zeus::CTransform modelXf(rot * scale, x174_nextGroupInterp.GetTargetPositionWorld());
    CGraphics::SetModelMatrix(modelXf);
    zeus::CColor color = g_tweakTargeting->GetThermalReticuleColor();
    color.a() *= visorFactor;
    const CModelFlags flags(7, 0, 0, color);
    xb8_thermalReticle->Draw(flags);
  }

  if (scanRet && visorFactor > 0.f) {
    float factor = visorFactor * x174_nextGroupInterp.GetFactor();
    const zeus::CMatrix3f scale(CalculateClampedScale(position, x174_nextGroupInterp.GetRadiusWorld(),
                                                      x174_nextGroupInterp.GetMinViewportClampScale() *
                                                          g_tweakTargeting->GetScanTargetClampMin(),
                                                      g_tweakTargeting->GetScanTargetClampMax(), mgr) *
                                (1.f / factor));
    const zeus::CTransform modelXf(rot * scale, x174_nextGroupInterp.GetTargetPositionWorld());
    CGraphics::SetModelMatrix(modelXf);
    // compare, GX_LESS, no update
    float alpha = 0.5f * factor;
    zeus::CColor color = g_tweakGuiColors->GetScanReticuleColor();
    color.a() *= alpha;
    for (size_t i = 0; i < m_scanRetRenderer.m_lineRenderers.size(); ++i) {
      const float lineWidth = i != 0 ? 2.5f : 1.f;
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

      for (size_t j = 0; j < m_scanRetRenderer.m_stripRenderers[i].size(); ++j) {
        const float xSign = j < 2 ? -1.f : 1.f;
        const float zSign = (j & 0x1) != 0 ? -1.f : 1.f;
        // begin line strip
        auto& stripRend = *m_scanRetRenderer.m_stripRenderers[i][j];
        stripRend.Reset();
        stripRend.AddVertex({0.5f * xSign, 0.f, 0.1f * zSign}, color, lineWidth);
        stripRend.AddVertex({0.5f * xSign, 0.f, 0.35f * zSign}, color, lineWidth);
        stripRend.AddVertex({0.35f * xSign, 0.f, 0.5f * zSign}, color, lineWidth);
        stripRend.AddVertex({0.1f * xSign, 0.f, 0.5f * zSign}, color, lineWidth);
        stripRend.Render();
      }
    }
  }
}

void CCompoundTargetReticle::DrawOrbitZoneGroup(const zeus::CMatrix3f& rot, const CStateManager& mgr) const {
  if (x28_noDrawTicks > 0) {
    return;
  }

  if (x1e8_crosshairsScale > 0.f && x34_crosshairs.IsLoaded()) {
    CGraphics::SetModelMatrix(zeus::CTransform(rot, xf4_targetPos) * zeus::CTransform::Scale(x1e8_crosshairsScale));
    zeus::CColor color = g_tweakTargeting->GetCrosshairsColor();
    color.a() *= x1e8_crosshairsScale;
    const CModelFlags flags(7, 0, 0, color);
    x34_crosshairs->Draw(flags);
  }
}

void CCompoundTargetReticle::UpdateTargetParameters(CTargetReticleRenderState& state, const CStateManager& mgr) {
  if (const auto act = TCastToConstPtr<CActor>(mgr.GetAllObjectList().GetObjectById(state.GetTargetId()))) {
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
  const auto tb = act.GetTouchBounds();
  const zeus::CAABox aabb = tb ? *tb : zeus::CAABox(act.GetAimPosition(mgr, 0.f), act.GetAimPosition(mgr, 0.f));

  float radius;
  const zeus::CVector3f delta = aabb.max - aabb.min;
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
  if (x20_prevState == EReticleState::Scan) {
    return act.GetOrbitPosition(mgr);
  }
  return act.GetAimPosition(mgr, 0.f);
}

zeus::CVector3f CCompoundTargetReticle::CalculateOrbitZoneReticlePosition(const CStateManager& mgr, bool lag) const {
  const CGameCamera* curCam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  const float distMul =
      224.f / float(g_tweakPlayer->GetOrbitScreenBoxHalfExtentY(0)) / std::tan(zeus::degToRad(0.5f * curCam->GetFov()));
  const zeus::CTransform camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
  zeus::CVector3f lookDir = camXf.basis[1];
  if (lag) {
    lookDir = x10_laggingOrientation.transform(lookDir);
  }
  return lookDir * distMul + camXf.origin;
}

bool CCompoundTargetReticle::IsGrappleTarget(TUniqueId uid, const CStateManager& mgr) const {
  return TCastToConstPtr<CScriptGrapplePoint>(mgr.GetAllObjectList().GetObjectById(uid)).operator bool();
}

float CCompoundTargetReticle::CalculateClampedScale(const zeus::CVector3f& pos, float scale, float clampMin,
                                                    float clampMax, const CStateManager& mgr) {
  const CGameCamera* cam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
  zeus::CVector3f viewPos = cam->GetTransform().transposeRotate(pos - cam->GetTransform().origin);
  viewPos = zeus::CVector3f(viewPos.x(), viewPos.z(), -viewPos.y());
  const float realX = cam->GetPerspectiveMatrix().multiplyOneOverW(viewPos).x();
  const float offsetX = cam->GetPerspectiveMatrix().multiplyOneOverW(viewPos + zeus::CVector3f(scale, 0.f, 0.f)).x();
  const float unclampedX = (offsetX - realX) * 640;
  return zeus::clamp(clampMin, unclampedX, clampMax) / unclampedX * scale;
}

void CCompoundTargetReticle::Touch() {
  if (x34_crosshairs.IsLoaded()) {
    x34_crosshairs->Touch(0);
  }

  if (x40_seeker.IsLoaded()) {
    x40_seeker->Touch(0);
  }

  if (x4c_lockConfirm.IsLoaded()) {
    x4c_lockConfirm->Touch(0);
  }

  if (x58_targetFlower.IsLoaded()) {
    x58_targetFlower->Touch(0);
  }

  if (x64_missileBracket.IsLoaded()) {
    x64_missileBracket->Touch(0);
  }

  if (x70_innerBeamIcon.IsLoaded()) {
    x70_innerBeamIcon->Touch(0);
  }

  if (x7c_lockFire.IsLoaded()) {
    x7c_lockFire->Touch(0);
  }

  if (x88_lockDagger.IsLoaded()) {
    x88_lockDagger->Touch(0);
  }

  if (x94_grapple.IsLoaded()) {
    x94_grapple->Touch(0);
  }

  if (xa0_chargeTickFirst.IsLoaded()) {
    xa0_chargeTickFirst->Touch(0);
  }

  if (xac_xrayRetRing.IsLoaded()) {
    xac_xrayRetRing->Touch(0);
  }

  if (xb8_thermalReticle.IsLoaded()) {
    xb8_thermalReticle->Touch(0);
  }

  if (xc4_chargeGauge.x0_model.IsLoaded()) {
    xc4_chargeGauge.x0_model->Touch(0);
  }

  for (SOuterItemInfo& info : xe0_outerBeamIconSquares) {
    if (info.x0_model.IsLoaded()) {
      info.x0_model->Touch(0);
    }
  }
}

} // namespace urde
