#include "MetroidPrime/CInGameGuiManager.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CTevCombiners.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Input/IController.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CAutoMapper.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameGlobalObjects.hpp"
#include "MetroidPrime/CMessageScreen.hpp"
#include "MetroidPrime/CPauseScreen.hpp"
#include "MetroidPrime/CPauseScreenBlur.hpp"
#include "MetroidPrime/CSaveGameScreen.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/HUD/CSamusHud.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Player/CPlayerVisor.hpp"
#include "MetroidPrime/Player/CSamusFaceReflection.hpp"
#include "MetroidPrime/SFX/UI.h"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakAutoMapper.hpp"
#include "dolphin/gx.h"
#include "float.h"
#include "limits.h"

#include "GuiSys/CGuiCamera.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiModel.hpp"
#include "Kyoto/CDependencyGroup.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "rstl/algorithm.hpp"

// Profiling labels retained in the retail string pool.
static const char* const skGuiElementNames[] = {
    "FaceplateDecoration", "     FaceReflection", "        PlayerVisor", "                Hud",
    "         AutoMapper", "        PauseScreen", "              Total",
};

static const char* const skInGameGuiDGRPs[] = {
    "InGameGui_DGRP", "Ice_DGRP",         "Phazon_DGRP",         "Plasma_DGRP",
    "Power_DGRP",     "Wave_DGRP",        "BallTransition_DGRP", "GravitySuit_DGRP",
    "Ice_Anim_DGRP",  "Plasma_Anim_DGRP", "PowerSuit_DGRP",      "Power_Anim_DGRP",
    "VariaSuit_DGRP", "Wave_Anim_DGRP",
};

static const char* const skPauseScreenDGRPs[] = {
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
    "PauseScreenTokens_DGRP",
};

float skMapScreenCameraOffset = 2.f;

struct SDumpableTextureInfo {
  CAssetId x0_id;
  int x4_score;
  TToken< CTexture > x8_token;

  SDumpableTextureInfo(int score, CAssetId id, TToken< CTexture >& token)
  : x0_id(id), x4_score(score), x8_token(token) {}
};
CHECK_SIZEOF(SDumpableTextureInfo, 0x10)

struct CTextureScoreGreaterThan {
  CTextureScoreGreaterThan() {}
  bool operator()(const SDumpableTextureInfo& a, const SDumpableTextureInfo& b) const {
    return a.x4_score < b.x4_score;
  }
};

CInGameGuiManager::TPauseScreenDGRPs CInGameGuiManager::LockPauseScreenDependencies() {
  TPauseScreenDGRPs ret;
  for (int i = 0; i < 13; ++i) {
    TToken< CDependencyGroup > token = gpSimplePool->GetObj(skPauseScreenDGRPs[i]);
    token.Lock();
    ret.push_back(token);
  }
  return ret;
}

CInGameGuiManager::CInGameGuiManager(const CStateManager& mgr, CArchitectureQueue& queue)
: x0_iggmPreLoad(gpSimplePool->GetObj("PreLoadIGGM_DGRP"))
, x18_loadPhase(kLP_LoadDepsGroup)
, x1c_rand(1234)
, x20_faceplateDecor(mgr)
, x30_playerVisor(nullptr)
, x34_samusHud(nullptr)
, x38_autoMapper(nullptr)
, x3c_pauseScreenBlur(nullptr)
, x40_samusReflection(nullptr)
, x44_messageScreen(nullptr)
, x48_pauseScreen(nullptr)
, x4c_saveUI(nullptr)
, x50_deathDot(gpSimplePool->GetObj("TXTR_DeathDot"))
, x5c_pauseScreenDGRPs(LockPauseScreenDependencies())
, x124_pauseGameHudMessage(kInvalidAssetId)
, x128_pauseGameHudTime(0.f)
, x144_basewidget_automapper(nullptr)
, x148_model_automapper(nullptr)
, x14c_basehud_camera(nullptr)
, x150_basewidget_functional(nullptr)
, x154_automapperRotate(CQuaternion::NoRotation())
, x164_automapperOffset(CVector3f::Zero())
, x170_camRotate(CQuaternion::NoRotation())
, x180_camOffset(CVector3f::Zero())
, x18c_mapCamXf(CTransform4f::Identity())
, x1bc_prevState(kIGGS_Zero)
, x1c0_nextState(kIGGS_Zero)
, x1d8_onScreenTexAlpha(0.f)
, x1dc_onScreenTexTok(nullptr)
, x1e0_helmetVisMode(gpTweakGui->GetHelmetVisMode())
, x1e4_enableTargetingManager(gpTweakGui->GetEnableTargetingManager())
, x1e8_enableAutoMapper(gpTweakGui->GetEnableAutoMapper())
, x1ec_hudVisMode(gpTweakGui->GetHudVisMode())
, x1f0_enablePlayerVisor(gpTweakGui->GetEnablePlayerVisor())
, x1f4_visorStaticAlpha(mgr.GetPlayer()->GetVisorStaticAlpha())
, x1f8_24_(false)
, x1f8_25_playerAlive(true)
, x1f8_26_deferTransition(false)
, x1f8_27_inSaveUI(true) {
  x50_deathDot.Lock();
  x0_iggmPreLoad.Lock();
  xc8_inGameGuiDGRPs.reserve(14);
  for (uint i = 0; i < 14; ++i) {
    TToken< CDependencyGroup > token = gpSimplePool->GetObj(skInGameGuiDGRPs[i]);
    token.Lock();
    xc8_inGameGuiDGRPs.push_back(token);
  }
}

