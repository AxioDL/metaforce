#include "CMFGame.hpp"
#include "CArchitectureQueue.hpp"
#include "GameGlobalObjects.hpp"
#include "CGameState.hpp"
#include "MP1.hpp"

namespace urde
{
namespace MP1
{

CMFGameLoader::CMFGameLoader()
: CMFGameLoaderBase("CMFGameLoader")
{
    switch (g_Main->GetFlowState())
    {
    case CMain::FlowState::Five:
    case CMain::FlowState::Six:
    {
        ResId mlvlId = g_GameState->CurrentWorldAssetId();
        //g_GameState->WorldTransitionManager()->
        //g_MemoryCardSys->
    }
    default:
    {
    }
    }
}

CIOWin::EMessageReturn CMFGameLoader::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue)
{
    std::shared_ptr<CWorldTransManager> wtMgr = g_GameState->WorldTransitionManager();

    switch (msg.GetType())
    {
    case EArchMsgType::TimerTick:
    {
        const CArchMsgParmReal32& tick = MakeMsg::GetParmTimerTick(msg);
    }
    default: break;
    }
    return EMessageReturn::Exit;
}

void CMFGameLoader::Draw() const
{
    g_GameState->WorldTransitionManager()->Draw();
}

}
}
