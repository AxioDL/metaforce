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
    x1f8_27_ = true;

    xc8_inGameGuiDGRPs.reserve(14);
    for (int i=0 ; i<14 ; ++i)
        xc8_inGameGuiDGRPs.push_back(g_SimplePool->GetObj(PauseScreenDGRPs[i]));
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

        CGuiFrame& baseHud = *x34_samusHud->x274_loadedBaseHud;
        x144_basewidget_automapper = baseHud.FindWidget("BaseWidget_AutoMapper");
        x148_model_automapper = static_cast<CGuiModel*>(baseHud.FindWidget("Model_AutoMapper"));
        x14c_basehud_camera = baseHud.GetFrameCamera();
        x150_basewidget_functional = baseHud.FindWidget("BaseWidget_Functional");

        x154_automapperRotate = zeus::CQuaternion(x144_basewidget_automapper->GetLocalTransform().basis);
        x164_automapperOffset = x144_basewidget_automapper->GetLocalPosition();
        x170_camRotate = zeus::CQuaternion::skNoRotation;
        x180_camOffset = x14c_basehud_camera->GetLocalPosition() +
                zeus::CVector3f(0.f, 2.f, g_tweakAutoMapper->GetCamVerticalOffset());

        zeus::CMatrix3f mtx(x170_camRotate);
        x18c_camXf = zeus::CTransform(mtx, x180_camOffset);

        BeginStateTransition(EInGameGuiState::One, stateMgr);
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

}
}
