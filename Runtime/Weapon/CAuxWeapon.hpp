#ifndef __URDE_CAUXWEAPON_HPP__
#define __URDE_CAUXWEAPON_HPP__

#include "CPlayerState.hpp"
#include "RetroTypes.hpp"
#include "CStateManager.hpp"

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
    void LoadIdle();
    bool IsLoaded() const { return x80_24_isLoaded; }
    void RenderMuzzleFx() const;
};

}

#endif // __URDE_CAUXWEAPON_HPP__
