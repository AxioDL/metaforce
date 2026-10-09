#include "MetroidPrime/Player/CWorldTransManager.hpp"

#include "GuiSys/CGuiTextSupport.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/Audio/CStreamAudioManager.hpp"
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

#if defined(TARGET_PC)
#include "Metaforce/Display.hpp"
#include "dolphin/gx/GXAurora.h"
#include "dolphin/gx/GXExtra.h"
#endif

#include "rstl/list.hpp"
#include "rstl/StringExtras.hpp"

struct CWorldTransManager::SModelDatas {
  CAnimRes mSamusRes;
  CModelData mSamusModelData;
  CModelData mBeamModelData;
  CModelData mPlatformModelData;
  CModelData mBgModelData;
  rstl::optional_object< CToken > mBeamModel;
  rstl::optional_object< CToken > mSuitModel;
  rstl::optional_object< CToken > mSuitSkin;
  CTransform4f mGunXf;
  rstl::vector< CLight > mLights;
  rstl::single_ptr< uchar > mDissolveTextureBuffer;
  CVector2f mShakeResult;
  CVector2f mShakeDelta;
  float mRandTimeout;
  float mBlurResult;
  float mBlurDelta;
  float mDissolveStartTime;
  float mDissolveEndTime;
  float mTransCompleteTime;
  bool mDissolveStarted;

  explicit SModelDatas(const CAnimRes& samusRes);
#if defined(TARGET_PC)
  ~SModelDatas();
#endif
};
NESTED_CHECK_SIZEOF(CWorldTransManager, SModelDatas, 0x1e0)

CWorldTransManager::CWorldTransManager() : mCurTime(0.f)
, mModelData(nullptr)
, mTextData(nullptr)
#if VERSION >= VERSION_GM8P_00
, mSecondaryTextData(nullptr)
#endif
, mRandom(99)
, mSfx(1189)
, mVolume(127)
, mPanning(64)
, mTransType(kTT_Disabled)
, mTextStartTime(0.f)
, mTransitionFinished(true)
, mStopSoon(false)
, mGoingUp(false)
, mFadeWhite(false)
, mTextDirty(false) {}

CWorldTransManager::~CWorldTransManager() {}

CWorldTransManager::SModelDatas::SModelDatas(const CAnimRes& samusRes) : mSamusRes(samusRes)
, mSamusModelData(CModelData::CModelDataNull())
, mBeamModelData(CModelData::CModelDataNull())
, mPlatformModelData(CModelData::CModelDataNull())
, mBgModelData(CModelData::CModelDataNull())
, mGunXf(CTransform4f::Identity())
#if VERSION == VERSION_GM8P_00
, mDissolveTextureBuffer(rs_new uchar[CGraphics::GetSpareBufferSize() * 2])
#else
, mDissolveTextureBuffer(rs_new uchar[0x8c000])
#endif
, mShakeResult(0.f, 0.f)
, mShakeDelta(0.f, 0.f)
, mRandTimeout(0.f)
, mBlurResult(0.f)
, mBlurDelta(0.f)
, mDissolveStartTime(99999.f)
, mDissolveEndTime(99999.f)
, mTransCompleteTime(99999.f)
, mDissolveStarted(false) {
  mLights.reserve(8);
}

#if defined(TARGET_PC)
CWorldTransManager::SModelDatas::~SModelDatas() {
  if (uchar* buffer = mDissolveTextureBuffer.release()) {
    GXDestroyCopyTex(buffer);
    CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame, buffer);
  }
}
#endif

