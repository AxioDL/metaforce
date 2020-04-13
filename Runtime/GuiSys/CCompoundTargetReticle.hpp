#pragma once

#include <vector>

#include "Runtime/CPlayerState.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/Graphics/CLineRenderer.hpp"
#include "Runtime/RetroTypes.hpp"

#include <zeus/CQuaternion.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CActor;
class CModel;
class CScriptGrapplePoint;
class CStateManager;

class CTargetReticleRenderState {
  TUniqueId x0_target;
  float x4_radiusWorld;
  zeus::CVector3f x8_positionWorld;
  float x14_factor;
  float x18_minVpClampScale;
  bool x1c_orbitZoneIdlePosition;

public:
  static const CTargetReticleRenderState skZeroRenderState;

  constexpr CTargetReticleRenderState(TUniqueId target, float radiusWorld, const zeus::CVector3f& positionWorld,
                                      float factor, float minVpClampScale, bool orbitZoneIdlePosition)
  : x0_target(target)
  , x4_radiusWorld(radiusWorld)
  , x8_positionWorld(positionWorld)
  , x14_factor(factor)
  , x18_minVpClampScale(minVpClampScale)
  , x1c_orbitZoneIdlePosition(orbitZoneIdlePosition) {}
  constexpr void SetTargetId(TUniqueId id) { x0_target = id; }
  constexpr void SetFactor(float factor) { x14_factor = factor; }
  constexpr void SetIsOrbitZoneIdlePosition(bool orbit) { x1c_orbitZoneIdlePosition = orbit; }
  constexpr float GetMinViewportClampScale() const { return x18_minVpClampScale; }
  constexpr float GetFactor() const { return x14_factor; }
  constexpr float GetRadiusWorld() const { return x4_radiusWorld; }
  constexpr const zeus::CVector3f& GetTargetPositionWorld() const { return x8_positionWorld; }
  constexpr bool GetIsOrbitZoneIdlePosition() const { return x1c_orbitZoneIdlePosition; }
  constexpr void SetTargetPositionWorld(const zeus::CVector3f& position) { x8_positionWorld = position; }
  constexpr void SetRadiusWorld(float radius) { x4_radiusWorld = radius; }
  constexpr TUniqueId GetTargetId() const { return x0_target; }
  constexpr void SetMinViewportClampScale(float scale) { x18_minVpClampScale = scale; }
  static void InterpolateWithClamp(const CTargetReticleRenderState& a, CTargetReticleRenderState& out,
                                   const CTargetReticleRenderState& b, float t);
};

class CCompoundTargetReticle {
public:
  struct SOuterItemInfo {
    TCachedToken<CModel> x0_model;
    float xc_offshootBaseAngle = 0.f;
    float x10_rotAng = 0.f;
    float x14_baseAngle = 0.f;
    float x18_offshootAngleDelta = 0.f;
    explicit SOuterItemInfo(std::string_view);
  };

private:
  enum class EReticleState { Combat, Scan, XRay, Thermal, Four, Unspecified };

