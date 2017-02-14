#include "CMFGame.hpp"
#include "CArchitectureQueue.hpp"
#include "GameGlobalObjects.hpp"
#include "CGameState.hpp"
#include "CStateManager.hpp"
#include "World/CPlayer.hpp"
#include "MP1.hpp"
#include "Character/CCharLayoutInfo.hpp"
#include "AutoMapper/CAutoMapper.hpp"

namespace urde
{
namespace MP1
{

CMFGame::CMFGame(const std::weak_ptr<CStateManager>& stateMgr, const std::weak_ptr<CInGameGuiManager>& guiMgr,
                 const CArchitectureQueue&)
: CMFGameBase("CMFGame"), x14_stateManager(stateMgr.lock()), x18_guiManager(guiMgr.lock())
{
    x2a_25_ = true;
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
    CMain* m = static_cast<CMain*>(g_Main);
    switch (m->GetFlowState())
    {
    case EFlowState::Five:
    case EFlowState::Six:
    {
        ResId mlvlId = g_GameState->CurrentWorldAssetId();
        if (g_MemoryCardSys->HasSaveWorldMemory(mlvlId))
        {
            const CSaveWorldMemory& savwMem = g_MemoryCardSys->GetSaveWorldMemory(mlvlId);
            if (savwMem.GetWorldNameId() != -1)
            {
                ResId wtMgrFont = g_ResFactory->TranslateOriginalToNew(0xB7BBD0B4);
                g_GameState->GetWorldTransitionManager()->EnableTransition(wtMgrFont,
                                                                           savwMem.GetWorldNameId(),
                                                                           1, false, 0.1f, 16.f, 1.f);
            }
        }
    }
    default: break;
    }

    if (g_GameState->CurrentWorldAssetId() == g_ResFactory->TranslateOriginalToNew(0x158EFE17) &&
        g_GameState->CurrentWorldState().GetCurrentAreaId() == 0)
    {
        const SObjectTag* strgTag = g_ResFactory->GetResourceIdByName("STRG_IntroLevelLoad");
        if (strgTag)
            g_GameState->GetWorldTransitionManager()->EnableTransition(-1, strgTag->id,
                                                                       0, false, 0.1f, 16.f, 1.f);
    }
}

static const char* LoadDepPAKs[] =
{
    "TestAnim",
    "SamusGun",
    "SamGunFx",
    nullptr
};

void CMFGameLoader::MakeLoadDependencyList()
{
    std::vector<SObjectTag> tags;
    for (int i=0 ; LoadDepPAKs[i] ; ++i)
        g_ResFactory->GetTagListForFile(LoadDepPAKs[i], tags);

    x1c_loadList.reserve(tags.size());
    for (const SObjectTag& tag : tags)
        x1c_loadList.push_back(g_SimplePool->GetObj(tag));
}

CIOWin::EMessageReturn CMFGameLoader::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue)
{
    std::shared_ptr<CWorldTransManager> wtMgr = g_GameState->GetWorldTransitionManager();

    switch (msg.GetType())
    {
    case EArchMsgType::TimerTick:
    {
        const CArchMsgParmReal32& tick = MakeMsg::GetParmTimerTick(msg);
        float dt = tick.x4_parm;
        if (!x2c_24_initialized)
        {
            if (x1c_loadList.empty())
            {
                MakeLoadDependencyList();
                wtMgr->StartTransition();
                return EMessageReturn::Exit;
            }
            u32 loadingCount = 0;
            for (CToken& tok : x1c_loadList)
            {
                tok.Lock();
                if (!tok.IsLoaded())
                    ++loadingCount;
            }
            wtMgr->Update(dt);
            if (loadingCount)
                return EMessageReturn::Exit;
            x2c_24_initialized = true;
        }
        else
        {
            wtMgr->Update(dt);
        }

        if (!x14_stateMgr)
        {
            CWorldTransManager::WaitForModelsAndTextures();
            CWorldState& wldState = g_GameState->CurrentWorldState();
            x14_stateMgr = std::make_shared<CStateManager>(wldState.RelayTracker(),
                                                           wldState.MapWorldInfo(),
                                                           g_GameState->GetPlayerState(),
                                                           wtMgr,
                                                           wldState.GetLayerState());
        }

        if (x14_stateMgr->xb3c_initPhase != CStateManager::InitPhase::Done)
        {
            CWorldState& wldState = g_GameState->CurrentWorldState();
            x14_stateMgr->InitializeState(wldState.GetWorldAssetId(),
                                          wldState.GetCurrentAreaId(),
                                          wldState.GetDesiredAreaAssetId());
            return EMessageReturn::Exit;
        }

        if (!x18_guiMgr)
            x18_guiMgr = std::make_shared<CInGameGuiManager>(*x14_stateMgr, queue);
        if (!x18_guiMgr->CheckLoadComplete(*x14_stateMgr))
            return EMessageReturn::Exit;

        x1c_loadList.clear();

        wtMgr->StartTextFadeOut();
        x2c_25_transitionFinished = wtMgr->IsTransitionFinished();
        return EMessageReturn::Exit;
    }
    case EArchMsgType::FrameBegin:
    {
        if (x2c_25_transitionFinished)
        {
            queue.Push(
            MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 10, 1000,
                                       std::make_shared<CMFGame>(x14_stateMgr, x18_guiMgr, queue)));
            return EMessageReturn::RemoveIOWinAndExit;
        }
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
