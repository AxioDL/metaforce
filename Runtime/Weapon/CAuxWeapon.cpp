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

void CAuxWeapon::Load(CPlayerState::EBeamId curBeam, CStateManager& mgr)
{

}

void CAuxWeapon::StopComboFx(CStateManager& mgr, bool b1)
{

}

bool CAuxWeapon::UpdateComboFx(float dt, const zeus::CVector3f& scale, const zeus::CVector3f& pos,
                               const zeus::CTransform& xf, CStateManager& mgr)
{
    return false;
}

void CAuxWeapon::Fire(bool underwater, CPlayerState::EBeamId currentBeam, EChargeState chargeState,
                      const zeus::CTransform& xf, CStateManager& mgr, EWeaponType type, TUniqueId homingId)
{

}

void CAuxWeapon::LoadIdle()
{

}

void CAuxWeapon::RenderMuzzleFx() const
{

}

TUniqueId CAuxWeapon::HasTarget(const CStateManager& mgr) const
{
    return {};
}

void CAuxWeapon::SetNewTarget(TUniqueId targetId, CStateManager& mgr)
{

}

}
