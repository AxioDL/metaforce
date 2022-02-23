#include "Runtime/MP1/Tweaks/CTweakPlayerGun.hpp"
#include "Runtime/Streams/IOStreams.hpp"

namespace metaforce::MP1 {
CTweakPlayerGun::CTweakPlayerGun(CInputStream& in) {
  x4_upLookAngle = in.ReadFloat();
  x8_downLookAngle = in.ReadFloat();
  xc_verticalSpread = in.ReadFloat();
  x10_horizontalSpread = in.ReadFloat();
  x14_highVerticalSpread = in.ReadFloat();
  x18_highHorizontalSpread = in.ReadFloat();
  x1c_lowVerticalSpread = in.ReadFloat();
  x20_lowHorizontalSpread = in.ReadFloat();
  x24_aimVerticalSpeed = in.ReadFloat();
  x28_aimHorizontalSpeed = in.ReadFloat();
  x2c_bombFuseTime = in.ReadFloat();
  x30_bombDropDelayTime = in.ReadFloat();
  x34_holoHoldTime = in.ReadFloat();
  x38_gunTransformTime = in.ReadFloat();
  x3c_gunHolsterTime = in.ReadFloat();
  x40_gunNotFiringTime = in.ReadFloat();
  x44_fixedVerticalAim = in.ReadFloat();
  x48_gunExtendDistance = in.ReadFloat();
  x4c_gunPosition = in.Get<zeus::CVector3f>();
  x58_ = in.Get<zeus::CVector3f>();
  x64_grapplingArmPosition = in.Get<zeus::CVector3f>();
  x70_bomb = in.Get<SShotParam>();
  x8c_powerBomb = in.Get<SShotParam>();
  x1d4_missile = in.Get<SShotParam>();
  for (auto& beam : xa8_beams) {
    beam = in.Get<SWeaponInfo>();
  }
  for (auto& combo : x1f0_combos) {
    combo = in.Get<SComboShotParam>();
  }

  for (float& r : x280_ricochetData) {
    r = in.ReadFloat();
  }
  x44_fixedVerticalAim = zeus::degToRad(x44_fixedVerticalAim);
}
} // namespace metaforce::MP1