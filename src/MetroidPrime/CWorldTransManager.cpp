#include "MetroidPrime/Player/CWorldTransManager.hpp"

#include "GuiSys/CGuiTextSupport.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/Cameras/CCameraBlurPass.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Factories/CCharacterFactory.hpp"
#include "MetroidPrime/Factories/CCharacterFactoryBuilder.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerRes.hpp"

#include "dolphin/gx/GXFrameBuffer.h"
#include "dolphin/gx/GXManage.h"
#include "dolphin/gx/GXTev.h"
#include "dolphin/gx/GXVert.h"
#include "dolphin/os.h"

#include "rstl/list.hpp"

struct CWorldTransManager::SModelDatas {
  CAnimRes x0_samusRes;
  CModelData x1c_samusModelData;
  CModelData x68_beamModelData;
  CModelData xb4_platformModelData;
  CModelData x100_bgModelData;
  rstl::optional_object< CToken > x14c_beamModel;
  rstl::optional_object< CToken > x158_suitModel;
  rstl::optional_object< CToken > x164_suitSkin;
  CTransform4f x170_gunXf;
  rstl::vector< CLight > x1a0_lights;
  rstl::single_ptr< uchar > x1b0_dissolveTextureBuffer;
  CVector2f x1b4_shakeResult;
  CVector2f x1bc_shakeDelta;
  float x1c4_randTimeout;
  float x1c8_blurResult;
  float x1cc_blurDelta;
  float x1d0_dissolveStartTime;
  float x1d4_dissolveEndTime;
  float x1d8_transCompleteTime;
  bool x1dc_dissolveStarted;

  explicit SModelDatas(const CAnimRes& samusRes);
};
NESTED_CHECK_SIZEOF(CWorldTransManager, SModelDatas, 0x1e0)

CWorldTransManager::CWorldTransManager() : x0_curTime(0.f)
, x4_modelData(nullptr)
, x8_textData(nullptr)
, x20_random(99)
, x24_sfx(1189)
, x2c_volume(127)
, x2d_panning(64)
, x30_transType(kTT_Disabled)
, x38_textStartTime(0.f)
, x44_24_transitionFinished(true)
, x44_25_stopSoon(false)
, x44_26_goingUp(false)
, x44_27_fadeWhite(false)
, x44_28_textDirty(false) {}

CWorldTransManager::~CWorldTransManager() {}

CWorldTransManager::SModelDatas::SModelDatas(const CAnimRes& samusRes) : x0_samusRes(samusRes)
, x1c_samusModelData(CModelData::CModelDataNull())
, x68_beamModelData(CModelData::CModelDataNull())
, xb4_platformModelData(CModelData::CModelDataNull())
, x100_bgModelData(CModelData::CModelDataNull())
, x170_gunXf(CTransform4f::Identity())
, x1b0_dissolveTextureBuffer(rs_new uchar[0x8c000])
, x1b4_shakeResult(0.f, 0.f)
, x1bc_shakeDelta(0.f, 0.f)
, x1c4_randTimeout(0.f)
, x1c8_blurResult(0.f)
, x1cc_blurDelta(0.f)
, x1d0_dissolveStartTime(99999.f)
, x1d4_dissolveEndTime(99999.f)
, x1d8_transCompleteTime(99999.f)
, x1dc_dissolveStarted(false) {
  x1a0_lights.reserve(8);
}

void CWorldTransManager::DisableTransition() {
  x30_transType = kTT_Disabled;
  x4_modelData = nullptr;
  x8_textData = nullptr;
  x44_26_goingUp = false;
}

int CWorldTransManager::GetSuitCharIdx() {
  const CPlayerState& state = *gpGameState->GetPlayerState();
  int suit = state.GetCurrentSuitRaw();
  if (state.GetIsFusionEnabled()) {
    switch (suit) {
    case CPlayerState::kPS_Power:
      suit = 4;
      break;
    case CPlayerState::kPS_Varia:
      suit = 7;
      break;
    case CPlayerState::kPS_Gravity:
      suit = 6;
      break;
    case CPlayerState::kPS_Phazon:
      suit = 8;
      break;
    }
  }
  return suit;
}

