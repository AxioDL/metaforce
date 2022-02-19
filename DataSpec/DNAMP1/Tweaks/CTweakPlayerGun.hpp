#pragma once

#include "../../DNACommon/Tweaks/ITweakPlayerGun.hpp"

namespace DataSpec::DNAMP1 {

struct CTweakPlayerGun final : ITweakPlayerGun {
  AT_DECL_DNA_YAML
  Value<float> x4_upLookAngle;
  Value<float> x8_downLookAngle;
  Value<float> xc_verticalSpread;
  Value<float> x10_horizontalSpread;
  Value<float> x14_highVerticalSpread;
  Value<float> x18_highHorizontalSpread;
  Value<float> x1c_lowVerticalSpread;
  Value<float> x20_lowHorizontalSpread;
  Value<float> x24_aimVerticalSpeed;
  Value<float> x28_aimHorizontalSpeed;
  Value<float> x2c_bombFuseTime;
  Value<float> x30_bombDropDelayTime;
  Value<float> x34_holoHoldTime;
  Value<float> x38_gunTransformTime;
  Value<float> x3c_gunHolsterTime;
  Value<float> x40_gunNotFiringTime;
  Value<float> x44_fixedVerticalAim;
  Value<float> x48_gunExtendDistance;
  Value<zeus::CVector3f> x4c_gunPosition;
  Value<zeus::CVector3f> x58_;
  Value<zeus::CVector3f> x64_grapplingArmPosition;
  SShotParam x70_bomb;
  SShotParam x8c_powerBomb;
  SShotParam x1d4_missile;
  SWeaponInfo xa8_beams[5];
  SComboShotParam x1f0_combos[5];    // Originally rstl::prereserved_vector<SShotParam,5>
  Value<float> x280_ricochetData[6]; // Originally rstl::prereserved_vector<float,5>, extended to 6 to capture
};
} // namespace DataSpec::DNAMP1
