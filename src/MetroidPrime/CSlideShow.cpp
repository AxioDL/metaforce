#include "MetroidPrime/CSlideShow.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/CArchMsgParmReal32.hpp"
#include "MetroidPrime/CArchMsgParmUserInput.hpp"
#include "MetroidPrime/CControlMapper.hpp"
#include "MetroidPrime/CMemoryCard.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Decode.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"

#include "GuiSys/CGuiTextSupport.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Audio/CStaticAudioPlayer.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CDependencyGroup.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerRes.hpp"
#include "MetroidPrime/Tweaks/CTweakSlideShow.hpp"

#if defined(TARGET_PC)
#include "Metaforce/Display.hpp"
#endif

static const char* const skGalleryName = "Gallery";
static const char* const skGalleryAssets = "GalleryAssets_DGRP";
static const char* const skGalleryTag = "TXTR_GalleryTag";
static const char* const skGalleryBorder = "CMDL_GalleryBorder";
static const char* const skAudioFile = "Audio/frontend_1.rsf";

static const char skImagePrefix[] = "&image=";
static const char skImageSuffix[] = ";";
static const char* const skUnlockMessages[] = {"STRG_SlideShow_Unlock1_",
                                               "STRG_SlideShow_Unlock2_"};

static CVector2f sZeroVector(0.f, 0.f);

static int GetStickDirection(float up, float down, float left, float right) {
  uint flags = 0;
  if (up > 0.f) {
    flags |= 1;
  }
  if (down > 0.f) {
    flags |= 2;
  }
  if (left > 0.f) {
    flags |= 4;
  }
  if (right > 0.f) {
    flags |= 8;
  }
  switch (flags) {
  case 1:
    return 1;
  case 5:
    return 2;
  case 4:
    return 3;
  case 6:
    return 4;
  case 2:
    return 5;
  case 10:
    return 6;
  case 8:
    return 7;
  case 9:
    return 8;
  default:
    return 0;
  }
}

static void DrawTexture(const rstl::auto_ptr< TToken< CTexture > >& token,
                        const CVector3f& position, const CColor& color,
                        const CVector2f* viewportOffset = nullptr,
                        const CVector2f* viewportSize = nullptr) {
  if (!token.null()) {
    const CTexture& texture = ***token;
    const float width = texture.GetWidth();
    const float height = texture.GetHeight();
    int left, top, vpWidth, vpHeight;
    CGraphics::GetViewport(left, top, vpWidth, vpHeight);
    if (viewportOffset != nullptr) {
      left = viewportOffset->GetX();
      top = viewportOffset->GetY();
    }
    if (viewportSize != nullptr) {
      vpWidth = viewportSize->GetX();
      vpHeight = viewportSize->GetY();
    }
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
    CGraphics::SetOrtho(left, left + vpWidth, top + vpHeight, top, -1.f, 1.f);
#if defined(TARGET_PC)
    metaforce::AdjustUiProjection();
#endif
    CGraphics::SetViewPointMatrix(CTransform4f::Identity());
    CGraphics::SetModelMatrix(CTransform4f::Translate(position));
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    texture.Load(GX_TEXMAP0, CTexture::kCM_Repeat);
    CGraphics::StreamBegin(kP_Quads);
    CGraphics::StreamColor(color);
    CGraphics::StreamTexcoord(0.f, 0.f);
    CGraphics::StreamVertex(CVector3f::Zero());
    CGraphics::StreamTexcoord(1.f, 0.f);
    CGraphics::StreamVertex(CVector3f(width, 0.f, 0.f));
    CGraphics::StreamTexcoord(1.f, 1.f);
    CGraphics::StreamVertex(CVector3f(width, 0.f, height));
    CGraphics::StreamTexcoord(0.f, 1.f);
    CGraphics::StreamVertex(CVector3f(0.f, 0.f, height));
    CGraphics::StreamEnd();
  }
}

CSlideShow::CSlideShow()
: CIOWin(rstl::string_l("SlideShow"))
, mPhase(0)
, mGalleryBorder(nullptr)
, x3c_(0)
, mTotalSlides(0)
, mGallery(0)
, mSlide(-1)
, mCrossfadeTimer(0.f)
, mRepeatTimer(0.f)
, mIdleTimer(0.f)
, mSlideNumberTimer(0.f)
, mControlsText(nullptr)
#if VERSION >= VERSION_GM8P_00
, mGalleryNameText(nullptr)
#endif
, mSlideNumberText(nullptr)
, mAudio(nullptr)
#if VERSION >= VERSION_GM8P_00
, mGalleryNames(gpSimplePool->GetObj(gpTweakSlideShow->GetGalleryNames().data()))
#endif
, mLStick(0)
, mCStick(0)
, mLTrigger(0)
, mRTrigger(0)
, mControlsOffset(32.f)
, mSlideNumberOffset(32.f)
, mFadeTimer(gpTweakSlideShow->GetFadeTime())
, mShowControls(true)
, mControlsFadeIn(false)
, mControlsFadeOut(false)
, mDisableInput(false)
, mExit(false)
, mIntroFade(true)
, mOutroFade(false)
, mGalleryChanged(true) {
  mSlideA.mParent = this;
  mSlideB.mParent = this;
  gpResourceFactory->GetResLoader().AddPakFileAsync(gpTweakSlideShow->GetPakName(), false, false);
  const int width = CGraphics::GetViewportWidth();
  const int height = CGraphics::GetViewportHeight();
  const CColor& fontColor = gpTweakSlideShow->GetFontColor();
  const CColor& outlineColor = gpTweakSlideShow->GetOutlineColor();
  const SObjectTag* font =
      gpResourceFactory->GetResourceIdByName(gpTweakSlideShow->GetFont().data());
#if VERSION >= VERSION_GM8P_00
  mControlsText = rs_new CGuiTextSupport(
      font->GetId(), width, height,
      CGuiTextProperties(false, true, kJustification_Center, kVerticalJustification_Bottom),
      fontColor, outlineColor, CColor::White(), gpSimplePool);
  mGalleryNameText = rs_new CGuiTextSupport(
      font->GetId(), width, height,
      CGuiTextProperties(false, true, kJustification_Left, kVerticalJustification_Bottom),
      fontColor, outlineColor, CColor::White(), gpSimplePool);
  mSlideNumberText = rs_new CGuiTextSupport(
      font->GetId(), width, height,
      CGuiTextProperties(false, true, kJustification_Right, kVerticalJustification_Bottom),
      fontColor, outlineColor, CColor::White(), gpSimplePool);
#else
  mControlsText = rs_new CGuiTextSupport(
      font->GetId(),
      CGuiTextProperties(false, true, kJustification_Center, kVerticalJustification_Bottom),
      fontColor, outlineColor, CColor::White(), width, height, gpSimplePool);
  mSlideNumberText = rs_new CGuiTextSupport(
      font->GetId(),
      CGuiTextProperties(false, true, kJustification_Right, kVerticalJustification_Bottom),
      fontColor, outlineColor, CColor::White(), width, height, gpSimplePool);
#endif
  const rstl::reserved_vector< CAssetId, 9 >* sticks[] = {&gpTweakPlayerRes->mLStick,
                                                          &gpTweakPlayerRes->mCStick};
  mStickTextures.reserve(18);
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 9; ++j) {
      mStickTextures.push_back(gpSimplePool->GetObj(SObjectTag('TXTR', (*sticks[i])[j])));
    }
  }
  SetTexturesLocked(mStickTextures, true);
  const rstl::reserved_vector< CAssetId, 2 >* buttons[] = {
      &gpTweakPlayerRes->mLTrigger, &gpTweakPlayerRes->mRTrigger,
      &gpTweakPlayerRes->mBButton, &gpTweakPlayerRes->mYButton};
  mButtonTextures.reserve(8);
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 2; ++j) {
      mButtonTextures.push_back(gpSimplePool->GetObj(SObjectTag('TXTR', (*buttons[i])[j])));
    }
  }
  SetTexturesLocked(mButtonTextures, true);