void CWorldTransManager::DisableTransition() {
  mTransType = kTT_Disabled;
  mModelData = nullptr;
  mTextData = nullptr;
#if VERSION >= VERSION_GM8P_00
  mSecondaryTextData = nullptr;
#endif
  mGoingUp = false;
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
  SModelDatas* data = mModelData.get();
  if (data == nullptr)
    return;

  if (data->mBeamModel && data->mBeamModel->IsLoaded()) {
    data->mBeamModelData = CModelData(
        CStaticRes(data->mBeamModel->GetTag().GetId(), data->mSamusRes.GetScale()));
    data->mBeamModel = rstl::optional_object< CToken >();
  }
  if (data->mSuitModel && data->mSuitSkin && data->mSuitModel->IsLoaded() &&
      data->mSuitSkin->IsLoaded()) {
    const int suit = GetSuitCharIdx();
    CAnimRes samusRes(data->mSamusRes.GetId(), suit,
                      data->mSamusRes.GetScale(), data->mSamusRes.GetDefaultAnim(), true);
    CModelData samusModel(samusRes);
    data->mSamusModelData = samusModel;
    data->mSamusModelData.AnimationData()->SetAnimation(
        CAnimPlaybackParms(samusRes.GetDefaultAnim(), -1, 1.f, true), false);
    data->mSuitModel = rstl::optional_object< CToken >();
    data->mSuitSkin = rstl::optional_object< CToken >();
  }
  if (!data->mSamusModelData.IsNull())
    data->mSamusModelData.Touch(CModelData::kWM_Normal, 0);
  if (!data->mPlatformModelData.IsNull())
    data->mPlatformModelData.Touch(CModelData::kWM_Normal, 0);
  if (!data->mBgModelData.IsNull())
    data->mBgModelData.Touch(CModelData::kWM_Normal, 0);
  if (!data->mBeamModelData.IsNull())
    data->mBeamModelData.Touch(CModelData::kWM_Normal, 0);
}

void CWorldTransManager::EnableTransition(const CAnimRes& samusRes, CAssetId platformRes,
                                         const CVector3f& platformScale, CAssetId bgRes,
                                         const CVector3f& bgScale, bool goingUp) {
  mStopSoon = false;
  mTransType = kTT_Enabled;
  mGoingUp = goingUp;
  mModelData = rs_new SModelDatas(samusRes);
  mTextData = nullptr;
#if VERSION >= VERSION_GM8P_00
  mSecondaryTextData = nullptr;
#endif
  mRandom.SetSeed(99);
  mModelData->mSamusModelData = CModelData(samusRes);
  mModelData->mSamusModelData.AnimationData()->SetAnimation(
      CAnimPlaybackParms(samusRes.GetDefaultAnim(), -1, 1.f, true), false);

  CAssetId beamRes =
      gpTweakPlayerRes->GetCinematicBeamResId(gpGameState->GetPlayerState()->GetCurrentBeam());
  mModelData->mBeamModel = gpSimplePool->GetObj(SObjectTag('CMDL', beamRes));
  mModelData->mBeamModel->Lock();
  {
    TLockedToken< CCharacterFactory > factory = gpCharacterFactoryBuilder->GetFactory(samusRes);
    const CCharacterInfo& info = factory->GetCharInfo(GetSuitCharIdx());
    mModelData->mSuitModel = gpSimplePool->GetObj(SObjectTag('CMDL', info.GetModelId()));
    mModelData->mSuitModel->Lock();
    mModelData->mSuitSkin = gpSimplePool->GetObj(SObjectTag('CSKR', info.GetSkinRulesId()));
    mModelData->mSuitSkin->Lock();
  }
  if (platformRes != kInvalidAssetId) {
    mModelData->mPlatformModelData = CModelData(CStaticRes(platformRes, platformScale));
    mModelData->mPlatformModelData.Touch(CModelData::kWM_Normal, 0);
  }
  if (bgRes != kInvalidAssetId) {
    mModelData->mBgModelData = CModelData(CStaticRes(bgRes, bgScale));
    mModelData->mBgModelData.Touch(CModelData::kWM_Normal, 0);
    const CAABox bounds = mModelData->mBgModelData.GetBounds();
    const float height = bounds.GetMaxPoint().GetZ() - bounds.GetMinPoint().GetZ();
    mBgHeight = height * bgScale.GetZ();
  } else {
    mBgHeight = 0.f;
  }
  StartTransition();
  TouchModels();
}

