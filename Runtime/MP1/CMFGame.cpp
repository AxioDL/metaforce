#include "CMFGame.hpp"
#include "CArchitectureQueue.hpp"
#include "GameGlobalObjects.hpp"
#include "CGameState.hpp"
#include "CStateManager.hpp"
#include "World/CPlayer.hpp"
#include "MP1.hpp"
#include "Character/CCharLayoutInfo.hpp"
#include "AutoMapper/CAutoMapper.hpp"
#include "CSamusHud.hpp"
#include "CPauseScreen.hpp"
#include "TCastTo.hpp"
#include "Audio/CMidiManager.hpp"
#include "Camera/CCinematicCamera.hpp"

namespace urde
{
namespace MP1
{

CMFGame::CMFGame(const std::weak_ptr<CStateManager>& stateMgr, const std::weak_ptr<CInGameGuiManager>& guiMgr,
                 const CArchitectureQueue&)
: CMFGameBase("CMFGame"), x14_stateManager(stateMgr.lock()), x18_guiManager(guiMgr.lock())
{
    x2a_25_samusAlive = true;
    static_cast<CMain&>(*g_Main).SetMFGameBuilt(true);
}

CIOWin::EMessageReturn CMFGame::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue)
{
    switch (msg.GetType())
    {
    case EArchMsgType::FrameBegin:
        x14_stateManager->FrameBegin(msg.GetParm<CArchMsgParmInt32>()->x4_parm);
        break;
    case EArchMsgType::TimerTick:
    {
        bool wasInitialized = x2a_24_initialized;
        x2a_24_initialized = true;
        float dt = MakeMsg::GetParmTimerTick(msg).x4_parm;
        switch (x1c_flowState)
        {
        case EGameFlowState::CinematicSkip:
        {
            x20_cineSkipTime += dt;
            const CEntity* cam = x14_stateManager->GetCameraManager()->GetCurrentCamera(*x14_stateManager);
            TCastToConstPtr<CCinematicCamera> cineCam = cam;
            if ((x20_cineSkipTime >= 1.f && x14_stateManager->SpecialSkipCinematic()) || !cineCam ||
                (cineCam->GetW1() & 0x10 && x28_skippedCineCam != cineCam->GetUniqueId()))
            {
                static_cast<CMain&>(*g_Main).SetScreenFading(false);
                x1c_flowState = EGameFlowState::InGame;
                x18_guiManager->StartFadeIn();
                x28_skippedCineCam = kInvalidUniqueId;
                break;
            }
        }
        case EGameFlowState::InGame:
        {
            x14_stateManager->SetActiveRandomToDefault();
            switch (x14_stateManager->GetDeferredStateTransition())
            {
            case EStateManagerTransition::InGame:
                x14_stateManager->Update(dt);
                if (!x14_stateManager->ShouldQuitGame())
                    break;
                //CGraphics::SetIsBeginSceneClearFb();
                break;
            case EStateManagerTransition::MapScreen:
                EnterMapScreen();
                break;
            case EStateManagerTransition::PauseGame:
                PauseGame();
                break;
            case EStateManagerTransition::LogBook:
                EnterLogBook();
                break;
            case EStateManagerTransition::SaveGame:
                SaveGame();
                break;
            case EStateManagerTransition::MessageScreen:
                EnterMessageScreen(x14_stateManager->GetHUDMessageTime());
                break;
            }

            if (x2a_25_samusAlive && !x14_stateManager->GetPlayerState()->IsPlayerAlive())
            {
                PlayerDied();
            }
            x14_stateManager->ClearActiveRandom();
            break;
        }
        case EGameFlowState::Paused:
        {
            if (x18_guiManager->WasInGame() && x18_guiManager->IsInGame())
            {
                x14_stateManager->SetInSaveUI(x18_guiManager->IsInSaveUI());
                UnpauseGame();
                if (x14_stateManager->GetPauseHUDMessage() != -1)
                    x14_stateManager->IncrementHUDMessageFrameCounter();
            }
            break;
        }
        case EGameFlowState::SamusDied:
        {
            if (x14_stateManager->GetPlayer().IsPlayerDeadEnough())
            {
                static_cast<CMain&>(*g_Main).SetFlowState(EFlowState::Four);
                queue.Push(MakeMsg::CreateQuitGameplay(EArchMsgTarget::Game));
            }
            else
            {
                x14_stateManager->SetActiveRandomToDefault();
                x14_stateManager->Update(dt);
                x14_stateManager->ClearActiveRandom();
            }
        }
        default: break;
        }

        x18_guiManager->Update(*x14_stateManager, dt, queue, IsCameraActiveFlow());
        if (!wasInitialized)
            g_GameState->GetWorldTransitionManager()->EndTransition();

        return EMessageReturn::Exit;
    }
    case EArchMsgType::UserInput:
    {
        if (!x2a_24_initialized)
            break;
        const CFinalInput& input = MakeMsg::GetParmUserInput(msg).x4_parm;
        if (x1c_flowState == EGameFlowState::InGame)
        {
            if (input.ControllerIdx() == 0)
            {
                const CEntity* cam = x14_stateManager->GetCameraManager()->GetCurrentCamera(*x14_stateManager);
                TCastToConstPtr<CCinematicCamera> cineCam = cam;
                if (input.PStart())
                {
                    if (cineCam && x14_stateManager->GetSkipCinematicSpecialFunction() != kInvalidUniqueId)
                    {
                        CMidiManager::StopAll();
                        x28_skippedCineCam = cineCam->GetUniqueId();
                        x1c_flowState = EGameFlowState::CinematicSkip;
                        x20_cineSkipTime = 0.f;
                    }
                    else if (!cineCam)
                    {
                        x14_stateManager->DeferStateTransition(EStateManagerTransition::PauseGame);
                    }
                }
                else if (input.PZ() && !cineCam && x14_stateManager->CanShowMapScreen())
                {
                    x14_stateManager->DeferStateTransition(EStateManagerTransition::MapScreen);
                }
            }

            x14_stateManager->SetActiveRandomToDefault();
            x14_stateManager->ProcessInput(input);
            x14_stateManager->ClearActiveRandom();
        }
        x18_guiManager->ProcessControllerInput(*x14_stateManager, input, queue);
        break;
    }
    case EArchMsgType::FrameEnd:
    {
        x14_stateManager->FrameEnd();
        if (x14_stateManager->ShouldQuitGame())
            queue.Push(MakeMsg::CreateQuitGameplay(EArchMsgTarget::Game));
        break;
    }
    case EArchMsgType::QuitGameplay:
        return EMessageReturn::RemoveIOWin;
    default: break;
    }

    return EMessageReturn::Normal;
}

void CMFGame::Touch()
{
    x14_stateManager->TouchSky();
    x14_stateManager->TouchPlayerActor();

    bool gunVisible = false;
    bool ballVisible = false;
    bool samusVisible = false;
    CPlayer& player = x14_stateManager->GetPlayer();
    switch (player.GetMorphballTransitionState())
    {
    case CPlayer::EPlayerMorphBallState::Unmorphed:
        gunVisible = true;
        break;
    case CPlayer::EPlayerMorphBallState::Morphed:
        ballVisible = true;
        break;
    case CPlayer::EPlayerMorphBallState::Morphing:
        ballVisible = true;
        samusVisible = true;
        break;
    case CPlayer::EPlayerMorphBallState::Unmorphing:
        gunVisible = true;
        samusVisible = true;
        break;
    }

    if (gunVisible)
        player.GetPlayerGun()->TouchModel(*x14_stateManager);
    if (samusVisible)
        player.ModelData()->Touch(*x14_stateManager, 0);
    if (ballVisible)
        player.GetMorphBall()->TouchModel(*x14_stateManager);
}

void CMFGame::Draw() const
{
    if (!x2a_24_initialized)
        return;

    const_cast<CMFGame&>(*this).Touch();
    if (x18_guiManager->GetIsGameDraw())
    {
        static_cast<CMain&>(*g_Main).SetGameFrameDrawn();
        x14_stateManager->PreRender();
        x14_stateManager->DrawWorld();
        x14_stateManager->GetPlayer().IsPlayerDeadEnough();
    }

    x18_guiManager->PreDraw(*x14_stateManager);
    x18_guiManager->Draw(*x14_stateManager);

    if (x1c_flowState == EGameFlowState::CinematicSkip)
    {
        float c = std::min(1.f, 1.f - x20_cineSkipTime);
        const_cast<CColoredQuadFilter&>(m_fadeToBlack).draw(zeus::CColor{c, c, c, c});
    }
}

void CMFGame::PlayerDied()
{
    x1c_flowState = EGameFlowState::SamusDied;
    x2a_25_samusAlive = false;
}

void CMFGame::UnpauseGame()
{
    x1c_flowState = EGameFlowState::InGame;
    CSfxManager::SetChannel(CSfxManager::ESfxChannels::Game);
    x14_stateManager->DeferStateTransition(EStateManagerTransition::InGame);
}

void CMFGame::EnterMessageScreen(float time)
{
    x1c_flowState = EGameFlowState::Paused;
    x18_guiManager->ShowPauseGameHudMessage(*x14_stateManager, x14_stateManager->GetPauseHUDMessage(), time);
}

void CMFGame::SaveGame()
{
    x1c_flowState = EGameFlowState::Paused;
    x18_guiManager->PauseGame(*x14_stateManager, EInGameGuiState::PauseSaveGame);
}

void CMFGame::EnterLogBook()
{
    x1c_flowState = EGameFlowState::Paused;
    x18_guiManager->PauseGame(*x14_stateManager, EInGameGuiState::PauseLogBook);
}

void CMFGame::PauseGame()
{
    x1c_flowState = EGameFlowState::Paused;
    x18_guiManager->PauseGame(*x14_stateManager, EInGameGuiState::PauseGame);
}

void CMFGame::EnterMapScreen()
{
    x1c_flowState = EGameFlowState::Paused;
    x18_guiManager->PauseGame(*x14_stateManager, EInGameGuiState::MapScreen);
    x14_stateManager->SetInMapScreen(true);
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
    case EArchMsgType::FrameEnd:
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
