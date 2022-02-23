#include "Runtime/MP1/Tweaks/CTweakTargeting.hpp"
#include "Runtime/Streams/IOStreams.hpp"

namespace metaforce::MP1 {
CTweakTargeting::CTweakTargeting(CInputStream& in, bool hasNewFields) {
  x4_targetRadiusMode = in.ReadLong();
  x8_currLockOnExitDuration = in.ReadFloat();
  xc_currLockOnEnterDuration = in.ReadFloat();
  x10_currLockOnSwitchDuration = in.ReadFloat();
  x14_lockConfirmScale = in.ReadFloat();
  x18_nextLockOnEnterDuration = in.ReadFloat();
  x1c_nextLockOnExitDuration = in.ReadFloat();
  x20_nextLockOnSwitchDuration = in.ReadFloat();
  x24_seekerScale = in.ReadFloat();
  x28_seekerAngleSpeed = in.ReadFloat();
  x2c_xrayRetAngleSpeed = in.ReadFloat();
  x30_ = in.Get<zeus::CVector3f>();
  x3c_ = in.Get<zeus::CVector3f>();
  x48_ = in.ReadFloat();
  x4c_ = in.ReadFloat();
  x50_orbitPointZOffset = in.ReadFloat();
  x54_orbitPointInTime = in.ReadFloat();
  x58_orbitPointOutTime = in.ReadFloat();
  x5c_ = in.ReadFloat();
  x60_ = in.Get<zeus::CVector3f>();
  x6c_ = in.Get<zeus::CVector3f>();
  x78_ = in.Get<zeus::CVector3f>();
  x84_ = in.Get<zeus::CVector3f>();
  x90_ = in.ReadFloat();
  x94_ = in.ReadFloat();
  x98_ = in.ReadFloat();
  x9c_ = in.ReadFloat();
  xa0_ = in.ReadFloat();
  xa4_ = in.ReadFloat();
  xa8_ = in.ReadFloat();
  xac_ = in.ReadFloat();
  xb0_thermalReticuleColor = in.Get<zeus::CColor>();
  xb4_targetFlowerScale = in.ReadFloat();
  xb8_targetFlowerColor = in.Get<zeus::CColor>();
  xbc_missileBracketDuration = in.ReadFloat();
  xc0_missileBracketScaleStart = in.ReadFloat();
  xc4_missileBracketScaleEnd = in.ReadFloat();
  xc8_missileBracketScaleDuration = in.ReadFloat();
  xcc_missileBracketColor = in.Get<zeus::CColor>();
  xd0_LockonDuration = in.ReadFloat();
  xd4_innerBeamScale = in.ReadFloat();
  xd8_innerBeamColorPower = in.Get<zeus::CColor>();
  xdc_innerBeamColorIce = in.Get<zeus::CColor>();
  xe0_innerBeamColorWave = in.Get<zeus::CColor>();
  xe4_innerBeamColorPlasma = in.Get<zeus::CColor>();
  xe8_chargeGaugeOvershootOffset = in.ReadFloat();
  xec_chargeGaugeOvershootDuration = in.ReadFloat();
  xf0_outerBeamSquaresScale = in.ReadFloat();
  xf4_outerBeamSquareColor = in.Get<zeus::CColor>();
  u32 outerBeamCount = in.ReadLong();
  xf8_outerBeamSquareAngles.resize(outerBeamCount);
  for (u32 i = 0; i < outerBeamCount; ++i) {
    read_reserved_vector(xf8_outerBeamSquareAngles[i], in);
  }
  read_reserved_vector(x108_chargeGaugeAngles, in);
  x118_chargeGaugeScale = in.ReadFloat();
  x11c_chargeGaugeNonFullColor = in.Get<zeus::CColor>();
  x120_chargeTickCount = in.ReadLong();
  x124_chargeTickAnglePitch = in.ReadFloat();
  x128_lockFireScale = in.ReadFloat();
  x12c_lockFireDuration = in.ReadFloat();
  x130_lockFireColor = in.Get<zeus::CColor>();
  x134_lockDaggerScaleStart = in.ReadFloat();
  x138_lockDaggerScaleEnd = in.ReadFloat();
  x13c_lockDaggerColor = in.Get<zeus::CColor>();
  x140_lockDaggerAngle0 = in.ReadFloat();
  x144_lockDaggerAngle1 = in.ReadFloat();
  x148_lockDaggerAngle2 = in.ReadFloat();
  x14c_lockConfirmColor = in.Get<zeus::CColor>();
  x150_seekerColor = in.Get<zeus::CColor>();
  x154_lockConfirmClampMin = in.ReadFloat();
  x158_lockConfirmClampMax = in.ReadFloat();
  x15c_targetFlowerClampMin = in.ReadFloat();
  x160_targetFlowerClampMax = in.ReadFloat();
  x164_seekerClampMin = in.ReadFloat();
  x168_seekerClampMax = in.ReadFloat();
  x16c_missileBracketClampMin = in.ReadFloat();
  x170_missileBracketClampMax = in.ReadFloat();
  x174_innerBeamClampMin = in.ReadFloat();
  x178_innerBeamClampMax = in.ReadFloat();
  x17c_chargeGaugeClampMin = in.ReadFloat();
  x180_chargeGaugeClampMax = in.ReadFloat();
  x184_lockFireClampMin = in.ReadFloat();
  x188_lockFireClampMax = in.ReadFloat();
  x18c_lockDaggerClampMin = in.ReadFloat();
  x190_lockDaggerClampMax = in.ReadFloat();
  x194_grappleSelectScale = in.ReadFloat();
  x198_grappleScale = in.ReadFloat();
  x19c_grappleClampMin = in.ReadFloat();
  x1a0_grappleClampMax = in.ReadFloat();
  x1a4_grapplePointSelectColor = in.Get<zeus::CColor>();
  x1a8_grapplePointColor = in.Get<zeus::CColor>();
  x1ac_lockedGrapplePointSelectColor = in.Get<zeus::CColor>();
  x1b0_grappleMinClampScale = in.ReadFloat();
  x1b4_chargeGaugePulseColorHigh = in.Get<zeus::CColor>();
  x1b8_fullChargeFadeDuration = in.ReadFloat();
  x1bc_orbitPointColor = in.Get<zeus::CColor>();
  x1c0_crosshairsColor = in.Get<zeus::CColor>();
  x1c4_crosshairsScaleDur = in.ReadFloat();
  x1c8_drawOrbitPoint = in.ReadBool();
  x1cc_chargeGaugePulseColorLow = in.Get<zeus::CColor>();
  x1d0_chargeGaugePulsePeriod = in.ReadFloat();
  x1d4_ = in.Get<zeus::CColor>();
  x1d8_ = in.Get<zeus::CColor>();
  x1dc_ = in.Get<zeus::CColor>();
  x1e0_ = in.ReadFloat();
  x1e4_ = in.ReadFloat();
  x1e8_ = in.ReadFloat();
  x1ec_ = in.ReadFloat();
  x1f0_ = in.ReadFloat();
  x1f4_ = in.ReadFloat();
  x1f8_ = in.ReadFloat();
  x1fc_ = in.ReadFloat();
  x200_ = in.ReadFloat();
  x204_ = in.ReadFloat();
  x208_ = in.ReadFloat();
  x20c_reticuleClampMin = in.ReadFloat();
  x210_reticuleClampMax = in.ReadFloat();
  x214_xrayRetRingColor = in.Get<zeus::CColor>();
  x218_reticuleScale = in.ReadFloat();
  x21c_scanTargetClampMin = in.ReadFloat();
  x220_scanTargetClampMax = in.ReadFloat();
  x224_angularLagSpeed = in.ReadFloat();

  if (hasNewFields) {
    x218_ = in.ReadFloat();
    x21c_ = in.ReadFloat();
  }

  x124_chargeTickAnglePitch = -zeus::degToRad(x124_chargeTickAnglePitch);
  x140_lockDaggerAngle0 = zeus::degToRad(x140_lockDaggerAngle0);
  x144_lockDaggerAngle1 = zeus::degToRad(x144_lockDaggerAngle1);
  x148_lockDaggerAngle2 = zeus::degToRad(x148_lockDaggerAngle2);
  x208_ = zeus::degToRad(x208_);
  for (int i = 0; i < 4; ++i) {
    for (float& f : xf8_outerBeamSquareAngles[i]) {
      f = zeus::degToRad(f);
    }
  }
  for (int i = 0; i < 4; ++i) {
    x108_chargeGaugeAngles[i] = zeus::degToRad(x108_chargeGaugeAngles[i]);
  }
}

} // namespace metaforce::MP1