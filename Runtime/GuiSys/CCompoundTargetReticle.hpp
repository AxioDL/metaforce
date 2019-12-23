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

  CTargetReticleRenderState(TUniqueId target, float radiusWorld, const zeus::CVector3f& positionWorld, float factor,
                            float minVpClampScale, bool orbitZoneIdlePosition)
  : x0_target(target)
  , x4_radiusWorld(radiusWorld)
  , x8_positionWorld(positionWorld)
  , x14_factor(factor)
  , x18_minVpClampScale(minVpClampScale)
  , x1c_orbitZoneIdlePosition(orbitZoneIdlePosition) {}
  void SetTargetId(TUniqueId id) { x0_target = id; }
  void SetFactor(float f) { x14_factor = f; }
  void SetIsOrbitZoneIdlePosition(bool b) { x1c_orbitZoneIdlePosition = b; }
  float GetMinViewportClampScale() const { return x18_minVpClampScale; }
  float GetFactor() const { return x14_factor; }
  float GetRadiusWorld() const { return x4_radiusWorld; }
  const zeus::CVector3f& GetTargetPositionWorld() const { return x8_positionWorld; }
  bool GetIsOrbitZoneIdlePosition() const { return x1c_orbitZoneIdlePosition; }
  void SetTargetPositionWorld(const zeus::CVector3f& pos) { x8_positionWorld = pos; }
  void SetRadiusWorld(float r) { x4_radiusWorld = r; }
  TUniqueId GetTargetId() const { return x0_target; }
  void SetMinViewportClampScale(float s) { x18_minVpClampScale = s; }
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
    SOuterItemInfo(std::string_view);
  };

private:
  enum class EReticleState { Combat, Scan, XRay, Thermal, Four, Unspecified };

  zeus::CQuaternion x0_leadingOrientation;
  zeus::CQuaternion x10_laggingOrientation;
  EReticleState x20_prevState = EReticleState::Unspecified;
  EReticleState x24_nextState = EReticleState::Unspecified;
  mutable u32 x28_noDrawTicks = 0;
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
  // u8 x21b_ = 0;
  // u32 x21c_;
  // u32 x220_;
  // u32 x228_;

  // RS5
  float x214_ = 0.f;
  float x21c_ = 0.f;
  float x220_ = 0.f;
  float x224_ = 0.f;
  zeus::CVector3f x228_ = zeus::skZero3f;
  zeus::CVector3f x234_ = zeus::skZero3f;
  zeus::CVector3f x240_ = zeus::skZero3f;
  float x24c_ = 0.f;
  float x250_ = 1.f;
  zeus::CColor x254_ = zeus::skPurple;
  float x258_ = 1.f;
  zeus::CColor x25c_ = zeus::skPurple;
  float x260_ = 1.f;
  float x264_ = 0.f;
  zeus::CVector3f x268_ = zeus::skZero3f;
  float x274_ = 1.f;
  float x278_ = 1.f;
  TUniqueId x27c_nextTargetId = kInvalidUniqueId;
  zeus::CVector3f x280_nextTargetOrbitLockOrigin = zeus::skZero3f;
  float x28c_orbitLockArmRotation = 0.f;
  zeus::CVector3f x290_uiScale = zeus::skOne3f;
  zeus::CColor x29c_nextTargetOrbitLockColor = zeus::skGreen;
  float x2a0_nextTargetOrbitLockAlpha = 0.f;
  zeus::CVector3f x2a4_uiPositionOffset = zeus::skZero3f;
  float x2b0_ = 0.f;
  zeus::CVector3f x2b4_nextTargetOrbitPosition = zeus::skZero3f;
  zeus::CVector3f x2c0_orbitLockArmNextOrigin = zeus::skZero3f;
  bool x2cc_orbitLockArmFading = false;
  float x2d0_orbitLockArmFadeAmt = 0.f;
  float x2d4_orbitLockArmFadeDt = 0.f;
  zeus::CVector3f x2d8_orbitLockArm1Origin = zeus::skZero3f;
  zeus::CVector3f x2e4_orbitLockArmFadeOrigin = zeus::skZero3f;
  TUniqueId x2f0_ = kInvalidUniqueId;
  zeus::CVector3f x2f2_orbitLockOrigin = zeus::skZero3f;
  zeus::CColor x300_orbitLockArmColor = zeus::skGreen;
  float x304_orbitLockArmAlpha = 1.f;
  zeus::CColor x308_orbitLockBaseColor = zeus::skGreen;
  float x30c_orbitLockBaseAlpha = 1.f;
  float x310_ = 0.f;
  float x314_ = 0.f;
  zeus::CVector3f x318_ = zeus::skZero3f;
  float x324_ = 0.f;
  zeus::CVector3f x328_orbitLockBaseScale = zeus::skOne3f;
  float x334_ = 0.f;
  float x338_orbitLockBracketsRotation = 0.f;
  float x33c_prevPlayerAngle = 0.f;
  float x340_orbitLockTechScrollTime = 0.f;
  float x344_orbitLockTechRotation = 0.f;
  float x348_playerAngle = 0.f;
  bool x34c_ = false;
  float x350_nextGrappleOrbitLockAlpha = 0.f;
  zeus::CVector3f x354_ = zeus::skZero3f;
  zeus::CVector3f x360_ = zeus::skOne3f;
  zeus::CColor x36c_lerped_color_ = zeus::skWhite;
  float x370_ = 1.f;
  float x374_ = 0.f;
  float x378_ = 0.f;
  float x37c_ = 0.f;
  float x380_ = 0.f;
  float x384_ = 0.f;
  u32 x388_ = 0;

  TCachedToken<CModel> x38c_combatAimingCenter;
  TCachedToken<CModel> x398_combatAimingArm;
  TCachedToken<CModel> x3a4_orbitLockArm;
  TCachedToken<CModel> x3b0_orbitLockTech;
  TCachedToken<CModel> x3bc_orbitLockBrackets;
  TCachedToken<CModel> x3c8_orbitLockBase;
  TCachedToken<CModel> x3d4_offScreen;
  TCachedToken<CModel> x3e0_scanReticleRing;
  TCachedToken<CModel> x3ec_scanReticleBracket;
  TCachedToken<CModel> x3f8_scanReticleProgress;

  float x404_;
  zeus::CVector3f x410_ = zeus::skZero3f;
  zeus::CVector3f x41c_ = zeus::skZero3f;
  zeus::CVector3f x428_ = zeus::skZero3f;
  zeus::CVector3f x434_ = zeus::skZero3f;
  int x440_ = 0;
  std::array<TUniqueId, 9> x444_;
  TUniqueId x456_ = kInvalidUniqueId;

  struct SScanReticuleRenderer {
    std::optional<CLineRenderer> m_lineRenderers[2];
    std::optional<CLineRenderer> m_stripRenderers[2][4];
    SScanReticuleRenderer();
  };
  mutable SScanReticuleRenderer m_scanRetRenderer;

  void DrawGrapplePoint(const CScriptGrapplePoint& point, float t, const CStateManager& mgr, const zeus::CMatrix3f& rot,
                        bool zEqual) const;