CInGameGuiManager::~CInGameGuiManager() {}

void CInGameGuiManager::InitializeDumpableARAMTextures() {
  int count = 0;
  for (AUTO(it, xc8_inGameGuiDGRPs.begin()); it != xc8_inGameGuiDGRPs.end(); ++it) {
    count += (*it)->GetCountForResType('TXTR');
  }
  xd8_inGameTextureIDs.reserve(count);

  for (AUTO(it, xc8_inGameGuiDGRPs.begin()); it != xc8_inGameGuiDGRPs.end(); ++it) {
    const rstl::vector< SObjectTag >& tags = (*it)->GetObjectTagVector();
    for (AUTO(tag, tags.begin()); tag != tags.end(); ++tag) {
      if (tag->GetType() == 'TXTR') {
        if (xd8_inGameTextureIDs.end() ==
            rstl::find(xd8_inGameTextureIDs.begin(), xd8_inGameTextureIDs.end(), tag->GetId())) {
          xd8_inGameTextureIDs.push_back(tag->GetId());
        }
      }
    }
  }
  xc8_inGameGuiDGRPs = rstl::vector< TToken< CDependencyGroup > >();

  const rstl::vector< SObjectTag >& tags = x5c_pauseScreenDGRPs[12]->GetObjectTagVector();
  xe8_pauseResources.reserve(tags.size());
  for (AUTO(it, tags.begin()); it != tags.end(); ++it) {
    xe8_pauseResources.push_back(gpSimplePool->GetObj(*it));
  }
}

bool CInGameGuiManager::CheckDGRPLoadComplete() {
  for (AUTO(it, x5c_pauseScreenDGRPs.begin()); it != x5c_pauseScreenDGRPs.end(); ++it) {
    if (!it->IsLoaded()) {
      return false;
    }
  }
  for (AUTO(it, xc8_inGameGuiDGRPs.begin()); it != xc8_inGameGuiDGRPs.end(); ++it) {
    if (!it->IsLoaded()) {
      return false;
    }
  }
  return true;
}

bool CInGameGuiManager::CheckLoadComplete(const CStateManager& mgr) {
  switch (x18_loadPhase) {
  case kLP_LoadDepsGroup: {
    if (x0_iggmPreLoad.IsLoaded()) {
      const rstl::vector< SObjectTag >& tags = x0_iggmPreLoad->GetObjectTagVector();
      x8_preLoadDeps.reserve(tags.size());
      for (AUTO(it, tags.begin()); it != tags.end(); ++it) {
        CToken token = gpSimplePool->GetObj(*it);
        token.Lock();
        x8_preLoadDeps.push_back(token);
      }
      x0_iggmPreLoad.Unlock();
      x18_loadPhase = kLP_PreLoadDeps;
    } else {
      return false;
    }
  }
  case kLP_PreLoadDeps: {
    for (AUTO(it, x8_preLoadDeps.begin()); it != x8_preLoadDeps.end(); ++it) {
      if (!it->IsLoaded()) {
        return false;
      }
    }
    x18_loadPhase = kLP_LoadDeps;
    x30_playerVisor = rs_new CPlayerVisor(mgr);
    x34_samusHud = rs_new CSamusHud(mgr);
    x38_autoMapper = rs_new CAutoMapper(mgr);
    x3c_pauseScreenBlur = rs_new CPauseScreenBlur();
    x40_samusReflection = rs_new CSamusFaceReflection(mgr);
  }
  case kLP_LoadDeps: {
    if (x38_autoMapper->CheckLoadComplete() && x34_samusHud->CheckLoadComplete(mgr) &&
        x50_deathDot.TryCache() && CheckDGRPLoadComplete()) {
      x8_preLoadDeps = rstl::vector< CToken >();
      InitializeDumpableARAMTextures();

      x144_basewidget_automapper =
          x34_samusHud->GetBaseHudFrame()->FindWidget(rstl::string_l("BaseWidget_AutoMapper"));
      x148_model_automapper = static_cast< CGuiModel* >(
          x34_samusHud->GetBaseHudFrame()->FindWidget(rstl::string_l("Model_AutoMapper")));
      x14c_basehud_camera = x34_samusHud->GetBaseHudFrame()->GetFrameCamera();
      x150_basewidget_functional =
          x34_samusHud->GetBaseHudFrame()->FindWidget(rstl::string_l("BaseWidget_Functional"));

      x154_automapperRotate =
          CQuaternion::FromMatrix(x144_basewidget_automapper->GetWorldTransform());
      x164_automapperOffset = x144_basewidget_automapper->GetWorldTransform().GetTranslation();
      x170_camRotate = CQuaternion::NoRotation();
      x180_camOffset =
          x14c_basehud_camera->GetWorldTransform().GetTranslation() +
          CVector3f(0.f, skMapScreenCameraOffset, gpTweakAutoMapper->xec_camVerticalOffset);
      x18c_mapCamXf = CTransform4f(x170_camRotate.BuildTransform(), x180_camOffset);
      BeginStateTransition(kIGGS_InGame, mgr);
      x18_loadPhase = kLP_Done;
    } else {
      return false;
    }
  }
  case kLP_Done:
    x34_samusHud->Touch();
    return true;
  default:
    return false;
  }
}

