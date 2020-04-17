#pragma once

#include "DataSpec/DNACommon/Tweaks/ITweakTargeting.hpp"

namespace DataSpec::DNAMP1 {
template<bool NewRep>
struct AT_SPECIALIZE_PARMS(true, false) CTweakTargeting final : public ITweakTargeting {
  AT_DECL_DNA_YAML
  Value<atUint32> x4_targetRadiusMode;
  Value<float> x8_currLockOnExitDuration;
  Value<float> xc_currLockOnEnterDuration;
  Value<float> x10_currLockOnSwitchDuration;
  Value<float> x14_lockConfirmScale;
  Value<float> x18_nextLockOnEnterDuration;
  Value<float> x1c_nextLockOnExitDuration;
  Value<float> x20_nextLockOnSwitchDuration;
  Value<float> x24_seekerScale;
  Value<float> x28_seekerAngleSpeed;
  Value<float> x2c_xrayRetAngleSpeed;
  Value<atVec3f> x30_;
  Value<atVec3f> x3c_;
  Value<float> x48_;
  Value<float> x4c_;
  Value<float> x50_orbitPointZOffset;
  Value<float> x54_orbitPointInTime;
  Value<float> x58_orbitPointOutTime;
  Value<float> x5c_;
  Value<atVec3f> x60_;
  Value<atVec3f> x6c_;
  Value<atVec3f> x78_;
  Value<atVec3f> x84_;
  Value<float> x90_;
  Value<float> x94_;
  Value<float> x98_;
  Value<float> x9c_;
  Value<float> xa0_;
  Value<float> xa4_;
  Value<float> xa8_;
  Value<float> xac_;
  DNAColor xb0_thermalReticuleColor;
  Value<float> xb4_targetFlowerScale;
  DNAColor xb8_targetFlowerColor;
  Value<float> xbc_missileBracketDuration;
  Value<float> xc0_missileBracketScaleStart;
  Value<float> xc4_missileBracketScaleEnd;
  Value<float> xc8_missileBracketScaleDuration;
  DNAColor xcc_missileBracketColor;
  Value<float> xd0_LockonDuration;
  Value<float> xd4_innerBeamScale;
  DNAColor xd8_innerBeamColorPower;
  DNAColor xdc_innerBeamColorIce;
  DNAColor xe0_innerBeamColorWave;
  DNAColor xe4_innerBeamColorPlasma;
  Value<float> xe8_chargeGaugeOvershootOffset;
  Value<float> xec_chargeGaugeOvershootDuration;
  Value<float> xf0_outerBeamSquaresScale;
  DNAColor xf4_outerBeamSquareColor;
  Value<atUint32> xf8_outerBeamSquareAngleCount;
  struct UnkVec : BigDNA {
    AT_DECL_DNA
    Value<atUint32> count;
    Vector<float, AT_DNA_COUNT(count)> floats;
  };
  Vector<UnkVec, AT_DNA_COUNT(xf8_outerBeamSquareAngleCount)> xf8_outerBeamSquareAngles;
  Value<atUint32> x108_chargeGaugeAngleCount;
  Vector<float, AT_DNA_COUNT(x108_chargeGaugeAngleCount)> x108_chargeGaugeAngles;
  Value<float> x118_chargeGaugeScale;
  DNAColor x11c_chargeGaugeNonFullColor;
  Value<atUint32> x120_chargeTickCount;
  Value<float> x124_chargeTickAnglePitch;
  Value<float> x128_lockFireScale;
  Value<float> x12c_lockFireDuration;
  DNAColor x130_lockFireColor;
  Value<float> x134_lockDaggerScaleStart;
  Value<float> x138_lockDaggerScaleEnd;
  DNAColor x13c_lockDaggerColor;
  Value<float> x140_lockDaggerAngle0;
  Value<float> x144_lockDaggerAngle1;
  Value<float> x148_lockDaggerAngle2;
  DNAColor x14c_lockConfirmColor;
  DNAColor x150_seekerColor;
  Value<float> x154_lockConfirmClampMin;
  Value<float> x158_lockConfirmClampMax;
  Value<float> x15c_targetFlowerClampMin;
  Value<float> x160_targetFlowerClampMax;
  Value<float> x164_seekerClampMin;
  Value<float> x168_seekerClampMax;
  Value<float> x16c_missileBracketClampMin;
  Value<float> x170_missileBracketClampMax;
  Value<float> x174_innerBeamClampMin;
  Value<float> x178_innerBeamClampMax;
  Value<float> x17c_chargeGaugeClampMin;
  Value<float> x180_chargeGaugeClampMax;
  Value<float> x184_lockFireClampMin;
  Value<float> x188_lockFireClampMax;
  Value<float> x18c_lockDaggerClampMin;
  Value<float> x190_lockDaggerClampMax;
  Value<float> x194_grappleSelectScale;
  Value<float> x198_grappleScale;
  Value<float> x19c_grappleClampMin;
  Value<float> x1a0_grappleClampMax;
  DNAColor x1a4_grapplePointSelectColor;
  DNAColor x1a8_grapplePointColor;
  DNAColor x1ac_lockedGrapplePointSelectColor;
  Value<float> x1b0_grappleMinClampScale;
  DNAColor x1b4_chargeGaugePulseColorHigh;
  Value<float> x1b8_fullChargeFadeDuration;
  DNAColor x1bc_orbitPointColor;
  DNAColor x1c0_crosshairsColor;
  Value<float> x1c4_crosshairsScaleDur;
  Value<bool> x1c8_drawOrbitPoint;
  DNAColor x1cc_chargeGaugePulseColorLow;
  Value<float> x1d0_chargeGaugePulsePeriod;
  DNAColor x1d4_;
  DNAColor x1d8_;
  DNAColor x1dc_;
  Value<float> x1e0_;
  Value<float> x1e4_;
  Value<float> x1e8_;
  Value<float> x1ec_;
  Value<float> x1f0_;
  Value<float> x1f4_;
  Value<float> x1f8_;
  Value<float> x1fc_;
  Value<float> x200_;
  Value<float> x204_;
  Value<float> x208_;
  Value<float> x20c_reticuleClampMin;
  Value<float> x210_reticuleClampMax;
  DNAColor x214_xrayRetRingColor;
  Value<float> x218_reticuleScale;
  Value<float> x21c_scanTargetClampMin;
  Value<float> x220_scanTargetClampMax;
  Value<float> x224_angularLagSpeed;

