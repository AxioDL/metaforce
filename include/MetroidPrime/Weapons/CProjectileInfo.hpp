#ifndef _CPROJECTILEINFO
#define _CPROJECTILEINFO

#include "MetroidPrime/CDamageInfo.hpp"

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"

class CPlayer;
class CWeaponDescription;

class CProjectileInfo {
  TCachedToken< CWeaponDescription > mWeaponDescription;
  CDamageInfo mDamageInfo;

public:
  CProjectileInfo(CAssetId, const CDamageInfo&);
  explicit CProjectileInfo(CInputStream&);

  float GetProjectileSpeed() const;
  CVector3f PredictInterceptPos(const CVector3f& gunPos, const CVector3f& aimPos,
                                const CPlayer& player, bool gravity, float dt);
  static CVector3f PredictInterceptPos(const CVector3f& gunPos, const CVector3f& aimPos,
                                       const CPlayer& player, bool gravity, float speed, float dt);

  TCachedToken< CWeaponDescription >& Token() { return mWeaponDescription; }

  const CDamageInfo& GetDamage() const { return mDamageInfo; }
  void SetDamage(const CDamageInfo& damage) { mDamageInfo = damage; }
};

#endif // _CPROJECTILEINFO