void CInGameGuiManager::StartFadeIn() {
  xf8_camFilter.SetFilter(CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_Fullscreen, 0.f,
                          CColor::Black(), kInvalidAssetId);
  xf8_camFilter.DisableFilter(0.5f);
}

void CInGameGuiManager::Draw(const CStateManager& mgr) const {
  if (!GetIsGameDraw()) {
    gpRender->SetRequestRGBA6(true);
  }
  if (x1d8_onScreenTexAlpha > 0.f && x1dc_onScreenTexTok->GetObject() != nullptr) {
    const CTexture& tex = *x1dc_onScreenTexTok->GetObject();
    gpRender->SetDepthReadWrite(false, false);
    gpRender->SetBlendMode_AlphaBlended();
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    const int w = x1c4_onScreenTex.x4_extent[0];
    const int h = x1c4_onScreenTex.x4_extent[1];
    const CViewport& viewport = CGraphics::GetViewport();
    const int x = viewport.mLeft + (viewport.mWidth - w) / 2 + x1c4_onScreenTex.xc_offset.GetX();
    const int y = viewport.mTop + (viewport.mHeight - h) / 2 - x1c4_onScreenTex.xc_offset.GetY();
    CGraphics::Render2D(tex, x, y, w, h, CColor::White().WithAlphaOf(x1d8_onScreenTexAlpha));
  }

  float staticAlpha = 0.f;
  const float deathTime = mgr.GetPlayer()->GetDeathTime();
  if (mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed && deathTime > 0.f) {
    staticAlpha = CMath::Clamp(0.f, deathTime / (0.3f * gkFirstPersonDeathTime), 1.f);
  }
  const bool notInCine = !mgr.GetCameraManager()->IsInCinematicCamera();
  const bool drawVisor = notInCine && IsInOrTransitioningToOrFromState(kIGGS_InGame);
  if (x3c_pauseScreenBlur->IsGameDraw()) {
    x34_samusHud->GetTargetingManager().Draw(mgr, true);
    CGraphics::SetDepthRange(1.f / 64.f, 1.f / 32.f);
    const bool scanVisor = mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Scan;
    if (drawVisor && x1f0_enablePlayerVisor != 0) {
      if (mgr.GetPlayer()->GetCameraState() == CPlayer::kCS_FirstPerson) {
        x20_faceplateDecor.Draw(mgr);
      }
      const bool targetingEnabled = x1e4_enableTargetingManager != 0;
      const CTargetingManager* targeting =
          scanVisor && targetingEnabled ? &x34_samusHud->GetTargetingManager() : nullptr;
      x30_playerVisor->Draw(mgr, targeting);
    }
    x40_samusReflection->Draw(mgr);
    if (drawVisor) {
      const bool hudVis = x1ec_hudVisMode != CTweakGui::kHud_Zero;
      const bool targeting = x1e4_enableTargetingManager != 0;
      CGraphics::SetDepthRange(1.f / 512.f, 1.f / 64.f);
      if (staticAlpha > 0.f) {
        CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend,
                                      CCameraFilterPass::kFS_RandomStatic,
                                      CColor::White().WithAlphaOf(staticAlpha), nullptr, 1.f);
      }
      x34_samusHud->Draw(mgr, x1f4_visorStaticAlpha * (1.f - staticAlpha), x1e0_helmetVisMode,
                         hudVis, targeting && !scanVisor);
    }
  }

  const bool preDrawBlur =
      IsInGameplayStateNotTransitioning() || IsInOrTransitioningToOrFromState(kIGGS_MapScreen);
  if (preDrawBlur) {
    x3c_pauseScreenBlur->Draw(mgr);
  }
  if (notInCine && x1e8_enableAutoMapper != 0 &&
      (x3c_pauseScreenBlur->IsGameDraw() || IsInOrTransitioningToOrFromState(kIGGS_MapScreen))) {
    const CPlayerState& playerState = *mgr.GetPlayerState();
    const CPlayer::EPlayerMorphBallState morphState =
        mgr.GetPlayer()->GetMorphballTransitionState();
    const float t = playerState.GetCurrentVisor() == CPlayerState::kPV_Combat
                        ? playerState.GetVisorTransitionFactor()
                        : 0.f;
    float mapAlpha;
    if (gpTweakGui->GetShowAutomapperInMorphball()) {
      mapAlpha = 1.f;
    } else if (morphState == CPlayer::kMS_Unmorphed) {
      mapAlpha = 1.f;
    } else {
      mapAlpha = 0.f;
    }
    CGuiCamera* camera = x34_samusHud->GetBaseHudFrame()->GetFrameCamera();
    camera->Draw(CGuiWidgetDrawParms(0.f, CVector3f::Zero()));
    CGraphics::SetDepthRange(0.f, 1.f / 512.f);
    x148_model_automapper->SetIsVisible(true);
    x148_model_automapper->Draw(CGuiWidgetDrawParms(1.f, CVector3f::Zero()));
    CGraphics::SetDepthWriteMode(true, kE_GEqual, false);
    x38_autoMapper->Draw(mgr, CTransform4f::Translate(0.f, 0.02f, 0.f) * x18c_mapCamXf,
                         mapAlpha * (x1f4_visorStaticAlpha * t));
    CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
    x148_model_automapper->SetIsVisible(false);
  }
  if (!preDrawBlur) {
    x3c_pauseScreenBlur->Draw(mgr);
  }
  if (x1e0_helmetVisMode != 0u && notInCine) {
    const float cameraOffset = x48_pauseScreen.null() ? 0.f : x48_pauseScreen->GetHelmetCamYOff();
    x34_samusHud->DrawHelmet(mgr, cameraOffset);
  }
  if (!x4c_saveUI.null()) {
    x4c_saveUI->Draw();
  }
  if (!x44_messageScreen.null()) {
    x44_messageScreen->Draw();
  }
  if (!x48_pauseScreen.null()) {
    x48_pauseScreen->Draw();
  }
  xf8_camFilter.Draw();

  if (deathTime > 0.f) {
    const float deathDuration =
        mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed
            ? gkFirstPersonDeathTime
            : gkBallDeathTime;
    const float zStart = deathDuration - 0.5f - 0.5f - 1.f;
    const float xStart = 0.5f + zStart;
    const float colorStart = 0.5f + xStart;
    const float alpha = CMath::Clamp(0.f, deathTime / deathDuration, 1.f);
    CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen,
                                  CColor::White().WithAlphaOf(alpha), nullptr, 1.f);
    if (deathTime > zStart) {
      const float zT = 1.f - CMath::Clamp(0.f, (deathTime - zStart) / 0.5f, 1.f);
      const float xT = CMath::Clamp(0.f, (deathTime - xStart) / 0.5f, 1.f);
      const float colorT = 1.f - CMath::Clamp(0.f, deathTime - colorStart, 1.f);
      int left, top, width, height;
      CGraphics::GetViewport(left, top, width, height);
      void* spareBuffer = CGraphics::GetDolphinSpareBuffer();
      GXSetTexCopySrc(left, top, width, height);
      const int halfWidth = width / 2;
      const int halfHeight = height / 2;
      GXGetTexBufferSize(halfWidth, halfHeight, GX_TF_RGB565, false, 0);
      GXSetTexCopyDst(halfWidth, halfHeight, GX_TF_RGB565, true);
      GXCopyTex(spareBuffer, false);
      GXPixModeSync();
      CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen,
                                    CColor::Black(), nullptr, 1.f);
      CGraphics::LoadDolphinSpareTexture(halfWidth, halfHeight, GX_TF_RGB565, spareBuffer,
                                         GX_TEXMAP0);
      const float zFactor = zT * zT * zT * zT * zT;
      const float z = 0.5f * (zFactor * (height - 12.f) + 12.f);
      const float negZ = -z;
      const float x = 0.5f * ((1.f - xT) * (width - 12.f) + 12.f);
      const float negX = -x;
      CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
      CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
      CGraphics::StreamBegin(kP_TriangleStrip);
      CGraphics::StreamColor(CColor::White().WithAlphaOf(colorT));
      CGraphics::StreamTexcoord(0.f, 0.f);
      CGraphics::StreamVertex(CVector3f(negX, 0.f, z));
      CGraphics::StreamTexcoord(0.f, 1.f);
      CGraphics::StreamVertex(CVector3f(negX, 0.f, negZ));
      CGraphics::StreamTexcoord(1.f, 0.f);
      CGraphics::StreamVertex(CVector3f(x, 0.f, z));
      CGraphics::StreamTexcoord(1.f, 1.f);
      CGraphics::StreamVertex(CVector3f(x, 0.f, negZ));
      CGraphics::StreamEnd();
      gpRender->SetBlendMode_ColorMultiply();
      x50_deathDot.GetObject()->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
      CGraphics::StreamBegin(kP_TriangleStrip);
      CGraphics::StreamColor(CColor::White().WithAlphaOf(colorT));
      CGraphics::StreamTexcoord(0.f, 0.f);
      CGraphics::StreamVertex(CVector3f(negX, 0.f, z));
      CGraphics::StreamTexcoord(0.f, 1.f);
      CGraphics::StreamVertex(CVector3f(negX, 0.f, negZ));
      CGraphics::StreamTexcoord(1.f, 0.f);
      CGraphics::StreamVertex(CVector3f(x, 0.f, z));
      CGraphics::StreamTexcoord(1.f, 1.f);
      CGraphics::StreamVertex(CVector3f(x, 0.f, negZ));
      CGraphics::StreamEnd();
    }
  }
}