#if VERSION >= VERSION_GM8P_00
  mGalleryNames.Lock();
#endif
}

CSlideShow::~CSlideShow() {
  gpResourceFactory->GetResLoader().RemovePakFile(gpTweakSlideShow->GetPakName());
}

uint CSlideShow::SlideShowGalleryFlags() {
  uint flags = 0;
  if (gpGameState != nullptr) {
    const int percent = gpGameState->SystemState().GetLogScanCount();
    if (percent >= 50) {
      flags |= 1;
    }
    if (percent == 100) {
      flags |= 2;
    }
    if (gpGameState->SystemState().GetHardModeBeat()) {
      flags |= 4;
    }
    if (gpGameState->SystemState().GetAllItemsCollected()) {
      flags |= 8;
    }
  }
  return flags;
}

void CSlideShow::BuildGalleryLists(uint flags) {
#if VERSION >= VERSION_GM8P_00
  const int count = mGalleryTXTRDeps.size() - 1;
  mGalleries.reserve(count);
  AUTO(it, mGalleryTXTRDeps.begin());
  AUTO(label, mGalleryLabels.begin());
  for (int i = 0; it != mGalleryTXTRDeps.end() && i < count; ++i) {
    if ((flags & (1 << i)) == 0) {
      it = mGalleryTXTRDeps.erase(it);
      label = mGalleryLabels.erase(label);
    } else {
      const int textureCount = it->GetT()->GetObjectTagVector().size();
      mGalleries.push_back(SGalleryData(i));
      SGalleryData& gallery = mGalleries.back();
      gallery.mTextures.reserve(textureCount);
      gallery.mSlides.reserve(textureCount);
      int slide = 0;
      int row = 0;
      int column = 0;
      int tiles = 0;
      int columns = 0;
      int missingRows = 0;
      while (slide < textureCount) {
        rstl::string name = CBasics::Stringize("%s_%02d_%03d", "slideshow", i, slide);
        const SObjectTag* tag = gpResourceFactory->GetResourceIdByName(name.data());
        if (tag != nullptr) {
          gallery.mTextures.push_back(tag);
          column = 1;
          ++tiles;
          columns = 1;
          ++missingRows;
          tag = nullptr;
        } else {
          name.append(CBasics::Stringize("_%02d%02d", column, row), -1);
          tag = gpResourceFactory->GetResourceIdByName(name.data());
          if (tag != nullptr) {
            gallery.mTextures.push_back(tag);
            ++column;
            ++tiles;
            columns = column;
            missingRows = 0;
          }
        }
        if (tag == nullptr) {
          if (missingRows == 1 && tiles > 0) {
            gallery.mSlides.push_back(rstl::pair< int, int >(gallery.mTextures.size(), columns));
            ++slide;
            row = 0;
            missingRows = 0;
            tiles = 0;
          } else {
            if (missingRows > 1) {
              break;
            }
            ++missingRows;
            ++row;
          }
          column = 0;
        }
      }
      ++it;
      ++label;
    }
  }
#else
  const int count = mGalleryTXTRDeps.size() - 1;
  mGalleries.reserve(count);
  AUTO(it, mGalleryTXTRDeps.begin());
  for (int i = 0; it != mGalleryTXTRDeps.end() && i < count; ++i) {
    if ((flags & (1 << i)) == 0) {
      it = mGalleryTXTRDeps.erase(it);
    } else {
      const int slides = it->GetT()->GetObjectTagVector().size();
      mGalleries.push_back(SGalleryData(i, rstl::vector< const SObjectTag* >()));
      SGalleryData& gallery = mGalleries.back();
      gallery.second.reserve(slides);
      for (int j = 0; j < slides; ++j) {
        const char* name = CBasics::Stringize("%s_%02d_%03d", "slideshow", i, j);
        const SObjectTag* tag = gpResourceFactory->GetResourceIdByName(name);
        gallery.second.push_back(tag);
      }
      ++it;
    }
  }
#endif
}

bool CSlideShow::LoadTXTRDep(const char* name) {
  const SObjectTag* tag = gpResourceFactory->GetResourceIdByName(name);
  if (tag != nullptr && tag->GetType() == 'DGRP') {
    if (mGalleryTXTRDeps.size() + 1 > mGalleryTXTRDeps.capacity()) {
      mGalleryTXTRDeps.reserve(mGalleryTXTRDeps.size() + 1);
    }
    mGalleryTXTRDeps.push_back(TToken< CDependencyGroup >(gpSimplePool->GetObj(*tag)));
  } else {
    return false;
  }
  return true;
}

