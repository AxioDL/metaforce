#pragma once

#include "Runtime/Tweaks/ITweakPlayerGun.hpp"

namespace metaforce::MP1 {
struct CTweakPlayerGun final : Tweaks::ITweakPlayerGun {
  float x4_upLookAngle;
  float x8_downLookAngle;
  float xc_verticalSpread;
  float x10_horizontalSpread;
  float x14_highVerticalSpread;
  float x18_highHorizontalSpread;
  float x1c_lowVerticalSpread;
  float x20_lowHorizontalSpread;
  float x24_aimVerticalSpeed;
  float x28_aimHorizontalSpeed;
  float x2c_bombFuseTime;
  float x30_bombDropDelayTime;
  float x34_holoHoldTime;
  float x38_gunTransformTime;
  float x3c_gunHolsterTime;
  float x40_gunNotFiringTime;
  float x44_fixedVerticalAim;
  float x48_gunExtendDistance;
  zeus::CVector3f x4c_gunPosition;
  zeus::CVector3f x58_;
  zeus::CVector3f x64_grapplingArmPosition;
  SShotParam x70_bomb;
  SShotParam x8c_powerBomb;
  SShotParam x1d4_missile;
  SWeaponInfo xa8_beams[5];
  SComboShotParam x1f0_combos[5];    // Originally rstl::reserved_vector<SShotParam,5>
  float x280_ricochetData[6]; // Originally rstl::reserved_vector<float,5>, extended to 6 to capture
                                     // PhazonBeam's value
  CTweakPlayerGun() = default;
  CTweakPlayerGun(CInputStream& in);
  float GetUpLookAngle() const override { return x4_upLookAngle; }
  float GetDownLookAngle() const override { return x8_downLookAngle; }
  float GetVerticalSpread() const override { return xc_verticalSpread; }
  float GetHorizontalSpread() const override { return x10_horizontalSpread; }
  float GetHighVerticalSpread() const override { return x14_highVerticalSpread; }
  float GetHighHorizontalSpread() const override { return x18_highHorizontalSpread; }
  float GetLowVerticalSpread() const override { return x1c_lowVerticalSpread; }
  float GetLowHorizontalSpread() const override { return x20_lowHorizontalSpread; }
  float GetAimVerticalSpeed() const override { return x24_aimVerticalSpeed; }
  float GetAimHorizontalSpeed() const override { return x28_aimHorizontalSpeed; }
  float GetBombFuseTime() const override { return x2c_bombFuseTime; }
  float GetBombDropDelayTime() const override { return x30_bombDropDelayTime; }
  float GetHoloHoldTime() const override { return x34_holoHoldTime; }
  float GetGunTransformTime() const override { return x38_gunTransformTime; }
  float GetGunHolsterTime() const override { return x3c_gunHolsterTime; }
  float GetGunNotFiringTime() const override { return x40_gunNotFiringTime; }
  float GetFixedVerticalAim() const override { return x44_fixedVerticalAim; }
  float GetGunExtendDistance() const override { return x48_gunExtendDistance; }
  const zeus::CVector3f& GetGunPosition() const override { return x4c_gunPosition; }
  const zeus::CVector3f& GetGrapplingArmPosition() const override { return x64_grapplingArmPosition; }
  float GetRichochetDamage(u32 type) const override {
    switch (type) {
    case 0: // Power
      return x280_ricochetData[0];
    case 1: // Ice
      return x280_ricochetData[1];
    case 2: // Wave
      return x280_ricochetData[2];
    case 3: // Plasma
      return x280_ricochetData[3];
    case 6: // Missile
      return x280_ricochetData[4];
    case 8: // Phazon
            /* Note: In order to return the same value as retail we have to do a bit of a hack
             * Retro accidentally forgot to load in PhazonBeam's richochet value, as a result, it loads the
             * pointer to CTweakParticle's vtable.
             */
#if MP_v1088

      return float(0x803D9CC4);
#else
      return x280_ricochetData[5];
#endif
    default:
      return 1.f;
    }
  }

  const SWeaponInfo& GetBeamInfo(s32 beam) const override {
    if (beam < 0 || beam >= 5) {
      return xa8_beams[0];
    }
    return xa8_beams[beam];
  }

  const SComboShotParam& GetComboShotInfo(s32 beam) const override {
    if (beam < 0 || beam >= 5) {
      return x1f0_combos[0];
    }
    return x1f0_combos[beam];
  }

  const SShotParam& GetBombInfo() const override { return x70_bomb; }
  const SShotParam& GetPowerBombInfo() const override { return x8c_powerBomb; }
  const SShotParam& GetMissileInfo() const { return x1d4_missile; }
};
} // namespace DataSpec::DNAMP1
