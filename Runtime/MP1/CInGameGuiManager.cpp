#include "Runtime/MP1/CInGameGuiManager.hpp"

#include <array>

#include "Runtime/CDependencyGroup.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Audio/CSfxManager.hpp"
#include "Runtime/AutoMapper/CAutoMapper.hpp"
#include "Runtime/Camera/CFirstPersonCamera.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/GuiSys/CGuiCamera.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"
#include "Runtime/GuiSys/CGuiModel.hpp"
#include "Runtime/GuiSys/CGuiWidgetDrawParms.hpp"
#include "Runtime/Input/CInputGenerator.hpp"
#include "Runtime/MP1/CSamusHud.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/World/CPlayer.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {

constexpr std::array InGameGuiDGRPs{
    "InGameGui_DGRP",      "Ice_DGRP",         "Phazon_DGRP",   "Plasma_DGRP",      "Power_DGRP",     "Wave_DGRP",
    "BallTransition_DGRP", "GravitySuit_DGRP", "Ice_Anim_DGRP", "Plasma_Anim_DGRP", "PowerSuit_DGRP", "Power_Anim_DGRP",
    "VariaSuit_DGRP",      "Wave_Anim_DGRP",
};

constexpr std::array PauseScreenDGRPs{
    "InventorySuitPower_DGRP",         "InventorySuitVaria_DGRP",        "InventorySuitGravity_DGRP",
    "InventorySuitPhazon_DGRP",        "InventorySuitFusionPower_DGRP",  "InventorySuitFusionVaria_DGRP",
    "InventorySuitFusionGravity_DGRP", "InventorySuitFusionPhazon_DGRP", "SamusBallANCS_DGRP",
    "SamusSpiderBallANCS_DGRP",        "PauseScreenDontDump_DGRP",       "PauseScreenDontDump_NoARAM_DGRP",
    "PauseScreenTokens_DGRP",
};

std::vector<TLockedToken<CDependencyGroup>> CInGameGuiManager::LockPauseScreenDependencies() {
  std::vector<TLockedToken<CDependencyGroup>> ret;
  ret.reserve(PauseScreenDGRPs.size());
  for (const char* const dgrp : PauseScreenDGRPs) {
    ret.emplace_back(g_SimplePool->GetObj(dgrp));
  }
  return ret;
}

bool CInGameGuiManager::CheckDGRPLoadComplete() const {
  for (const auto& dgrp : x5c_pauseScreenDGRPs)
    if (!dgrp.IsLoaded())
      return false;
  for (const auto& dgrp : xc8_inGameGuiDGRPs)
    if (!dgrp.IsLoaded())
      return false;
  return true;
}

void CInGameGuiManager::BeginStateTransition(EInGameGuiState state, CStateManager& stateMgr) {
  if (x1c0_nextState == state)
    return;

  x1bc_prevState = x1c0_nextState;
  x1c0_nextState = state;

  switch (state) {
  case EInGameGuiState::InGame: {
    CSfxManager::SetChannel(CSfxManager::ESfxChannels::Game);
    x4c_saveUI.reset();
    x38_autoMapper->UnmuteAllLoopedSounds();
    break;
  }
  case EInGameGuiState::PauseHUDMessage: {
    x44_messageScreen = std::make_unique<CMessageScreen>(x124_pauseGameHudMessage, x128_pauseGameHudTime);
    break;
  }
  case EInGameGuiState::PauseSaveGame: {
    x4c_saveUI = std::make_unique<CSaveGameScreen>(ESaveContext::InGame, g_GameState->GetCardSerial());
    break;
  }
  default: {
    if (x1bc_prevState >= EInGameGuiState::Zero && x1bc_prevState <= EInGameGuiState::InGame)
      x1f8_26_deferTransition = true;
    break;
  }
  }

  x3c_pauseScreenBlur->OnNewInGameGuiState(state, stateMgr);
  if (!x1f8_26_deferTransition)
    DoStateTransition(stateMgr);
}