CIOWin::EMessageReturn CSlideShow::OnMessage(const CArchitectureMessage& msg,
                                             CArchitectureQueue& queue) {
  switch (msg.GetType()) {
  case kAM_TimerTick: {
    if (mExit) {
      return kMR_RemoveIOWinAndExit;
    }
    const float dt = MakeMsg::GetParmTimerTick(msg).GetReal();
    switch (mPhase) {
    case 0:
      if (!gpResourceFactory->GetResLoader().AreAllPaksLoaded()) {
        gpResourceFactory->GetResLoader().AsyncIdlePakLoading();
        break;
      }
      mPhase = 1;
    case 1:
      if (mGalleryTXTRDeps.empty()) {
        mGalleryTXTRDeps.reserve(5);
        for (int i = 1;; ++i) {
          const rstl::string name = CBasics::Stringize("%s%02d_DGRP", skGalleryName, i);
          if (!LoadTXTRDep(name.data())) {
            break;
          }
        }
        LoadTXTRDep(skGalleryAssets);
        SetDependenciesLocked(mGalleryTXTRDeps, true);
      }
      if (!AreAllDepsLoaded(mGalleryTXTRDeps)) {
        break;
      }
#if VERSION >= VERSION_GM8P_00
      mPhase = 2;
    case 2: {
      if (mGalleryBorder.null()) {
        const SObjectTag* tag = gpResourceFactory->GetResourceIdByName(skGalleryBorder);
        mGalleryBorder = rs_new TToken< CModel >(gpSimplePool->GetObj(*tag));
        mGalleryBorder->Lock();
      }
      if (!mGalleryNames.IsLoaded() || !mGalleryBorder->IsLoaded()) {
        break;
      }
      const CStringTable& strings = **mGalleryNames;
      const int count = mGalleryTXTRDeps.size() - 1;
      mGalleryLabels.reserve(count);
      for (int i = 0; i < count; ++i) {
        mGalleryLabels.push_back(rstl::wstring(mGalleryNames->GetString(i)));
      }
      mPhase = 4;
    }
    case 3:
      BuildGalleryLists(SlideShowGalleryFlags());
      for (int i = 0; i < mGalleries.size(); ++i) {
        mTotalSlides += mGalleries[i].mSlides.size();
      }
      AdvanceSlide(true);
      mPhase = 4;
#else
      mPhase = 3;
    case 3:
      BuildGalleryLists(SlideShowGalleryFlags());
      for (int i = 0; i < mGalleries.size(); ++i) {
        mTotalSlides += mGalleries[i].second.size();
      }
      AdvanceSlide(true);
      mPhase = 2;
    case 2:
      if (mGalleryTags.size() != mGalleries.size()) {
        mGalleryTags.reserve(mGalleries.size());
        for (int i = 0; i < mGalleries.size(); ++i) {
          const char* name = CBasics::Stringize("%s%02d", skGalleryTag, mGalleries[i].first + 1);
          const SObjectTag* tag = gpResourceFactory->GetResourceIdByName(name);
          if (tag == nullptr) {
            mGalleryTags.push_back(rstl::auto_ptr< TToken< CTexture > >());
          } else {
            mGalleryTags.push_back(rs_new TToken< CTexture >(gpSimplePool->GetObj(*tag)));
            mGalleryTags.back()->Lock();
          }
        }
        if (mGalleryBorder.null()) {
          const SObjectTag* tag = gpResourceFactory->GetResourceIdByName(skGalleryBorder);
          mGalleryBorder = rs_new TToken< CModel >(gpSimplePool->GetObj(*tag));
          mGalleryBorder->Lock();
        }
      }
      if (!mGalleryBorder->IsLoaded()) {
        break;
      }
      for (int i = 0; i < mGalleryTags.size(); ++i) {
        if (!mGalleryTags[i].null() && !mGalleryTags[i]->IsLoaded()) {
          break;
        }
      }
      mPhase = 4;
#endif
    case 4:
      if (mAudio.null()) {
        mAudio = rs_new CStaticAudioPlayer(rstl::string_l(skAudioFile), 0x65af0, 0x1e1db0);
      }
      if (!mAudio->IsReady()) {
        break;
      }
      UpdateMusicVolume(gpTweakSlideShow->GetMusicFadeTime());
      mAudio->StartMixOut();
      mPhase = 5;
    case 5: {
      if (mIntroFade || mOutroFade) {
        mFadeTimer = rstl::max_val(0.f, mFadeTimer - dt);
        if (mFadeTimer <= 0.f) {
          if (mDisableInput) {
            mExit = true;
          } else {
            mIntroFade = false;
            mOutroFade = false;
            mFadeTimer = gpTweakSlideShow->GetMusicFadeTime();
          }
        }
      }
      mGalleryBorder->GetT()->Touch(0);
      LoadSlide();
      if (mSlideB.IsReady()) {
        const float crossfadeTime = gpTweakSlideShow->GetCrossfadeTime();
        if (mCrossfadeTimer > crossfadeTime) {
          mSlideA = mSlideB;
          SetPanSfx(false);
          SetZoomSfx(false);
          mSlideA.mMulColor.SetAlpha(1.f);
          mSlideB.Reset();
          mCrossfadeTimer = 0.f;
        } else {
          const float alpha = CMath::Clamp(0.f, mCrossfadeTimer / crossfadeTime, 1.f);
          mSlideA.mMulColor.SetAlpha(1.f - alpha);
          mSlideB.mMulColor.SetAlpha(alpha);
          mCrossfadeTimer += dt;
        }
      } else if (CMath::AbsF(mRepeatTimer) > gpTweakSlideShow->GetSlideRepeatTime()) {
        AdvanceSlide(mRepeatTimer > 0.f);
        mRepeatTimer = 0.f;
      }
      const float idleTime =
          IsControlsAnimating() ? 0.f : gpTweakSlideShow->GetSlideNumberIdleTime();
      mIdleTimer = CMath::Clamp(0.f, mIdleTimer + dt, idleTime);
      if (mIdleTimer >= gpTweakSlideShow->GetSlideNumberIdleTime()) {
        mSlideNumberTimer += dt;
      } else {
        mSlideNumberTimer -= dt;
      }
      mSlideNumberTimer =
          CMath::Clamp(0.f, mSlideNumberTimer, gpTweakSlideShow->GetSlideNumberFadeTime());
      UpdateControls(dt);
      UpdateSlideNumber(dt);
      if (mOutroFade) {
        UpdateMusicVolume(mFadeTimer);
      }
      break;
    }
    }
    break;
  }
  case kAM_UserInput: {
    const CArchMsgParmUserInput parm = MakeMsg::GetParmUserInput(msg);
    const CFinalInput input = parm.GetUserInput();
    if (input.ControllerNumber() == 0) {
      return ProcessUserInput(input);
    }
    break;
  }
  }
  return kMR_Exit;
}

