#pragma once

#include "Runtime/Tweaks/ITweakTargeting.hpp"
#include "Runtime/rstl.hpp"

namespace metaforce::MP1 {
struct CTweakTargeting final : public Tweaks::ITweakTargeting {
  u32 x4_targetRadiusMode{};
  float x8_currLockOnExitDuration{};
  float xc_currLockOnEnterDuration{};
  float x10_currLockOnSwitchDuration{};
  float x14_lockConfirmScale{};
  float x18_nextLockOnEnterDuration{};
  float x1c_nextLockOnExitDuration{};
  float x20_nextLockOnSwitchDuration{};
  float x24_seekerScale{};
  float x28_seekerAngleSpeed{};
  float x2c_xrayRetAngleSpeed{};
  zeus::CVector3f x30_;
  zeus::CVector3f x3c_;
  float x48_{};
  float x4c_{};
  float x50_orbitPointZOffset{};
  float x54_orbitPointInTime{};
  float x58_orbitPointOutTime{};
  float x5c_{};
  zeus::CVector3f x60_;
  zeus::CVector3f x6c_;
  zeus::CVector3f x78_;
  zeus::CVector3f x84_;
  float x90_{};
  float x94_{};
  float x98_{};
  float x9c_{};
  float xa0_{};
  float xa4_{};
  float xa8_{};
  float xac_{};
  zeus::CColor xb0_thermalReticuleColor;
  float xb4_targetFlowerScale{};
  zeus::CColor xb8_targetFlowerColor;
  float xbc_missileBracketDuration{};
  float xc0_missileBracketScaleStart{};
  float xc4_missileBracketScaleEnd{};
  float xc8_missileBracketScaleDuration{};
  zeus::CColor xcc_missileBracketColor;
  float xd0_LockonDuration{};
  float xd4_innerBeamScale{};
  zeus::CColor xd8_innerBeamColorPower;
  zeus::CColor xdc_innerBeamColorIce;
  zeus::CColor xe0_innerBeamColorWave;
  zeus::CColor xe4_innerBeamColorPlasma;
  float xe8_chargeGaugeOvershootOffset{};
  float xec_chargeGaugeOvershootDuration{};
  float xf0_outerBeamSquaresScale{};
  zeus::CColor xf4_outerBeamSquareColor;
  rstl::reserved_vector<rstl::reserved_vector<float, 9>, 4> xf8_outerBeamSquareAngles;
  rstl::reserved_vector<float, 4> x108_chargeGaugeAngles{};
  float x118_chargeGaugeScale{};
  zeus::CColor x11c_chargeGaugeNonFullColor;
  u32 x120_chargeTickCount{};
  float x124_chargeTickAnglePitch{};
  float x128_lockFireScale{};
  float x12c_lockFireDuration{};
  zeus::CColor x130_lockFireColor;
  float x134_lockDaggerScaleStart{};
  float x138_lockDaggerScaleEnd{};
  zeus::CColor x13c_lockDaggerColor;
  float x140_lockDaggerAngle0{};
  float x144_lockDaggerAngle1{};
  float x148_lockDaggerAngle2{};
  zeus::CColor x14c_lockConfirmColor;
  zeus::CColor x150_seekerColor;
  float x154_lockConfirmClampMin{};
  float x158_lockConfirmClampMax{};
  float x15c_targetFlowerClampMin{};
  float x160_targetFlowerClampMax{};
  float x164_seekerClampMin{};
  float x168_seekerClampMax{};
  float x16c_missileBracketClampMin{};
  float x170_missileBracketClampMax{};
  float x174_innerBeamClampMin{};
  float x178_innerBeamClampMax{};
  float x17c_chargeGaugeClampMin{};
  float x180_chargeGaugeClampMax{};
  float x184_lockFireClampMin{};
  float x188_lockFireClampMax{};
  float x18c_lockDaggerClampMin{};
  float x190_lockDaggerClampMax{};
  float x194_grappleSelectScale{};
  float x198_grappleScale{};
  float x19c_grappleClampMin{};
  float x1a0_grappleClampMax{};
  zeus::CColor x1a4_grapplePointSelectColor;
  zeus::CColor x1a8_grapplePointColor;
  zeus::CColor x1ac_lockedGrapplePointSelectColor;
  float x1b0_grappleMinClampScale{};
  zeus::CColor x1b4_chargeGaugePulseColorHigh;
  float x1b8_fullChargeFadeDuration{};
  zeus::CColor x1bc_orbitPointColor;
  zeus::CColor x1c0_crosshairsColor;
  float x1c4_crosshairsScaleDur{};
  bool x1c8_drawOrbitPoint{};
  zeus::CColor x1cc_chargeGaugePulseColorLow;
  float x1d0_chargeGaugePulsePeriod{};
  zeus::CColor x1d4_;
  zeus::CColor x1d8_;
  zeus::CColor x1dc_;
  float x1e0_{};
  float x1e4_{};
  float x1e8_{};
  float x1ec_{};
  float x1f0_{};
  float x1f4_{};
  float x1f8_{};
  float x1fc_{};
  float x200_{};
  float x204_{};
  float x208_{};
  float x20c_reticuleClampMin{};
  float x210_reticuleClampMax{};
  zeus::CColor x214_xrayRetRingColor;
  float x218_reticuleScale{};
  float x21c_scanTargetClampMin{};
  float x220_scanTargetClampMax{};
  float x224_angularLagSpeed{};