void CInGameGuiManager::PreDraw(CStateManager& mgr, bool isCameraActive) {
  if (!x48_pauseScreen.null()) {
    x48_pauseScreen->PreDraw();
  }
  if (isCameraActive) {
    x40_samusReflection->PreDraw(mgr);
  }
}

void CInGameGuiManager::Update(const CStateManager& mgr, float dt, CArchitectureQueue& queue,
                               bool cameraActive) {
  EnsureStates(mgr);
  if (x1d8_onScreenTexAlpha == 0.f) {
    x1dc_onScreenTexTok = nullptr;
  }
  const SOnScreenTex& pending = mgr.GetPendingScreenTex();
  if (pending.x0_id != x1c4_onScreenTex.x0_id) {
    if (x1dc_onScreenTexTok.null()) {
      x1c4_onScreenTex.x0_id = pending.x0_id;
      x1c4_onScreenTex.x4_extent = pending.x4_extent;
      x1c4_onScreenTex.xc_offset = pending.xc_offset;
      if (x1c4_onScreenTex.x0_id != kInvalidAssetId) {
        x1dc_onScreenTexTok = rs_new TCachedToken< CTexture >(
            gpSimplePool->GetObj(SObjectTag('TXTR', x1c4_onScreenTex.x0_id)));
        x1dc_onScreenTexTok->Lock();
        x1d8_onScreenTexAlpha = FLT_EPSILON;
      }
    } else {
      if (pending.x0_id == kInvalidAssetId && pending.x4_extent == CVector2i(0, 0)) {
        x1c4_onScreenTex.x4_extent = pending.x4_extent;
        x1c4_onScreenTex.x0_id = kInvalidAssetId;
        x1d8_onScreenTexAlpha = 0.f;
      } else {
        x1d8_onScreenTexAlpha = rstl::max_val(0.f, x1d8_onScreenTexAlpha - dt);
      }
    }
  } else if (x1c4_onScreenTex.x0_id != kInvalidAssetId && !x1dc_onScreenTexTok.null() &&
             x1dc_onScreenTexTok->TryCache()) {
    x1d8_onScreenTexAlpha = rstl::min_val(1.f, x1d8_onScreenTexAlpha + dt);
  }

  if (cameraActive) {
    const float visorStaticAlpha = mgr.GetPlayer()->GetVisorStaticAlpha();
    if (visorStaticAlpha != x1f4_visorStaticAlpha) {
      if (TCastToConstPtr< CFirstPersonCamera >(mgr.GetCameraManager()->GetCurrentCamera(mgr))) {
        if (CMath::AbsF(visorStaticAlpha - x1f4_visorStaticAlpha) < 0.5f) {
          if (x1f4_visorStaticAlpha == 0.f) {
            CSfxManager::SfxStart(SFXui_x_hudon_00, 127, 64, false, CSfxManager::kMedPriority,
                                  false, CSfxManager::kAllAreas);
          } else if (x1f4_visorStaticAlpha == 1.f) {
            CSfxManager::SfxStart(SFXui_x_hudoff_00, 127, 64, false, CSfxManager::kMedPriority,
                                  false, CSfxManager::kAllAreas);
          }
        }
      }
    }
    x1f4_visorStaticAlpha = visorStaticAlpha;
  }
  if (cameraActive) {
    x20_faceplateDecor.Update(dt, mgr);
  }
  if (cameraActive) {
    x40_samusReflection->Update(dt, mgr, x1c_rand);
  }
  if (x1f0_enablePlayerVisor != 0 && cameraActive) {
    x30_playerVisor->Update(dt, mgr);
  }
  if (cameraActive && x1f8_25_playerAlive) {
    x34_samusHud->Update(dt, mgr, x1e0_helmetVisMode, x1ec_hudVisMode != CTweakGui::kHud_Zero,
                         x1e4_enableTargetingManager != 0);
  }
  if (x1e8_enableAutoMapper != 0) {
    UpdateAutoMapper(mgr, dt);
  }
  x3c_pauseScreenBlur->Update(dt, mgr, x12c_dumpedTextures.empty());

  if (!x4c_saveUI.null()) {
    const CIOWin::EMessageReturn ret = x4c_saveUI->Update(dt);
    if (ret != CIOWin::kMR_Normal) {
      x1f8_27_inSaveUI = ret == CIOWin::kMR_Exit;
      BeginStateTransition(kIGGS_InGame, mgr);
    }
  } else if (!x44_messageScreen.null()) {
    const float blurAmt = x3c_pauseScreenBlur->GetBlurAmt();
    if (!x44_messageScreen->Update(dt, blurAmt)) {
      BeginStateTransition(kIGGS_InGame, mgr);
    }
  }
  if (!x48_pauseScreen.null()) {
    x48_pauseScreen->Update(dt, mgr, x1c_rand, queue);
    if (!IsStateTransitioning()) {
      if (x48_pauseScreen->ShouldSwitchToMapScreen()) {
        BeginStateTransition(kIGGS_MapScreen, mgr);
      } else if (x48_pauseScreen->ShouldSwitchToInGame()) {
        BeginStateTransition(kIGGS_InGame, mgr);
      }
    }
  }
  x34_samusHud->Touch();
  x30_playerVisor->Touch();
  x34_samusHud->GetTargetingManager().Touch();

  if (IsStateTransitioning()) {
    if (InGameGuiStates::IsGameplayState(x1c0_nextState)) {
      TryReloadAreaTextures();
    }
    const CAutoMapper& mapper = *x38_autoMapper;
    const bool mapperIsTransitioning = mapper.GetCurrentState() != mapper.GetNextState();
    if ((!mapperIsTransitioning || x1e8_enableAutoMapper == 0) &&
        x3c_pauseScreenBlur->IsNotTransitioning()) {
      TryCompleteStateTransition(queue);
    }
  }
  xf8_camFilter.Update(dt);
  if (mgr.GetCameraManager()->IsInCinematicCamera()) {
    mgr.SetViewportScaleX(1.f);
    mgr.SetViewportScaleY(1.f);
  } else {
    const float scaleX = rstl::min_val(x34_samusHud->GetDesiredViewportScaleX(),
                                       x30_playerVisor->GetDesiredViewportScaleX(mgr));
    const float scaleY = rstl::min_val(x34_samusHud->GetDesiredViewportScaleY(),
                                       x30_playerVisor->GetDesiredViewportScaleY(mgr));
    mgr.SetViewportScaleX(scaleX);
    mgr.SetViewportScaleY(scaleY);
  }
  x1f8_25_playerAlive = mgr.GetPlayerState()->IsAlive();
}