void CSlideShow::Draw() const {
  if (mPhase == 5) {
    if (mSlideA.IsReady()) {
      mSlideA.Draw();
    }
    if (mSlideB.IsReady()) {
      mSlideB.Draw();
    }
    if (!mSlideNumberText.null() &&
        mSlideNumberTimer != gpTweakSlideShow->GetSlideNumberFadeTime()) {
      DrawSlideNumber();
    }
    if (IsControlsAnimating()) {
      DrawControls();
    }
    if (mIntroFade || mOutroFade) {
      float alpha = mFadeTimer / (mIntroFade ? gpTweakSlideShow->GetFadeTime()
                                                        : gpTweakSlideShow->GetMusicFadeTime());
      if (mOutroFade) {
        alpha = 1.f - alpha;
      }
      CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen,
                                    CColor::Black().WithAlphaOf(alpha), nullptr, 1.f);
    }
  }
}

bool IsDataLoreResearchScan(const CAssetId id) {
  const rstl::vector< CMemoryCard::ScanState >& states = gpMemoryCard->GetScanStates();
  AUTO(it, rstl::binary_find(
               gpMemoryCard->GetScanStates().begin(), gpMemoryCard->GetScanStates().end(), id,
               rstl::pair_sorter_finder< CMemoryCard::ScanState, rstl::less< CAssetId > >(
                   rstl::less< CAssetId >())));
  if (it != states.end()) {
    const uint category = it->second;
    bool counts;
    if (category == CWorldSaveGameInfo::kSC_Data || category == CWorldSaveGameInfo::kSC_Lore ||
        category == CWorldSaveGameInfo::kSC_Creature) {
      counts = true;
    } else {
      counts = false;
    }
    if (counts) {
      return true;
    }
  }
  return false;
}

rstl::pair< int, int > CStateManager::CalculateScanCompletionRate() const {
  const CPlayerState& player = *GetPlayerState();
  int total = 0;
  int complete = 0;
  int i = 0;
  const int count = player.GetScanTimes().size();
  for (; i < count; ++i) {
    const rstl::pair< CAssetId, float >& scan = player.GetScanTimes()[i];
    const uint category = gpMemoryCard->GetScanStates()[i].second;
    bool counts;
    if (category == CWorldSaveGameInfo::kSC_Data || category == CWorldSaveGameInfo::kSC_Lore ||
        category == CWorldSaveGameInfo::kSC_Creature) {
      counts = true;
    } else {
      counts = false;
    }
    if (counts) {
      ++total;
      if (scan.second == 1.f) {
        ++complete;
      }
    }
  }
  return rstl::pair< int, int >(complete, total);
}

CAssetId UpdatePersistentScanPercent(int previous, int current, int total) {
  if (previous != current) {
    const float interval = gpTweakSlideShow->GetScanPercentInterval();
    const float percent = 100.f * (float(current) / total);
    const int saved = gpGameState->SystemState().GetLogScanCount();
    const int scanPercent = int(percent);
    const int previousStep = int((100.f * (float(previous) / total)) / interval);
    const int step = int(percent / interval);
    const bool firstTime = scanPercent > saved;
    if (firstTime) {
      gpGameState->SystemState().SetLogScanCount(scanPercent);
    }
    if (step > previousStep) {
      const rstl::string name =
          rstl::string(skUnlockMessages[CMath::Clamp(0, step - 1, 1)]) + (firstTime ? "1" : "2");
      return gpResourceFactory->GetResourceIdByName(name.data())->GetId();
    }
  }
  return kInvalidAssetId;
}

