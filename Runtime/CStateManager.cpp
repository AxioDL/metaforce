#include "CStateManager.hpp"
#include "Camera/CCameraShakeData.hpp"
#include "CSortedLists.hpp"
#include "CWeaponMgr.hpp"
#include "CFluidPlaneManager.hpp"
#include "World/CEnvFxManager.hpp"
#include "World/CActorModelParticles.hpp"
#include "World/CTeamAiTypes.hpp"
#include "Input/CRumbleManager.hpp"

namespace urde
{

CStateManager::CStateManager(const std::weak_ptr<CScriptMailbox>&,
                             const std::weak_ptr<CMapWorldInfo>&,
                             const std::weak_ptr<CPlayerState>&,
                             const std::weak_ptr<CWorldTransManager>&)
{
}

void CStateManager::SendScriptMsg(TUniqueId uid, TEditorId eid, EScriptObjectMessage msg, EScriptObjectState state)
{
}

}