void CInGameGuiManager::UpdateAutoMapper(const CStateManager& mgr, float dt) {
  x38_autoMapper->Update(dt, mgr);
  const CTransform4f xf = x148_model_automapper->Parent()->GetWorldTransform() *
                          x144_basewidget_automapper->GetTransform();
  x154_automapperRotate = CQuaternion::FromMatrix(xf);
  x164_automapperOffset = xf.GetTranslation();

  const CTransform4f& cameraXf = x14c_basehud_camera->GetWorldTransform();
  x170_camRotate = CQuaternion::FromMatrix(cameraXf);
  x180_camOffset = cameraXf.GetTranslation() + skMapScreenCameraOffset * cameraXf.GetForward() +
                   gpTweakAutoMapper->xec_camVerticalOffset * cameraXf.GetUp();

  const float frameLength =
      CMath::SlowTangentR(0.5f * CMath::Deg2Rad(x14c_basehud_camera->GetParms().perspective.fov)) /
      0.7f;
  const float scaleX = frameLength * gpTweakAutoMapper->xe4_automapperScaleX;
  const float scaleZ = frameLength * gpTweakAutoMapper->xe8_automapperScaleZ;
  if (x38_autoMapper->IsFullyOutOfMiniMapState()) {
    x148_model_automapper->SetO2WTransform(
        CTransform4f(x170_camRotate.BuildTransform(), x180_camOffset) *
        CTransform4f::Scale(scaleX, 1.f, scaleZ));
    x18c_mapCamXf = CTransform4f(x170_camRotate.BuildTransform(), x180_camOffset) *
                    CTransform4f::Scale(frameLength, 1.f, frameLength);
    x148_model_automapper->SetColor(gpTweakAutoMapper->x24_automapperWidgetColor);
  } else if (x38_autoMapper->IsFullyInMiniMapState()) {
    x148_model_automapper->SetO2WTransform(
        CTransform4f(x154_automapperRotate.BuildTransform(), x164_automapperOffset));
    x18c_mapCamXf = x148_model_automapper->GetWorldTransform();
    x148_model_automapper->SetColor(gpTweakAutoMapper->x38_automapperWidgetMiniColor);
  } else {
    float t;
    if (x38_autoMapper->GetNextState() != CAutoMapper::kAMS_MiniMap) {
      t = x38_autoMapper->GetInterp();
    } else {
      t = 1.f - x38_autoMapper->GetInterp();
    }
    const CQuaternion rotate = CQuaternion::Slerp(x154_automapperRotate, x170_camRotate, t);
    const CVector3f offset = CVector3f::Lerp(x164_automapperOffset, x180_camOffset, t);
    const float st = t * (frameLength - 1.f) + 1.f;
    x18c_mapCamXf =
        CTransform4f(rotate.BuildTransform(), offset) * CTransform4f::Scale(st, 1.f, st);
    x148_model_automapper->SetO2WTransform(
        CTransform4f(rotate.BuildTransform(), offset) *
        CTransform4f::Scale(t * (scaleX - 1.f) + 1.f, 1.f, t * (scaleZ - 1.f) + 1.f));
    const CColor color = CColor::Lerp(gpTweakAutoMapper->x38_automapperWidgetMiniColor,
                                      gpTweakAutoMapper->x24_automapperWidgetColor, t);
    x148_model_automapper->SetColor(color);
  }
}

