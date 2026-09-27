#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

#include "MetroidPrime/CSteeringBehaviors.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "Weapons/CProjectileWeapon.hpp"

#include "Weapons/CWeaponDescription.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CProjectileInfo::CProjectileInfo(CAssetId proj, const CDamageInfo& dInfo)
: mWeaponDescription(gpSimplePool->GetObj(SObjectTag('WPSC', proj))), mDamageInfo(dInfo) {}

CProjectileInfo::CProjectileInfo(CInputStream& in)
: mWeaponDescription(gpSimplePool->GetObj(SObjectTag('WPSC', CAssetId(in.ReadLong()))))
, mDamageInfo(in) {}

float CProjectileInfo::GetProjectileSpeed() const {
  float result = 45000.0f;
  TToken< CWeaponDescription > token(mWeaponDescription);

  if (token->mIVEC) {
    CVector3f vec = CVector3f::Zero();
    token->mIVEC->GetValue(0, vec);
    result = vec.Magnitude() / CProjectileWeapon::GetTickPeriod();
  }
  return result;
}

CVector3f CProjectileInfo::PredictInterceptPos(const CVector3f& gunPos, const CVector3f& aimPos,
                                               const CPlayer& player, bool gravity, float dt) {
  return PredictInterceptPos(gunPos, aimPos, player, gravity, GetProjectileSpeed(), dt);
}

CVector3f CProjectileInfo::PredictInterceptPos(const CVector3f& gunPos, const CVector3f& aimPos,
                                               const CPlayer& player, bool gravity, float speed,
                                               float dt) {
  CVector3f ret = CVector3f::Zero();
  const CVector3f playerVel = player.GetDampedClampedVelocityWR();
  const CVector3f gravVec(0.f, 0.f, player.GetGravity());
  bool result = false;

  switch (player.GetOrbitState()) {
  case CPlayer::kOS_OrbitObject:
  case CPlayer::kOS_OrbitPoint:
  case CPlayer::kOS_OrbitCarcass:
  case CPlayer::kOS_ForcedOrbitObject:
  case CPlayer::kOS_Grapple: {
    const CVector3f& orbitPoint = player.GetOrbitPoint();
    if (gravity && player.GetPlayerMovementState() == NPlayer::kMS_ApplyJump) {
      result = CSteeringBehaviors::ProjectOrbitalIntersection(gunPos, speed, dt, aimPos, playerVel,
                                                              gravVec, orbitPoint, ret);
      break;
    }
    CVector3f vel = playerVel.CanBeNormalized()
                        ? (player.GetAverageSpeed() * playerVel.AsNormalized())
                        : playerVel;
    result = CSteeringBehaviors::ProjectOrbitalIntersection(gunPos, speed, dt, aimPos, vel,
                                                            orbitPoint, ret);
    break;
  }
  case CPlayer::kOS_NoOrbit:
    if (gravity && player.GetPlayerMovementState() == NPlayer::kMS_ApplyJump) {
      result = CSteeringBehaviors::ProjectLinearIntersection(gunPos, speed, aimPos, playerVel,
                                                             gravVec, ret);
    } else {
      result = CSteeringBehaviors::ProjectLinearIntersection(gunPos, speed, aimPos, playerVel, ret);
    }
    break;
  }

  if (!result) {
    ret = aimPos + playerVel * 1.5f;
  }

  return ret;
}