  zeus::CQuaternion x0_leadingOrientation;
  zeus::CQuaternion x10_laggingOrientation;
  EReticleState x20_prevState = EReticleState::Unspecified;
  EReticleState x24_nextState = EReticleState::Unspecified;
  u32 x28_noDrawTicks = 0;
  float x2c_overshootOffsetHalf;
  float x30_premultOvershootOffset;
  TCachedToken<CModel> x34_crosshairs;
  TCachedToken<CModel> x40_seeker;
  TCachedToken<CModel> x4c_lockConfirm;
  TCachedToken<CModel> x58_targetFlower;
  TCachedToken<CModel> x64_missileBracket;
  TCachedToken<CModel> x70_innerBeamIcon;
  TCachedToken<CModel> x7c_lockFire;
  TCachedToken<CModel> x88_lockDagger;
  TCachedToken<CModel> x94_grapple;
  TCachedToken<CModel> xa0_chargeTickFirst;
  TCachedToken<CModel> xac_xrayRetRing;
  TCachedToken<CModel> xb8_thermalReticle;
  SOuterItemInfo xc4_chargeGauge;
  std::vector<SOuterItemInfo> xe0_outerBeamIconSquares;
  TUniqueId xf0_targetId;
  TUniqueId xf2_nextTargetId;
  zeus::CVector3f xf4_targetPos;
  zeus::CVector3f x100_laggingTargetPos;
  CTargetReticleRenderState x10c_currGroupInterp = CTargetReticleRenderState::skZeroRenderState;
  CTargetReticleRenderState x12c_currGroupA = CTargetReticleRenderState::skZeroRenderState;
  CTargetReticleRenderState x14c_currGroupB = CTargetReticleRenderState::skZeroRenderState;
  float x16c_currGroupDur = 0.f;
  float x170_currGroupTimer = 0.f;
  CTargetReticleRenderState x174_nextGroupInterp = CTargetReticleRenderState::skZeroRenderState;
  CTargetReticleRenderState x194_nextGroupA = CTargetReticleRenderState::skZeroRenderState;
  CTargetReticleRenderState x1b4_nextGroupB = CTargetReticleRenderState::skZeroRenderState;
  float x1d4_nextGroupDur = 0.f;
  float x1d8_nextGroupTimer = 0.f;
  TUniqueId x1dc_grapplePoint0 = kInvalidUniqueId;
  TUniqueId x1de_grapplePoint1 = kInvalidUniqueId;
  float x1e0_grapplePoint0T = 0.f;
  float x1e4_grapplePoint1T = 0.f;
  float x1e8_crosshairsScale = 0.f;
  float x1ec_seekerAngle = 0.f;
  float x1f0_xrayRetAngle = 0.f;
  bool x1f4_missileActive = false;
  float x1f8_missileBracketTimer = 0.f;
  float x1fc_missileBracketScaleTimer = 0.f;
  CPlayerState::EBeamId x200_beam = CPlayerState::EBeamId::Power;
  float x204_chargeGaugeOvershootTimer = 0.f;
  float x208_lockonTimer;
  float x20c_ = 0.f;
  float x210_lockFireTimer = 0.f;
  float x214_fullChargeFadeTimer = 0.f;
  bool x218_beamShot = false;
  bool x219_missileShot = false;
  bool x21a_fullyCharged = false;
  u8 x21b_ = 0;
  u32 x21c_ = 0;
  u32 x220_ = 0;
  u32 x228_ = 0;

  struct SScanReticuleRenderer {
    std::optional<CLineRenderer> m_lineRenderers[2];
    std::optional<CLineRenderer> m_stripRenderers[2][4];
    SScanReticuleRenderer();
  };
  SScanReticuleRenderer m_scanRetRenderer;

  void DrawGrapplePoint(const CScriptGrapplePoint& point, float t, const CStateManager& mgr, const zeus::CMatrix3f& rot,
                        bool zEqual) const;

public:
  explicit CCompoundTargetReticle(const CStateManager&);

  void SetLeadingOrientation(const zeus::CQuaternion& o) { x0_leadingOrientation = o; }
  bool CheckLoadComplete() { return true; }
  EReticleState GetDesiredReticleState(const CStateManager&) const;
  void Update(float, const CStateManager&);
  void UpdateCurrLockOnGroup(float, const CStateManager&);
  void UpdateNextLockOnGroup(float, const CStateManager&);
  void UpdateOrbitZoneGroup(float, const CStateManager&);
  void Draw(const CStateManager&, bool hideLockon);
  void DrawGrappleGroup(const zeus::CMatrix3f& rot, const CStateManager&, bool) const;
  void DrawCurrLockOnGroup(const zeus::CMatrix3f& rot, const CStateManager&) const;
  void DrawNextLockOnGroup(const zeus::CMatrix3f& rot, const CStateManager&);
  void DrawOrbitZoneGroup(const zeus::CMatrix3f& rot, const CStateManager&) const;
  void UpdateTargetParameters(CTargetReticleRenderState&, const CStateManager&);
  float CalculateRadiusWorld(const CActor&, const CStateManager&) const;
  zeus::CVector3f CalculatePositionWorld(const CActor&, const CStateManager&) const;
  zeus::CVector3f CalculateOrbitZoneReticlePosition(const CStateManager& mgr, bool lag) const;
  bool IsGrappleTarget(TUniqueId, const CStateManager&) const;
  static float CalculateClampedScale(const zeus::CVector3f&, float, float, float, const CStateManager&);
  void Touch();
};
} // namespace urde