void CWorldTransManager::TouchModels() {
  SModelDatas* data = x4_modelData.get();
  if (data == nullptr)
    return;

  if (data->x14c_beamModel && data->x14c_beamModel->IsLoaded()) {
    data->x68_beamModelData = CModelData(
        CStaticRes(data->x14c_beamModel->GetTag().GetId(), data->x0_samusRes.GetScale()));
    data->x14c_beamModel = rstl::optional_object< CToken >();
  }
  if (data->x158_suitModel && data->x164_suitSkin && data->x158_suitModel->IsLoaded() &&
      data->x164_suitSkin->IsLoaded()) {
    const int suit = GetSuitCharIdx();
    CAnimRes samusRes(data->x0_samusRes.GetId(), suit,
                      data->x0_samusRes.GetScale(), data->x0_samusRes.GetDefaultAnim(), true);
    CModelData samusModel(samusRes);
    data->x1c_samusModelData = samusModel;
    data->x1c_samusModelData.AnimationData()->SetAnimation(
        CAnimPlaybackParms(samusRes.GetDefaultAnim(), -1, 1.f, true), false);
    data->x158_suitModel = rstl::optional_object< CToken >();
    data->x164_suitSkin = rstl::optional_object< CToken >();
  }
  if (!data->x1c_samusModelData.IsNull())
    data->x1c_samusModelData.Touch(CModelData::kWM_Normal, 0);
  if (!data->xb4_platformModelData.IsNull())
    data->xb4_platformModelData.Touch(CModelData::kWM_Normal, 0);
  if (!data->x100_bgModelData.IsNull())
    data->x100_bgModelData.Touch(CModelData::kWM_Normal, 0);
  if (!data->x68_beamModelData.IsNull())
    data->x68_beamModelData.Touch(CModelData::kWM_Normal, 0);
}

void CWorldTransManager::EnableTransition(const CAnimRes& samusRes, CAssetId platformRes,
                                         const CVector3f& platformScale, CAssetId bgRes,
                                         const CVector3f& bgScale, bool goingUp) {
  x44_25_stopSoon = false;
  x30_transType = kTT_Enabled;
  x44_26_goingUp = goingUp;
  x4_modelData = rs_new SModelDatas(samusRes);
  x8_textData = nullptr;
  x20_random.SetSeed(99);
  x4_modelData->x1c_samusModelData = CModelData(samusRes);
  x4_modelData->x1c_samusModelData.AnimationData()->SetAnimation(
      CAnimPlaybackParms(samusRes.GetDefaultAnim(), -1, 1.f, true), false);

  CAssetId beamRes =
      gpTweakPlayerRes->GetCinematicBeamResId(gpGameState->GetPlayerState()->GetCurrentBeam());
  x4_modelData->x14c_beamModel = gpSimplePool->GetObj(SObjectTag('CMDL', beamRes));
  x4_modelData->x14c_beamModel->Lock();
  {
    TLockedToken< CCharacterFactory > factory = gpCharacterFactoryBuilder->GetFactory(samusRes);
    const CCharacterInfo& info = factory->GetCharInfo(GetSuitCharIdx());
    x4_modelData->x158_suitModel = gpSimplePool->GetObj(SObjectTag('CMDL', info.GetModelId()));
    x4_modelData->x158_suitModel->Lock();
    x4_modelData->x164_suitSkin = gpSimplePool->GetObj(SObjectTag('CSKR', info.GetSkinRulesId()));
    x4_modelData->x164_suitSkin->Lock();
  }
  if (platformRes != kInvalidAssetId) {
    x4_modelData->xb4_platformModelData = CModelData(CStaticRes(platformRes, platformScale));
    x4_modelData->xb4_platformModelData.Touch(CModelData::kWM_Normal, 0);
  }
  if (bgRes != kInvalidAssetId) {
    x4_modelData->x100_bgModelData = CModelData(CStaticRes(bgRes, bgScale));
    x4_modelData->x100_bgModelData.Touch(CModelData::kWM_Normal, 0);
    const CAABox bounds = x4_modelData->x100_bgModelData.GetBounds();
    const float height = bounds.GetMaxPoint().GetZ() - bounds.GetMinPoint().GetZ();
    x1c_bgHeight = height * bgScale.GetZ();
  } else {
    x1c_bgHeight = 0.f;
  }
  StartTransition();
  TouchModels();
}