public:
  CCompoundTargetReticle(const CStateManager&);

  void SetLeadingOrientation(const zeus::CQuaternion& o) { x0_leadingOrientation = o; }
  bool CheckLoadComplete() { return true; }
  EReticleState GetDesiredReticleState(const CStateManager&) const;
  void Update(float, const CStateManager&);
  void UpdateNextLockOnGroupRS5(float dt, const CStateManager& mgr);
  void UpdateCurrLockOnGroupRS5(float dt, const CStateManager& mgr);
  void UpdateNewGroup3(float, const CStateManager&);
  void UpdateNewGroup4(float, const CStateManager&);
  void UpdateNewGroup5(float, const CStateManager&);
  void UpdateNewGroup6(float, const CStateManager&);
  void UpdateCurrLockOnGroup(float, const CStateManager&);
  void UpdateNextLockOnGroup(float, const CStateManager&);
  void UpdateOrbitZoneGroup(float, const CStateManager&);
  void Draw(const CStateManager&, bool hideLockon) const;
  void DrawNextLockOnGroupRS5(const zeus::CMatrix3f& rot, const CStateManager& mgr) const;
  void DrawCurrLockOnGroupRS5(const zeus::CMatrix3f& rot, const CStateManager& mgr) const;
  void DrawNewGroup3(const zeus::CMatrix3f& rot, const CStateManager&) const;
  void DrawNewGroup4(const zeus::CMatrix3f& rot, const CStateManager&) const;
  void DrawNewGroup5(const zeus::CMatrix3f& rot, const CStateManager&) const;
  void DrawCurrLockOnGroup(const zeus::CMatrix3f& rot, const CStateManager&) const;
  void DrawNextLockOnGroup(const zeus::CMatrix3f& rot, const CStateManager&) const;
  void DrawOrbitZoneGroup(const zeus::CMatrix3f& rot, const CStateManager&) const;
  void DrawGrappleGroup(const zeus::CMatrix3f& rot, const CStateManager&, bool) const;
  void UpdateTargetParameters(CTargetReticleRenderState&, const CStateManager&);
  float CalculateRadiusWorld(const CActor&, const CStateManager&) const;
  zeus::CVector3f CalculatePositionWorld(const CActor&, const CStateManager&) const;
  zeus::CVector3f CalculateOrbitZoneReticlePosition(const CStateManager& mgr, bool lag) const;
  bool IsGrappleTarget(TUniqueId, const CStateManager&) const;
  static float CalculateClampedScale(const zeus::CVector3f&, float, float, float, const CStateManager&);
  void Touch();
};
} // namespace urde