CIOWin::EMessageReturn CSlideShow::ProcessUserInput(const CFinalInput& input) {
  if (!mDisableInput) {
    if (IsControlsAnimating()) {
      UpdateControlsText(input);
    }
    if (input.PB()) {
      SetPanSfx(false);
      SetZoomSfx(false);
      CSfxManager::SfxStart(0x446, 127, 64, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
      mDisableInput = true;
      mOutroFade = true;
      return kMR_Exit;
    }
    if (input.PY()) {
      SetShowControls(!mShowControls);
      if (mShowControls) {
        mGalleryChanged = true;
        CSfxManager::SfxStart(0x5b1, 127, 64, false, CSfxManager::kMedPriority, false,
                              CSfxManager::kAllAreas);
      } else {
        CSfxManager::SfxStart(0x5b0, 127, 64, false, CSfxManager::kMedPriority, false,
                              CSfxManager::kAllAreas);
      }
    }
    bool changed = false;
    if (input.PDPRight()) {
      mGallery = (mGallery + 1) % mGalleries.size();
      mSlide = -1;
      changed = true;
      mGalleryChanged = true;
      AdvanceSlide(true);
    } else if (input.PDPLeft()) {
      --mGallery;
      if (mGallery < 0) {
        mGallery = mGalleries.size() - 1;
      }
      mSlide = -1;
      changed = true;
      mGalleryChanged = true;
      AdvanceSlide(true);
    } else if (ControlMapper::GetPressInput(ControlMapper::kC_MapCircleRight, input) ||
               input.PA()) {
      changed = true;
      AdvanceSlide(true);
    } else if (ControlMapper::GetPressInput(ControlMapper::kC_MapCircleLeft, input)) {
      changed = true;
      AdvanceSlide(false);
    } else {
      float right = ControlMapper::GetAnalogInput(ControlMapper::kC_MapCircleRight, input);
      if (right || input.DA()) {
        mRepeatTimer = rstl::max_val(0.f, mRepeatTimer);
        mRepeatTimer += input.Time();
      } else {
        float left = ControlMapper::GetAnalogInput(ControlMapper::kC_MapCircleLeft, input);
        if (left) {
          mRepeatTimer = rstl::min_val(0.f, mRepeatTimer);
          mRepeatTimer -= input.Time();
        } else {
          mRepeatTimer = 0.f;
        }
      }
    }
    if (changed) {
      mRepeatTimer = 0.f;
      mIdleTimer = 0.f;
    }
    if (mSlideA.IsReady()) {
      return mSlideA.ProcessUserInput(input);
    }
  }
  return kMR_Exit;
}

CIOWin::EMessageReturn CSlideShow::AdvanceSlide(bool forward) {
  if (!mGalleries.empty()) {
    CSfxManager::SfxStart(0x445, 127, 64, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    if (forward) {
      ++mSlide;
    } else {
      --mSlide;
    }
    const int gallery = mGallery;
    if (mSlide < 0) {
      --mGallery;
      mGalleryChanged = true;
#if VERSION >= VERSION_GM8P_00
    } else if (mSlide >= mGalleries[mGallery].mSlides.size()) {
#else
    } else if (mSlide >= mGalleries[mGallery].second.size()) {
#endif
      ++mGallery;
      mGalleryChanged = true;
    }
    if (mGallery < 0) {
      mGallery = mGalleries.size() - 1;
#if VERSION >= VERSION_GM8P_00
      mSlide = mGalleries[mGallery].mSlides.size() - 1;
#else
      mSlide = mGalleries[mGallery].second.size() - 1;
#endif
    } else if (mGallery >= mGalleries.size()) {
      mSlide = 0;
      mGallery = 0;
    } else if (mGallery > gallery) {
      mSlide = 0;
    } else if (mGallery < gallery) {
#if VERSION >= VERSION_GM8P_00
      mSlide = mGalleries[mGallery].mSlides.size() - 1;
#else
      mSlide = mGalleries[mGallery].second.size() - 1;
#endif
    }
  }
  return kMR_Exit;
}

void CSlideShow::LoadSlide() {
#if VERSION >= VERSION_GM8P_00
  if (mSlideB.mTextures.empty() &&
      (mSlideA.mGallery != mGallery || mSlideA.mSlide != mSlide)) {
    const SGalleryData& gallery = mGalleries[mGallery];
    mSlideA.mStopLoading = true;
    const int first = mSlide == 0 ? 0 : gallery.mSlides[mSlide - 1].first;
    const int end = gallery.mSlides[mSlide].first;
    mSlideB.mTextures.reserve(end - first);
    for (int i = first; i < end; ++i) {
      const SObjectTag* tag = gallery.mTextures[i];
      if (tag != nullptr && gpResourceFactory->GetResourceTypeById(tag->GetId()) == 'TXTR') {
        mSlideB.mTextures.push_back(STexture());
        mSlideB.mTextures.back().mToken = rs_new TToken< CTexture >(gpSimplePool->GetObj(*tag));
      }
    }
    mSlideB.mGallery = mGallery;
    mSlideB.mSlide = mSlide;
    mSlideB.mColumns = gallery.mSlides[mSlide].second;
    mSlideB.InitializeViewport();
  }
  if (mSlideB.IsLoaded() && !mSlideB.mReady) {
    mSlideB.InitializeViewport();
  }
#else
  if (mSlideB.mTexture.null() &&
      (mSlideA.mGallery != mGallery || mSlideA.mSlide != mSlide)) {
    const SObjectTag* tag = mGalleries[mGallery].second[mSlide];
    if (tag != nullptr && gpResourceFactory->GetResourceTypeById(tag->GetId()) == 'TXTR') {
      mSlideB.mGallery = mGallery;
      mSlideB.mSlide = mSlide;
      mSlideB.mTexture = rs_new TToken< CTexture >(gpSimplePool->GetObj(*tag));
      mSlideB.mTexture->Lock();
      mSlideB.InitializeViewport();
    }
  }
  if (mSlideB.IsLoaded() && !mSlideB.mReady) {
    mSlideB.InitializeViewport();
  }
#endif
}

void CSlideShow::SetShowControls(const bool show) {
  mControlsFadeIn = show;
  mControlsFadeOut = !show;
  mShowControls = show;
}

bool CSlideShow::IsControlsAnimating() const {
  return mShowControls || mControlsFadeIn || mControlsFadeOut;
}

float CSlideShow::GetControlsTextHeight() const {
  const rstl::pair< CVector2i, CVector2i >& bounds = mControlsText->GetBounds();
  return bounds.first.GetY() - bounds.second.GetY();
}

float CSlideShow::GetControlsHeight() const {
  const int width = CGraphics::GetViewportWidth();
  const CAABox& bounds = mGalleryBorder->GetT()->GetCubeModel()->GetBoundingBox();
  return ((width - 32.f) / (bounds.GetMaxPoint().GetX() - bounds.GetMinPoint().GetX())) *
         (bounds.GetMaxPoint().GetZ() - bounds.GetMinPoint().GetZ());
}

void CSlideShow::UpdateControls(float dt) {
  if (!mControlsText.null()) {
    if (!IsControlsAnimating()) {
      mControlsOffset = -GetControlsHeight();
    } else {
      if (mControlsFadeIn) {
        const float height = GetControlsHeight();
        const float end = 32.f;
        mControlsOffset += (dt / gpTweakSlideShow->GetControlsFadeTime()) * (end + height);
        if (mControlsOffset >= end) {
          mControlsFadeIn = false;
          mControlsOffset = end;
        }
      } else if (mControlsFadeOut) {
        const float end = -GetControlsHeight();
        const float height = 32.f + GetControlsHeight();
        const float step = dt / gpTweakSlideShow->GetControlsFadeTime();
        mControlsOffset -= step * height;
        if (mControlsOffset <= end) {
          mControlsFadeOut = false;
          mShowControls = false;
          mControlsOffset = end;
        }
      }
    }
    mSlideNumberOffset = mControlsOffset + GetControlsHeight();
    if (mSlideNumberOffset < 32.f) {
      mSlideNumberOffset = 32.f;
    }
    mControlsText->Update(dt);
  }
}

void CSlideShow::UpdateControlsText(const CFinalInput& input) {
  if (!mControlsText.null()) {
    mCStick =
        GetStickDirection(ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveForward, input),
                          ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveBack, input),
                          ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveLeft, input),
                          ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveRight, input));
    mLStick =
        GetStickDirection(ControlMapper::GetAnalogInput(ControlMapper::kC_MapCircleDown, input),
                          ControlMapper::GetAnalogInput(ControlMapper::kC_MapCircleUp, input),
                          ControlMapper::GetAnalogInput(ControlMapper::kC_MapCircleLeft, input),
                          ControlMapper::GetAnalogInput(ControlMapper::kC_MapCircleRight, input));
    const float zoomIn = ControlMapper::GetAnalogInput(ControlMapper::kC_MapZoomIn, input);
    const float zoomOut = ControlMapper::GetAnalogInput(ControlMapper::kC_MapZoomOut, input);
    mRTrigger = zoomIn > 0.f ? 1 : 0;
    mLTrigger = zoomOut > 0.f ? 1 : 0;
#if VERSION >= VERSION_GM8P_00
    enum { kFirstControlString = 4 };
