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

#if defined(TARGET_PC)
#include "Metaforce/Display.hpp"
#endif

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
  CAssetId mId;
  int mScore;
  TToken< CTexture > mToken;

  SDumpableTextureInfo(int score, CAssetId id, TToken< CTexture >& token)
  : mId(id), mScore(score), mToken(token) {}
};
CHECK_SIZEOF(SDumpableTextureInfo, 0x10)

struct CTextureScoreGreaterThan {
  CTextureScoreGreaterThan() {}
  bool operator()(const SDumpableTextureInfo& a, const SDumpableTextureInfo& b) const {
    return a.mScore < b.mScore;
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
: mIggmPreLoad(gpSimplePool->GetObj("PreLoadIGGM_DGRP"))
, mLoadPhase(kLP_LoadDepsGroup)
, mRand(1234)
, mFaceplateDecor(mgr)
, mPlayerVisor(nullptr)
, mSamusHud(nullptr)
, mAutoMapper(nullptr)
, mPauseScreenBlur(nullptr)
, mSamusReflection(nullptr)
, mMessageScreen(nullptr)
, mPauseScreen(nullptr)
, mSaveUI(nullptr)
, mDeathDot(gpSimplePool->GetObj("TXTR_DeathDot"))
, mPauseScreenDGRPs(LockPauseScreenDependencies())
, mPauseGameHudMessage(kInvalidAssetId)
, mPauseGameHudTime(0.f)
, mBasewidget_automapper(nullptr)
, mModel_automapper(nullptr)
, mBasehud_camera(nullptr)
, mBasewidget_functional(nullptr)
, mAutomapperRotate(CQuaternion::NoRotation())
, mAutomapperOffset(CVector3f::Zero())
, mCamRotate(CQuaternion::NoRotation())
, mCamOffset(CVector3f::Zero())
, mMapCamXf(CTransform4f::Identity())
, mPrevState(kIGGS_Zero)
, mNextState(kIGGS_Zero)
, mOnScreenTexAlpha(0.f)
, mOnScreenTexTok(nullptr)
, mHelmetVisMode(gpTweakGui->GetHelmetVisMode())
, mEnableTargetingManager(gpTweakGui->GetEnableTargetingManager())
, mEnableAutoMapper(gpTweakGui->GetEnableAutoMapper())
, mHudVisMode(gpTweakGui->GetHudVisMode())
, mEnablePlayerVisor(gpTweakGui->GetEnablePlayerVisor())
, mVisorStaticAlpha(mgr.GetPlayer()->GetVisorStaticAlpha())
, x1f8_24_(false)
, mPlayerAlive(true)
, mDeferTransition(false)
, mInSaveUI(true) {
  mDeathDot.Lock();
  mIggmPreLoad.Lock();
  mInGameGuiDGRPs.reserve(14);
  for (uint i = 0; i < 14; ++i) {
    TToken< CDependencyGroup > token = gpSimplePool->GetObj(skInGameGuiDGRPs[i]);
    token.Lock();
    mInGameGuiDGRPs.push_back(token);
  }
}

CInGameGuiManager::~CInGameGuiManager() {}

void CInGameGuiManager::InitializeDumpableARAMTextures() {
  int count = 0;
  for (AUTO(it, mInGameGuiDGRPs.begin()); it != mInGameGuiDGRPs.end(); ++it) {
    count += (*it)->GetCountForResType('TXTR');
  }
  mInGameTextureIDs.reserve(count);

  for (AUTO(it, mInGameGuiDGRPs.begin()); it != mInGameGuiDGRPs.end(); ++it) {
    const rstl::vector< SObjectTag >& tags = (*it)->GetObjectTagVector();
    for (AUTO(tag, tags.begin()); tag != tags.end(); ++tag) {
      if (tag->GetType() == 'TXTR') {
        if (mInGameTextureIDs.end() ==
            rstl::find(mInGameTextureIDs.begin(), mInGameTextureIDs.end(), tag->GetId())) {
          mInGameTextureIDs.push_back(tag->GetId());
        }
      }
    }
  }
  mInGameGuiDGRPs = rstl::vector< TToken< CDependencyGroup > >();

  const rstl::vector< SObjectTag >& tags = mPauseScreenDGRPs[12]->GetObjectTagVector();
  mPauseResources.reserve(tags.size());
  for (AUTO(it, tags.begin()); it != tags.end(); ++it) {
    mPauseResources.push_back(gpSimplePool->GetObj(*it));
  }
}

bool CInGameGuiManager::CheckDGRPLoadComplete() {
  for (AUTO(it, mPauseScreenDGRPs.begin()); it != mPauseScreenDGRPs.end(); ++it) {
    if (!it->IsLoaded()) {
      return false;
    }
  }
  for (AUTO(it, mInGameGuiDGRPs.begin()); it != mInGameGuiDGRPs.end(); ++it) {
    if (!it->IsLoaded()) {
      return false;
    }
  }
  return true;
}

bool CInGameGuiManager::CheckLoadComplete(const CStateManager& mgr) {
  switch (mLoadPhase) {
  case kLP_LoadDepsGroup: {
    if (mIggmPreLoad.IsLoaded()) {
      const rstl::vector< SObjectTag >& tags = mIggmPreLoad->GetObjectTagVector();
      mPreLoadDeps.reserve(tags.size());
      for (AUTO(it, tags.begin()); it != tags.end(); ++it) {
        CToken token = gpSimplePool->GetObj(*it);
        token.Lock();
        mPreLoadDeps.push_back(token);
      }
      mIggmPreLoad.Unlock();
      mLoadPhase = kLP_PreLoadDeps;
    } else {
      return false;
    }
  }
  case kLP_PreLoadDeps: {
    for (AUTO(it, mPreLoadDeps.begin()); it != mPreLoadDeps.end(); ++it) {
      if (!it->IsLoaded()) {
        return false;
      }
    }
    mLoadPhase = kLP_LoadDeps;
    mPlayerVisor = rs_new CPlayerVisor(mgr);
    mSamusHud = rs_new CSamusHud(mgr);
    mAutoMapper = rs_new CAutoMapper(mgr);
    mPauseScreenBlur = rs_new CPauseScreenBlur();
    mSamusReflection = rs_new CSamusFaceReflection(mgr);
  }
  case kLP_LoadDeps: {
    if (mAutoMapper->CheckLoadComplete() && mSamusHud->CheckLoadComplete(mgr) &&
        mDeathDot.TryCache() && CheckDGRPLoadComplete()) {
      mPreLoadDeps = rstl::vector< CToken >();
      InitializeDumpableARAMTextures();

      mBasewidget_automapper =
          mSamusHud->GetBaseHudFrame()->FindWidget(rstl::string_l("BaseWidget_AutoMapper"));
      mModel_automapper = static_cast< CGuiModel* >(
          mSamusHud->GetBaseHudFrame()->FindWidget(rstl::string_l("Model_AutoMapper")));
      mBasehud_camera = mSamusHud->GetBaseHudFrame()->GetFrameCamera();
      mBasewidget_functional =
          mSamusHud->GetBaseHudFrame()->FindWidget(rstl::string_l("BaseWidget_Functional"));

      mAutomapperRotate =
          CQuaternion::FromMatrix(mBasewidget_automapper->GetWorldTransform());
      mAutomapperOffset = mBasewidget_automapper->GetWorldTransform().GetTranslation();
      mCamRotate = CQuaternion::NoRotation();
      mCamOffset =
          mBasehud_camera->GetWorldTransform().GetTranslation() +
          CVector3f(0.f, skMapScreenCameraOffset, gpTweakAutoMapper->mCamVerticalOffset);
      mMapCamXf = CTransform4f(mCamRotate.BuildTransform(), mCamOffset);
      BeginStateTransition(kIGGS_InGame, mgr);
      mLoadPhase = kLP_Done;
    } else {
      return false;
    }
  }
  case kLP_Done:
    mSamusHud->Touch();
    return true;
  default:
    return false;
  }
}

void CInGameGuiManager::StartFadeIn() {
  mCamFilter.SetFilter(CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_Fullscreen, 0.f,
                          CColor::Black(), kInvalidAssetId);
  mCamFilter.DisableFilter(0.5f);
}

void CInGameGuiManager::Draw(const CStateManager& mgr) const {
  if (!GetIsGameDraw()) {
    gpRender->SetRequestRGBA6(true);
  }
  if (mOnScreenTexAlpha > 0.f && mOnScreenTexTok->GetObject() != nullptr) {
    const CTexture& tex = *mOnScreenTexTok->GetObject();
    gpRender->SetDepthReadWrite(false, false);
    gpRender->SetBlendMode_AlphaBlended();
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    const int h = mOnScreenTex.mExtent[1];
    const int w = mOnScreenTex.mExtent[0];
    const CViewport& viewport = CGraphics::GetViewport();
    const int x = viewport.mLeft + (viewport.mWidth - w) / 2 + mOnScreenTex.mOffset.GetX();
    const int y = viewport.mTop + (viewport.mHeight - h) / 2 - mOnScreenTex.mOffset.GetY();
#if defined(TARGET_PC)
    metaforce::RenderUiTexture(tex, x, y, w, h, CColor::White().WithAlphaOf(mOnScreenTexAlpha));
#else
    CGraphics::Render2D(tex, x, y, w, h, CColor::White().WithAlphaOf(mOnScreenTexAlpha));
#endif
  }

  float staticAlpha = 0.f;
  const float deathTime = mgr.GetPlayer()->GetDeathTime();
  if (mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed && deathTime > 0.f) {
    staticAlpha = CMath::Clamp(0.f, deathTime / (0.3f * gkFirstPersonDeathTime), 1.f);
  }
  const bool notInCine = !mgr.GetCameraManager()->IsInCinematicCamera();
  const bool drawVisor = notInCine && IsInOrTransitioningToOrFromState(kIGGS_InGame);
  if (mPauseScreenBlur->IsGameDraw()) {
    mSamusHud->GetTargetingManager().Draw(mgr, true);
    CGraphics::SetDepthRange(1.f / 64.f, 1.f / 32.f);
    const bool scanVisor = mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Scan;
    if (drawVisor && mEnablePlayerVisor != 0) {
      if (mgr.GetPlayer()->GetCameraState() == CPlayer::kCS_FirstPerson) {
        mFaceplateDecor.Draw(mgr);
      }
      const bool targetingEnabled = mEnableTargetingManager != 0;
      const CTargetingManager* targeting =
          scanVisor && targetingEnabled ? &mSamusHud->GetTargetingManager() : nullptr;
      mPlayerVisor->Draw(mgr, targeting);
    }
    mSamusReflection->Draw(mgr);
    if (drawVisor) {
      const bool hudVis = mHudVisMode != CTweakGui::kHud_Zero;
      const bool targeting = mEnableTargetingManager != 0;
      CGraphics::SetDepthRange(1.f / 512.f, 1.f / 64.f);
      if (staticAlpha > 0.f) {
        CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend,
                                      CCameraFilterPass::kFS_RandomStatic,
                                      CColor::White().WithAlphaOf(staticAlpha), nullptr, 1.f);
      }
      mSamusHud->Draw(mgr, mVisorStaticAlpha * (1.f - staticAlpha), mHelmetVisMode,
                         hudVis, targeting && !scanVisor);
    }
  }

  const bool preDrawBlur =
      IsInGameplayStateNotTransitioning() || IsInOrTransitioningToOrFromState(kIGGS_MapScreen);
  if (preDrawBlur) {
    mPauseScreenBlur->Draw(mgr);
  }
  if (notInCine && mEnableAutoMapper != 0 &&
      (mPauseScreenBlur->IsGameDraw() || IsInOrTransitioningToOrFromState(kIGGS_MapScreen))) {
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
    CGuiCamera* camera = mSamusHud->GetBaseHudFrame()->GetFrameCamera();
    camera->Draw(CGuiWidgetDrawParms(0.f, CVector3f::Zero()));
    CGraphics::SetDepthRange(0.f, 1.f / 512.f);
    mModel_automapper->SetIsVisible(true);
    mModel_automapper->Draw(CGuiWidgetDrawParms(1.f, CVector3f::Zero()));
    CGraphics::SetDepthWriteMode(true, kE_GEqual, false);
    mAutoMapper->Draw(mgr, CTransform4f::Translate(0.f, 0.02f, 0.f) * mMapCamXf,
                         mapAlpha * (mVisorStaticAlpha * t));
    CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
    mModel_automapper->SetIsVisible(false);
  }
  if (!preDrawBlur) {
    mPauseScreenBlur->Draw(mgr);
  }
  if (mHelmetVisMode != 0u && notInCine) {
    const float cameraOffset = mPauseScreen.null() ? 0.f : mPauseScreen->GetHelmetCamYOff();
    mSamusHud->DrawHelmet(mgr, cameraOffset);
  }
  if (!mSaveUI.null()) {
    mSaveUI->Draw();
  }
  if (!mMessageScreen.null()) {
    mMessageScreen->Draw();
  }
  if (!mPauseScreen.null()) {
    mPauseScreen->Draw();
  }
  mCamFilter.Draw();

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
      mDeathDot.GetObject()->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
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
  if (!mPauseScreen.null()) {
    mPauseScreen->PreDraw();
  }
  if (isCameraActive) {
    mSamusReflection->PreDraw(mgr);
  }
}

void CInGameGuiManager::Update(const CStateManager& mgr, float dt, CArchitectureQueue& queue,
                               bool cameraActive) {
  EnsureStates(mgr);
  if (mOnScreenTexAlpha == 0.f) {
    mOnScreenTexTok = nullptr;
  }
  const SOnScreenTex& pending = mgr.GetPendingScreenTex();
  if (pending.mId != mOnScreenTex.mId) {
    if (mOnScreenTexTok.null()) {
      mOnScreenTex.mId = pending.mId;
      mOnScreenTex.mExtent = pending.mExtent;
      mOnScreenTex.mOffset = pending.mOffset;
      if (mOnScreenTex.mId != kInvalidAssetId) {
        mOnScreenTexTok = rs_new TCachedToken< CTexture >(
            gpSimplePool->GetObj(SObjectTag('TXTR', mOnScreenTex.mId)));
        mOnScreenTexTok->Lock();
        mOnScreenTexAlpha = FLT_EPSILON;
      }
    } else {
      if (pending.mId == kInvalidAssetId && pending.mExtent == CVector2i(0, 0)) {
        mOnScreenTex.mExtent = pending.mExtent;
        mOnScreenTex.mId = kInvalidAssetId;
        mOnScreenTexAlpha = 0.f;
      } else {
        mOnScreenTexAlpha = rstl::max_val(0.f, mOnScreenTexAlpha - dt);
      }
    }
  } else if (mOnScreenTex.mId != kInvalidAssetId && !mOnScreenTexTok.null() &&
             mOnScreenTexTok->TryCache()) {
    mOnScreenTexAlpha = rstl::min_val(1.f, mOnScreenTexAlpha + dt);
  }

  if (cameraActive) {
    const float visorStaticAlpha = mgr.GetPlayer()->GetVisorStaticAlpha();
    if (visorStaticAlpha != mVisorStaticAlpha) {
      if (TCastToConstPtr< CFirstPersonCamera >(mgr.GetCameraManager()->GetCurrentCamera(mgr))) {
        if (CMath::AbsF(visorStaticAlpha - mVisorStaticAlpha) < 0.5f) {
          if (mVisorStaticAlpha == 0.f) {
            CSfxManager::SfxStart(SFXui_x_hudon_00, 127, 64, false, CSfxManager::kMedPriority,
                                  false, CSfxManager::kAllAreas);
          } else if (mVisorStaticAlpha == 1.f) {
            CSfxManager::SfxStart(SFXui_x_hudoff_00, 127, 64, false, CSfxManager::kMedPriority,
                                  false, CSfxManager::kAllAreas);
          }
        }
      }
    }
    mVisorStaticAlpha = visorStaticAlpha;
  }
  if (cameraActive) {
    mFaceplateDecor.Update(dt, mgr);
  }
  if (cameraActive) {
    mSamusReflection->Update(dt, mgr, mRand);
  }
  if (mEnablePlayerVisor != 0 && cameraActive) {
    mPlayerVisor->Update(dt, mgr);
  }
  if (cameraActive && mPlayerAlive) {
    mSamusHud->Update(dt, mgr, mHelmetVisMode, mHudVisMode != CTweakGui::kHud_Zero,
                         mEnableTargetingManager != 0);
  }
  if (mEnableAutoMapper != 0) {
    UpdateAutoMapper(mgr, dt);
  }
  mPauseScreenBlur->Update(dt, mgr, mDumpedTextures.empty());

  if (!mSaveUI.null()) {
    const CIOWin::EMessageReturn ret = mSaveUI->Update(dt);
    if (ret != CIOWin::kMR_Normal) {
      mInSaveUI = ret == CIOWin::kMR_Exit;
      BeginStateTransition(kIGGS_InGame, mgr);
    }
  } else if (!mMessageScreen.null()) {
    const float blurAmt = mPauseScreenBlur->GetBlurAmt();
    if (!mMessageScreen->Update(dt, blurAmt)) {
      BeginStateTransition(kIGGS_InGame, mgr);
    }
  }
  if (!mPauseScreen.null()) {
    mPauseScreen->Update(dt, mgr, mRand, queue);
    if (!IsStateTransitioning()) {
      if (mPauseScreen->ShouldSwitchToMapScreen()) {
        BeginStateTransition(kIGGS_MapScreen, mgr);
      } else if (mPauseScreen->ShouldSwitchToInGame()) {
        BeginStateTransition(kIGGS_InGame, mgr);
      }
    }
  }
  mSamusHud->Touch();
  mPlayerVisor->Touch();
  mSamusHud->GetTargetingManager().Touch();

  if (IsStateTransitioning()) {
    if (InGameGuiStates::IsGameplayState(mNextState)) {
      TryReloadAreaTextures();
    }
    const CAutoMapper& mapper = *mAutoMapper;
    const bool mapperIsTransitioning = mapper.GetCurrentState() != mapper.GetNextState();
    if ((!mapperIsTransitioning || mEnableAutoMapper == 0) &&
        mPauseScreenBlur->IsNotTransitioning()) {
      TryCompleteStateTransition(queue);
    }
  }
  mCamFilter.Update(dt);
  if (mgr.GetCameraManager()->IsInCinematicCamera()) {
    mgr.SetViewportScaleX(1.f);
    mgr.SetViewportScaleY(1.f);
  } else {
    const float scaleX = rstl::min_val(mSamusHud->GetDesiredViewportScaleX(),
                                       mPlayerVisor->GetDesiredViewportScaleX(mgr));
    const float scaleY = rstl::min_val(mSamusHud->GetDesiredViewportScaleY(),
                                       mPlayerVisor->GetDesiredViewportScaleY(mgr));
    mgr.SetViewportScaleX(scaleX);
    mgr.SetViewportScaleY(scaleY);
  }
  mPlayerAlive = mgr.GetPlayerState()->IsAlive();
}

void CInGameGuiManager::UpdateAutoMapper(const CStateManager& mgr, float dt) {
  mAutoMapper->Update(dt, mgr);
  const CTransform4f xf = mModel_automapper->Parent()->GetWorldTransform() *
                          mBasewidget_automapper->GetTransform();
  mAutomapperRotate = CQuaternion::FromMatrix(xf);
  mAutomapperOffset = xf.GetTranslation();

  const CTransform4f& cameraXf = mBasehud_camera->GetWorldTransform();
  mCamRotate = CQuaternion::FromMatrix(cameraXf);
  mCamOffset = cameraXf.GetTranslation() + skMapScreenCameraOffset * cameraXf.GetForward() +
                   gpTweakAutoMapper->mCamVerticalOffset * cameraXf.GetUp();

  const float frameLength =
      CMath::SlowTangentR(0.5f * CMath::Deg2Rad(mBasehud_camera->GetParms().perspective.fov)) /
      0.7f;
  const float scaleX = frameLength * gpTweakAutoMapper->mAutomapperScaleX;
  const float scaleZ = frameLength * gpTweakAutoMapper->mAutomapperScaleZ;
  if (mAutoMapper->IsFullyOutOfMiniMapState()) {
    mModel_automapper->SetO2WTransform(
        CTransform4f(mCamRotate.BuildTransform(), mCamOffset) *
        CTransform4f::Scale(scaleX, 1.f, scaleZ));
    mMapCamXf = CTransform4f(mCamRotate.BuildTransform(), mCamOffset) *
                    CTransform4f::Scale(frameLength, 1.f, frameLength);
    mModel_automapper->SetColor(gpTweakAutoMapper->mAutomapperWidgetColor);
  } else if (mAutoMapper->IsFullyInMiniMapState()) {
    mModel_automapper->SetO2WTransform(
        CTransform4f(mAutomapperRotate.BuildTransform(), mAutomapperOffset));
    mMapCamXf = mModel_automapper->GetWorldTransform();
    mModel_automapper->SetColor(gpTweakAutoMapper->mAutomapperWidgetMiniColor);
  } else {
    float t;
    if (mAutoMapper->GetNextState() != CAutoMapper::kAMS_MiniMap) {
      t = mAutoMapper->GetInterp();
    } else {
      t = 1.f - mAutoMapper->GetInterp();
    }
    const CQuaternion rotate = CQuaternion::Slerp(mAutomapperRotate, mCamRotate, t);
    const CVector3f offset = CVector3f::Lerp(mAutomapperOffset, mCamOffset, t);
    const float st = t * (frameLength - 1.f) + 1.f;
    mMapCamXf =
        CTransform4f(rotate.BuildTransform(), offset) * CTransform4f::Scale(st, 1.f, st);
    mModel_automapper->SetO2WTransform(
        CTransform4f(rotate.BuildTransform(), offset) *
        CTransform4f::Scale(t * (scaleX - 1.f) + 1.f, 1.f, t * (scaleZ - 1.f) + 1.f));
    const CColor color = CColor::Lerp(gpTweakAutoMapper->mAutomapperWidgetMiniColor,
                                      gpTweakAutoMapper->mAutomapperWidgetColor, t);
    mModel_automapper->SetColor(color);
  }
}

void CInGameGuiManager::TryCompleteStateTransition(CArchitectureQueue& queue) {
  if (mNextState != kIGGS_PauseGame && mNextState != kIGGS_PauseLogBook) {
    if (!mPauseScreen.null() && mPauseScreen->IsTransitioning()) {
      return;
    }
    mPauseScreen = nullptr;
  }
  if (InGameGuiStates::IsGameplayState(mNextState)) {
    mMessageScreen = nullptr;
    if (!TryReloadAreaTextures()) {
      return;
    }
    CModel::EnableTextureTimeout();
    RefreshHudOptions();
  }
  mPrevState = mNextState;
}

void CInGameGuiManager::RefreshHudOptions() { mSamusHud->RefreshHudOptions(); }

void CInGameGuiManager::ProcessControllerInput(const CStateManager& mgr, const CFinalInput& input,
                                               CArchitectureQueue& queue) {
  if (input.ControllerNumber() == 0) {
    if (!IsInGameplayStateNotTransitioning()) {
      if (IsInPausedStateNotTransitioning()) {
        if (mPrevState == kIGGS_MapScreen) {
          if (mAutoMapper->IsInMapperState(CAutoMapper::kAMS_MapScreen) ||
              mAutoMapper->IsInMapperState(CAutoMapper::kAMS_MapScreenUniverse)) {
            mAutoMapper->ProcessControllerInput(input, mgr);
            if (mAutoMapper->CanLeaveMapScreen(mgr)) {
              BeginStateTransition(kIGGS_InGame, mgr);
            }
          }
          return;
        }
        if (mPrevState == kIGGS_PauseSaveGame) {
          mSaveUI->ProcessUserInput(input);
          return;
        }
        if (mPrevState == kIGGS_PauseHUDMessage) {
          mMessageScreen->ProcessControllerInput(input);
          return;
        }
        if (!mPauseScreen.null()) {
          mPauseScreen->ProcessControllerInput(mgr, input);
        }
      }
    } else {
      mSamusHud->ProcessControllerInput(input);
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
  mPauseGameHudMessage = message;
  mPauseGameHudTime = time;
  PauseGame(mgr, kIGGS_PauseHUDMessage);
}

bool CInGameGuiManager::GetIsGameDraw() const { return mPauseScreenBlur->IsGameDraw(); }

void CInGameGuiManager::BeginStateTransition(EInGameGuiState state, const CStateManager& mgr) {
  if (mNextState == state) {
    return;
  }

  mPrevState = mNextState;
  mNextState = state;

  if (state == kIGGS_InGame) {
    CSfxManager::SetChannel(CSfxManager::kSC_Game);
    mSaveUI = nullptr;
    mAutoMapper->UnmuteAllLoopedSounds();
  } else if (state == kIGGS_PauseHUDMessage) {
    mMessageScreen = rs_new CMessageScreen(mPauseGameHudMessage, mPauseGameHudTime);
  } else if (state == kIGGS_PauseSaveGame) {
    mSaveUI = rs_new CSaveGameScreen(kSC_InGame, gpGameState->GetCardSerial());
  } else if (InGameGuiStates::IsGameplayState(mPrevState)) {
    mDeferTransition = true;
  }
  mPauseScreenBlur->OnNewInGameGuiState(state, mgr);
  if (!mDeferTransition) {
    DoStateTransition(mgr);
  }
}

void CInGameGuiManager::DoStateTransition(const CStateManager& mgr) {
  mSamusHud->OnNewInGameGuiState(mNextState, mgr);
  mAutoMapper->OnNewInGameGuiState(mNextState, mgr);
  if ((mNextState == kIGGS_PauseGame || mNextState == kIGGS_PauseLogBook) &&
      mPauseScreen.null()) {
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
    const CPauseScreen::ESubScreen screen = mNextState == kIGGS_PauseLogBook
                                                ? CPauseScreen::kSS_LogBook
                                                : CPauseScreen::kSS_Inventory;
    mPauseScreen = rs_new CPauseScreen(screen, **mPauseScreenDGRPs[suitResIdx],
                                          **mPauseScreenDGRPs[suitResIdx]);
  }

  const bool needsLock = InGameGuiStates::IsPausedState(mNextState);
  for (AUTO(it, mPauseResources.begin()); it != mPauseResources.end(); ++it) {
    if (needsLock) {
      it->Lock();
    } else {
      it->Unlock();
    }
  }
}

void CInGameGuiManager::EnsureStates(const CStateManager& mgr) {
  if (mDeferTransition && !mPauseScreenBlur->IsGameDraw()) {
    DestroyAreaTextures(mgr);
    mDeferTransition = false;
    DoStateTransition(mgr);
  }
}

bool CInGameGuiManager::IsTextureInPauseScreen(CAssetId id) const {
  for (int i = 0; i < 13; ++i) {
    TToken< CDependencyGroup > token = mPauseScreenDGRPs[i];
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
  for (AUTO(it, mInGameTextureIDs.begin()); it != mInGameTextureIDs.end(); ++it) {
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
          mDumpedTextures.push_back(TDumpedTexture(id, token));
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
    TToken< CTexture >& token = it->mToken;
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
      texture.UnloadBitmapData(it->mId);
      mDumpedTextures.push_back(TDumpedTexture(it->mId, token));
    }
    memoryFreed += texture.GetMemoryAllocated();
  }

  CTexture::sCurrentFrameCount = 0;
  mDumpedTextures.sort(
      rstl::pair_sorter_finder< TDumpedTexture, rstl::less< CAssetId > >(rstl::less< CAssetId >()));
#endif
  CModel::DisableTextureTimeout();
}

const bool CInGameGuiManager::TryReloadAreaTextures() {
  bool complete = true;
  AUTO(it, mDumpedTextures.begin());
  while (it != mDumpedTextures.end()) {
    if (it->second->TryReloadBitmapData(*gpResourceFactory)) {
      it = mDumpedTextures.erase(it);
    } else {
      complete = false;
      ++it;
    }
  }
  return complete;
}