  bool x224_ = true;
  bool x225_ = false;
  bool x226_ = true;
  bool x227_ = true;
  bool x22c_ = true;
  bool x22d_ = false;
  bool x22e_ = true;
  bool x22f_ = true;
  bool x234_ = true;
  bool x235_ = false;
  bool x236_ = true;
  bool x237_ = true;
  zeus::CVector3f x23c_ = zeus::skZero3f;

  float x2c8_ = 0.25f;
  float x2cc_ = 0.35f;
  zeus::CColor x2d0_ = (zeus::Comp32)0xb6e6ffff;
  float x2d4_ = 0.39215687f;
  zeus::CColor x2d8_ = (zeus::Comp32)0xa82a00ff;
  float x2dc_ = 0.78431374f;
  zeus::CVector3f x2e0_ = zeus::CVector3f(0.f, 0.f, 0.46f);
  float x2ec_ = 0.25f;
  float x2f0_ = 0.25f;
  float x2f4_ = 120.f;
  float x2f8_ = 0.25f;
  float x2fc_ = 3.5f;
  float x300_ = 0.35f;
  zeus::CColor x304_ = (zeus::Comp32)0xa82a00ff;
  float x308_ = 0.78431374f;
  zeus::CColor x30c_ = (zeus::Comp32)0x89d6ffff;
  float x310_ = 0.5019608f;
  float x314_ = 11.25f;
  float x318_ = 0.25f;
  float x31c_ = 0.125f;
  zeus::CColor x320_ = (zeus::Comp32)0xffca28ff;
  float x324_ = 0.78431374f;
  zeus::CColor x328_ = (zeus::Comp32)0x89d6ffff;
  float x32c_ = 0.19607843f;
  float x330_ = 0.f;
  float x334_ = 0.25f;
  float x338_ = 3.f;
  float x33c_ = 0.25f;
  float x340_ = 0.25f;
  float x344_ = 0.25f;
  float x348_ = 0.25f;
  float x34c_ = 45.f;
  float x350_ = 0.5f;
  float x354_ = 0.65f;
  float x358_ = 1.5f;
  float x35c_ = 0.18f;
  float x360_ = 0.15f;
  float x364_ = 0.25f;
  zeus::CColor x368_ = static_cast<zeus::Comp32>(0x56c1fb9f);
  zeus::CColor x36c_ = static_cast<zeus::Comp32>(0x49c3f6a0);
  zeus::CColor x370_ = static_cast<zeus::Comp32>(0x49c3f631);
  zeus::CColor x374_ = static_cast<zeus::Comp32>(0xff8930ff);
  zeus::CColor x378_ = static_cast<zeus::Comp32>(0xff2f28ff);
  zeus::CColor x37c_ = static_cast<zeus::Comp32>(0x93e9ffff);
  zeus::CColor x380_ = static_cast<zeus::Comp32>(0xff6b60ff);

