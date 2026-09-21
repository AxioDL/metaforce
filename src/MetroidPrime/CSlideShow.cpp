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
, x14_phase(0)
, x38_galleryBorder(nullptr)
, x3c_(0)
, x40_totalSlides(0)
, x44_gallery(0)
, x48_slide(-1)
, x4c_crossfadeTimer(0.f)
, x50_repeatTimer(0.f)
, x54_idleTimer(0.f)
, x58_slideNumberTimer(0.f)
, xc4_controlsText(nullptr)
, xc8_slideNumberText(nullptr)
, xcc_audio(nullptr)
, xe8_lStick(0)
, xec_cStick(0)
, xf0_lTrigger(0)
, xf4_rTrigger(0)
, x128_controlsOffset(32.f)
, x12c_slideNumberOffset(32.f)
, x130_fadeTimer(gpTweakSlideShow->GetFadeTime())
, x134_24_showControls(true)
, x134_25_controlsFadeIn(false)
, x134_26_controlsFadeOut(false)
, x134_28_disableInput(false)
, x134_29_exit(false)
, x134_30_introFade(true)
, x134_31_outroFade(false)
, x135_24_galleryChanged(true) {
  x5c_slideA.x0_parent = this;
  x90_slideB.x0_parent = this;
  gpResourceFactory->GetResLoader().AddPakFileAsync(gpTweakSlideShow->GetPakName(), false, false);
  const int width = CGraphics::GetViewportWidth();
  const int height = CGraphics::GetViewportHeight();
  const CColor& fontColor = gpTweakSlideShow->GetFontColor();
  const CColor& outlineColor = gpTweakSlideShow->GetOutlineColor();
  const SObjectTag* font =
      gpResourceFactory->GetResourceIdByName(gpTweakSlideShow->GetFont().data());
  xc4_controlsText = rs_new CGuiTextSupport(
      font->GetId(),
      CGuiTextProperties(false, true, kJustification_Center, kVerticalJustification_Bottom),
      fontColor, outlineColor, CColor::White(), width, height, gpSimplePool);
  xc8_slideNumberText = rs_new CGuiTextSupport(
      font->GetId(),
      CGuiTextProperties(false, true, kJustification_Right, kVerticalJustification_Bottom),
      fontColor, outlineColor, CColor::White(), width, height, gpSimplePool);
  const rstl::reserved_vector< CAssetId, 9 >* sticks[] = {&gpTweakPlayerRes->x20_lStick,
                                                          &gpTweakPlayerRes->x48_cStick};
  xf8_stickTextures.reserve(18);
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 9; ++j) {
      xf8_stickTextures.push_back(gpSimplePool->GetObj(SObjectTag('TXTR', (*sticks[i])[j])));
    }
  }
  SetTexturesLocked(xf8_stickTextures, true);
  const rstl::reserved_vector< CAssetId, 2 >* buttons[] = {
      &gpTweakPlayerRes->x70_lTrigger, &gpTweakPlayerRes->x7c_rTrigger,
      &gpTweakPlayerRes->xa0_bButton, &gpTweakPlayerRes->xb8_yButton};
  x108_buttonTextures.reserve(8);
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 2; ++j) {
      x108_buttonTextures.push_back(gpSimplePool->GetObj(SObjectTag('TXTR', (*buttons[i])[j])));
    }
  }
  SetTexturesLocked(x108_buttonTextures, true);
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
  const int count = x18_galleryTXTRDeps.size() - 1;
  x28_galleries.reserve(count);
  AUTO(it, x18_galleryTXTRDeps.begin());
  for (int i = 0; it != x18_galleryTXTRDeps.end() && i < count; ++i) {
    if ((flags & (1 << i)) == 0) {
      it = x18_galleryTXTRDeps.erase(it);
    } else {
      const int slides = it->GetT()->GetObjectTagVector().size();
      x28_galleries.push_back(SGalleryData(i, rstl::vector< const SObjectTag* >()));
      SGalleryData& gallery = x28_galleries.back();
      gallery.second.reserve(slides);
      for (int j = 0; j < slides; ++j) {
        const char* name = CBasics::Stringize("%s_%02d_%03d", "slideshow", i, j);
        const SObjectTag* tag = gpResourceFactory->GetResourceIdByName(name);
        gallery.second.push_back(tag);
      }
      ++it;
    }
  }
}