#else
    enum { kFirstControlString = 0x37 };
    const CStringTable& strings = *gpStringTable;
#endif
    rstl::wstring text;
    text.reserve(256);
#if VERSION >= VERSION_GM8P_00
    const CStringTable& strings = **mGalleryNames;
#endif
    text.append(CStringExtras::ConvertToUNICODE(
        CBasics::Stringize("%sSI,0.6,1.0,%8.8X%s", skImagePrefix,
                           gpTweakPlayerRes->mLStick[mLStick], skImageSuffix)));
    text.append(strings.GetString(kFirstControlString + 1), -1);
    text.append(CStringExtras::ConvertToUNICODE(rstl::string_l("   ")));
    text.append(CStringExtras::ConvertToUNICODE(CBasics::Stringize(
        "%s%8.8X%s", skImagePrefix, gpTweakPlayerRes->mLTrigger[mLTrigger], skImageSuffix)));
    text.append(CStringExtras::ConvertToUNICODE(rstl::string_l(" ")));
    text.append(strings.GetString(kFirstControlString + 3), -1);
    text.append(CStringExtras::ConvertToUNICODE(rstl::string_l(" ")));
    text.append(CStringExtras::ConvertToUNICODE(CBasics::Stringize(
        "%s%8.8X%s", skImagePrefix, gpTweakPlayerRes->mRTrigger[mRTrigger], skImageSuffix)));
    text.append(CStringExtras::ConvertToUNICODE(rstl::string_l("  ")));
    text.append(CStringExtras::ConvertToUNICODE(
        CBasics::Stringize("%sSI,0.6,1.0,%8.8X%s", skImagePrefix,
                           gpTweakPlayerRes->mCStick[mCStick], skImageSuffix)));
    text.append(strings.GetString(kFirstControlString + 2), -1);
    text.append(CStringExtras::ConvertToUNICODE(rstl::string_l("   ")));
    text.append(CStringExtras::ConvertToUNICODE(CBasics::Stringize(
        "%sSI,1.0,1.0,%8.8X%s", skImagePrefix, gpTweakPlayerRes->mYButton[0], skImageSuffix)));
    text.append(CStringExtras::ConvertToUNICODE(rstl::string_l(" ")));
    text.append(strings.GetString(kFirstControlString), -1);
    text.append(CStringExtras::ConvertToUNICODE(rstl::string_l("   ")));
    text.append(CStringExtras::ConvertToUNICODE(CBasics::Stringize(
        "%sSI,0.6,1.0,%8.8X%s", skImagePrefix, gpTweakPlayerRes->mBButton[0], skImageSuffix)));
    text.append(CStringExtras::ConvertToUNICODE(rstl::string_l(" ")));
    text.append(strings.GetString(kFirstControlString + 6), -1);
    mControlsText->SetText(text);
  }
}

void CSlideShow::SetPanSfx(bool active) {
  if (active) {
    if (!mPanSfx) {
      mPanSfx = CSfxManager::SfxStart(0x5ae, 127, 64, false, CSfxManager::kMedPriority, true,
                                         CSfxManager::kAllAreas);
    }
  } else {
    CSfxManager::SfxStop(mPanSfx);
    mPanSfx.Clear();
  }
}

void CSlideShow::SetZoomSfx(bool active) {
  if (active) {
    if (!mZoomSfx) {
      mZoomSfx = CSfxManager::SfxStart(0x5af, 127, 64, false, CSfxManager::kMedPriority, true,
                                          CSfxManager::kAllAreas);
    }
  } else {
    CSfxManager::SfxStop(mZoomSfx);
    mZoomSfx.Clear();
  }
}

void CSlideShow::UpdateMusicVolume(float time) {
  const float volume = CMath::Clamp(0.f, time / gpTweakSlideShow->GetMusicFadeTime(), 1.f);
  const uchar outputVolume =
      CCast::ToUint8(0.7421875f * volume * gpGameState->GameOptions().GetMusicVolume());
  mAudio->SetVolume(outputVolume);
  mAudio->StartMixOut();
}

void CSlideShow::SetTexturesLocked(rstl::vector< CToken >& textures, bool locked) {
  for (AUTO(it, textures.begin()); it != textures.end(); ++it) {
    if (locked) {
      it->Lock();
    } else {
      it->Unlock();
    }
  }
}

void CSlideShow::SetDependenciesLocked(rstl::vector< TToken< CDependencyGroup > >& deps,
                                       bool locked) {
  for (AUTO(it, deps.begin()); it != deps.end(); ++it) {
    if (locked) {
      it->Lock();
    } else {
      it->Unlock();
    }
  }
}

bool CSlideShow::AreAllDepsLoaded(const rstl::vector< TToken< CDependencyGroup > >& deps) const {
  for (AUTO(it, deps.begin()); it != deps.end(); ++it) {
    if (!it->IsLoaded()) {
      return false;
    }
  }
  return true;
}

void CSlideShow::DrawSlideNumber() const {
#if VERSION >= VERSION_GM8P_00
  if (!mSlideNumberText.null()) {
    const int height = CGraphics::GetViewportHeight();
    const float fadeTime = gpTweakSlideShow->GetSlideNumberFadeTime();
    const float alpha = CMath::Clamp(0.f, (fadeTime - mSlideNumberTimer) / fadeTime, 1.f);
    const float y = height + mSlideNumberOffset;
    CGraphics::SetCullMode(kCM_None);
    gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
    gpRender->SetDepthReadWrite(false, false);
    gpRender->SetModelMatrix(CTransform4f::Translate(-32.f, 0.f, y));
    mSlideNumberText->SetGeometryColor(CColor::White().WithAlphaModulatedBy(alpha));
    mSlideNumberText->Render();
    if (!mGalleryNameText.null() && mGalleryChanged) {
      gpRender->SetModelMatrix(CTransform4f::Translate(32.f, 0.f, y));
      mGalleryNameText->SetGeometryColor(CColor::White().WithAlphaModulatedBy(alpha));
      mGalleryNameText->SetText(mGalleryLabels[mGallery]);
      mGalleryNameText->Render();
    }
  }
#else
  if (!mSlideNumberText.null()) {
    const int height = CGraphics::GetViewportHeight();
    const float fadeTime = gpTweakSlideShow->GetSlideNumberFadeTime();
    const float alpha = CMath::Clamp(0.f, (fadeTime - mSlideNumberTimer) / fadeTime, 1.f);
    gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
#if defined(TARGET_PC)
    metaforce::AdjustUiProjection();
#endif
    gpRender->SetModelMatrix(CTransform4f::Translate(-32.f, 0.f, height + mSlideNumberOffset));
    CGraphics::SetCullMode(kCM_None);
    gpRender->SetDepthReadWrite(false, false);
    mSlideNumberText->SetGeometryColor(CColor::White().WithAlphaModulatedBy(alpha));
    mSlideNumberText->Render();
    const rstl::auto_ptr< TToken< CTexture > >& tag = mGalleryTags[mGallery];
    if (!tag.null() && mGalleryChanged) {
      DrawTexture(tag, CVector3f(32.f, 0.f, mSlideNumberOffset),
                  gpTweakSlideShow->GetFontColor().WithAlphaModulatedBy(alpha));
    }
  }
#endif
}

