#include "CMFGame.hpp"
#include "CArchitectureQueue.hpp"
#include "GameGlobalObjects.hpp"
#include "CGameState.hpp"
#include "CStateManager.hpp"
#include "World/CPlayer.hpp"
#include "MP1.hpp"
#include "Character/CCharLayoutInfo.hpp"

namespace urde
{
namespace MP1
{

CMFGame::CMFGame(const std::weak_ptr<CStateManager>& stateMgr, const std::weak_ptr<CInGameGuiManager>& guiMgr,
                 const CArchitectureQueue&)
: CMFGameBase("CMFGame"), x14_stateManager(stateMgr.lock()), x18_guiManager(guiMgr.lock()), x2a_25_(true)
{
    //g_Main->x160_25_ = true;
}

CIOWin::EMessageReturn CMFGame::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue)
{
    return EMessageReturn::Normal;
}

void CMFGame::Draw() const
{

}

CMFGameLoader::CMFGameLoader() : CMFGameLoaderBase("CMFGameLoader")
{
    switch (g_Main->GetFlowState())
    {
    case CMain::EFlowState::Five:
    case CMain::EFlowState::Six:
    {
        ResId mlvlId = g_GameState->CurrentWorldAssetId();
        // g_GameState->WorldTransitionManager()->
        // g_MemoryCardSys->
    }
    default:
    {
    }
    }
}

CIOWin::EMessageReturn CMFGameLoader::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue)
{
    std::shared_ptr<CWorldTransManager> wtMgr = g_GameState->GetWorldTransitionManager();

    switch (msg.GetType())
    {
    case EArchMsgType::TimerTick:
    {
        const CArchMsgParmReal32& tick = MakeMsg::GetParmTimerTick(msg);
    }
    default:
        break;
    }
    return EMessageReturn::Exit;
}

void CMFGameLoader::Draw() const { g_GameState->GetWorldTransitionManager()->Draw(); }

void CMFGameLoader::Touch()
{
    x14_stateMgr->TouchSky();
    x14_stateMgr->TouchPlayerActor();
    x14_stateMgr->Player()->Touch();
}
}
}