void CWorldTransManager::StartTransition() {
  x0_curTime = 0.f;
  x18_bgOffset = 0.f;
  x44_24_transitionFinished = false;
  x44_28_textDirty = true;
}

void CWorldTransManager::EndTransition() { DisableTransition(); }

void CWorldTransManager::Update(float dt) {
  x0_curTime += dt;
  switch (x30_transType) {
  case kTT_Enabled:
    UpdateEnabled(dt);
    break;
  case kTT_Text:
    UpdateText(dt);
    break;
  case kTT_Disabled:
    UpdateDisabled(dt);
    break;
  }
}

void CWorldTransManager::UpdateDisabled(float) {
  if (x0_curTime > 2.f)
    x44_24_transitionFinished = true;
}

void CWorldTransManager::UpdateEnabled(const float dt) {
  if (!x4_modelData.null() && !x4_modelData->x1c_samusModelData.IsNull()) {
    if (x44_25_stopSoon && !x4_modelData->x1dc_dissolveStarted && x0_curTime >= 2.f) {
      x4_modelData->x1dc_dissolveStarted = true;
      x4_modelData->x1d0_dissolveStartTime = x0_curTime;
      x4_modelData->x1d4_dissolveEndTime = 4.f + x0_curTime - 2.f;
      x4_modelData->x1d8_transCompleteTime = 5.f + x0_curTime - 2.f;
    }
    if (x0_curTime > x4_modelData->x1d8_transCompleteTime && x4_modelData->x1dc_dissolveStarted)
      x44_24_transitionFinished = true;

    static const char* const kGunLocator = "GUN_LCTR";
    x4_modelData->x1c_samusModelData.AdvanceAnimationIgnoreParticles(dt, x20_random, true);
    x4_modelData->x170_gunXf =
        x4_modelData->x1c_samusModelData.GetScaledLocatorTransform(rstl::string_l(kGunLocator));
    x4_modelData->x1c4_randTimeout -= dt;
    if (x4_modelData->x1c4_randTimeout <= 0.f) {
      x4_modelData->x1c4_randTimeout = x20_random.Range(0.016666668f, 0.1f);
      CVector2f randVec(x20_random.Range(-0.025f, 0.025f), x20_random.Range(-0.075f, 0.075f));
      x4_modelData->x1bc_shakeDelta =
          (randVec - x4_modelData->x1b4_shakeResult) / x4_modelData->x1c4_randTimeout;
      const float blur = x20_random.Range(-2.f, 4.f);
      x4_modelData->x1cc_blurDelta =
          (blur - x4_modelData->x1c8_blurResult) / x4_modelData->x1c4_randTimeout;
    }
    x4_modelData->x1b4_shakeResult += x4_modelData->x1bc_shakeDelta * dt;
    x4_modelData->x1c8_blurResult += dt * x4_modelData->x1cc_blurDelta;
  }

  float delta = 50.f * dt;
  if (x44_26_goingUp)
    delta = -delta;
  x18_bgOffset += delta;
  if (x18_bgOffset > x1c_bgHeight)
    x18_bgOffset -= x1c_bgHeight;
  if (x18_bgOffset < 0.f)
    x18_bgOffset += x1c_bgHeight;
  UpdateLights(dt);
}

