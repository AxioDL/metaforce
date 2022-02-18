#include "Runtime/World/CPatternedInfo.hpp"

#include "Runtime/Audio/CSfxManager.hpp"

namespace metaforce {

CPatternedInfo::CPatternedInfo(CInputStream& in, u32 pcount)
: x0_mass(in.ReadFloat())
, x4_speed(in.ReadFloat())
, x8_turnSpeed(in.ReadFloat())
, xc_detectionRange(in.ReadFloat())
, x10_detectionHeightRange(in.ReadFloat())
, x14_dectectionAngle(in.ReadFloat())
, x18_minAttackRange(in.ReadFloat())
, x1c_maxAttackRange(in.ReadFloat())
, x20_averageAttackTime(in.ReadFloat())
, x24_attackTimeVariation(in.ReadFloat())
, x28_leashRadius(in.ReadFloat())
, x2c_playerLeashRadius(in.ReadFloat())
, x30_playerLeashTime(in.ReadFloat())
, x34_contactDamageInfo(in)
, x50_damageWaitTime(in.ReadFloat())
, x54_healthInfo(in)
, x5c_damageVulnerability(in)
, xc4_halfExtent(in.ReadFloat())
, xc8_height(in.ReadFloat())
, xcc_bodyOrigin(in.Get<zeus::CVector3f>())
, xd8_stepUpHeight(in.ReadFloat())
, xdc_xDamage(in.ReadFloat())
, xe0_frozenXDamage(in.ReadFloat())
, xe4_xDamageDelay(in.ReadFloat())
, xe8_deathSfx(CSfxManager::TranslateSFXID(in.ReadLong()))
, xec_animParams(in)
, xf8_active(in.ReadBool())
, xfc_stateMachineId(in.ReadLong())
, x100_intoFreezeDur(in.ReadFloat())
, x104_outofFreezeDur(in.ReadFloat())
, x108_freezeDur(in.ReadFloat())
, x10c_pathfindingIndex(in.ReadLong())
, x110_particle1Scale(in.Get<zeus::CVector3f>())
, x11c_particle1(in)
, x120_electric(in) {
  if (pcount >= 36)
    x124_particle2Scale = in.Get<zeus::CVector3f>();
  if (pcount >= 37)
    x130_particle2 = in.Get<CAssetId>();
  if (pcount >= 38)
    x134_iceShatterSfx = CSfxManager::TranslateSFXID(in.ReadLong());
}

std::pair<bool, u32> CPatternedInfo::HasCorrectParameterCount(CInputStream& in) {
  u32 pcount = in.ReadLong();
  return {(pcount >= 35 && pcount <= 38), pcount};
}
} // namespace metaforce