void CInGameGuiManager::TryCompleteStateTransition(CArchitectureQueue& queue) {
  if (x1c0_nextState != kIGGS_PauseGame && x1c0_nextState != kIGGS_PauseLogBook) {
    if (!x48_pauseScreen.null() && x48_pauseScreen->IsTransitioning()) {
      return;
    }
    x48_pauseScreen = nullptr;
  }
  if (InGameGuiStates::IsGameplayState(x1c0_nextState)) {
    x44_messageScreen = nullptr;
    if (!TryReloadAreaTextures()) {
      return;
    }
    CModel::EnableTextureTimeout();
    RefreshHudOptions();
  }
  x1bc_prevState = x1c0_nextState;
}

void CInGameGuiManager::RefreshHudOptions() { x34_samusHud->RefreshHudOptions(); }

void CInGameGuiManager::ProcessControllerInput(const CStateManager& mgr, const CFinalInput& input,
                                               CArchitectureQueue& queue) {
  if (input.ControllerNumber() == 0) {
    if (!IsInGameplayStateNotTransitioning()) {
      if (IsInPausedStateNotTransitioning()) {
        if (x1bc_prevState == kIGGS_MapScreen) {
          if (x38_autoMapper->IsInMapperState(CAutoMapper::kAMS_MapScreen) ||
              x38_autoMapper->IsInMapperState(CAutoMapper::kAMS_MapScreenUniverse)) {
            x38_autoMapper->ProcessControllerInput(input, mgr);
            if (x38_autoMapper->CanLeaveMapScreen(mgr)) {
              BeginStateTransition(kIGGS_InGame, mgr);
            }
          }
          return;
        }
        if (x1bc_prevState == kIGGS_PauseSaveGame) {
          x4c_saveUI->ProcessUserInput(input);
          return;
        }
        if (x1bc_prevState == kIGGS_PauseHUDMessage) {
          x44_messageScreen->ProcessControllerInput(input);
          return;
        }
        if (!x48_pauseScreen.null()) {
          x48_pauseScreen->ProcessControllerInput(mgr, input);
        }
      }
    } else {
      x34_samusHud->ProcessControllerInput(input);
    }
  }
}