void CWorldTransManager::StartTransition() {
  mCurTime = 0.f;
  mBgOffset = 0.f;
  mTransitionFinished = false;
  mTextDirty = true;
}

void CWorldTransManager::EndTransition() { DisableTransition(); }

void CWorldTransManager::Update(float dt) {
  mCurTime += dt;
  switch (mTransType) {
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
  if (mCurTime > 2.f)
    mTransitionFinished = true;
}

void CWorldTransManager::UpdateEnabled(const float dt) {
  if (!mModelData.null() && !mModelData->mSamusModelData.IsNull()) {
    if (mStopSoon && !mModelData->mDissolveStarted && mCurTime >= 2.f) {
      mModelData->mDissolveStarted = true;
      mModelData->mDissolveStartTime = mCurTime;
      mModelData->mDissolveEndTime = 4.f + mCurTime - 2.f;
      mModelData->mTransCompleteTime = 5.f + mCurTime - 2.f;
    }
    if (mCurTime > mModelData->mTransCompleteTime && mModelData->mDissolveStarted)
      mTransitionFinished = true;

    static const char* const kGunLocator = "GUN_LCTR";
    mModelData->mSamusModelData.AdvanceAnimationIgnoreParticles(dt, mRandom, true);
    mModelData->mGunXf =
        mModelData->mSamusModelData.GetScaledLocatorTransform(rstl::string_l(kGunLocator));
    mModelData->mRandTimeout -= dt;
    if (mModelData->mRandTimeout <= 0.f) {
      mModelData->mRandTimeout = mRandom.Range(0.016666668f, 0.1f);
      CVector2f randVec(mRandom.Range(-0.025f, 0.025f), mRandom.Range(-0.075f, 0.075f));
      mModelData->mShakeDelta =
          (randVec - mModelData->mShakeResult) / mModelData->mRandTimeout;
      const float blur = mRandom.Range(-2.f, 4.f);
      mModelData->mBlurDelta =
          (blur - mModelData->mBlurResult) / mModelData->mRandTimeout;
    }
    mModelData->mShakeResult += mModelData->mShakeDelta * dt;
    mModelData->mBlurResult += dt * mModelData->mBlurDelta;
  }

  float delta = 50.f * dt;
  if (mGoingUp)
    delta = -delta;
  mBgOffset += delta;
  if (mBgOffset > mBgHeight)
    mBgOffset -= mBgHeight;
  if (mBgOffset < 0.f)
    mBgOffset += mBgHeight;
  UpdateLights(dt);
}

void CWorldTransManager::Draw() const {
  switch (mTransType) {
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
  if (mModelData.null())
    return;

  rstl::vector< CLight >& lights = mModelData->mLights;
  lights.clear();
  const CVector3f lightPos(0.f, 10.f, 0.f);
  CLight spot = CLight::BuildSpot(lightPos, CVector3f::Back(), CColor::White(), 90.f);
  spot.SetAttenuation(1.f, 0.f, 0.f);
  CLight movingSpot = spot;
  movingSpot.SetPosition(lightPos + CVector3f(0.f, 0.f, 2.f * mBgOffset - mBgHeight));
  float intensity = 1.f;
  if (!mGoingUp && mBgHeight - mBgOffset < 2.f)
    intensity = (mBgHeight - mBgOffset) / 2.f;
  else if (mGoingUp && mBgOffset < 2.f)
    intensity = mBgOffset / 2.f;

  if (intensity < 1.f) {
    CLight nextSpot = spot;
    nextSpot.SetPosition(lightPos + CVector3f(0.f, 0.f, mGoingUp ? mBgHeight : -mBgHeight));
    nextSpot.SetColor(CColor::Lerp(CColor::Black(), spot.GetColor(), 1.f - intensity));
    lights.push_back(nextSpot);
    movingSpot.SetColor(CColor::Lerp(CColor::Black(), movingSpot.GetColor(), intensity));
  }
  lights.push_back(movingSpot);
}
void CWorldTransManager::DrawAllModels() const {
  SModelDatas& data = *mModelData.get();
  CActorLights lights(0, CVector3f::Zero(), 4, 4);
  lights.BuildFakeLightList(data.mLights, CColor(0.1f, 0.1f, 0.1f, 1.f));
  if (!data.mBgModelData.IsNull()) {
    data.mBgModelData.Render(
        CModelData::kWM_Normal, CTransform4f::Translate(0.f, 0.f, -(2.f * mBgHeight - mBgOffset)),
        &lights, CModelFlags::Normal());
    data.mBgModelData.Render(
        CModelData::kWM_Normal, CTransform4f::Translate(0.f, 0.f, mBgOffset - mBgHeight),
        &lights, CModelFlags::Normal());
    data.mBgModelData.Render(CModelData::kWM_Normal, CTransform4f::Translate(0.f, 0.f, mBgOffset),
                                 &lights, CModelFlags::Normal());
  }
  if (!data.mPlatformModelData.IsNull())
    data.mPlatformModelData.Render(CModelData::kWM_Normal, CTransform4f::Identity(),
                                       &lights, CModelFlags::Normal());
  if (!data.mSamusModelData.IsNull()) {
    const CTransform4f& samusXf = CTransform4f::Identity();
    data.mSamusModelData.AnimationData()->PreRender();
    data.mSamusModelData.Render(CModelData::kWM_Normal, samusXf, &lights, CModelFlags::Normal());
    if (!data.mBeamModelData.IsNull())
      data.mBeamModelData.Render(CModelData::kWM_Normal, samusXf * data.mGunXf,
                                     &lights, CModelFlags::Normal());
  }
}
void CWorldTransManager::DrawFirstPass() const {
  const float rotationT = CMath::Clamp(0.f, mCurTime / 25.f, 100.f);
  const float translationT = CMath::Clamp(0.f, mCurTime / 10.f, 1.f);
  const CRelAngle angle = CRelAngle::FromDegrees(360.f * rotationT + 180.f - 90.f);
  const float cameraX = mModelData->mShakeResult.GetX();
  const CTransform4f viewXf =
      CTransform4f::RotateZ(angle) *
      CTransform4f::Translate(cameraX,
                              -3.5f * (1.f - translationT) + -3.5f,
                              2.f + mModelData->mShakeResult.GetY());
  CGraphics::SetViewPointMatrix(viewXf);
  DrawAllModels();
  if (mModelData->mBlurResult > 0.f) {
    const CGraphics::CProjectionState projection = CGraphics::GetProjectionState();
    CCameraBlurPass blurPass;
    blurPass.SetBlur(CCameraBlurPass::kBT_LoBlur, mModelData->mBlurResult, 0.f, false);
    blurPass.Draw();
    CGraphics::SetProjectionState(projection);
  }
}

void CWorldTransManager::DrawSecondPass() const {
  const float t = CMath::Clamp(0.f, (2.f + (mCurTime - mModelData->mDissolveStartTime)) / 5.f, 1.f);
  const CRelAngle angle = CRelAngle::FromDegrees(48.f * t + 180.f - 24.f);
  const CVector3f& scale = mModelData->mSamusRes.GetScale();
  const CTransform4f viewXf =
      CTransform4f::RotateZ(angle) *
      CTransform4f::Translate(CVector3f(-0.1f * scale.GetX(), -0.5f * scale.GetY(), 1.5f * scale.GetZ()));
  CGraphics::SetViewPointMatrix(viewXf);
  DrawAllModels();
}
void CWorldTransManager::DrawEnabled() const {
  if (mModelData.null())
    return;

  const float fov = CCameraManager::GetDefaultFirstPersonVerticalFOV();
  const float nearPlane = CCameraManager::GetDefaultFirstPersonNearClipDistance();
  const float farPlane = CCameraManager::GetDefaultFirstPersonFarClipDistance();
#if defined(TARGET_PC)
  gpRender->SetPerspective(
      fov,
      metaforce::AdjustDisplayAspect(CCast::LtoF(CGraphics::GetViewportWidth()) /
                                     CCast::LtoF(CGraphics::GetViewportHeight())),
      nearPlane, farPlane);
#else
  gpRender->SetPerspective(
      fov, CCast::LtoF(CGraphics::GetViewportWidth()) / CCast::LtoF(CGraphics::GetViewportHeight()),
      nearPlane, farPlane);
#endif
  gpRender->SetRequestRGBA6(true);
  const float drawTime = mCurTime;
  if (drawTime <= mModelData->mDissolveStartTime) {
    DrawFirstPass();
  } else if (drawTime >= mModelData->mDissolveEndTime) {
    DrawSecondPass();
  } else {
    float alpha = 1.f - CMath::Clamp(0.f, (drawTime - mModelData->mDissolveStartTime) / 2.f, 1.f);
    alpha *= alpha;
    int left, top, width, height;
    CGraphics::GetViewport(left, top, width, height);
    uchar* const buffer = mModelData->mDissolveTextureBuffer.get();
#if defined(TARGET_PC)
    GXCreateFrameBuffer(width, height);
    gpRender->SetViewport(0, CGraphics::GetRenderMode().efbHeight - height, width, height);
#endif
    DrawFirstPass();
    CGX::SetZMode(true, GX_LEQUAL, true);
    GXSetTexCopyDst(width, height, GX_TF_RGB565, false);
#if defined(TARGET_PC)
    GXSetTexCopySrc(0, 0, width, height);
#else
    GXSetTexCopySrc(left, top, width, height);
#endif
    GXCopyTex(buffer, true);
#if defined(TARGET_PC)
    GXRestoreFrameBuffer();
    gpRender->SetViewport(left, CGraphics::GetRenderMode().efbHeight - top - height, width, height);
#endif
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
  const float fadeTime = mCurTime;
  float filterAlpha = 0.f;
  if (fadeTime < 0.25f)
    filterAlpha = 1.f - fadeTime / 0.25f;
  else if (fadeTime > mModelData->mTransCompleteTime)
    filterAlpha = 1.f;
  else if (fadeTime > mModelData->mTransCompleteTime - 0.25f)
    filterAlpha = 1.f - (mModelData->mTransCompleteTime - fadeTime) / 0.25f;
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
  if (!mSfxHandle && mSfx != CSfxManager::kInternalInvalidSfxId)
    mSfxHandle = CSfxManager::SfxStart(mSfx, mVolume, mPanning, false,
                                        CSfxManager::kMedPriority, true);
}

void CWorldTransManager::SfxStop() {
  if (mSfxHandle) {
    CSfxManager::SfxStop(mSfxHandle);
    mSfxHandle.Clear();
  }
}

void CWorldTransManager::SetSfx(ushort sfx, uchar volume, uchar panning) {
  mSfx = sfx;
  mVolume = volume;
  mPanning = panning;
}

static CVector2i GetViewportSize() {
#if VERSION >= VERSION_GM8J_00
  return CVector2i(640, 448);
#else
  return CVector2i(CGraphics::GetViewportWidth(), CGraphics::GetViewportHeight());
#endif
}

void CWorldTransManager::EnableTransition(int fontId, int stringId, int stringIdx, const bool fadeWhite,
                                        float chFadeTime, float chFadeRate, float textStartTime) {
  mStrIdx = stringIdx;
  mTextStartTime = textStartTime;
  mStopSoon = false;
  mTransType = kTT_Text;
  mModelData = nullptr;
  mFadeWhite = fadeWhite;
  mTextData = rs_new CGuiTextSupport(
      fontId, CGuiTextProperties(false, true, kJustification_Center, kVerticalJustification_Center),
      CColor::White(), CColor::Black(), CColor::White(), 640, 448, gpSimplePool);
  mTextData->SetTypeWriteEffectOptions(true, chFadeTime, chFadeRate);
  mStrTable = TToken< CStringTable >(gpSimplePool->GetObj(SObjectTag('STRG', stringId)));
  mStrTable->Lock();
  mTextData->SetText(rstl::wstring_l(L""));
  StartTransition();
}

#if VERSION >= VERSION_GM8P_00
void CWorldTransManager::EnableTransition(
    int fontId, int stringId, int stringIdx, const bool fadeWhite, const rstl::string& audioFile,
    int volume, bool showSecondaryText, float chFadeTime, float chFadeRate, float textStartTime,
    float textEndDelay, float secondaryTextStartTime, float secondaryTextFadeDuration) {
  mAudioFile = audioFile;
  mStrIdx = stringIdx;
  mShowSecondaryText = showSecondaryText;
  mTextStartTime = textStartTime;
  mTextEndDelay = textEndDelay;
  mSecondaryTextStartTime = secondaryTextStartTime;
  mSecondaryTextFadeDuration = rstl::max_val(0.0001f, secondaryTextFadeDuration);
  mVolume = volume;
  mStopSoon = false;
  mTransType = kTT_Text;
  mModelData = nullptr;
  mFadeWhite = fadeWhite;

  const CVector2i viewport = GetViewportSize();
  mTextData = rs_new CGuiTextSupport(
      fontId, viewport.GetX(), viewport.GetY(),
      CGuiTextProperties(true, true, kJustification_Center, kVerticalJustification_Center),
      CColor::White(), CColor::Black(), CColor::White(), gpSimplePool);
  mTextData->SetTypeWriteEffectOptions(true, chFadeTime, chFadeRate);
  mTextData->SetText(rstl::wstring_l(L""));
  if (mShowSecondaryText) {
    mSecondaryTextData = rs_new CGuiTextSupport(
        fontId, viewport.GetX(), 120,
        CGuiTextProperties(true, true, kJustification_Center, kVerticalJustification_Center),
        CColor::White(), CColor::Black(), CColor::White(), gpSimplePool);
    mSecondaryTextData->SetText(rstl::wstring_l(L""));
    mSecondaryTextData->SetGeometryColor(CColor::Black());
  }
  mStrTable = TToken< CStringTable >(gpSimplePool->GetObj(SObjectTag('STRG', stringId)));
  mStrTable->Lock();
  StartTransition();
}
#endif

void CWorldTransManager::UpdateText(float dt) {
  if (mTextDirty) {
    TToken< CStringTable > strTable = *mStrTable;
    if (strTable.IsLoaded()) {
      if (mStrIdx < strTable->GetStringCount()) {
        const rstl::wstring text = strTable->GetString(mStrIdx);
        mTextData->SetText(text);
#if VERSION >= VERSION_GM8P_00
        if (mShowSecondaryText) {
          mSecondaryTextData->SetText(strTable->GetString(mStrIdx + 1));
        }
#endif
      }
#if VERSION >= VERSION_GM8P_00
      const int audioFileComparison =
          CStringExtras::CompareCaseInsensitive(mAudioFile, rstl::string_l("UseStringTable"));
      if (audioFileComparison == 0 && mStrIdx + 1 < strTable->GetStringCount()) {
        const rstl::wstring audioFile = strTable->GetString(mStrIdx + 1);
        mAudioFile = CStringExtras::ConvertToANSI(audioFile);
      }
#endif
      mSfxInterval = 0.f;
      mTextDirty = false;
    } else if (mCurTime >= mTextStartTime) {
      mTextStartTime += dt;
    }
  }
  if (mCurTime >= mTextStartTime) {
#if VERSION >= VERSION_GM8P_00
    if (mAudioFile.length() != 0 && CDvdFile::FileExists(mAudioFile.c_str())) {
      CStreamAudioManager::PlaySoftwareAudio(CStreamAudioManager::kSC_Default, mAudioFile, 0.f, 0.f,
                                            mVolume, false);
      mAudioFile = rstl::string_l("");
    }
#endif
    mTextData->Update(dt);
#if VERSION >= VERSION_GM8P_00
    if (mShowSecondaryText) {
      const float alpha = rstl::min_val(
          1.f, rstl::max_val(0.f, mCurTime - mTextStartTime - mSecondaryTextStartTime) /
                   mSecondaryTextFadeDuration);
      mSecondaryTextData->SetGeometryColor(
          CColor::White().WithAlphaModulatedBy(0.75f * (alpha * alpha)));
      mSecondaryTextData->Update(dt);
    }
#endif
    const float printed = mTextData->GetNumCharactersPrinted();
    const float charsPerSfx = gpTweakGui->GetWorldTransManagerCharsPerSfx();
    if (printed >= mSfxInterval + charsPerSfx) {
      mSfxInterval += charsPerSfx;
      CSfxManager::SfxStart(0x59e, 127, 64);
    }
  }
  if (mStopSoon) {
#if VERSION >= VERSION_GM8P_00
    bool advanceStopTimer = false;
    if (mTextEndDelay + (1.f + mTextData->GetTotalAnimationTime()) < mTextData->GetCurTime()) {
      advanceStopTimer = true;
      if (mCurTime - mStopTime > 1.f)
        mTransitionFinished = true;
    }
    if (!advanceStopTimer) {
      mStopTime = mCurTime;
    }
#else
    if (1.f + mTextData->GetTotalAnimationTime() < mTextData->GetCurTime()) {
      if (mCurTime - mStopTime > 1.f)
        mTransitionFinished = true;
    } else {
      mStopTime = mCurTime;
    }
#endif
  }
}

void CWorldTransManager::DrawText() const {
  gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
#if defined(TARGET_PC)
  metaforce::AdjustUiProjection();
#endif
#if VERSION >= VERSION_GM8P_00
  gpRender->SetModelMatrix(CTransform4f::Translate(0.f, 0.f, GetViewportSize().GetY()));
#else
  gpRender->SetModelMatrix(CTransform4f::Translate(0.f, 0.f, 448.f));
#endif
  CGraphics::SetCullMode(kCM_None);
  gpRender->SetDepthReadWrite(false, false);
  gpRender->SetBlendMode_AdditiveAlpha();
  mTextData->Render();
#if VERSION >= VERSION_GM8P_00
  if (mShowSecondaryText) {
#if VERSION >= VERSION_GM8J_00
    const CVector2i viewport = GetViewportSize();
    const float scale = 0.8f;
    CTransform4f xf = CTransform4f::Scale(scale);
    xf = CTransform4f::Translate(0.5f * (1.f - scale) * viewport.GetX(), 0.f, 120.f) * xf;
#else
    CTransform4f xf = CTransform4f::Scale(1.f);
    xf = CTransform4f::Translate(0.f, 0.f, 120.f) * xf;
#endif
    gpRender->SetModelMatrix(xf);
    mSecondaryTextData->Render();
  }
#endif

  float filterAlpha = 0.f;
  if (mCurTime < 1.f)
    filterAlpha = 1.f - rstl::min_val(1.f, mCurTime);
  else if (mStopSoon)
    filterAlpha = rstl::min_val(1.f, mCurTime - mStopTime);
  if (filterAlpha > 0.f) {
    const CColor filterColor =
        (mFadeWhite ? CColor::White() : CColor::Black()).WithAlphaOf(filterAlpha);
    CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen,
                                  filterColor, nullptr, 1.f);
  }
  CGraphics::SetIsBeginSceneClearFb(true);
}

void CWorldTransManager::StartTextFadeOut() {
  if (!mStopSoon)
    mStopTime = mCurTime;
  mStopSoon = true;
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
