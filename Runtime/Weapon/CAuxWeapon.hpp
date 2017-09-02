#ifndef __URDE_CAUXWEAPON_HPP__
#define __URDE_CAUXWEAPON_HPP__

#include "CPlayerState.hpp"
#include "RetroTypes.hpp"
#include "CStateManager.hpp"
#include "CGunWeapon.hpp"

namespace urde
{

class CAuxWeapon
{
    bool x80_24_isLoaded : 1;
public:
    explicit CAuxWeapon(TUniqueId id);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    bool IsComboFxActive(const CStateManager& mgr) const;
    void Load(CPlayerState::EBeamId curBeam, CStateManager& mgr);
    void StopComboFx(CStateManager& mgr, bool b1);
    bool UpdateComboFx(float dt, const zeus::CVector3f& scale, const zeus::CVector3f& pos,
                       const zeus::CTransform& xf, CStateManager& mgr);
    void Fire(bool underwater, CPlayerState::EBeamId currentBeam, EChargeState chargeState,
              const zeus::CTransform& xf, CStateManager& mgr, EWeaponType type, TUniqueId homingId);
    void LoadIdle();
    bool IsLoaded() const { return x80_24_isLoaded; }
    void RenderMuzzleFx() const;
    TUniqueId HasTarget(const CStateManager& mgr) const;
    void SetNewTarget(TUniqueId targetId, CStateManager& mgr);
};

}

#endif // __URDE_CAUXWEAPON_HPP__
