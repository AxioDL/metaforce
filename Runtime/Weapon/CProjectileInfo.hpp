#ifndef __URDE_CPROJECTILEINFO_HPP__
#define __URDE_CPROJECTILEINFO_HPP__

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"
#include "World/CDamageInfo.hpp"
#include "Particle/CWeaponDescription.hpp"

namespace urde
{
class CPlayer;
class CProjectileInfo
{
    TToken<CWeaponDescription> x0_weaponDescription;
    CDamageInfo xc_damageInfo;
public:
    CProjectileInfo(CAssetId, const CDamageInfo&);

    zeus::CVector3f PredictInterceptPos(const zeus::CVector3f&, const zeus::CVector3f&, const CPlayer&, bool);

    CDamageInfo GetDamage() const { return xc_damageInfo; }
    const TToken<CWeaponDescription>& Token() { return x0_weaponDescription; }
};
}
#endif // __URDE_CPROJECTILEINFO_HPP__
