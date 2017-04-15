#include "CInGameGuiManager.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "World/CPlayer.hpp"
#include "CDependencyGroup.hpp"
#include "CSimplePool.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiCamera.hpp"
#include "GuiSys/CGuiModel.hpp"
#include "AutoMapper/CAutoMapper.hpp"
#include "Particle/CGenDescription.hpp"
#include "Audio/CSfxManager.hpp"
#include "CPauseScreen.hpp"
#include "CSamusHud.hpp"
#include "CInventoryScreen.hpp"
#include "CPauseScreen.hpp"
#include "Input/CInputGenerator.hpp"

namespace urde
{
namespace MP1
{

static const char* InGameGuiDGRPs[] =
{
    "InGameGui_DGRP",
    "Ice_DGRP",
    "Phazon_DGRP",
    "Plasma_DGRP",
    "Power_DGRP",
    "Wave_DGRP",
    "BallTransition_DGRP",
    "GravitySuit_DGRP",
    "Ice_Anim_DGRP",
    "Plasma_Anim_DGRP",
    "PowerSuit_DGRP",
    "Power_Anim_DGRP",
    "VariaSuit_DGRP",
    "Wave_Anim_DGRP"
};

static const char* PauseScreenDGRPs[] =
{
    "InventorySuitPower_DGRP",
    "InventorySuitVaria_DGRP",
    "InventorySuitGravity_DGRP",
    "InventorySuitPhazon_DGRP",
    "InventorySuitFusionPower_DGRP",
    "InventorySuitFusionVaria_DGRP",
    "InventorySuitFusionGravity_DGRP",
    "InventorySuitFusionPhazon_DGRP",
    "SamusBallANCS_DGRP",
    "SamusSpiderBallANCS_DGRP",
    "PauseScreenDontDump_DGRP",
    "PauseScreenDontDump_NoARAM_DGRP",
    "PauseScreenTokens_DGRP"
};

std::vector<TLockedToken<CDependencyGroup>> CInGameGuiManager::LockPauseScreenDependencies()
{
    std::vector<TLockedToken<CDependencyGroup>> ret;
    ret.reserve(13);
    for (int i=0 ; i<13 ; ++i)
        ret.push_back(g_SimplePool->GetObj(PauseScreenDGRPs[i]));
    return ret;
}

bool CInGameGuiManager::CheckDGRPLoadComplete() const
{
    for (const auto& dgrp : x5c_pauseScreenDGRPs)
        if (!dgrp.IsLoaded())
            return false;
    for (const auto& dgrp : xc8_inGameGuiDGRPs)
        if (!dgrp.IsLoaded())
            return false;
    return true;
}

void CInGameGuiManager::BeginStateTransition(EInGameGuiState state, CStateManager& stateMgr)
{
    if (x1c0_nextState == state)
        return;

    x1bc_prevState = x1c0_nextState;
    x1c0_nextState = state;

    switch (state)
    {
    case EInGameGuiState::InGame:
    {
        CSfxManager::SetChannel(CSfxManager::ESfxChannels::Game);
        x4c_saveUI.reset();
        x38_autoMapper->UnmuteAllLoopedSounds();
        break;
    }
    case EInGameGuiState::PauseHUDMessage:
    {
        x44_messageScreen = std::make_unique<CMessageScreen>(x124_pauseGameHudMessage, x128_pauseGameHudTime);
        break;
    }
    case EInGameGuiState::PauseSaveGame:
    {
        x4c_saveUI = std::make_unique<CSaveGameScreen>(ESaveContext::InGame, g_GameState->GetCardSerial());
        break;
    }
    default:
    {
        if (x1bc_prevState >= EInGameGuiState::Zero && x1bc_prevState <= EInGameGuiState::InGame)
            x1f8_26_deferTransition = true;
        break;
    }
    }

    x3c_pauseScreen->OnNewInGameGuiState(state, stateMgr);
    if (!x1f8_26_deferTransition)
        DoStateTransition(stateMgr);

}

void CInGameGuiManager::EnsureStates(CStateManager& stateMgr)
{
    if (x1f8_26_deferTransition)
    {
        if (!x3c_pauseScreen->GetX50_25())
        {
            DestroyAreaTextures(stateMgr);
            x1f8_26_deferTransition = false;
            DoStateTransition(stateMgr);
        }
    }
}

void CInGameGuiManager::DoStateTransition(CStateManager& stateMgr)
{
    x34_samusHud->OnNewInGameGuiState(x1c0_nextState, stateMgr);
    x38_autoMapper->OnNewInGameGuiState(x1c0_nextState, stateMgr);

    bool needsLock;
    switch (x1c0_nextState)
    {
    case EInGameGuiState::PauseGame:
    case EInGameGuiState::PauseLogBook:
        if (!x48_inventoryScreen)
        {
            auto pState = stateMgr.GetPlayerState();
            CPlayerState::EPlayerSuit suit = pState->GetCurrentSuitRaw();
            int suitResIdx;
            if (pState->IsFusionEnabled())
            {
                switch (suit)
                {
                case CPlayerState::EPlayerSuit::Phazon:
                    suitResIdx = 7;
                    break;
                case CPlayerState::EPlayerSuit::Gravity:
                    suitResIdx = 6;
                    break;
                case CPlayerState::EPlayerSuit::Varia:
                    suitResIdx = 5;
                    break;
                default:
                    suitResIdx = 4;
                    break;
                }
            }
            else
            {
                switch (suit)
                {
                case CPlayerState::EPlayerSuit::Phazon:
                    suitResIdx = 3;
                    break;
                case CPlayerState::EPlayerSuit::Gravity:
                    suitResIdx = 2;
                    break;
                case CPlayerState::EPlayerSuit::Varia:
                    suitResIdx = 1;
                    break;
                default:
                    suitResIdx = 0;
                    break;
                }
            }

            int w1 = x1c0_nextState == EInGameGuiState::PauseLogBook ? 0 : 2;
            CDependencyGroup* suitGrp = x5c_pauseScreenDGRPs[suitResIdx].GetObj();
            x48_inventoryScreen = std::make_unique<CInventoryScreen>(w1, *suitGrp, *suitGrp);
        }

    case EInGameGuiState::MapScreen:
    case EInGameGuiState::PauseSaveGame:
    case EInGameGuiState::PauseHUDMessage:
        needsLock = true;
        break;
    default:
        needsLock = false;
        break;
    }

    for (CToken& tok : xe8_pauseResources)
    {
        if (needsLock)
            tok.Lock();
        else
            tok.Unlock();
    }
}

void CInGameGuiManager::DestroyAreaTextures(CStateManager& stateMgr)
{

}

void CInGameGuiManager::TryReloadAreaTextures()
{

}

CInGameGuiManager::CInGameGuiManager(CStateManager& stateMgr,
                                     CArchitectureQueue& archQueue)
: x0_iggmPreLoad(g_SimplePool->GetObj("PreLoadIGGM_DGRP")), x1c_rand(1234),
  x20_faceplateDecor(stateMgr), x50_deathDot(g_SimplePool->GetObj("TXTR_DeathDot")),
  x5c_pauseScreenDGRPs(LockPauseScreenDependencies())
{
    x1e0_helmetVisMode = g_tweakGui->GetHelmetVisMode();
    x1e4_enableTargetingManager = g_tweakGui->GetEnableTargetingManager();
    x1e8_enableAutoMapper = g_tweakGui->GetEnableAutoMapper();
    x1ec_hudVisMode = g_tweakGui->GetHudVisMode();
    x1f0_enablePlayerVisor = g_tweakGui->GetEnablePlayerVisor();

    x1f4_player74c = stateMgr.GetPlayer().Get74C();

    x1f8_25_ = true;
    x1f8_27_inSaveUI = true;

    xc8_inGameGuiDGRPs.reserve(14);
    for (int i=0 ; i<14 ; ++i)
        xc8_inGameGuiDGRPs.push_back(g_SimplePool->GetObj(InGameGuiDGRPs[i]));
}

bool CInGameGuiManager::CheckLoadComplete(CStateManager& stateMgr)
{
    switch (x18_loadPhase)
    {
    case ELoadPhase::LoadDepsGroup:
    {
        if (!x0_iggmPreLoad.IsLoaded())
            return false;
        const auto& tags = x0_iggmPreLoad->GetObjectTagVector();
        x8_preLoadDeps.reserve(tags.size());
        for (const SObjectTag& tag : tags)
        {
            x8_preLoadDeps.push_back(g_SimplePool->GetObj(tag));
            x8_preLoadDeps.back().Lock();
        }
        x0_iggmPreLoad.Unlock();
        x18_loadPhase = ELoadPhase::PreLoadDeps;
    }
    case ELoadPhase::PreLoadDeps:
    {
        for (CToken& tok : x8_preLoadDeps)
            if (!tok.IsLoaded())
                return false;
        x18_loadPhase = ELoadPhase::LoadDeps;
        x30_playerVisor = std::make_unique<CPlayerVisor>(stateMgr);
        x34_samusHud = std::make_unique<CSamusHud>(stateMgr);
        x38_autoMapper = std::make_unique<CAutoMapper>(stateMgr);
        x3c_pauseScreen = std::make_unique<CPauseScreen>();
        x40_samusReflection = std::make_unique<CSamusFaceReflection>(stateMgr);
    }
    case ELoadPhase::LoadDeps:
    {
        if (!x38_autoMapper->CheckLoadComplete())
            return false;
        if (!x34_samusHud->CheckLoadComplete(stateMgr))
            return false;
        if (!x50_deathDot.IsLoaded())
            return false;
        if (!CheckDGRPLoadComplete())
            return false;
        x8_preLoadDeps.clear();

        CGuiFrame& baseHud = *x34_samusHud->x274_loadedFrmeBaseHud;
        x144_basewidget_automapper = baseHud.FindWidget("BaseWidget_AutoMapper");
        x148_model_automapper = static_cast<CGuiModel*>(baseHud.FindWidget("Model_AutoMapper"));
        x14c_basehud_camera = baseHud.GetFrameCamera();
        x150_basewidget_functional = baseHud.FindWidget("BaseWidget_Functional");

        x154_automapperRotate = zeus::CQuaternion(x144_basewidget_automapper->GetWorldTransform().basis);
        x164_automapperOffset = x144_basewidget_automapper->GetLocalPosition();
        x170_camRotate = zeus::CQuaternion::skNoRotation;
        x180_camOffset = x14c_basehud_camera->GetLocalPosition() +
                zeus::CVector3f(0.f, 2.f, g_tweakAutoMapper->GetCamVerticalOffset());

        zeus::CMatrix3f mtx(x170_camRotate);
        x18c_camXf = zeus::CTransform(mtx, x180_camOffset);

        BeginStateTransition(EInGameGuiState::InGame, stateMgr);
        x18_loadPhase = ELoadPhase::Done;
    }
    case ELoadPhase::Done:
    {
        x34_samusHud->Touch();
        return true;
    }
    default:
        return false;
    }
}

void CInGameGuiManager::Update(CStateManager& stateMgr, float dt, CArchitectureQueue& archQueue, bool)
{
    EnsureStates(stateMgr);
}

bool CInGameGuiManager::IsInGameStateNotTransitioning() const
{
    return (x1bc_prevState >= EInGameGuiState::Zero && x1bc_prevState <= EInGameGuiState::InGame &&
            x1c0_nextState >= EInGameGuiState::Zero && x1c0_nextState <= EInGameGuiState::InGame);
}

bool CInGameGuiManager::IsInPausedStateNotTransitioning() const
{
    return (x1bc_prevState >= EInGameGuiState::MapScreen && x1bc_prevState <= EInGameGuiState::PauseHUDMessage &&
            x1c0_nextState >= EInGameGuiState::MapScreen && x1c0_nextState <= EInGameGuiState::PauseHUDMessage);
}

void CInGameGuiManager::ProcessControllerInput(CStateManager& stateMgr, const CFinalInput& input,
                                               CArchitectureQueue& archQueue)
{
    if (input.ControllerIdx() == 0)
    {
        if (!IsInGameStateNotTransitioning())
        {
            if (IsInPausedStateNotTransitioning())
            {
                if (x1bc_prevState == EInGameGuiState::MapScreen)
                {
                    if (x38_autoMapper->IsInMapperState(CAutoMapper::EAutoMapperState::MapScreen) ||
                        x38_autoMapper->IsInMapperState(CAutoMapper::EAutoMapperState::MapScreenUniverse))
                    {
                        x38_autoMapper->ProcessControllerInput(input, stateMgr);
                        if (x38_autoMapper->CanLeaveMapScreen(stateMgr))
                            BeginStateTransition(EInGameGuiState::InGame, stateMgr);
                    }
                    return;
                }
                if (x1bc_prevState == EInGameGuiState::PauseSaveGame)
                {
                    x4c_saveUI->ProcessUserInput(input);
                    return;
                }
                if (x1bc_prevState == EInGameGuiState::PauseHUDMessage)
                {
                    x44_messageScreen->ProcessControllerInput(input);
                    return;
                }
                if (x48_inventoryScreen)
                    x48_inventoryScreen->ProcessControllerInput(stateMgr, input);
            }
        }
        else
        {
            x34_samusHud->ProcessControllerInput(input);
        }
    }
}

void CInGameGuiManager::PreDraw(CStateManager& stateMgr)
{

}

void CInGameGuiManager::Draw(CStateManager& stateMgr)
{

}

void CInGameGuiManager::ShowPauseGameHudMessage(CStateManager& stateMgr, ResId pauseMsg, float time)
{
    x124_pauseGameHudMessage = pauseMsg;
    x128_pauseGameHudTime = time;
    PauseGame(stateMgr, EInGameGuiState::PauseHUDMessage);
}

void CInGameGuiManager::PauseGame(CStateManager& stateMgr, EInGameGuiState state)
{
    g_InputGenerator->SetMotorState(EIOPort::Zero, EMotorState::Stop);
    CSfxManager::SetChannel(CSfxManager::ESfxChannels::PauseScreen);
    BeginStateTransition(state, stateMgr);
}

void CInGameGuiManager::StartFadeIn()
{

}

bool CInGameGuiManager::GetIsGameDraw() const
{
    return x3c_pauseScreen->GetX50_25();
}

std::string CInGameGuiManager::GetIdentifierForMidiEvent(ResId world, ResId area,
                                                         const std::string& midiObj)
{
    return hecl::Format("World %8.8x Area %8.8x MidiObject: %s",
                        u32(world), u32(area), midiObj.c_str());
}

}
}
