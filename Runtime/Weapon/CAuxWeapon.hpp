#ifndef __URDE_CAUXWEAPON_HPP__
#define __URDE_CAUXWEAPON_HPP__

#include "RetroTypes.hpp"
#include "CStateManager.hpp"

namespace urde
{

class CAuxWeapon
{
public:
    explicit CAuxWeapon(TUniqueId id);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    bool IsComboFxActive(const CStateManager& mgr) const;
    void Load(int curBeam, CStateManager& mgr);
};

}

#endif // __URDE_CAUXWEAPON_HPP__