void CSlideShow::UpdateSlideNumber(float dt) {
  if (!mSlideNumberText.null() && !mGalleries.empty()) {
    int slide = mSlide;
    for (int i = 0; i < mGallery; ++i) {
#if VERSION >= VERSION_GM8P_00
      slide += mGalleries[i].mSlides.size();
#else
      slide += mGalleries[i].second.size();
#endif
    }
    const rstl::string text = CBasics::Stringize("%d/%d", slide + 1, mTotalSlides);
    mSlideNumberText->SetText(text);
    mSlideNumberText->Update(dt);
  }
  if (mSlideNumberTimer == gpTweakSlideShow->GetSlideNumberFadeTime()) {
    mGalleryChanged = false;
  }
}

void CSlideShow::DrawControls() const {
  if (!mControlsText.null()) {
    const int height = CGraphics::GetViewportHeight();
    gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
#if defined(TARGET_PC)
    metaforce::AdjustUiProjection();
#endif
    gpRender->SetModelMatrix(CTransform4f::Translate(0.f, 0.f, height + mControlsOffset));
    CGraphics::SetCullMode(kCM_None);
    gpRender->SetDepthReadWrite(false, false);
    mControlsText->Render();
    DrawControlsBorder();
  }
}

void CSlideShow::DrawControlsBorder() const {
  if (!mGalleryBorder.null()) {
    const int width = CGraphics::GetViewportWidth();
    const int height = CGraphics::GetViewportHeight();
    gpRender->SetViewportOrtho(true, -4096.f, 4096.f);
#if defined(TARGET_PC)
    metaforce::AdjustUiProjection();
#endif
    CGraphics::SetViewPointMatrix(CTransform4f::Identity());
    const CAABox bounds = mGalleryBorder->GetT()->GetCubeModel()->GetBoundingBox();
    const float scale =
        (width - 32.f) / (bounds.GetMaxPoint().GetX() - bounds.GetMinPoint().GetX());
    CTransform4f xf = CTransform4f::Translate(bounds.GetCenterPoint() * -1.f * scale);
    xf *= CTransform4f::Scale(scale, 1.f, scale);
    xf.AddTranslation(CVector3f(
        0.f, 0.f,
        scale * (bounds.GetMaxPoint().GetZ() - bounds.GetMinPoint().GetZ()) / 2.f - height / 2.f));
    xf.AddTranslation(
        CVector3f(0.f, 0.f, GetControlsTextHeight() / 2.f + (5.f + mControlsOffset)));
    CGraphics::SetModelMatrix(xf);
    const CModelFlags flags(CModelFlags::kT_One, gpTweakSlideShow->GetBorderColor());
    gpRender->DrawModelFlat(*mGalleryBorder->GetT(), flags, false, TModelPositions(),
                            TModelNormals());
    gpRender->SetBlendMode_AlphaBlended();
  }
}

CIOWin::EMessageReturn CSlideShow::SSlideData::ProcessUserInput(const CFinalInput& input) {
  if (IsReady()) {
#if VERSION < VERSION_GM8P_00
    const CTexture* texture = **mTexture;
#endif
    const int width = CGraphics::GetViewportWidth();
    const int height = CGraphics::GetViewportHeight();
#if VERSION >= VERSION_GM8P_00
    const float texWidth = mTextureWidth;
    const float texHeight = mTextureHeight;
#else
    const float texWidth = texture->GetWidth();
    const float texHeight = texture->GetHeight();
#endif
    const float aspect = float(width) / height;
#if VERSION >= VERSION_GM8P_00
    const float& textureSize = rstl::max_val(mTextureWidth, mTextureHeight);
#endif
    const float zoomIn = ControlMapper::GetAnalogInput(ControlMapper::kC_MapZoomIn, input);
    const float zoomOut = ControlMapper::GetAnalogInput(ControlMapper::kC_MapZoomOut, input);
#if VERSION >= VERSION_GM8P_00
    const float zoom = textureSize * (zoomOut - zoomIn) / 1024.f;
#endif
    const CVector2f oldSize = mVpSize;
#if VERSION >= VERSION_GM8P_00
    if (zoom != 0.f) {
#else
    if (zoomOut - zoomIn != 0.f) {
#endif
      CVector2f offset = mVpSize;
#if VERSION >= VERSION_GM8P_00
      const float delta = gpTweakSlideShow->GetZoomSpeed() * zoom;
#else
      const float delta = gpTweakSlideShow->GetZoomSpeed() * (zoomOut - zoomIn);
#endif
      mVpSize[0] += aspect * delta;
      mVpSize[1] += delta;
      mVpSize[0] = CMath::Clamp(float(width), mVpSize.GetX(), mCanvasSize.GetX());
      mVpSize[1] = CMath::Clamp(float(height), mVpSize.GetY(), mCanvasSize.GetY());
      offset -= mVpSize;
      offset /= 2.f;
      mVpOffset += offset;
    }
    mParent->SetZoomSfx(!(oldSize == mVpSize));
    const CVector2f oldOffset = mVpOffset;
    const float forward = ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveForward, input);
    const float back = ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveBack, input);
    const float left = ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveLeft, input);
    const float right = ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveRight, input);
#if VERSION >= VERSION_GM8P_00
    const float speed = gpTweakSlideShow->GetPanSpeed() *
                        rstl::max_val(mTextureWidth, mTextureHeight) / 1024.f;
#else
    const float speed = gpTweakSlideShow->GetPanSpeed();