void CWorldTransManager::Draw() const {
  switch (x30_transType) {
  case kTT_Enabled:
    DrawEnabled();
    break;
  case kTT_Text:
    DrawText();
    break;
  case kTT_Disabled:
    DrawDisabled();
    break;
  }
}

void CWorldTransManager::UpdateLights(float) {
  if (x4_modelData.null())
    return;

  rstl::vector< CLight >& lights = x4_modelData->x1a0_lights;
  lights.clear();
  const CVector3f lightPos(0.f, 10.f, 0.f);
  CLight spot = CLight::BuildSpot(lightPos, CVector3f::Back(), CColor::White(), 90.f);
  spot.SetAttenuation(1.f, 0.f, 0.f);
  CLight movingSpot = spot;
  movingSpot.SetPosition(lightPos + CVector3f(0.f, 0.f, 2.f * x18_bgOffset - x1c_bgHeight));
  float intensity = 1.f;
  if (!x44_26_goingUp && x1c_bgHeight - x18_bgOffset < 2.f)
    intensity = (x1c_bgHeight - x18_bgOffset) / 2.f;
  else if (x44_26_goingUp && x18_bgOffset < 2.f)
    intensity = x18_bgOffset / 2.f;

  if (intensity < 1.f) {
    CLight nextSpot = spot;
    nextSpot.SetPosition(lightPos + CVector3f(0.f, 0.f, x44_26_goingUp ? x1c_bgHeight : -x1c_bgHeight));
    nextSpot.SetColor(CColor::Lerp(CColor::Black(), spot.GetColor(), 1.f - intensity));
    lights.push_back(nextSpot);
    movingSpot.SetColor(CColor::Lerp(CColor::Black(), movingSpot.GetColor(), intensity));
  }
  lights.push_back(movingSpot);
}
void CWorldTransManager::DrawAllModels() const {
  SModelDatas& data = *x4_modelData.get();
  CActorLights lights(0, CVector3f::Zero(), 4, 4);
  lights.BuildFakeLightList(data.x1a0_lights, CColor(0.1f, 0.1f, 0.1f, 1.f));
  if (!data.x100_bgModelData.IsNull()) {
    data.x100_bgModelData.Render(
        CModelData::kWM_Normal, CTransform4f::Translate(0.f, 0.f, -(2.f * x1c_bgHeight - x18_bgOffset)),
        &lights, CModelFlags::Normal());
    data.x100_bgModelData.Render(
        CModelData::kWM_Normal, CTransform4f::Translate(0.f, 0.f, x18_bgOffset - x1c_bgHeight),
        &lights, CModelFlags::Normal());
    data.x100_bgModelData.Render(CModelData::kWM_Normal, CTransform4f::Translate(0.f, 0.f, x18_bgOffset),
                                 &lights, CModelFlags::Normal());
  }
  if (!data.xb4_platformModelData.IsNull())
    data.xb4_platformModelData.Render(CModelData::kWM_Normal, CTransform4f::Identity(),
                                       &lights, CModelFlags::Normal());
  if (!data.x1c_samusModelData.IsNull()) {
    const CTransform4f& samusXf = CTransform4f::Identity();
    data.x1c_samusModelData.AnimationData()->PreRender();
    data.x1c_samusModelData.Render(CModelData::kWM_Normal, samusXf, &lights, CModelFlags::Normal());
    if (!data.x68_beamModelData.IsNull())
      data.x68_beamModelData.Render(CModelData::kWM_Normal, samusXf * data.x170_gunXf,
                                     &lights, CModelFlags::Normal());
  }
}
void CWorldTransManager::DrawFirstPass() const {
  const float rotationT = CMath::Clamp(0.f, x0_curTime / 25.f, 100.f);
  const float translationT = CMath::Clamp(0.f, x0_curTime / 10.f, 1.f);
  const CRelAngle angle = CRelAngle::FromDegrees(360.f * rotationT + 180.f - 90.f);
  const float cameraX = x4_modelData->x1b4_shakeResult.GetX();
  const CTransform4f viewXf =
      CTransform4f::RotateZ(angle) *
      CTransform4f::Translate(cameraX,
                              -3.5f * (1.f - translationT) + -3.5f,
                              2.f + x4_modelData->x1b4_shakeResult.GetY());
  CGraphics::SetViewPointMatrix(viewXf);
  DrawAllModels();
  if (x4_modelData->x1c8_blurResult > 0.f) {
    const CGraphics::CProjectionState projection = CGraphics::GetProjectionState();
    CCameraBlurPass blurPass;
    blurPass.SetBlur(CCameraBlurPass::kBT_LoBlur, x4_modelData->x1c8_blurResult, 0.f, false);
    blurPass.Draw();
    CGraphics::SetProjectionState(projection);
  }
}

