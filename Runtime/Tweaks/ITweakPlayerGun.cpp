#include "Runtime/Tweaks/ITweakPlayerGun.hpp"
#include "Runtime/Streams/IOStreams.hpp"

namespace metaforce {
SShotParam::SShotParam(CInputStream& in)
: x0_weaponType(in.ReadLong())
, x8_damage(in.ReadFloat())
, xc_radiusDamage(in.ReadFloat())
, x10_radius(in.ReadFloat())
, x14_knockback(in.ReadFloat()) {}

SWeaponInfo::SWeaponInfo(CInputStream& in)
: x0_coolDown(in.ReadFloat()), x4_normal(in.Get<SShotParam>()), x20_charged(in.Get<SChargedShotParam>()) {}
} // namespace metaforce