void CInGameGuiManager::PauseGame(const CStateManager& mgr, EInGameGuiState state) {
  gpController->SetMotorState(kIOP_Player1, kMS_Stop);
  CSfxManager::SetChannel(CSfxManager::kSC_PauseScreen);
  BeginStateTransition(state, mgr);
}

void CInGameGuiManager::ShowPauseGameHudMessage(const CStateManager& mgr, CAssetId message,
                                                float time) {
  x124_pauseGameHudMessage = message;
  x128_pauseGameHudTime = time;
  PauseGame(mgr, kIGGS_PauseHUDMessage);
}

bool CInGameGuiManager::GetIsGameDraw() const { return x3c_pauseScreenBlur->IsGameDraw(); }

void CInGameGuiManager::BeginStateTransition(EInGameGuiState state, const CStateManager& mgr) {
  if (x1c0_nextState == state) {
    return;
  }

  x1bc_prevState = x1c0_nextState;
  x1c0_nextState = state;

  if (state == kIGGS_InGame) {
    CSfxManager::SetChannel(CSfxManager::kSC_Game);
    x4c_saveUI = nullptr;
    x38_autoMapper->UnmuteAllLoopedSounds();
  } else if (state == kIGGS_PauseHUDMessage) {
    x44_messageScreen = rs_new CMessageScreen(x124_pauseGameHudMessage, x128_pauseGameHudTime);
  } else if (state == kIGGS_PauseSaveGame) {
    x4c_saveUI = rs_new CSaveGameScreen(kSC_InGame, gpGameState->GetCardSerial());
  } else if (InGameGuiStates::IsGameplayState(x1bc_prevState)) {
    x1f8_26_deferTransition = true;
  }
  x3c_pauseScreenBlur->OnNewInGameGuiState(state, mgr);
  if (!x1f8_26_deferTransition) {
    DoStateTransition(mgr);
  }
}

void CInGameGuiManager::DoStateTransition(const CStateManager& mgr) {
  x34_samusHud->OnNewInGameGuiState(x1c0_nextState, mgr);
  x38_autoMapper->OnNewInGameGuiState(x1c0_nextState, mgr);
  if ((x1c0_nextState == kIGGS_PauseGame || x1c0_nextState == kIGGS_PauseLogBook) &&
      x48_pauseScreen.null()) {
    const CPlayerState& playerState = *mgr.GetPlayerState();
    const CPlayerState::EPlayerSuit suit = playerState.GetCurrentSuitRaw();

    enum EPauseDependency {
      kPD_Power,
      kPD_Varia,
      kPD_Gravity,
      kPD_Phazon,
      kPD_FusionPower,
      kPD_FusionVaria,
      kPD_FusionGravity,
      kPD_FusionPhazon
    };

    const EPauseDependency suitResIdx =
        playerState.GetIsFusionEnabled() ? (suit == CPlayerState::kPS_Phazon    ? kPD_FusionPhazon
                                            : suit == CPlayerState::kPS_Gravity ? kPD_FusionGravity
                                            : suit == CPlayerState::kPS_Varia   ? kPD_FusionVaria
                                                                                : kPD_FusionPower)
                                         : (suit == CPlayerState::kPS_Phazon    ? kPD_Phazon
                                            : suit == CPlayerState::kPS_Gravity ? kPD_Gravity
                                            : suit == CPlayerState::kPS_Varia   ? kPD_Varia
                                                                                : kPD_Power);
    const CPauseScreen::ESubScreen screen = x1c0_nextState == kIGGS_PauseLogBook
                                                ? CPauseScreen::kSS_LogBook
                                                : CPauseScreen::kSS_Inventory;
    x48_pauseScreen = rs_new CPauseScreen(screen, **x5c_pauseScreenDGRPs[suitResIdx],
                                          **x5c_pauseScreenDGRPs[suitResIdx]);
  }

  const bool needsLock = InGameGuiStates::IsPausedState(x1c0_nextState);
  for (AUTO(it, xe8_pauseResources.begin()); it != xe8_pauseResources.end(); ++it) {
    if (needsLock) {
      it->Lock();
    } else {
      it->Unlock();
    }
  }
}

