#include "CProjectileInfo.hpp"
#include "World/CDamageInfo.hpp"
#include "GameGlobalObjects.hpp"
#include "CProjectileWeapon.hpp"
#include "World/CPlayer.hpp"
#include "Character/CSteeringBehaviors.hpp"
#include "CSimplePool.hpp"

namespace urde {

CProjectileInfo::CProjectileInfo(urde::CInputStream& in)
: x0_weaponDescription(g_SimplePool->GetObj({SBIG('WPSC'), CAssetId(in)})), xc_damageInfo(in) {}

CProjectileInfo::CProjectileInfo(CAssetId proj, const CDamageInfo& dInfo)
: x0_weaponDescription(g_SimplePool->GetObj({SBIG('WPSC'), proj})), xc_damageInfo(dInfo) {}

zeus::CVector3f CProjectileInfo::PredictInterceptPos(const zeus::CVector3f& gunPos, const zeus::CVector3f& aimPos,
                                                     const CPlayer& player, bool gravity, float speed, float dt) {
  zeus::CVector3f ret;
  zeus::CVector3f playerVel = player.GetDampedClampedVelocityWR();
  zeus::CVector3f gravVec(0.f, 0.f, player.GetGravity());
  bool result;
  switch (player.GetOrbitState()) {
  case CPlayer::EPlayerOrbitState::OrbitObject:
  case CPlayer::EPlayerOrbitState::OrbitPoint:
  case CPlayer::EPlayerOrbitState::OrbitCarcass:
  case CPlayer::EPlayerOrbitState::ForcedOrbitObject:
  case CPlayer::EPlayerOrbitState::Grapple: {
    if (gravity && player.GetPlayerMovementState() == CPlayer::EPlayerMovementState::ApplyJump) {
      result = CSteeringBehaviors::ProjectOrbitalIntersection(gunPos, speed, dt, aimPos, playerVel, gravVec,
                                                              player.GetOrbitPoint(), ret);
      break;
    }
    zeus::CVector3f vel;
    if (playerVel.canBeNormalized())
      vel = playerVel.normalized() * player.GetAverageSpeed();
    else
      vel = playerVel;
    result = CSteeringBehaviors::ProjectOrbitalIntersection(gunPos, speed, dt, aimPos, vel,
                                                            player.GetOrbitPoint(), ret);
    break;
  }
  case CPlayer::EPlayerOrbitState::NoOrbit:
    if (gravity && player.GetPlayerMovementState() == CPlayer::EPlayerMovementState::ApplyJump)
      result = CSteeringBehaviors::ProjectLinearIntersection(gunPos, speed, aimPos, playerVel, gravVec, ret);
    else
      result = CSteeringBehaviors::ProjectLinearIntersection(gunPos, speed, aimPos, playerVel, ret);
  }
  if (!result)
    ret = playerVel * 1.5f + aimPos;
  return ret;
}

float CProjectileInfo::GetProjectileSpeed() const {
  auto wpsc = x0_weaponDescription;
  if (wpsc->x4_IVEC) {
    zeus::CVector3f vec;
    wpsc->x4_IVEC->GetValue(0, vec);
    return vec.magnitude() / CProjectileWeapon::GetTickPeriod();
  }
  return 45000.0f;
}

zeus::CVector3f CProjectileInfo::PredictInterceptPos(const zeus::CVector3f& gunPos, const zeus::CVector3f& aimPos,
                                                     const CPlayer& player, bool gravity, float dt) {
  return PredictInterceptPos(gunPos, aimPos, player, gravity, GetProjectileSpeed(), dt);
}

} // namespace urde