void CWorldTransManager::DrawSecondPass() const {
  const float t = CMath::Clamp(0.f, (2.f + (x0_curTime - x4_modelData->x1d0_dissolveStartTime)) / 5.f, 1.f);
  const CRelAngle angle = CRelAngle::FromDegrees(48.f * t + 180.f - 24.f);
  const CVector3f& scale = x4_modelData->x0_samusRes.GetScale();
  const CTransform4f viewXf =
      CTransform4f::RotateZ(angle) *
      CTransform4f::Translate(CVector3f(-0.1f * scale.GetX(), -0.5f * scale.GetY(), 1.5f * scale.GetZ()));
  CGraphics::SetViewPointMatrix(viewXf);
  DrawAllModels();
}
void CWorldTransManager::DrawEnabled() const {
  if (x4_modelData.null())
    return;

  const float fov = CCameraManager::GetDefaultFirstPersonVerticalFOV();
  const float nearPlane = CCameraManager::GetDefaultFirstPersonNearClipDistance();
  const float farPlane = CCameraManager::GetDefaultFirstPersonFarClipDistance();
  gpRender->SetPerspective(fov,
                          CCast::LtoF(CGraphics::GetViewportWidth()) /
                              CCast::LtoF(CGraphics::GetViewportHeight()),
                          nearPlane, farPlane);
  gpRender->SetRequestRGBA6(true);
  const float drawTime = x0_curTime;
  if (drawTime <= x4_modelData->x1d0_dissolveStartTime) {
    DrawFirstPass();
  } else if (drawTime >= x4_modelData->x1d4_dissolveEndTime) {
    DrawSecondPass();
  } else {
    float alpha = 1.f - CMath::Clamp(0.f, (drawTime - x4_modelData->x1d0_dissolveStartTime) / 2.f, 1.f);
    alpha *= alpha;
    int left, top, width, height;
    CGraphics::GetViewport(left, top, width, height);
    uchar* const buffer = x4_modelData->x1b0_dissolveTextureBuffer.get();
    DrawFirstPass();
    CGX::SetZMode(true, GX_LEQUAL, true);
    GXSetTexCopyDst(width, height, GX_TF_RGB565, false);
    GXSetTexCopySrc(left, top, width, height);
    GXCopyTex(buffer, true);
    DrawSecondPass();
    CGraphics::SetOrtho(0.f, width, 0.f, height, -4096.f, 4096.f);
    CGraphics::SetViewPointMatrix(CTransform4f::Identity());
    gpRender->SetModelMatrix(CTransform4f::Identity());
    GXPixModeSync();
    CGraphics::LoadDolphinSpareTexture(width, height, GX_TF_RGB565, buffer, CGraphics::kSpareBufferTexMapID);
    static const GXVtxDescList vtxDescrs[] = {
        {GX_VA_POS, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT}, {GX_VA_NULL, GX_NONE}};
    CGX::SetVtxDescv(vtxDescrs);
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetTevColor(GX_TEVREG0, CColor(1.f, 1.f, 1.f, alpha).GetGXColor());
    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A0);
    CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
    CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
    CGX::SetNumTexGens(1);
    CGX::SetNumTevStages(1);
    CGX::SetNumChans(0);
    CGX::SetTevDirect(GX_TEVSTAGE0);
    CGX::SetNumIndStages(0);
    CGX::SetZMode(false, GX_ALWAYS, false);
    CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);
    GXPosition3f32(0.f, 0.5f, 0.f);
    GXTexCoord2f32(0.f, 0.f);
    GXPosition3f32(0.f, 0.5f, height);
    GXTexCoord2f32(0.f, 1.f);
    GXPosition3f32(width, 0.5f, height);
    GXTexCoord2f32(1.f, 1.f);
    GXPosition3f32(width, 0.5f, 0.f);
    GXTexCoord2f32(1.f, 0.f);
    CGX::End();
  }
  CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_CinemaBars,
                                CColor::Black(), nullptr, 1.f);
  const float fadeTime = x0_curTime;
  float filterAlpha = 0.f;
  if (fadeTime < 0.25f)
    filterAlpha = 1.f - fadeTime / 0.25f;
  else if (fadeTime > x4_modelData->x1d8_transCompleteTime)
    filterAlpha = 1.f;
  else if (fadeTime > x4_modelData->x1d8_transCompleteTime - 0.25f)
    filterAlpha = 1.f - (x4_modelData->x1d8_transCompleteTime - fadeTime) / 0.25f;
  if (filterAlpha > 0.f) {
    const CColor filterColor(0.f, 0.f, 0.f, filterAlpha);
    CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen,
                                  filterColor, nullptr, 1.f);
  }
  CGraphics::SetIsBeginSceneClearFb(true);
}
void CWorldTransManager::DrawDisabled() const {
  const CColor color = CColor(uchar(0), uchar(0), uchar(0), uchar(3));
  CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen,
                                color, nullptr, 1.f);
}