void CInGameGuiManager::EnsureStates(const CStateManager& mgr) {
  if (x1f8_26_deferTransition && !x3c_pauseScreenBlur->IsGameDraw()) {
    DestroyAreaTextures(mgr);
    x1f8_26_deferTransition = false;
    DoStateTransition(mgr);
  }
}

bool CInGameGuiManager::IsTextureInPauseScreen(CAssetId id) const {
  for (int i = 0; i < 13; ++i) {
    TToken< CDependencyGroup > token = x5c_pauseScreenDGRPs[i];
    const rstl::vector< SObjectTag >& tags = token->GetObjectTagVector();
    for (AUTO(it, tags.begin()); it != tags.end(); ++it) {
      if (id == it->GetId()) {
        return true;
      }
    }
  }
  return false;
}

void CInGameGuiManager::DestroyAreaTextures(const CStateManager& mgr) {
#if !defined(TARGET_PC)
  rstl::vector< SDumpableTextureInfo > candidates;
  candidates.reserve(64);
  const CWorld& world = *mgr.GetWorld();
  const CGameArea& currentArea = *world.GetArea(world.GetCurrentAreaId());
  for (int i = -1; i < currentArea.GetNumAttachedAreas(); ++i) {
    if (candidates.size() == candidates.capacity()) {
      break;
    }
    const TAreaId areaId = i == -1 ? currentArea.GetId() : currentArea.GetAttachedAreaId(i);
    if (!world.GetArea(areaId)->IsPostConstructed()) {
      continue;
    }
    const CGameArea& area = *world.GetArea(areaId);
    for (int j = 0; j < area.GetTokenCount(); ++j) {
      const rstl::pair< uint, uint >& asset = area.GetAssetID(j);
      if (!IsTextureInPauseScreen(asset.first) && asset.second == 'TXTR') {
        TToken< CTexture > token = gpSimplePool->GetObj(SObjectTag(asset.second, asset.first));
        if (token.IsLoaded()) {
          CTexture& texture = **token;
          if (texture.GetNumberOfMipMaps() < 2 && texture.GetTexelFormat() != kTF_C4 &&
              texture.fn_8030F088() == 0) {
            candidates.push_back(
                SDumpableTextureInfo(texture.GetMemoryAllocated(), asset.first, token));
            if (candidates.size() == candidates.capacity()) {
              break;
            }
          }
        }
      }
    }
  }

  int memoryFreed = 0;
  for (AUTO(it, xd8_inGameTextureIDs.begin()); it != xd8_inGameTextureIDs.end(); ++it) {
    CAssetId id = *it;
    if (!IsTextureInPauseScreen(id)) {
      TToken< CTexture > token = gpSimplePool->GetObj(SObjectTag('TXTR', id));
      if (token.IsLoaded()) {
        if (memoryFreed >= 0x100000) {
          break;
        }
        CTexture& texture = **token;
        if (texture.GetTexelFormat() != kTF_C4 && texture.fn_8030F088() == 0) {
          const int size = texture.GetMemoryAllocated();
          x12c_dumpedTextures.push_back(TDumpedTexture(id, token));
          texture.UnloadBitmapData(id);
          memoryFreed += size;
        }
      }
    }
  }

  rstl::sort(candidates.begin(), candidates.end(), CTextureScoreGreaterThan());
  CFrameDelayedKiller::StallAndFlushAllAllocations();
  CTexture::sCurrentFrameCount = INT_MAX;
  for (AUTO(it, candidates.begin()); it != candidates.end(); ++it) {
    if (memoryFreed >= 0x100000) {
      break;
    }
    TToken< CTexture >& token = it->x8_token;
    CTexture& texture = **token;
    bool transferred = false;
    if (!texture.GetNoSwap()) {
      texture.LoadToARAM();
      if (texture.IsARAMTransferInProgress()) {
        while (texture.IsARAMTransferInProgress()) {
          CARAMToken::UpdateAllDMAs();
        }
        transferred = true;
      }
    }
    if (!transferred) {
      texture.UnloadBitmapData(it->x0_id);
      x12c_dumpedTextures.push_back(TDumpedTexture(it->x0_id, token));
    }
    memoryFreed += texture.GetMemoryAllocated();
  }

  CTexture::sCurrentFrameCount = 0;
  x12c_dumpedTextures.sort(
      rstl::pair_sorter_finder< TDumpedTexture, rstl::less< CAssetId > >(rstl::less< CAssetId >()));
#endif
  CModel::DisableTextureTimeout();
}

const bool CInGameGuiManager::TryReloadAreaTextures() {
  bool complete = true;
  AUTO(it, x12c_dumpedTextures.begin());
  while (it != x12c_dumpedTextures.end()) {
    if (it->second->TryReloadBitmapData(*gpResourceFactory)) {
      it = x12c_dumpedTextures.erase(it);
    } else {
      complete = false;
      ++it;
    }
  }
  return complete;
}