  // RS5
  Vector<float, AT_DNA_COUNT(NewRep == true ? 1 : 0)> x218_;
  Vector<float, AT_DNA_COUNT(NewRep == true ? 1 : 0)> x21c_;
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
  zeus::CColor x368_ = (zeus::Comp32)0x56c1fb9f;
  zeus::CColor x36c_ = (zeus::Comp32)0x49c3f6a0;
  zeus::CColor x370_ = (zeus::Comp32)0x49c3f631;
  zeus::CColor x374_ = (zeus::Comp32)0xff8930ff;
  zeus::CColor x378_ = (zeus::Comp32)0xff2f28ff;
  zeus::CColor x37c_ = (zeus::Comp32)0x93e9ffff;
  zeus::CColor x380_ = (zeus::Comp32)0xff6b60ff;

  CTweakTargeting() = default;
  CTweakTargeting(athena::io::IStreamReader& r) {
    this->read(r);
    x124_chargeTickAnglePitch = -zeus::degToRad(x124_chargeTickAnglePitch);
    x140_lockDaggerAngle0 = zeus::degToRad(x140_lockDaggerAngle0);
    x144_lockDaggerAngle1 = zeus::degToRad(x144_lockDaggerAngle1);
    x148_lockDaggerAngle2 = zeus::degToRad(x148_lockDaggerAngle2);
    x208_ = zeus::degToRad(x208_);
    for (int i = 0; i < 4; ++i)
      for (float& f : xf8_outerBeamSquareAngles[i].floats)
        f = zeus::degToRad(f);
    for (int i = 0; i < 4; ++i)
      x108_chargeGaugeAngles[i] = zeus::degToRad(x108_chargeGaugeAngles[i]);
  }

  atUint32 GetTargetRadiusMode() const override { return x4_targetRadiusMode; }
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
  const float* GetOuterBeamSquareAngles(int i) const override { return xf8_outerBeamSquareAngles[i].floats.data(); }
  float GetChargeGaugeAngle(int i) const override { return x108_chargeGaugeAngles[i]; }
  float GetChargeGaugeScale() const override { return x118_chargeGaugeScale; }
  const zeus::CColor& GetChargeGaugeNonFullColor() const override { return x11c_chargeGaugeNonFullColor; }
  atUint32 GetChargeTickCount() const override { return x120_chargeTickCount; }
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
} // namespace DataSpec::DNAMP1