void CWorldTransManager::SfxStart() {
  if (!x28_sfxHandle && x24_sfx != CSfxManager::kInternalInvalidSfxId)
    x28_sfxHandle = CSfxManager::SfxStart(x24_sfx, x2c_volume, x2d_panning, false,
                                        CSfxManager::kMedPriority, true);
}

void CWorldTransManager::SfxStop() {
  if (x28_sfxHandle) {
    CSfxManager::SfxStop(x28_sfxHandle);
    x28_sfxHandle.Clear();
  }
}

void CWorldTransManager::SetSfx(ushort sfx, uchar volume, uchar panning) {
  x24_sfx = sfx;
  x2c_volume = volume;
  x2d_panning = panning;
}

void CWorldTransManager::EnableTransition(int fontId, int stringId, int stringIdx, const bool fadeWhite,
                                        float chFadeTime, float chFadeRate, float textStartTime) {
  x40_strIdx = stringIdx;
  x38_textStartTime = textStartTime;
  x44_25_stopSoon = false;
  x30_transType = kTT_Text;
  x4_modelData = nullptr;
  x44_27_fadeWhite = fadeWhite;
  x8_textData = rs_new CGuiTextSupport(
      fontId, CGuiTextProperties(false, true, kJustification_Center, kVerticalJustification_Center),
      CColor::White(), CColor::Black(), CColor::White(), 640, 448, gpSimplePool);
  x8_textData->SetTypeWriteEffectOptions(true, chFadeTime, chFadeRate);
  xc_strTable = TToken< CStringTable >(gpSimplePool->GetObj(SObjectTag('STRG', stringId)));
  xc_strTable->Lock();
  x8_textData->SetText(rstl::wstring_l(L""));
  StartTransition();
}

