#include "CAuxWeapon.hpp"

namespace urde
{

CAuxWeapon::CAuxWeapon(TUniqueId id)
{

}

void CAuxWeapon::AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&)
{

}

bool CAuxWeapon::IsComboFxActive(const CStateManager& mgr) const
{
    return false;
}

void CAuxWeapon::Load(int curBeam, CStateManager& mgr)
{

}

}