bool CSlideShow::LoadTXTRDep(const char* name) {
  const SObjectTag* tag = gpResourceFactory->GetResourceIdByName(name);
  if (tag != nullptr && tag->GetType() == 'DGRP') {
    if (x18_galleryTXTRDeps.size() + 1 > x18_galleryTXTRDeps.capacity()) {
      x18_galleryTXTRDeps.reserve(x18_galleryTXTRDeps.size() + 1);
    }
    x18_galleryTXTRDeps.push_back(TToken< CDependencyGroup >(gpSimplePool->GetObj(*tag)));
  } else {
    return false;
  }
  return true;
}

CIOWin::EMessageReturn CSlideShow::OnMessage(const CArchitectureMessage& msg,
                                             CArchitectureQueue& queue) {
  switch (msg.GetType()) {
  case kAM_TimerTick: {
    if (x134_29_exit) {
      return kMR_RemoveIOWinAndExit;
    }
    const float dt = MakeMsg::GetParmTimerTick(msg).GetReal();
    switch (x14_phase) {
    case 0:
      if (!gpResourceFactory->GetResLoader().AreAllPaksLoaded()) {
        gpResourceFactory->GetResLoader().AsyncIdlePakLoading();
        break;
      }
      x14_phase = 1;
    case 1:
      if (x18_galleryTXTRDeps.empty()) {
        x18_galleryTXTRDeps.reserve(5);
        for (int i = 1;; ++i) {
          const rstl::string name = CBasics::Stringize("%s%02d_DGRP", skGalleryName, i);
          if (!LoadTXTRDep(name.data())) {
            break;
          }
        }
        LoadTXTRDep(skGalleryAssets);
        SetDependenciesLocked(x18_galleryTXTRDeps, true);
      }
      if (!AreAllDepsLoaded(x18_galleryTXTRDeps)) {
        break;
      }
      x14_phase = 3;
    case 3:
      BuildGalleryLists(SlideShowGalleryFlags());
      for (int i = 0; i < x28_galleries.size(); ++i) {
        x40_totalSlides += x28_galleries[i].second.size();
      }
      AdvanceSlide(true);
      x14_phase = 2;
    case 2:
      if (xd0_galleryTags.size() != x28_galleries.size()) {
        xd0_galleryTags.reserve(x28_galleries.size());
        for (int i = 0; i < x28_galleries.size(); ++i) {
          const char* name = CBasics::Stringize("%s%02d", skGalleryTag, x28_galleries[i].first + 1);
          const SObjectTag* tag = gpResourceFactory->GetResourceIdByName(name);
          if (tag == nullptr) {
            xd0_galleryTags.push_back(rstl::auto_ptr< TToken< CTexture > >());
          } else {
            xd0_galleryTags.push_back(rs_new TToken< CTexture >(gpSimplePool->GetObj(*tag)));
            xd0_galleryTags.back()->Lock();
          }
        }
        if (x38_galleryBorder.null()) {
          const SObjectTag* tag = gpResourceFactory->GetResourceIdByName(skGalleryBorder);
          x38_galleryBorder = rs_new TToken< CModel >(gpSimplePool->GetObj(*tag));
          x38_galleryBorder->Lock();
        }
      }
      if (!x38_galleryBorder->IsLoaded()) {
        break;
      }
      for (int i = 0; i < xd0_galleryTags.size(); ++i) {
        if (!xd0_galleryTags[i].null() && !xd0_galleryTags[i]->IsLoaded()) {
          break;
        }
      }
      x14_phase = 4;
    case 4:
      if (xcc_audio.null()) {
        xcc_audio = rs_new CStaticAudioPlayer(rstl::string_l(skAudioFile), 0x65af0, 0x1e1db0);
      }
      if (!xcc_audio->IsReady()) {
        break;
      }
      UpdateMusicVolume(gpTweakSlideShow->GetMusicFadeTime());
      xcc_audio->StartMixOut();
      x14_phase = 5;
    case 5: {
      if (x134_30_introFade || x134_31_outroFade) {
        x130_fadeTimer = rstl::max_val(0.f, x130_fadeTimer - dt);
        if (x130_fadeTimer <= 0.f) {
          if (x134_28_disableInput) {
            x134_29_exit = true;
          } else {
            x134_30_introFade = false;
            x134_31_outroFade = false;
            x130_fadeTimer = gpTweakSlideShow->GetMusicFadeTime();
          }
        }
      }
      x38_galleryBorder->GetT()->Touch(0);
      LoadSlide();
      if (x90_slideB.IsReady()) {
        const float crossfadeTime = gpTweakSlideShow->GetCrossfadeTime();
        if (x4c_crossfadeTimer > crossfadeTime) {
          x5c_slideA = x90_slideB;
          SetPanSfx(false);
          SetZoomSfx(false);
          x5c_slideA.x30_mulColor.SetAlpha(1.f);
          x90_slideB.Reset();
          x4c_crossfadeTimer = 0.f;
        } else {
          const float alpha = CMath::Clamp(0.f, x4c_crossfadeTimer / crossfadeTime, 1.f);
          x5c_slideA.x30_mulColor.SetAlpha(1.f - alpha);
          x90_slideB.x30_mulColor.SetAlpha(alpha);
          x4c_crossfadeTimer += dt;
        }
      } else if (CMath::AbsF(x50_repeatTimer) > gpTweakSlideShow->GetSlideRepeatTime()) {
        AdvanceSlide(x50_repeatTimer > 0.f);
        x50_repeatTimer = 0.f;
      }
      const float idleTime =
          IsControlsAnimating() ? 0.f : gpTweakSlideShow->GetSlideNumberIdleTime();
      x54_idleTimer = CMath::Clamp(0.f, x54_idleTimer + dt, idleTime);
      if (x54_idleTimer >= gpTweakSlideShow->GetSlideNumberIdleTime()) {
        x58_slideNumberTimer += dt;
      } else {
        x58_slideNumberTimer -= dt;
      }
      x58_slideNumberTimer =
          CMath::Clamp(0.f, x58_slideNumberTimer, gpTweakSlideShow->GetSlideNumberFadeTime());
      UpdateControls(dt);
      UpdateSlideNumber(dt);
      if (x134_31_outroFade) {
        UpdateMusicVolume(x130_fadeTimer);
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
  if (x14_phase == 5) {
    if (x5c_slideA.IsReady()) {
      x5c_slideA.Draw();
    }
    if (x90_slideB.IsReady()) {
      x90_slideB.Draw();
    }
    if (!xc8_slideNumberText.null() &&
        x58_slideNumberTimer != gpTweakSlideShow->GetSlideNumberFadeTime()) {
      DrawSlideNumber();
    }
    if (IsControlsAnimating()) {
      DrawControls();
    }
    if (x134_30_introFade || x134_31_outroFade) {
      float alpha = x130_fadeTimer / (x134_30_introFade ? gpTweakSlideShow->GetFadeTime()
                                                        : gpTweakSlideShow->GetMusicFadeTime());
      if (x134_31_outroFade) {
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
  if (!x134_28_disableInput) {
    if (IsControlsAnimating()) {
      UpdateControlsText(input);
    }
    if (input.PB()) {
      SetPanSfx(false);
      SetZoomSfx(false);
      CSfxManager::SfxStart(0x446, 127, 64, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
      x134_28_disableInput = true;
      x134_31_outroFade = true;
      return kMR_Exit;
    }
    if (input.PY()) {
      SetShowControls(!x134_24_showControls);
      if (x134_24_showControls) {
        x135_24_galleryChanged = true;
        CSfxManager::SfxStart(0x5b1, 127, 64, false, CSfxManager::kMedPriority, false,
                              CSfxManager::kAllAreas);
      } else {
        CSfxManager::SfxStart(0x5b0, 127, 64, false, CSfxManager::kMedPriority, false,
                              CSfxManager::kAllAreas);
      }
    }
    bool changed = false;
    if (input.PDPRight()) {
      x44_gallery = (x44_gallery + 1) % x28_galleries.size();
      x48_slide = -1;
      changed = true;
      x135_24_galleryChanged = true;
      AdvanceSlide(true);
    } else if (input.PDPLeft()) {
      --x44_gallery;
      if (x44_gallery < 0) {
        x44_gallery = x28_galleries.size() - 1;
      }
      x48_slide = -1;
      changed = true;
      x135_24_galleryChanged = true;
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
        x50_repeatTimer = rstl::max_val(0.f, x50_repeatTimer);
        x50_repeatTimer += input.Time();
      } else {
        float left = ControlMapper::GetAnalogInput(ControlMapper::kC_MapCircleLeft, input);
        if (left) {
          x50_repeatTimer = rstl::min_val(0.f, x50_repeatTimer);
          x50_repeatTimer -= input.Time();
        } else {
          x50_repeatTimer = 0.f;
        }
      }
    }
    if (changed) {
      x50_repeatTimer = 0.f;
      x54_idleTimer = 0.f;
    }
    if (x5c_slideA.IsReady()) {
      return x5c_slideA.ProcessUserInput(input);
    }
  }
  return kMR_Exit;
}
CIOWin::EMessageReturn CSlideShow::AdvanceSlide(bool forward) {
  if (!x28_galleries.empty()) {
    CSfxManager::SfxStart(0x445, 127, 64, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    if (forward) {
      ++x48_slide;
    } else {
      --x48_slide;
    }
    const int gallery = x44_gallery;
    if (x48_slide < 0) {
      --x44_gallery;
      x135_24_galleryChanged = true;
    } else if (x48_slide >= x28_galleries[x44_gallery].second.size()) {
      ++x44_gallery;
      x135_24_galleryChanged = true;
    }
    if (x44_gallery < 0) {
      x44_gallery = x28_galleries.size() - 1;
      x48_slide = x28_galleries[x44_gallery].second.size() - 1;
    } else if (x44_gallery >= x28_galleries.size()) {
      x48_slide = 0;
      x44_gallery = 0;
    } else if (x44_gallery > gallery) {
      x48_slide = 0;
    } else if (x44_gallery < gallery) {
      x48_slide = x28_galleries[x44_gallery].second.size() - 1;
    }
  }
  return kMR_Exit;
}
void CSlideShow::LoadSlide() {
  if (x90_slideB.xc_texture.null() &&
      (x5c_slideA.x4_gallery != x44_gallery || x5c_slideA.x8_slide != x48_slide)) {
    const SObjectTag* tag = x28_galleries[x44_gallery].second[x48_slide];
    if (tag != nullptr && gpResourceFactory->GetResourceTypeById(tag->GetId()) == 'TXTR') {
      x90_slideB.x4_gallery = x44_gallery;
      x90_slideB.x8_slide = x48_slide;
      x90_slideB.xc_texture = rs_new TToken< CTexture >(gpSimplePool->GetObj(*tag));
      x90_slideB.xc_texture->Lock();
      x90_slideB.InitializeViewport();
    }
  }
  if (x90_slideB.IsLoaded() && !x90_slideB.x14_ready) {
    x90_slideB.InitializeViewport();
  }
}

void CSlideShow::SetShowControls(const bool show) {
  x134_25_controlsFadeIn = show;
  x134_26_controlsFadeOut = !show;
  x134_24_showControls = show;
}

bool CSlideShow::IsControlsAnimating() const {
  return x134_24_showControls || x134_25_controlsFadeIn || x134_26_controlsFadeOut;
}

float CSlideShow::GetControlsTextHeight() const {
  const rstl::pair< CVector2i, CVector2i >& bounds = xc4_controlsText->GetBounds();
  return bounds.first.GetY() - bounds.second.GetY();
}

float CSlideShow::GetControlsHeight() const {
  const int width = CGraphics::GetViewportWidth();
  const CAABox& bounds = x38_galleryBorder->GetT()->GetCubeModel()->GetBoundingBox();
  return ((width - 32.f) / (bounds.GetMaxPoint().GetX() - bounds.GetMinPoint().GetX())) *
         (bounds.GetMaxPoint().GetZ() - bounds.GetMinPoint().GetZ());
}

void CSlideShow::UpdateControls(float dt) {
  if (!xc4_controlsText.null()) {
    if (!IsControlsAnimating()) {
      x128_controlsOffset = -GetControlsHeight();
    } else {
      if (x134_25_controlsFadeIn) {
        const float height = GetControlsHeight();
        const float end = 32.f;
        x128_controlsOffset += (dt / gpTweakSlideShow->GetControlsFadeTime()) * (end + height);
        if (x128_controlsOffset >= end) {
          x134_25_controlsFadeIn = false;
          x128_controlsOffset = end;
        }
      } else if (x134_26_controlsFadeOut) {
        const float end = -GetControlsHeight();
        const float height = 32.f + GetControlsHeight();
        const float step = dt / gpTweakSlideShow->GetControlsFadeTime();
        x128_controlsOffset -= step * height;
        if (x128_controlsOffset <= end) {
          x134_26_controlsFadeOut = false;
          x134_24_showControls = false;
          x128_controlsOffset = end;
        }
      }
    }
    x12c_slideNumberOffset = x128_controlsOffset + GetControlsHeight();
    if (x12c_slideNumberOffset < 32.f) {
      x12c_slideNumberOffset = 32.f;
    }
    xc4_controlsText->Update(dt);
  }
}
void CSlideShow::UpdateControlsText(const CFinalInput& input) {
  if (!xc4_controlsText.null()) {
    xec_cStick =
        GetStickDirection(ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveForward, input),
                          ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveBack, input),
                          ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveLeft, input),
                          ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveRight, input));
    xe8_lStick =
        GetStickDirection(ControlMapper::GetAnalogInput(ControlMapper::kC_MapCircleDown, input),
                          ControlMapper::GetAnalogInput(ControlMapper::kC_MapCircleUp, input),
                          ControlMapper::GetAnalogInput(ControlMapper::kC_MapCircleLeft, input),
                          ControlMapper::GetAnalogInput(ControlMapper::kC_MapCircleRight, input));
    const float zoomIn = ControlMapper::GetAnalogInput(ControlMapper::kC_MapZoomIn, input);
    const float zoomOut = ControlMapper::GetAnalogInput(ControlMapper::kC_MapZoomOut, input);
    xf4_rTrigger = zoomIn > 0.f ? 1 : 0;
    xf0_lTrigger = zoomOut > 0.f ? 1 : 0;
    const CStringTable& strings = *gpStringTable;
    rstl::wstring text;
    text.reserve(256);
    text.append(CStringExtras::ConvertToUNICODE(
        CBasics::Stringize("%sSI,0.6,1.0,%8.8X%s", skImagePrefix,
                           gpTweakPlayerRes->x20_lStick[xe8_lStick], skImageSuffix)));
    text.append(strings.GetString(0x38), -1);
    text.append(CStringExtras::ConvertToUNICODE(rstl::string_l("   ")));
    text.append(CStringExtras::ConvertToUNICODE(CBasics::Stringize(
        "%s%8.8X%s", skImagePrefix, gpTweakPlayerRes->x70_lTrigger[xf0_lTrigger], skImageSuffix)));
    text.append(CStringExtras::ConvertToUNICODE(rstl::string_l(" ")));
    text.append(strings.GetString(0x3a), -1);
    text.append(CStringExtras::ConvertToUNICODE(rstl::string_l(" ")));
    text.append(CStringExtras::ConvertToUNICODE(CBasics::Stringize(
        "%s%8.8X%s", skImagePrefix, gpTweakPlayerRes->x7c_rTrigger[xf4_rTrigger], skImageSuffix)));
    text.append(CStringExtras::ConvertToUNICODE(rstl::string_l("  ")));
    text.append(CStringExtras::ConvertToUNICODE(
        CBasics::Stringize("%sSI,0.6,1.0,%8.8X%s", skImagePrefix,
                           gpTweakPlayerRes->x48_cStick[xec_cStick], skImageSuffix)));
    text.append(strings.GetString(0x39), -1);
    text.append(CStringExtras::ConvertToUNICODE(rstl::string_l("   ")));
    text.append(CStringExtras::ConvertToUNICODE(CBasics::Stringize(
        "%sSI,1.0,1.0,%8.8X%s", skImagePrefix, gpTweakPlayerRes->xb8_yButton[0], skImageSuffix)));
    text.append(CStringExtras::ConvertToUNICODE(rstl::string_l(" ")));
    text.append(strings.GetString(0x37), -1);
    text.append(CStringExtras::ConvertToUNICODE(rstl::string_l("   ")));
    text.append(CStringExtras::ConvertToUNICODE(CBasics::Stringize(
        "%sSI,0.6,1.0,%8.8X%s", skImagePrefix, gpTweakPlayerRes->xa0_bButton[0], skImageSuffix)));
    text.append(CStringExtras::ConvertToUNICODE(rstl::string_l(" ")));
    text.append(strings.GetString(0x3d), -1);
    xc4_controlsText->SetText(text);
  }
}

void CSlideShow::SetPanSfx(bool active) {
  if (active) {
    if (!xe0_panSfx) {
      xe0_panSfx = CSfxManager::SfxStart(0x5ae, 127, 64, false, CSfxManager::kMedPriority, true,
                                         CSfxManager::kAllAreas);
    }
  } else {
    CSfxManager::SfxStop(xe0_panSfx);
    xe0_panSfx.Clear();
  }
}

void CSlideShow::SetZoomSfx(bool active) {
  if (active) {
    if (!xe4_zoomSfx) {
      xe4_zoomSfx = CSfxManager::SfxStart(0x5af, 127, 64, false, CSfxManager::kMedPriority, true,
                                          CSfxManager::kAllAreas);
    }
  } else {
    CSfxManager::SfxStop(xe4_zoomSfx);
    xe4_zoomSfx.Clear();
  }
}

void CSlideShow::UpdateMusicVolume(float time) {
  const float volume = CMath::Clamp(0.f, time / gpTweakSlideShow->GetMusicFadeTime(), 1.f);
  const uchar outputVolume =
      CCast::ToUint8(0.7421875f * volume * gpGameState->GameOptions().GetMusicVolume());
  xcc_audio->SetVolume(outputVolume);
  xcc_audio->StartMixOut();
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
  if (!xc8_slideNumberText.null()) {
    const int height = CGraphics::GetViewportHeight();
    const float fadeTime = gpTweakSlideShow->GetSlideNumberFadeTime();
    const float alpha = CMath::Clamp(0.f, (fadeTime - x58_slideNumberTimer) / fadeTime, 1.f);
    gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
    gpRender->SetModelMatrix(CTransform4f::Translate(-32.f, 0.f, height + x12c_slideNumberOffset));
    CGraphics::SetCullMode(kCM_None);
    gpRender->SetDepthReadWrite(false, false);
    xc8_slideNumberText->SetGeometryColor(CColor::White().WithAlphaModulatedBy(alpha));
    xc8_slideNumberText->Render();
    const rstl::auto_ptr< TToken< CTexture > >& tag = xd0_galleryTags[x44_gallery];
    if (!tag.null() && x135_24_galleryChanged) {
      DrawTexture(tag, CVector3f(32.f, 0.f, x12c_slideNumberOffset),
                  gpTweakSlideShow->GetFontColor().WithAlphaModulatedBy(alpha));
    }
  }
}
void CSlideShow::UpdateSlideNumber(float dt) {
  if (!xc8_slideNumberText.null() && !x28_galleries.empty()) {
    int slide = x48_slide;
    for (int i = 0; i < x44_gallery; ++i) {
      slide += x28_galleries[i].second.size();
    }
    const rstl::string text = CBasics::Stringize("%d/%d", slide + 1, x40_totalSlides);
    xc8_slideNumberText->SetText(text);
    xc8_slideNumberText->Update(dt);
  }
  if (x58_slideNumberTimer == gpTweakSlideShow->GetSlideNumberFadeTime()) {
    x135_24_galleryChanged = false;
  }
}
void CSlideShow::DrawControls() const {
  if (!xc4_controlsText.null()) {
    const int height = CGraphics::GetViewportHeight();
    gpRender->SetViewportOrtho(false, -4096.f, 4096.f);
    gpRender->SetModelMatrix(CTransform4f::Translate(0.f, 0.f, height + x128_controlsOffset));
    CGraphics::SetCullMode(kCM_None);
    gpRender->SetDepthReadWrite(false, false);
    xc4_controlsText->Render();
    DrawControlsBorder();
  }
}
void CSlideShow::DrawControlsBorder() const {
  if (!x38_galleryBorder.null()) {
    const int width = CGraphics::GetViewportWidth();
    const int height = CGraphics::GetViewportHeight();
    gpRender->SetViewportOrtho(true, -4096.f, 4096.f);
    CGraphics::SetViewPointMatrix(CTransform4f::Identity());
    const CAABox bounds = x38_galleryBorder->GetT()->GetCubeModel()->GetBoundingBox();
    const float scale =
        (width - 32.f) / (bounds.GetMaxPoint().GetX() - bounds.GetMinPoint().GetX());
    CTransform4f xf = CTransform4f::Translate(bounds.GetCenterPoint() * -1.f * scale);
    xf *= CTransform4f::Scale(scale, 1.f, scale);
    xf.AddTranslation(CVector3f(
        0.f, 0.f,
        scale * (bounds.GetMaxPoint().GetZ() - bounds.GetMinPoint().GetZ()) / 2.f - height / 2.f));
    xf.AddTranslation(
        CVector3f(0.f, 0.f, GetControlsTextHeight() / 2.f + (5.f + x128_controlsOffset)));
    CGraphics::SetModelMatrix(xf);
    const CModelFlags flags(CModelFlags::kT_One, gpTweakSlideShow->GetBorderColor());
    gpRender->DrawModelFlat(*x38_galleryBorder->GetT(), flags, false, TModelPositions(),
                            TModelNormals());
    gpRender->SetBlendMode_AlphaBlended();
  }
}

CIOWin::EMessageReturn CSlideShow::SSlideData::ProcessUserInput(const CFinalInput& input) {
  if (IsReady()) {
    const CTexture* texture = **xc_texture;
    const int width = CGraphics::GetViewportWidth();
    const int height = CGraphics::GetViewportHeight();
    const float texWidth = texture->GetWidth();
    const float texHeight = texture->GetHeight();
    const float aspect = float(width) / height;
    const float zoomIn = ControlMapper::GetAnalogInput(ControlMapper::kC_MapZoomIn, input);
    const float zoomOut = ControlMapper::GetAnalogInput(ControlMapper::kC_MapZoomOut, input);
    const CVector2f oldSize = x20_vpSize;
    if (zoomOut - zoomIn != 0.f) {
      CVector2f offset = x20_vpSize;
      const float delta = gpTweakSlideShow->GetZoomSpeed() * (zoomOut - zoomIn);
      x20_vpSize[0] += aspect * delta;
      x20_vpSize[1] += delta;
      x20_vpSize[0] = CMath::Clamp(float(width), x20_vpSize.GetX(), x28_canvasSize.GetX());
      x20_vpSize[1] = CMath::Clamp(float(height), x20_vpSize.GetY(), x28_canvasSize.GetY());
      offset -= x20_vpSize;
      offset /= 2.f;
      x18_vpOffset += offset;
    }
    x0_parent->SetZoomSfx(!(oldSize == x20_vpSize));
    const CVector2f oldOffset = x18_vpOffset;
    const float forward = ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveForward, input);
    const float back = ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveBack, input);
    const float left = ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveLeft, input);
    const float right = ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveRight, input);
    const float speed = gpTweakSlideShow->GetPanSpeed();
    x18_vpOffset[0] -= speed * left;
    x18_vpOffset[0] += speed * right;
    x18_vpOffset[1] += speed * forward;
    x18_vpOffset[1] -= speed * back;
    x18_vpOffset[0] =
        CMath::Clamp(0.f, x18_vpOffset.GetX(), x28_canvasSize.GetX() - x20_vpSize.GetX());
    x18_vpOffset[1] =
        CMath::Clamp(0.f, x18_vpOffset.GetY(), x28_canvasSize.GetY() - x20_vpSize.GetY());
    float halfX = 0.f;
    const float availableX = texWidth / 2.f - x20_vpSize.GetX() / 2.f;
    if (availableX > halfX) {
      halfX = availableX;
    }
    float halfY = 0.f;
    const float availableY = texHeight / 2.f - x20_vpSize.GetY() / 2.f;
    if (availableY > halfY) {
      halfY = availableY;
    }
    x18_vpOffset[0] = CMath::Clamp(x28_canvasSize.GetX() / 2.f - halfX,
                                   x20_vpSize.GetX() / 2.f + x18_vpOffset.GetX(),
                                   x28_canvasSize.GetX() / 2.f + halfX) -
                      x20_vpSize.GetX() / 2.f;
    x18_vpOffset[1] = CMath::Clamp(x28_canvasSize.GetY() / 2.f - halfY,
                                   x20_vpSize.GetY() / 2.f + x18_vpOffset.GetY(),
                                   x28_canvasSize.GetY() / 2.f + halfY) -
                      x20_vpSize.GetY() / 2.f;
    x0_parent->SetPanSfx(!(oldOffset == x18_vpOffset));
  }
  return kMR_Exit;
}

void CSlideShow::SSlideData::InitializeViewport() {
  if (IsLoaded()) {
    const CTexture* texture = **xc_texture;
    const float width = texture->GetWidth();
    const float height = texture->GetHeight();
    const float texAspect = width / height;
    const float aspect = float(CGraphics::GetViewportWidth()) / CGraphics::GetViewportHeight();
    x18_vpOffset = sZeroVector;
    if (texAspect != aspect) {
      if (texAspect > aspect) {
        x20_vpSize = CVector2f(width, width / aspect);
      } else {
        x20_vpSize = CVector2f(height * aspect, height);
      }
    }
    x28_canvasSize = x20_vpSize;
    x14_ready = true;
  }
}

void CSlideShow::SSlideData::Draw() const {
  if (IsReady()) {
    const int width = (**xc_texture)->GetWidth();
    const int height = (**xc_texture)->GetHeight();
    const CVector3f offset((x28_canvasSize.GetX() - width) / 2.f, 0.f,
                           (x28_canvasSize.GetY() - height) / 2.f);
    DrawTexture(xc_texture, offset, x30_mulColor, &x18_vpOffset, &x20_vpSize);
  }
}

void CSlideShow::SSlideData::Reset() {
  x4_gallery = -1;
  x8_slide = -1;
  xc_texture = rstl::auto_ptr< TToken< CTexture > >();
  x14_ready = false;
  x18_vpOffset = sZeroVector;
  x20_vpSize = sZeroVector;
  x28_canvasSize = sZeroVector;
  x30_mulColor = CColor::White();
  x30_mulColor.SetAlpha(0.f);
}

bool CSlideShow::GetIsContinueDraw() const { return false; }