  CTweakTargeting() = default;
  CTweakTargeting(CInputStream& r);
  u32 GetTargetRadiusMode() const override { return x4_targetRadiusMode; }
  float GetCurrLockOnExitDuration() const override { return x8_currLockOnExitDuration; }
  float GetCurrLockOnEnterDuration() const override { return xc_currLockOnEnterDuration; }
  float GetCurrLockOnSwitchDuration() const override { return x10_currLockOnSwitchDuration; }
  float GetLockConfirmScale() const override { return x14_lockConfirmScale; }
  float GetNextLockOnEnterDuration() const override { return x18_nextLockOnEnterDuration; }
  float GetNextLockOnExitDuration() const override { return x1c_nextLockOnExitDuration; }
  float GetNextLockOnSwitchDuration() const override { return x20_nextLockOnSwitchDuration; }
  float GetSeekerScale() const override { return x24_seekerScale; }
  float GetSeekerAngleSpeed() const override { return x28_seekerAngleSpeed; }
  float GetXRayRetAngleSpeed() const override { return x2c_xrayRetAngleSpeed; }
  float GetOrbitPointZOffset() const override { return x50_orbitPointZOffset; }
  float GetOrbitPointInTime() const override { return x54_orbitPointInTime; }
  float GetOrbitPointOutTime() const override { return x58_orbitPointOutTime; }
  const zeus::CColor& GetThermalReticuleColor() const override { return xb0_thermalReticuleColor; }
  float GetTargetFlowerScale() const override { return xb4_targetFlowerScale; }
  const zeus::CColor& GetTargetFlowerColor() const override { return xb8_targetFlowerColor; }
  float GetMissileBracketDuration() const override { return xbc_missileBracketDuration; }
  float GetMissileBracketScaleStart() const override { return xc0_missileBracketScaleStart; }
  float GetMissileBracketScaleEnd() const override { return xc4_missileBracketScaleEnd; }
  float GetMissileBracketScaleDuration() const override { return xc8_missileBracketScaleDuration; }
  const zeus::CColor& GetMissileBracketColor() const override { return xcc_missileBracketColor; }
  float GetChargeGaugeOvershootOffset() const override { return xe8_chargeGaugeOvershootOffset; }
  float GetChargeGaugeOvershootDuration() const override { return xec_chargeGaugeOvershootDuration; }
  float GetOuterBeamSquaresScale() const override { return xf0_outerBeamSquaresScale; }
  const zeus::CColor& GetOuterBeamSquareColor() const override { return xf4_outerBeamSquareColor; }
  float GetLockonDuration() const override { return xd0_LockonDuration; }
  float GetInnerBeamScale() const override { return xd4_innerBeamScale; }
  const zeus::CColor& GetInnerBeamColorPower() const override { return xd8_innerBeamColorPower; }
  const zeus::CColor& GetInnerBeamColorIce() const override { return xdc_innerBeamColorIce; }
  const zeus::CColor& GetInnerBeamColorWave() const override { return xe0_innerBeamColorWave; }
  const zeus::CColor& GetInnerBeamColorPlasma() const override { return xe4_innerBeamColorPlasma; }
  const float* GetOuterBeamSquareAngles(int i) const override { return xf8_outerBeamSquareAngles[i].data(); }
  float GetChargeGaugeAngle(int i) const override { return x108_chargeGaugeAngles[i]; }
  float GetChargeGaugeScale() const override { return x118_chargeGaugeScale; }
  const zeus::CColor& GetChargeGaugeNonFullColor() const override { return x11c_chargeGaugeNonFullColor; }
  u32 GetChargeTickCount() const override { return x120_chargeTickCount; }
  float GetChargeTickAnglePitch() const override { return x124_chargeTickAnglePitch; }
  float GetLockFireScale() const override { return x128_lockFireScale; }
  float GetLockFireDuration() const override { return x12c_lockFireDuration; }
  const zeus::CColor& GetLockFireColor() const override { return x130_lockFireColor; }
  float GetLockDaggerScaleStart() const override { return x134_lockDaggerScaleStart; }
  float GetLockDaggerScaleEnd() const override { return x138_lockDaggerScaleEnd; }
  const zeus::CColor& GetLockDaggerColor() const override { return x13c_lockDaggerColor; }
  float GetLockDaggerAngle0() const override { return x140_lockDaggerAngle0; }
  float GetLockDaggerAngle1() const override { return x144_lockDaggerAngle1; }
  float GetLockDaggerAngle2() const override { return x148_lockDaggerAngle2; }
  const zeus::CColor& GetLockConfirmColor() const override { return x14c_lockConfirmColor; }
  const zeus::CColor& GetSeekerColor() const override { return x150_seekerColor; }
  float GetLockConfirmClampMin() const override { return x154_lockConfirmClampMin; }
  float GetLockConfirmClampMax() const override { return x158_lockConfirmClampMax; }
  float GetTargetFlowerClampMin() const override { return x15c_targetFlowerClampMin; }
  float GetTargetFlowerClampMax() const override { return x160_targetFlowerClampMax; }
  float GetSeekerClampMin() const override { return x164_seekerClampMin; }
  float GetSeekerClampMax() const override { return x168_seekerClampMax; }
  float GetMissileBracketClampMin() const override { return x16c_missileBracketClampMin; }
  float GetMissileBracketClampMax() const override { return x170_missileBracketClampMax; }
  float GetInnerBeamClampMin() const override { return x174_innerBeamClampMin; }
  float GetInnerBeamClampMax() const override { return x178_innerBeamClampMax; }
  float GetChargeGaugeClampMin() const override { return x17c_chargeGaugeClampMin; }
  float GetChargeGaugeClampMax() const override { return x180_chargeGaugeClampMax; }
  float GetLockFireClampMin() const override { return x184_lockFireClampMin; }
  float GetLockFireClampMax() const override { return x188_lockFireClampMax; }
  float GetLockDaggerClampMin() const override { return x18c_lockDaggerClampMin; }
  float GetLockDaggerClampMax() const override { return x190_lockDaggerClampMax; }
  float GetGrappleSelectScale() const override { return x194_grappleSelectScale; }
  float GetGrappleScale() const override { return x198_grappleScale; }
  float GetGrappleClampMin() const override { return x19c_grappleClampMin; }
  float GetGrappleClampMax() const override { return x1a0_grappleClampMax; }
  const zeus::CColor& GetGrapplePointSelectColor() const override { return x1a4_grapplePointSelectColor; }
  const zeus::CColor& GetGrapplePointColor() const override { return x1a8_grapplePointColor; }
  const zeus::CColor& GetLockedGrapplePointSelectColor() const override { return x1ac_lockedGrapplePointSelectColor; }
  float GetGrappleMinClampScale() const override { return x1b0_grappleMinClampScale; }
  const zeus::CColor& GetChargeGaugePulseColorHigh() const override { return x1b4_chargeGaugePulseColorHigh; }
  float GetFullChargeFadeDuration() const override { return x1b8_fullChargeFadeDuration; }
  const zeus::CColor& GetOrbitPointColor() const override { return x1bc_orbitPointColor; }
  const zeus::CColor& GetCrosshairsColor() const override { return x1c0_crosshairsColor; }
  float GetCrosshairsScaleDuration() const override { return x1c4_crosshairsScaleDur; }
  bool DrawOrbitPoint() const override { return x1c8_drawOrbitPoint; }
  const zeus::CColor& GetChargeGaugePulseColorLow() const override { return x1cc_chargeGaugePulseColorLow; }
  float GetChargeGaugePulsePeriod() const override { return x1d0_chargeGaugePulsePeriod; }
  float GetReticuleClampMin() const override { return x20c_reticuleClampMin; }
  float GetReticuleClampMax() const override { return x210_reticuleClampMax; }
  const zeus::CColor& GetXRayRetRingColor() const override { return x214_xrayRetRingColor; }
  float GetReticuleScale() const override { return x218_reticuleScale; }
  float GetScanTargetClampMin() const override { return x21c_scanTargetClampMin; }
  float GetScanTargetClampMax() const override { return x220_scanTargetClampMax; }
  float GetAngularLagSpeed() const override { return x224_angularLagSpeed; }
};
} // namespace metaforce::MP1