void CWorldTransManager::UpdateText(float dt) {
  if (x44_28_textDirty) {
    TToken< CStringTable > strTable = *xc_strTable;
    if (strTable.IsLoaded()) {
      if (x40_strIdx < strTable->GetStringCount())
        x8_textData->SetText(strTable->GetString(x40_strIdx));
      x3c_sfxInterval = 0.f;
      x44_28_textDirty = false;
    } else if (x0_curTime >= x38_textStartTime) {
      x38_textStartTime += dt;
    }
  }
  if (x0_curTime >= x38_textStartTime) {
    x8_textData->Update(dt);
    const float printed = x8_textData->GetNumCharactersPrinted();
    const float charsPerSfx = gpTweakGui->GetWorldTransManagerCharsPerSfx();
    if (printed >= x3c_sfxInterval + charsPerSfx) {
      x3c_sfxInterval += charsPerSfx;
      CSfxManager::SfxStart(0x59e, 127, 64);
    }
  }
  if (x44_25_stopSoon) {
    if (1.f + x8_textData->GetTotalAnimationTime() < x8_textData->GetCurTime()) {
      if (x0_curTime - x34_stopTime > 1.f)
        x44_24_transitionFinished = true;
    } else {
      x34_stopTime = x0_curTime;
    }
  }
}
void CWorldTransManager::DrawText() const {
  gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
  gpRender->SetModelMatrix(CTransform4f::Translate(0.f, 0.f, 448.f));
  CGraphics::SetCullMode(kCM_None);
  gpRender->SetDepthReadWrite(false, false);
  gpRender->SetBlendMode_AdditiveAlpha();
  x8_textData->Render();

  float filterAlpha = 0.f;
  if (x0_curTime < 1.f)
    filterAlpha = 1.f - rstl::min_val(1.f, x0_curTime);
  else if (x44_25_stopSoon)
    filterAlpha = rstl::min_val(1.f, x0_curTime - x34_stopTime);
  if (filterAlpha > 0.f) {
    const CColor filterColor =
        (x44_27_fadeWhite ? CColor::White() : CColor::Black()).WithAlphaOf(filterAlpha);
    CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen,
                                  filterColor, nullptr, 1.f);
  }
  CGraphics::SetIsBeginSceneClearFb(true);
}

void CWorldTransManager::StartTextFadeOut() {
  if (!x44_25_stopSoon)
    x34_stopTime = x0_curTime;
  x44_25_stopSoon = true;
}

bool CWorldTransManager::WaitForModelsAndTextures() {
#if defined(TARGET_PC)
  CFrameDelayedKiller::StallAndFlushAllAllocations();
#else
  rstl::vector< SObjectTag > tags = gpSimplePool->GetReferencedTags();
  CTexture::sCurrentFrameCount = 0x7fffffff;
  rstl::list< CARAMToken > modelData;
  CFrameDelayedKiller::StallAndFlushAllAllocations();
  for (int pass = 0; pass < 2; ++pass) {
    for (AUTO(it, tags.begin()); it != tags.end(); ++it) {
      if (gpSimplePool->GetObj(*it).IsLoaded()) {
        if (it->GetType() == 'TXTR') {
          TToken< CTexture > texture = gpSimplePool->GetObj(*it);
          if (pass == 0) {
            texture->MakeSwappable();
            texture->LoadToARAM();
            if (texture->IsARAMTransferInProgress()) {
              while (texture->IsARAMTransferInProgress())
                CARAMToken::UpdateAllDMAs();
            }
          } else {
            texture->LoadToMRAM();
          }
        } else if (it->GetType() == 'CMDL') {
          TToken< CModel > modelToken = gpSimplePool->GetObj(*it);
          CModel* model = *modelToken;
          if (pass == 0) {
            rstl::auto_ptr< uchar > data = model->GetData();
            const uint dataSize = OSRoundUp32B(model->GetDataSize());
            CARAMToken token(data.release(), dataSize, 1);
            token.LoadToARAM();
            token.ForceSyncARAM();
            modelData.push_back(token);
          } else {
            void* data = modelData.front().ForceSyncMRAM();
            modelData.pop_front();
            model->RemapData(static_cast< uchar* >(data));
          }
        }
      }
    }
  }
  CTexture::sCurrentFrameCount = 0;
#endif
  return true;
}