void CInGameGuiManager::EnsureStates(CStateManager& stateMgr) {
  if (x1f8_26_deferTransition) {
    if (!x3c_pauseScreenBlur->IsGameDraw()) {
      DestroyAreaTextures(stateMgr);
      x1f8_26_deferTransition = false;
      DoStateTransition(stateMgr);
    }
  }
}

void CInGameGuiManager::DoStateTransition(CStateManager& stateMgr) {
  x34_samusHud->OnNewInGameGuiState(x1c0_nextState, stateMgr);
  x38_autoMapper->OnNewInGameGuiState(x1c0_nextState, stateMgr);

  bool needsLock;
  switch (x1c0_nextState) {
  case EInGameGuiState::PauseGame:
  case EInGameGuiState::PauseLogBook:
    if (!x48_pauseScreen) {
      const auto& pState = stateMgr.GetPlayerState();
      const CPlayerState::EPlayerSuit suit = pState->GetCurrentSuitRaw();
      int suitResIdx;
      if (pState->IsFusionEnabled()) {
        switch (suit) {
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
      } else {
        switch (suit) {
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

      CPauseScreen::ESubScreen screen = x1c0_nextState == EInGameGuiState::PauseLogBook
                                            ? CPauseScreen::ESubScreen::LogBook
                                            : CPauseScreen::ESubScreen::Inventory;
      CDependencyGroup* suitGrp = x5c_pauseScreenDGRPs[suitResIdx].GetObj();
      x48_pauseScreen = std::make_unique<CPauseScreen>(screen, *suitGrp, *suitGrp);
    }
    [[fallthrough]];

  case EInGameGuiState::MapScreen:
  case EInGameGuiState::PauseSaveGame:
  case EInGameGuiState::PauseHUDMessage:
    needsLock = true;
    break;
  default:
    needsLock = false;
    break;
  }

  for (CToken& tok : xe8_pauseResources) {
    if (needsLock)
      tok.Lock();
    else
      tok.Unlock();
  }
}

void CInGameGuiManager::DestroyAreaTextures(CStateManager& stateMgr) {}

void CInGameGuiManager::TryReloadAreaTextures() {}

CInGameGuiManager::CInGameGuiManager(CStateManager& stateMgr, CArchitectureQueue& archQueue)
: x0_iggmPreLoad(g_SimplePool->GetObj("PreLoadIGGM_DGRP"))
, x1c_rand(1234)
, x20_faceplateDecor(stateMgr)
, x50_deathDot(g_SimplePool->GetObj("TXTR_DeathDot"))
, x5c_pauseScreenDGRPs(LockPauseScreenDependencies()) {
  x1e0_helmetVisMode = g_tweakGui->GetHelmetVisMode();
  x1e4_enableTargetingManager = g_tweakGui->GetEnableTargetingManager();
  x1e8_enableAutoMapper = g_tweakGui->GetEnableAutoMapper();
  x1ec_hudVisMode = g_tweakGui->GetHudVisMode();
  x1f0_enablePlayerVisor = g_tweakGui->GetEnablePlayerVisor();

  x1f4_visorStaticAlpha = stateMgr.GetPlayer().GetVisorStaticAlpha();

  xc8_inGameGuiDGRPs.reserve(InGameGuiDGRPs.size());
  for (const char* const dgrp : InGameGuiDGRPs) {
    xc8_inGameGuiDGRPs.emplace_back(g_SimplePool->GetObj(dgrp));
  }
}

bool CInGameGuiManager::CheckLoadComplete(CStateManager& stateMgr) {
  switch (x18_loadPhase) {
  case ELoadPhase::LoadDepsGroup: {
    if (!x0_iggmPreLoad.IsLoaded())
      return false;
    const auto& tags = x0_iggmPreLoad->GetObjectTagVector();
    x8_preLoadDeps.reserve(tags.size());
    for (const SObjectTag& tag : tags) {
      x8_preLoadDeps.push_back(g_SimplePool->GetObj(tag));
      x8_preLoadDeps.back().Lock();
    }
    x0_iggmPreLoad.Unlock();
    x18_loadPhase = ELoadPhase::PreLoadDeps;
    [[fallthrough]];
  }
  case ELoadPhase::PreLoadDeps: {
    for (CToken& tok : x8_preLoadDeps)
      if (!tok.IsLoaded())
        return false;
    x18_loadPhase = ELoadPhase::LoadDeps;
    x30_playerVisor = std::make_unique<CPlayerVisor>(stateMgr);
    x34_samusHud = std::make_unique<CSamusHud>(stateMgr);
    x38_autoMapper = std::make_unique<CAutoMapper>(stateMgr);
    x3c_pauseScreenBlur = std::make_unique<CPauseScreenBlur>();
    x40_samusReflection = std::make_unique<CSamusFaceReflection>(stateMgr);
    [[fallthrough]];
  }
  case ELoadPhase::LoadDeps: {
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
    x170_camRotate = zeus::CQuaternion();
    x180_camOffset =
        x14c_basehud_camera->GetLocalPosition() + zeus::CVector3f(0.f, 2.f, g_tweakAutoMapper->GetCamVerticalOffset());

    zeus::CMatrix3f mtx(x170_camRotate);
    x18c_mapCamXf = zeus::CTransform(mtx, x180_camOffset);

    BeginStateTransition(EInGameGuiState::InGame, stateMgr);
    x18_loadPhase = ELoadPhase::Done;
    [[fallthrough]];
  }
  case ELoadPhase::Done: {
    x34_samusHud->Touch();
    return true;
  }
  default:
    return false;
  }
}

void CInGameGuiManager::RefreshHudOptions() { x34_samusHud->RefreshHudOptions(); }

void CInGameGuiManager::OnNewPauseScreenState(CArchitectureQueue& archQueue) {
  if (x1c0_nextState != EInGameGuiState::PauseGame && x1c0_nextState != EInGameGuiState::PauseLogBook) {
    if (x48_pauseScreen && x48_pauseScreen->IsTransitioning())
      return;
    x48_pauseScreen.reset();
  }

  if (x1c0_nextState >= EInGameGuiState::Zero && x1c0_nextState <= EInGameGuiState::InGame) {
    if (x44_messageScreen)
      x44_messageScreen.reset();
    RefreshHudOptions();
  }
  x1bc_prevState = x1c0_nextState;
}

void CInGameGuiManager::UpdateAutoMapper(float dt, CStateManager& stateMgr) {
  x38_autoMapper->Update(dt, stateMgr);
  zeus::CTransform xf =
      x148_model_automapper->GetParent()->GetWorldTransform() * x144_basewidget_automapper->GetTransform();
  x154_automapperRotate = zeus::CQuaternion(xf.basis);
  x164_automapperOffset = xf.origin;

  x170_camRotate = zeus::CQuaternion(x14c_basehud_camera->GetWorldTransform().basis);
  x180_camOffset = x14c_basehud_camera->GetWorldTransform().basis[1] * 2.f + x14c_basehud_camera->GetWorldPosition() +
                   x14c_basehud_camera->GetWorldTransform().basis[2] * g_tweakAutoMapper->GetCamVerticalOffset();

  float frameLength = std::tan(zeus::degToRad(x14c_basehud_camera->GetProjection().xbc_fov) / 2.f) / 0.7f;
  float scaleX = frameLength * g_tweakAutoMapper->GetAutomapperScaleX();
  float scaleZ = frameLength * g_tweakAutoMapper->GetAutomapperScaleZ();
  if (x38_autoMapper->IsFullyOutOfMiniMapState()) {
    x148_model_automapper->SetO2WTransform(zeus::CTransform(x170_camRotate, x180_camOffset) *
                                           zeus::CTransform::Scale(scaleX, 1.f, scaleZ));
    x18c_mapCamXf =
        zeus::CTransform(x170_camRotate, x180_camOffset) * zeus::CTransform::Scale(frameLength, 1.f, frameLength);
    x148_model_automapper->SetColor(g_tweakAutoMapper->GetAutomapperWidgetColor());
  } else if (x38_autoMapper->IsFullyInMiniMapState()) {
    x148_model_automapper->SetO2WTransform(zeus::CTransform(x154_automapperRotate, x164_automapperOffset));
    x18c_mapCamXf = x148_model_automapper->GetWorldTransform();
    x148_model_automapper->SetColor(g_tweakAutoMapper->GetAutomapperWidgetMiniColor());
  } else {
    float t;
    if (x38_autoMapper->GetNextState() != CAutoMapper::EAutoMapperState::MiniMap)
      t = x38_autoMapper->GetInterp();
    else
      t = 1.f - x38_autoMapper->GetInterp();
    float st = t * (frameLength - 1.f) + 1.f;
    zeus::CQuaternion rotate = zeus::CQuaternion::slerp(x154_automapperRotate, x170_camRotate, t);
    zeus::CVector3f offset = x164_automapperOffset * (1.f - t) + x180_camOffset * t;
    x18c_mapCamXf = zeus::CTransform(rotate, offset) * zeus::CTransform::Scale(st, 1.f, st);
    x148_model_automapper->SetO2WTransform(
        zeus::CTransform(rotate, offset) *
        zeus::CTransform::Scale(t * (scaleX - 1.f) + 1.f, 1.f, t * (scaleZ - 1.f) + 1.f));
    x148_model_automapper->SetColor(zeus::CColor::lerp(g_tweakAutoMapper->GetAutomapperWidgetMiniColor(),
                                                       g_tweakAutoMapper->GetAutomapperWidgetColor(), t));
  }
}

void CInGameGuiManager::Update(CStateManager& stateMgr, float dt, CArchitectureQueue& archQueue, bool useHud) {
  EnsureStates(stateMgr);

  if (x1d8_onScreenTexAlpha == 0.f)
    x1dc_onScreenTexTok = TLockedToken<CTexture>();

  if (x1c4_onScreenTex.x0_id != stateMgr.GetPendingScreenTex().x0_id) {
    if (!x1dc_onScreenTexTok) {
      x1c4_onScreenTex = stateMgr.GetPendingScreenTex();
      if (x1c4_onScreenTex.x0_id.IsValid()) {
        x1dc_onScreenTexTok = g_SimplePool->GetObj({FOURCC('TXTR'), x1c4_onScreenTex.x0_id});
        x1d8_onScreenTexAlpha = FLT_EPSILON;
      }
    } else {
      if (!stateMgr.GetPendingScreenTex().x0_id.IsValid() &&
          stateMgr.GetPendingScreenTex().x4_origin == zeus::CVector2i(0, 0)) {
        x1c4_onScreenTex.x4_origin = stateMgr.GetPendingScreenTex().x4_origin;
        x1c4_onScreenTex.x0_id = {};
        x1d8_onScreenTexAlpha = 0.f;
      } else {
        x1d8_onScreenTexAlpha = std::max(0.f, x1d8_onScreenTexAlpha - dt);
      }
    }
  } else if (x1c4_onScreenTex.x0_id.IsValid() && x1dc_onScreenTexTok.IsLoaded()) {
    x1d8_onScreenTexAlpha = std::min(x1d8_onScreenTexAlpha + dt, 1.f);
  }

  if (useHud) {
    if (stateMgr.GetPlayer().GetVisorStaticAlpha() != x1f4_visorStaticAlpha) {
      if (TCastToPtr<CFirstPersonCamera>(stateMgr.GetCameraManager()->GetCurrentCamera(stateMgr))) {
        if (std::fabs(stateMgr.GetPlayer().GetVisorStaticAlpha() - x1f4_visorStaticAlpha) < 0.5f) {
          if (x1f4_visorStaticAlpha == 0.f)
            CSfxManager::SfxStart(SFXui_hud_reboot, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
          else if (x1f4_visorStaticAlpha == 1.f)
            CSfxManager::SfxStart(SFXui_hud_shutdown, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        }
      }
    }
    x1f4_visorStaticAlpha = stateMgr.GetPlayer().GetVisorStaticAlpha();
    x20_faceplateDecor.Update(dt, stateMgr);
    x40_samusReflection->Update(dt, stateMgr, x1c_rand);
    if (x1f0_enablePlayerVisor)
      x30_playerVisor->Update(dt, stateMgr);
    if (x1f8_25_playerAlive)
      x34_samusHud->Update(dt, stateMgr, x1e0_helmetVisMode, x1ec_hudVisMode != EHudVisMode::Zero,
                           x1e4_enableTargetingManager);
  }

  if (x1e8_enableAutoMapper)
    UpdateAutoMapper(dt, stateMgr);

  x3c_pauseScreenBlur->Update(dt, stateMgr, !x140_);

  if (x4c_saveUI) {
    CIOWin::EMessageReturn ret = x4c_saveUI->Update(dt);
    if (ret != CIOWin::EMessageReturn::Normal) {
      x1f8_27_exitSaveUI = ret == CIOWin::EMessageReturn::Exit;
      BeginStateTransition(EInGameGuiState::InGame, stateMgr);
    }
  } else if (x44_messageScreen) {
    if (!x44_messageScreen->Update(dt, x3c_pauseScreenBlur->GetBlurAmt()))
      BeginStateTransition(EInGameGuiState::InGame, stateMgr);
  }

  if (x48_pauseScreen) {
    x48_pauseScreen->Update(dt, stateMgr, x1c_rand, archQueue);
    if (x1bc_prevState == x1c0_nextState) {
      if (x48_pauseScreen->ShouldSwitchToMapScreen())
        BeginStateTransition(EInGameGuiState::MapScreen, stateMgr);
      else if (x48_pauseScreen->ShouldSwitchToInGame())
        BeginStateTransition(EInGameGuiState::InGame, stateMgr);
    }
  }

  x34_samusHud->Touch();
  x30_playerVisor->Touch();
  x34_samusHud->GetTargetingManager().Touch();

  if (x1bc_prevState != x1c0_nextState) {
    if (x1c0_nextState == EInGameGuiState::Zero || x1c0_nextState == EInGameGuiState::InGame)
      TryReloadAreaTextures();
    if ((!x38_autoMapper->IsInMapperStateTransition() || !x1e8_enableAutoMapper) &&
        x3c_pauseScreenBlur->IsNotTransitioning())
      OnNewPauseScreenState(archQueue);
  }

  xf8_camFilter.Update(dt);
  if (stateMgr.GetCameraManager()->IsInCinematicCamera()) {
    stateMgr.SetViewportScale(zeus::CVector2f(1.f, 1.f));
  } else {
    stateMgr.SetViewportScale(zeus::CVector2f(
        std::min(x30_playerVisor->GetDesiredViewportScaleX(stateMgr), x34_samusHud->GetViewportScale().x()),
        std::min(x30_playerVisor->GetDesiredViewportScaleY(stateMgr), x34_samusHud->GetViewportScale().y())));
  }

  x1f8_25_playerAlive = stateMgr.GetPlayerState()->IsPlayerAlive();
}

bool CInGameGuiManager::IsInGameStateNotTransitioning() const {
  return (x1bc_prevState >= EInGameGuiState::Zero && x1bc_prevState <= EInGameGuiState::InGame &&
          x1c0_nextState >= EInGameGuiState::Zero && x1c0_nextState <= EInGameGuiState::InGame);
}

bool CInGameGuiManager::IsInPausedStateNotTransitioning() const {
  return (x1bc_prevState >= EInGameGuiState::MapScreen && x1bc_prevState <= EInGameGuiState::PauseHUDMessage &&
          x1c0_nextState >= EInGameGuiState::MapScreen && x1c0_nextState <= EInGameGuiState::PauseHUDMessage);
}

void CInGameGuiManager::ProcessControllerInput(CStateManager& stateMgr, const CFinalInput& input,
                                               CArchitectureQueue& archQueue) {
  if (input.ControllerIdx() == 0) {
    if (!IsInGameStateNotTransitioning()) {
      if (IsInPausedStateNotTransitioning()) {
        if (x1bc_prevState == EInGameGuiState::MapScreen) {
          if (x38_autoMapper->IsInMapperState(CAutoMapper::EAutoMapperState::MapScreen) ||
              x38_autoMapper->IsInMapperState(CAutoMapper::EAutoMapperState::MapScreenUniverse)) {
            x38_autoMapper->ProcessControllerInput(input, stateMgr);
            if (x38_autoMapper->CanLeaveMapScreen(stateMgr))
              BeginStateTransition(EInGameGuiState::InGame, stateMgr);
          }
          return;
        }
        if (x1bc_prevState == EInGameGuiState::PauseSaveGame) {
          x4c_saveUI->ProcessUserInput(input);
          return;
        }
        if (x1bc_prevState == EInGameGuiState::PauseHUDMessage) {
          x44_messageScreen->ProcessControllerInput(input);
          return;
        }
        if (x48_pauseScreen)
          x48_pauseScreen->ProcessControllerInput(stateMgr, input);
      }
    } else {
      x34_samusHud->ProcessControllerInput(input);
    }
  }
}

void CInGameGuiManager::PreDraw(CStateManager& stateMgr, bool cameraActive) {
  if (x48_pauseScreen)
    x48_pauseScreen->PreDraw();
  if (cameraActive)
    x40_samusReflection->PreDraw(stateMgr);
}

void CInGameGuiManager::Draw(CStateManager& stateMgr) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CInGameGuiManager::Draw", zeus::skBlue);
  // if (!GetIsGameDraw())
  //    g_Renderer->x318_26_requestRGBA6 = true;
  if (x1d8_onScreenTexAlpha > 0.f && x1dc_onScreenTexTok.IsLoaded()) {
    if (!m_onScreenQuad || m_onScreenQuad->GetTex().GetObj() != x1dc_onScreenTexTok.GetObj())
      m_onScreenQuad.emplace(EFilterType::Blend, x1dc_onScreenTexTok);

    // No depth read/write
    // Alpha blend
    int w =
        (g_Viewport.x0_left + (x1c4_onScreenTex.x4_origin.x - g_Viewport.x8_width) / 2 + x1c4_onScreenTex.xc_extent.x) -
        x1c4_onScreenTex.x4_origin.x;
    int h =
        (g_Viewport.x4_top + (x1c4_onScreenTex.x4_origin.y - g_Viewport.xc_height) / 2 - x1c4_onScreenTex.xc_extent.y) -
        x1c4_onScreenTex.x4_origin.y;
    zeus::CRectangle rect(x1c4_onScreenTex.x4_origin.x / float(g_Viewport.x8_width),
                          x1c4_onScreenTex.x4_origin.y / float(g_Viewport.xc_height), w / float(g_Viewport.x8_width),
                          h / float(g_Viewport.xc_height));
    m_onScreenQuad->draw(zeus::CColor(1.f, x1d8_onScreenTexAlpha), 1.f, rect);
  }

  float staticAlpha = 0.f;
  if (stateMgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed &&
      stateMgr.GetPlayer().GetDeathTime() > 0.f)
    staticAlpha = zeus::clamp(0.f, stateMgr.GetPlayer().GetDeathTime() / (0.3f * 2.5f), 1.f);

  bool notInCine = !stateMgr.GetCameraManager()->IsInCinematicCamera();
  bool drawVisor = false;
  /* Let's always draw the HUD except in cinematic mode */
  if (notInCine /* && (x1bc_prevState == EInGameGuiState::InGame || x1c0_nextState == EInGameGuiState::InGame) */)
    drawVisor = true;

  // if (x3c_pauseScreenBlur->IsGameDraw())
  {
    x34_samusHud->GetTargetingManager().Draw(stateMgr, true);
    CGraphics::SetDepthRange(DEPTH_SCREEN_ACTORS, DEPTH_GUN);
    bool scanVisor = stateMgr.GetPlayerState()->GetActiveVisor(stateMgr) == CPlayerState::EPlayerVisor::Scan;
    if (drawVisor && x1f0_enablePlayerVisor) {
      if (stateMgr.GetPlayer().GetCameraState() == CPlayer::EPlayerCameraState::FirstPerson)
        x20_faceplateDecor.Draw(stateMgr);
      CTargetingManager* tgtMgr = nullptr;
      if (scanVisor && x1e4_enableTargetingManager)
        tgtMgr = &x34_samusHud->GetTargetingManager();
      x30_playerVisor->Draw(stateMgr, tgtMgr);
    }
    x40_samusReflection->Draw(stateMgr);
    if (drawVisor) {
      CGraphics::SetDepthRange(DEPTH_HUD, DEPTH_SCREEN_ACTORS);
      if (staticAlpha > 0.f)
        m_randomStatic.draw(zeus::CColor(1.f, staticAlpha), 1.f);
      x34_samusHud->Draw(stateMgr, x1f4_visorStaticAlpha * (1.f - staticAlpha), x1e0_helmetVisMode,
                         x1ec_hudVisMode != EHudVisMode::Zero, x1e4_enableTargetingManager && !scanVisor);
    }
  }

  bool preDrawBlur = true;
  if (x1bc_prevState >= EInGameGuiState::Zero && x1bc_prevState <= EInGameGuiState::InGame)
    if (x1bc_prevState != EInGameGuiState::MapScreen && x1c0_nextState != EInGameGuiState::MapScreen)
      preDrawBlur = false;
  if (preDrawBlur)
    x3c_pauseScreenBlur->Draw(stateMgr);

  if (notInCine && x1e8_enableAutoMapper &&
      (x3c_pauseScreenBlur->IsGameDraw() || x1bc_prevState == EInGameGuiState::MapScreen ||
       x1c0_nextState == EInGameGuiState::MapScreen)) {
    float t;
    if (stateMgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Combat)
      t = stateMgr.GetPlayerState()->GetVisorTransitionFactor();
    else
      t = 0.f;

    float mapAlpha;
    if (g_tweakGui->GetShowAutomapperInMorphball())
      mapAlpha = 1.f;
    else if (stateMgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed)
      mapAlpha = 1.f;
    else
      mapAlpha = 0.f;

    x34_samusHud->GetBaseHudFrame()->GetFrameCamera()->Draw(CGuiWidgetDrawParms(0.f, zeus::skZero3f));
    CGraphics::SetDepthRange(DEPTH_NEAR, DEPTH_HUD);
    x148_model_automapper->SetIsVisible(true);
    x148_model_automapper->Draw(CGuiWidgetDrawParms(1.f, zeus::skZero3f));
    // ZTest no write
    x38_autoMapper->Draw(stateMgr, zeus::CTransform::Translate(0.f, 0.02f, 0.f) * x18c_mapCamXf,
                         mapAlpha * x1f4_visorStaticAlpha * t);
    // Zest and write
    x148_model_automapper->SetIsVisible(false);
  }

  if (!preDrawBlur)
    x3c_pauseScreenBlur->Draw(stateMgr);

  if (x1e0_helmetVisMode != EHelmetVisMode::ReducedUpdate && notInCine) {
    float camYOff;
    if (!x48_pauseScreen)
      camYOff = 0.f;
    else
      camYOff = x48_pauseScreen->GetHelmetCamYOff();
    x34_samusHud->DrawHelmet(stateMgr, camYOff);
  }

  if (x4c_saveUI)
    x4c_saveUI->Draw();

  if (x44_messageScreen)
    x44_messageScreen->Draw();

  if (x48_pauseScreen)
    x48_pauseScreen->Draw();

  xf8_camFilter.Draw();

  if (stateMgr.GetPlayer().GetDeathTime() > 0.f) {
    float dieDur;
    if (stateMgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed)
      dieDur = 2.5f;
    else
      dieDur = 6.f;

    float alpha = zeus::clamp(0.f, stateMgr.GetPlayer().GetDeathTime() / dieDur, 1.f);
    m_deathWhiteout.draw(zeus::CColor(1.f, alpha));

    float zStart = dieDur - 0.5f - 0.5f - 1.f;
    float xStart = 0.5f - zStart;
    float colStart = 0.5f - xStart;
    if (stateMgr.GetPlayer().GetDeathTime() > zStart) {
      float zT = 1.f - zeus::clamp(0.f, (stateMgr.GetPlayer().GetDeathTime() - zStart) / 0.5f, 1.f);
      float xT = 1.f - zeus::clamp(0.f, (stateMgr.GetPlayer().GetDeathTime() - xStart) / 0.5f, 1.f);
      float colT = 1.f - zeus::clamp(0.f, (stateMgr.GetPlayer().GetDeathTime() - colStart) / 0.5f, 1.f);
      SClipScreenRect rect(g_Viewport);
      CGraphics::ResolveSpareTexture(rect);
      m_deathBlackout.draw(zeus::skBlack);
      float z = 0.5f * (zT * zT * zT * zT * zT * (g_Viewport.xc_height - 12.f) + 12.f);
      float x = 0.5f * (xT * (g_Viewport.x8_width - 12.f) + 12.f);

      const std::array<CTexturedQuadFilter::Vert, 4> verts{{
          {{-x, 0.f, z}, {0.f, 0.f}},
          {{-x, 0.f, -z}, {0.f, 1.f}},
          {{x, 0.f, z}, {1.f, 0.f}},
          {{x, 0.f, -z}, {1.f, 1.f}},
      }};

      if (!m_deathRenderTexQuad)
        m_deathRenderTexQuad.emplace(EFilterType::Blend, CGraphics::g_SpareTexture.get());
      m_deathRenderTexQuad->drawVerts(zeus::CColor(1.f, colT), verts);

      if (!m_deathDotQuad)
        m_deathDotQuad.emplace(EFilterType::Multiply, x50_deathDot);
      m_deathDotQuad->drawVerts(zeus::CColor(1.f, colT), verts);
    }
  }
}

void CInGameGuiManager::ShowPauseGameHudMessage(CStateManager& stateMgr, CAssetId pauseMsg, float time) {
  x124_pauseGameHudMessage = pauseMsg;
  x128_pauseGameHudTime = time;
  PauseGame(stateMgr, EInGameGuiState::PauseHUDMessage);
}

void CInGameGuiManager::PauseGame(CStateManager& stateMgr, EInGameGuiState state) {
  g_InputGenerator->SetMotorState(EIOPort::Zero, EMotorState::Stop);
  CSfxManager::SetChannel(CSfxManager::ESfxChannels::PauseScreen);
  BeginStateTransition(state, stateMgr);
}

void CInGameGuiManager::StartFadeIn() {
  xf8_camFilter.SetFilter(EFilterType::Multiply, EFilterShape::Fullscreen, 0.f, zeus::skBlack, -1);
  xf8_camFilter.DisableFilter(0.5f);
}

bool CInGameGuiManager::GetIsGameDraw() const {
  // Always draw world for URDE, even while paused
  return true;
  // return x3c_pauseScreenBlur->IsGameDraw();
}

} // namespace urde::MP1
