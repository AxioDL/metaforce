#ifndef __URDE_CPOWERBEAM_HPP__
#define __URDE_CPOWERBEAM_HPP__

#include "CGunWeapon.hpp"

namespace urde
{

class CPowerBeam final : public CGunWeapon
{
    enum class ESmokeState
    {
        Inactive,
        Active,
        Done
    };
    TCachedToken<CGenDescription> x21c_shotSmoke;
    TCachedToken<CGenDescription> x228_power2nd1;
    std::unique_ptr<CElementGen> x234_shotSmokeGen;
    std::unique_ptr<CElementGen> x238_power2ndGen;
    float x23c_smokeTimer = 0.f;
    ESmokeState x240_smokeState = ESmokeState::Inactive;
    bool x244_24 : 1;
    bool x244_25_loaded : 1;
    void ReInitVariables();
public:
    CPowerBeam(CAssetId characterId, EWeaponType type, TUniqueId playerId,
               EMaterialTypes playerMaterial, const zeus::CVector3f& scale);

    void PreRenderGunFx(const CStateManager& mgr, const zeus::CTransform& xf);
    void PostRenderGunFx(const CStateManager& mgr, const zeus::CTransform& xf);
    void UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr, const zeus::CTransform& xf);
    void Fire(bool underwater, float dt, EChargeState chargeState, const zeus::CTransform& xf,
              CStateManager& mgr, TUniqueId homingTarget, float chargeFactor1, float chargeFactor2);
    void EnableSecondaryFx(ESecondaryFxType type);
    void Update(float dt, CStateManager& mgr);
    void Load(CStateManager& mgr, bool subtypeBasePose);
    void Unload(CStateManager& mgr);
    bool IsLoaded() const;
};

}

#endif // __URDE_CPOWERBEAM_HPP__