#endif
    mVpOffset[0] -= speed * left;
    mVpOffset[0] += speed * right;
    mVpOffset[1] += speed * forward;
    mVpOffset[1] -= speed * back;
    mVpOffset[0] =
        CMath::Clamp(0.f, mVpOffset.GetX(), mCanvasSize.GetX() - mVpSize.GetX());
    mVpOffset[1] =
        CMath::Clamp(0.f, mVpOffset.GetY(), mCanvasSize.GetY() - mVpSize.GetY());
    float halfX = 0.f;
    const float availableX = texWidth / 2.f - mVpSize.GetX() / 2.f;
    if (availableX > halfX) {
      halfX = availableX;
    }
    float halfY = 0.f;
    const float availableY = texHeight / 2.f - mVpSize.GetY() / 2.f;
    if (availableY > halfY) {
      halfY = availableY;
    }
    mVpOffset[0] = CMath::Clamp(mCanvasSize.GetX() / 2.f - halfX,
                                   mVpSize.GetX() / 2.f + mVpOffset.GetX(),
                                   mCanvasSize.GetX() / 2.f + halfX) -
                      mVpSize.GetX() / 2.f;
    mVpOffset[1] = CMath::Clamp(mCanvasSize.GetY() / 2.f - halfY,
                                   mVpSize.GetY() / 2.f + mVpOffset.GetY(),
                                   mCanvasSize.GetY() / 2.f + halfY) -
                      mVpSize.GetY() / 2.f;
    mParent->SetPanSfx(!(oldOffset == mVpOffset));
  }
  return kMR_Exit;
}

void CSlideShow::SSlideData::InitializeViewport() {
  if (IsLoaded()) {
#if VERSION >= VERSION_GM8P_00
    const float width = (**mTextures.front().mToken)->GetWidth();
    const float height = (**mTextures.front().mToken)->GetHeight();
    mTextureWidth = width * mColumns;
    mTextureHeight = height * (mTextures.size() / mColumns);
    const float texAspect = mTextureWidth / mTextureHeight;
#else
    const CTexture* texture = **mTexture;
    const float width = texture->GetWidth();
    const float height = texture->GetHeight();
    const float texAspect = width / height;
#endif
    const float aspect = float(CGraphics::GetViewportWidth()) / CGraphics::GetViewportHeight();
    mVpOffset = sZeroVector;
    if (texAspect != aspect) {
      if (texAspect > aspect) {
#if VERSION >= VERSION_GM8P_00
        mVpSize = CVector2f(mTextureWidth, mTextureWidth / aspect);
#else
        mVpSize = CVector2f(width, width / aspect);
#endif
      } else {
#if VERSION >= VERSION_GM8P_00
        mVpSize = CVector2f(mTextureHeight * aspect, mTextureHeight);
#else
        mVpSize = CVector2f(height * aspect, height);
#endif
      }
    }
#if VERSION >= VERSION_GM8P_00
    for (int i = 0; i < mTextures.size(); ++i) {
      const float x = width * (i % mColumns);
      const float y = height * (i / mColumns);
      mTextures[i].mRightTop = CVector2f(x + width, y);
      mTextures[i].mLeftBottom = CVector2f(x, y + height);
    }
#endif
    mCanvasSize = mVpSize;
    mReady = true;
  }
}

#if VERSION >= VERSION_GM8P_00
const bool CSlideShow::SSlideData::IsLoaded() const {
  if (mTextures.empty()) {
    return false;
  }

  bool loaded = true;
  if (mTextures.front().mToken.null() || !mTextures.front().mToken->IsLoaded()) {
    loaded = false;
  }

  if (!mStopLoading) {
    for (int i = 0; i < mTextures.size(); ++i) {
      if (!mTextures[i].mToken.null()) {
        mTextures[i].mToken->Lock();
        if (!mTextures[i].mToken->IsLoaded()) {
          break;
        }
        mTextures[i].mAlpha = rstl::min_val(mTextures[i].mAlpha + 0.01f, 1.f);
      }
    }
  } else {
    for (int i = 0; i < mTextures.size(); ++i) {
      if (!mTextures[i].mToken.null() && mTextures[i].mToken->IsLocked() &&
          !mTextures[i].mToken->IsLoaded()) {
        mTextures[i].mToken->Unlock();
        break;
      }
    }
  }

  return loaded;
}
#endif

void CSlideShow::SSlideData::Draw() const {
#if VERSION >= VERSION_GM8P_00
  if (IsReady()) {
    const CVector2f leftBottom(mVpOffset.GetX(), mVpOffset.GetY() + mVpSize.GetY());
    const CVector2f rightTop(mVpOffset.GetX() + mVpSize.GetX(), mVpOffset.GetY());
    const float x = (mCanvasSize.GetX() - mTextureWidth) / 2.f;
    const float y = (mCanvasSize.GetY() - mTextureHeight) / 2.f;
    for (int i = 0; i < mTextures.size(); ++i) {
      const STexture& texture = mTextures[i];
      if (texture.mAlpha > 0.f && !(leftBottom.GetX() > x + texture.mRightTop.GetX()) &&
          !(leftBottom.GetY() < y + texture.mRightTop.GetY()) &&
          !(rightTop.GetX() < x + texture.mLeftBottom.GetX()) &&
          !(rightTop.GetY() > y + texture.mLeftBottom.GetY())) {
        DrawTexture(texture.mToken, CVector3f(x + texture.mLeftBottom.GetX(), 0.f,
                                               y + texture.mRightTop.GetY()),
                    mMulColor.WithAlphaModulatedBy(texture.mAlpha), &mVpOffset, &mVpSize);
      }
    }
  }
#else
  if (IsReady()) {
    const int width = (**mTexture)->GetWidth();
    const int height = (**mTexture)->GetHeight();
    const CVector3f offset((mCanvasSize.GetX() - width) / 2.f, 0.f,
                           (mCanvasSize.GetY() - height) / 2.f);
    DrawTexture(mTexture, offset, mMulColor, &mVpOffset, &mVpSize);
  }
#endif
}

void CSlideShow::SSlideData::Reset() {
  mGallery = -1;
  mSlide = -1;
#if VERSION >= VERSION_GM8P_00
  mTextures = rstl::vector< STexture >();
  mColumns = 0;
#else
  mTexture = rstl::auto_ptr< TToken< CTexture > >();
#endif
  mReady = false;
  mVpOffset = sZeroVector;
  mVpSize = sZeroVector;
  mCanvasSize = sZeroVector;
  mMulColor = CColor::White();
  mMulColor.SetAlpha(0.f);
}

bool CSlideShow::GetIsContinueDraw() const { return false; }
