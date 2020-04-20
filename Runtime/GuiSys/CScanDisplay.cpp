#include "Runtime/GuiSys/CScanDisplay.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Audio/CSfxManager.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/GuiSys/CAuiImagePane.hpp"
#include "Runtime/GuiSys/CGuiCamera.hpp"
#include "Runtime/GuiSys/CGuiModel.hpp"
#include "Runtime/GuiSys/CGuiTextPane.hpp"
#include "Runtime/GuiSys/CGuiWidget.hpp"
#include "Runtime/GuiSys/CStringTable.hpp"
#include "Runtime/Input/CFinalInput.hpp"
#include "Runtime/MP1/CPauseScreenBase.hpp"

#include <zeus/CTransform.hpp>

namespace urde {
void CScanDisplay::CDataDot::Update(float dt) {
  if (x20_remTime > 0.f) {
    x20_remTime = std::max(0.f, x20_remTime - dt);
    float d = 0.f;
    if (x1c_transDur > 0.f)
      d = x20_remTime / x1c_transDur;
    xc_curPos = zeus::CVector2f::lerp(x14_targetPos, x4_startPos, d);
  }

  if (x24_alpha > x28_desiredAlpha) {
    float tmp = x24_alpha - 2.f * dt;
    x24_alpha = std::max(tmp, x28_desiredAlpha);
  } else if (x24_alpha < x28_desiredAlpha) {
    float tmp = 2.f * dt + x24_alpha;
    x24_alpha = std::min(tmp, x28_desiredAlpha);
  }
}

void CScanDisplay::CDataDot::Draw(const zeus::CColor& col, float radius) {
  if (x24_alpha == 0.f) {
    return;
  }

  if (x0_dotState != EDotState::Hidden) {
    const zeus::CTransform xf = zeus::CTransform::Translate(xc_curPos.x(), 0.f, xc_curPos.y());
    CGraphics::SetModelMatrix(xf);
    zeus::CColor useColor = col;
    useColor.a() *= x24_alpha;
    const std::array<CTexturedQuadFilter::Vert, 4> verts{{
        {{-radius, 0.f, radius}, {0.f, 1.f}},
        {{-radius, 0.f, -radius}, {0.f, 0.f}},
        {{radius, 0.f, radius}, {1.f, 1.f}},
        {{radius, 0.f, -radius}, {1.f, 0.f}},
    }};
    m_quad.drawVerts(useColor, verts);
  }
}

void CScanDisplay::CDataDot::StartTransitionTo(const zeus::CVector2f& vec, float dur) {
  x20_remTime = dur;
  x1c_transDur = dur;
  x4_startPos = xc_curPos;
  x14_targetPos = vec;
}

void CScanDisplay::CDataDot::SetDestPosition(const zeus::CVector2f& pos) {
  if (x20_remTime <= 0.f)
    xc_curPos = pos;
  else
    x14_targetPos = pos;
}

CScanDisplay::CScanDisplay(const CGuiFrame& selHud) : xa0_selHud(selHud) {
  x0_dataDot = g_SimplePool->GetObj("TXTR_DataDot");
  for (size_t i = 0; i < xbc_dataDots.capacity(); ++i) {
    xbc_dataDots.emplace_back(x0_dataDot);
  }
  x170_paneStates.resize(x170_paneStates.capacity());
}

void CScanDisplay::ProcessInput(const CFinalInput& input) {
  if (xc_state == EScanState::Inactive || xc_state == EScanState::Done)
    return;

  if (xc_state == EScanState::DownloadComplete && x1a4_xAlpha == 0.f) {
    if (input.PA() || input.PSpecialKey(boo::ESpecialKey::Enter) || input.PMouseButton(boo::EMouseButton::Primary)) {
      if (xa8_message->TextSupport().GetCurTime() < xa8_message->TextSupport().GetTotalAnimationTime()) {
        xa8_message->TextSupport().SetCurTime(xa8_message->TextSupport().GetTotalAnimationTime());
      } else {
        xc_state = EScanState::ViewingScan;
        x1a4_xAlpha = 1.f;
        CSfxManager::SfxStart(SFXui_scan_next_page, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
      }
    }
  } else if (xc_state == EScanState::ViewingScan) {
    int oldCounter = x1ac_pageCounter;
    int totalPages = xac_scrollMessage->TextSupport().GetTotalPageCount();
    if ((input.PA() || input.PSpecialKey(boo::ESpecialKey::Enter) || input.PMouseButton(boo::EMouseButton::Primary)) &&
        totalPages != -1) {
      CGuiTextSupport& supp = !x1ac_pageCounter ? xa8_message->TextSupport() : xac_scrollMessage->TextSupport();
      if (supp.GetCurTime() < supp.GetTotalAnimationTime())
        supp.SetCurTime(supp.GetTotalAnimationTime());
      else
        x1ac_pageCounter = std::min(totalPages, x1ac_pageCounter + 1);
    }
    if (x1ac_pageCounter != oldCounter) {
      CSfxManager::SfxStart(SFXui_scan_next_page, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
      if (x1ac_pageCounter == 0) {
        xa8_message->SetIsVisible(true);
        xac_scrollMessage->SetIsVisible(false);
      } else {
        if (oldCounter == 0) {
          xa8_message->SetIsVisible(false);
          xac_scrollMessage->SetIsVisible(true);
        }
        xac_scrollMessage->TextSupport().SetPage(x1ac_pageCounter - 1);
        SetScanMessageTypeEffect(xac_scrollMessage, !x1b4_scanComplete);
      }
    }
  }

  float xAlpha = 0.f;
  float aAlpha = 0.f;
  float dashAlpha = 0.f;
  if (xc_state == EScanState::DownloadComplete) {
    xAlpha = std::min(2.f * x1a4_xAlpha, 1.f);
    aAlpha = (1.f - xAlpha) * std::fabs(x1b0_aPulse);
  } else if (xc_state == EScanState::ViewingScan) {
    if (x1ac_pageCounter < xac_scrollMessage->TextSupport().GetTotalPageCount())
      aAlpha = std::fabs(x1b0_aPulse);
    else
      dashAlpha = 1.f;
  }

  xb0_xmark->SetVisibility(xAlpha > 0.f, ETraversalMode::Children);
  xb4_abutton->SetVisibility(aAlpha > 0.f, ETraversalMode::Children);
  xb8_dash->SetVisibility(dashAlpha > 0.f, ETraversalMode::Children);

  xb0_xmark->SetColor(zeus::CColor(1.f, xAlpha));
  xb4_abutton->SetColor(zeus::CColor(1.f, aAlpha));
  xb8_dash->SetColor(zeus::CColor(0.53f, 0.84f, 1.f, dashAlpha));
}

float CScanDisplay::GetDownloadStartTime(size_t idx) const {
  if (!x14_scannableInfo) {
    return 0.f;
  }

  const float nTime = x14_scannableInfo->GetBucket(idx).x4_appearanceRange;
  float maxTime = 0.f;
  for (size_t i = 0; i < CScannableObjectInfo::NumBuckets; ++i) {
    const float iTime = x14_scannableInfo->GetBucket(i).x4_appearanceRange;
    if (iTime < nTime) {
      maxTime = std::max(iTime, maxTime);
    }
  }

  return maxTime + g_tweakGui->GetScanAppearanceDuration();
}

float CScanDisplay::GetDownloadFraction(size_t idx, float scanningTime) const {
  if (!x14_scannableInfo) {
    return 0.f;
  }

  const float appearTime = GetDownloadStartTime(idx);
  const float appearRange = x14_scannableInfo->GetBucket(idx).x4_appearanceRange;
  if (appearTime == appearRange) {
    return 1.f;
  }

  return zeus::clamp(0.f, (scanningTime - appearTime) / (appearRange - appearTime), 1.f);
}

void CScanDisplay::StartScan(TUniqueId id, const CScannableObjectInfo& scanInfo, CGuiTextPane* message,
                             CGuiTextPane* scrollMessage, CGuiWidget* textGroup, CGuiModel* xmark, CGuiModel* abutton,
                             CGuiModel* dash, float scanTime) {
  x1b4_scanComplete = scanTime >= scanInfo.GetTotalDownloadTime();
  x14_scannableInfo.emplace(scanInfo);
  x10_objId = id;
  xc_state = EScanState::Downloading;
  x1ac_pageCounter = 0;
  x1a4_xAlpha = 0.f;
  xa8_message = message;
  xac_scrollMessage = scrollMessage;
  xa4_textGroup = textGroup;
  xb0_xmark = xmark;
  xb4_abutton = abutton;
  xb8_dash = dash;
  xa4_textGroup->SetVisibility(true, ETraversalMode::Children);
  xa4_textGroup->SetColor(zeus::CColor(1.f, 0.f));
  xa8_message->TextSupport().SetText(u"");
  xac_scrollMessage->TextSupport().SetText(u"");

  for (size_t i = 0; i < 20; ++i) {
    auto* pane = static_cast<CAuiImagePane*>(xa0_selHud.FindWidget(MP1::CPauseScreenBase::GetImagePaneName(i)));
    zeus::CColor color = g_tweakGuiColors->GetScanDisplayImagePaneColor();
    color.a() = 0.f;
    pane->SetColor(color);
    pane->SetTextureID0({}, g_SimplePool);
    pane->SetAnimationParms(zeus::skZero2f, 0.f, 0.f);
    size_t pos = SIZE_MAX;
    for (size_t j = 0; j < CScannableObjectInfo::NumBuckets; ++j) {
      if (x14_scannableInfo->GetBucket(j).x8_imagePos == i) {
        pos = j;
        break;
      }
    }
    if (pos != SIZE_MAX) {
      x170_paneStates[pos].second = pane;
    }
  }

  for (size_t i = 0; i < x170_paneStates.size(); ++i) {
    std::pair<float, CAuiImagePane*>& state = x170_paneStates[i];
    if (state.second) {
      const CScannableObjectInfo::SBucket& bucket = x14_scannableInfo->GetBucket(i);
      if (bucket.x14_interval > 0.f) {
        state.second->SetAnimationParms(zeus::CVector2f(bucket.xc_size.x, bucket.xc_size.y), bucket.x14_interval,
                                        bucket.x18_fadeDuration);
      }
      state.second->SetTextureID0(bucket.x0_texture, g_SimplePool);
      state.second->SetFlashFactor(0.f);

      const float startTime = GetDownloadStartTime(i);
      if (scanTime >= startTime) {
        x170_paneStates[i].first = 0.f;
      } else {
        x170_paneStates[i].first = -1.f;
      }
    }
  }

  const CAssetId strId = x14_scannableInfo->GetStringTableId();
  if (strId.IsValid()) {
    x194_scanStr = g_SimplePool->GetObj({FOURCC('STRG'), strId});
  }

  for (size_t i = 0; i < CScannableObjectInfo::NumBuckets; ++i) {
    const u32 pos = x14_scannableInfo->GetBucket(i).x8_imagePos;
    CDataDot& dot = xbc_dataDots[i];
    if (pos != UINT32_MAX) {
      if (GetDownloadStartTime(i) - g_tweakGui->GetScanAppearanceDuration() < scanTime) {
        dot.SetAlpha(0.f);
        dot.SetDotState(CDataDot::EDotState::Done);
      } else {
        dot.SetDesiredAlpha(1.f);
        dot.SetDotState(CDataDot::EDotState::Seek);
        dot.StartTransitionTo(zeus::skZero2f, FLT_EPSILON);
        dot.Update(FLT_EPSILON);
      }
    } else {
      dot.SetDotState(CDataDot::EDotState::Hidden);
    }
  }
}

void CScanDisplay::StopScan() {
  if (xc_state == EScanState::Done || xc_state == EScanState::Inactive) {
    return;
  }

  xc_state = EScanState::Done;
  for (auto& dataDot : xbc_dataDots) {
    dataDot.SetDesiredAlpha(0.f);
  }
}

void CScanDisplay::SetScanMessageTypeEffect(CGuiTextPane* pane, bool type) {
  if (type)
    pane->TextSupport().SetTypeWriteEffectOptions(true, 0.1f, 60.f);
  else
    pane->TextSupport().SetTypeWriteEffectOptions(false, 0.f, 0.f);
}

void CScanDisplay::Update(float dt, float scanningTime) {
  if (xc_state == EScanState::Inactive)
    return;

  bool active = false;
  if (xc_state == EScanState::Done) {
    x1a8_bodyAlpha = std::max(0.f, x1a8_bodyAlpha - 2.f * dt);
    if (x1a8_bodyAlpha > 0.f)
      active = true;
  } else {
    active = true;
    x1a8_bodyAlpha = std::min(x1a8_bodyAlpha + 2.f * dt, 1.f);
    if (xc_state == EScanState::DownloadComplete) {
      if (xac_scrollMessage->TextSupport().GetIsTextSupportFinishedLoading())
        x1a4_xAlpha = std::max(0.f, x1a4_xAlpha - dt);
      if (x1a4_xAlpha < 0.5f) {
        x1b0_aPulse += 2.f * dt;
        if (x1b0_aPulse > 1.f)
          x1b0_aPulse -= 2.f;
      }
    } else if (xc_state == EScanState::ViewingScan) {
      x1b0_aPulse += 2.f * dt;
      if (x1b0_aPulse > 1.f)
        x1b0_aPulse -= 2.f;
      if (x1a4_xAlpha == 1.f) {
        xa8_message->TextSupport().SetText(x194_scanStr->GetString(0));
        SetScanMessageTypeEffect(xa8_message, !x1b4_scanComplete);
      }
    } else if (xc_state == EScanState::Downloading && scanningTime >= x14_scannableInfo->GetTotalDownloadTime() &&
               x194_scanStr.IsLoaded()) {
      if (x1b4_scanComplete || x14_scannableInfo->GetCategory() == 0) {
        xc_state = EScanState::ViewingScan;
        x1b0_aPulse = x1a4_xAlpha = 1.f;
        CSfxManager::SfxStart(SFXui_scan_complete, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
      } else {
        xc_state = EScanState::DownloadComplete;
        x1b0_aPulse = x1a4_xAlpha = 1.f;
        xa8_message->TextSupport().SetText(std::u16string(g_MainStringTable->GetString(29)) +
                                           g_MainStringTable->GetString(x14_scannableInfo->GetCategory() + 30) +
                                           g_MainStringTable->GetString(30));
        SetScanMessageTypeEffect(xa8_message, true);
        CSfxManager::SfxStart(SFXui_new_scan_complete, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
      }

      if (x194_scanStr->GetStringCount() > 2) {
        xac_scrollMessage->TextSupport().SetText(x194_scanStr->GetString(2), true);
        SetScanMessageTypeEffect(xac_scrollMessage, !x1b4_scanComplete);
      }
      xac_scrollMessage->SetIsVisible(false);
    }
  }

  for (size_t i = 0; i < x170_paneStates.size(); ++i) {
    if (x170_paneStates[i].second == nullptr) {
      continue;
    }

    if (x170_paneStates[i].first > 0.f) {
      x170_paneStates[i].first = std::max(0.f, x170_paneStates[i].first - dt);
      float tmp;
      if (x170_paneStates[i].first > g_tweakGui->GetScanPaneFadeOutTime()) {
        tmp = 1.f -
              (x170_paneStates[i].first - g_tweakGui->GetScanPaneFadeOutTime()) / g_tweakGui->GetScanPaneFadeInTime();
      } else {
        tmp = x170_paneStates[i].first / g_tweakGui->GetScanPaneFadeOutTime();
      }
      x170_paneStates[i].second->SetFlashFactor(tmp * g_tweakGui->GetScanPaneFlashFactor() * x1a8_bodyAlpha);
    }

    const float alphaMul =
        ((xc_state == EScanState::Downloading) ? GetDownloadFraction(i, scanningTime) : 1.f) * x1a8_bodyAlpha;
    zeus::CColor color = g_tweakGuiColors->GetScanDisplayImagePaneColor();
    color.a() *= alphaMul;
    x170_paneStates[i].second->SetColor(color);
    x170_paneStates[i].second->SetDeResFactor(1.f - alphaMul);

    if (GetDownloadStartTime(i) - g_tweakGui->GetScanAppearanceDuration() < scanningTime) {
      CDataDot& dot = xbc_dataDots[i];
      switch (dot.GetDotState()) {
      case CDataDot::EDotState::Seek:
      case CDataDot::EDotState::Hold:
        dot.SetDotState(CDataDot::EDotState::RevealPane);
        dot.StartTransitionTo(zeus::skZero2f, g_tweakGui->GetScanAppearanceDuration());
        break;
      case CDataDot::EDotState::RevealPane: {
        const float tmp = dot.GetTransitionFactor();
        if (tmp == 0.f) {
          dot.SetDotState(CDataDot::EDotState::Done);
          dot.SetDesiredAlpha(0.f);
          CSfxManager::SfxStart(SFXui_scan_pane_reveal, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
          x170_paneStates[i].first = g_tweakGui->GetScanPaneFadeOutTime() + g_tweakGui->GetScanPaneFadeInTime();
        }
        break;
      }
      default:
        break;
      }
    }
  }

  for (size_t i = 0; i < xbc_dataDots.size(); ++i) {
    CDataDot& dot = xbc_dataDots[i];
    switch (dot.GetDotState()) {
    case CDataDot::EDotState::Hidden:
      continue;
    case CDataDot::EDotState::Seek:
    case CDataDot::EDotState::Hold: {
      float tmp = dot.GetTransitionFactor();
      if (tmp == 0.f) {
        float vpRatio = g_Viewport.xc_height / 480.f;
        float posRand = g_tweakGui->GetScanDataDotPosRandMagnitude() * vpRatio;
        float durMin = dot.GetDotState() == CDataDot::EDotState::Hold ? g_tweakGui->GetScanDataDotHoldDurationMin()
                                                                      : g_tweakGui->GetScanDataDotSeekDurationMin();
        float durMax = dot.GetDotState() == CDataDot::EDotState::Hold ? g_tweakGui->GetScanDataDotHoldDurationMax()
                                                                      : g_tweakGui->GetScanDataDotSeekDurationMax();
        zeus::CVector2f vec(
            dot.GetDotState() == CDataDot::EDotState::Hold ? dot.GetCurrPosition().x()
                                                           : (posRand * (rand() / float(RAND_MAX)) - 0.5f * posRand),
            dot.GetDotState() == CDataDot::EDotState::Hold ? dot.GetCurrPosition().y()
                                                           : (posRand * (rand() / float(RAND_MAX)) - 0.5f * posRand));
        float dur = (durMax - durMin) * (rand() / float(RAND_MAX)) + durMin;
        dot.StartTransitionTo(vec, dur);
        dot.SetDotState(dot.GetDotState() == CDataDot::EDotState::Hold ? CDataDot::EDotState::Seek
                                                                       : CDataDot::EDotState::Hold);
      }
      break;
    }
    case CDataDot::EDotState::RevealPane:
    case CDataDot::EDotState::Done: {
      const zeus::CVector3f& panePos = x170_paneStates[i].second->GetWorldPosition();
      zeus::CVector3f screenPos = xa0_selHud.GetFrameCamera()->ConvertToScreenSpace(panePos);
      zeus::CVector2f viewportCoords(screenPos.x() * g_Viewport.x8_width * 0.5f,
                                     screenPos.y() * g_Viewport.xc_height * 0.5f);
      dot.SetDestPosition(viewportCoords);
      break;
    }
    default:
      break;
    }
    dot.Update(dt);
  }

  if (!active) {
    xc_state = EScanState::Inactive;
    x10_objId = kInvalidUniqueId;
    x14_scannableInfo = std::nullopt;
    xa8_message->TextSupport().SetText(u"");
    xac_scrollMessage->TextSupport().SetText(u"");
    xa4_textGroup->SetVisibility(false, ETraversalMode::Children);
    xb0_xmark->SetVisibility(false, ETraversalMode::Children);
    xb4_abutton->SetVisibility(false, ETraversalMode::Children);
    xb8_dash->SetVisibility(false, ETraversalMode::Children);
    xa8_message = nullptr;
    xac_scrollMessage = nullptr;
    xa4_textGroup = nullptr;
    xb0_xmark = nullptr;
    xb4_abutton = nullptr;
    xb8_dash = nullptr;
    x170_paneStates.clear();
    x170_paneStates.resize(4);
    x194_scanStr = TLockedToken<CStringTable>();
    x1ac_pageCounter = 0;
    x1b4_scanComplete = false;
  } else {
    xa4_textGroup->SetColor(zeus::CColor(1.f, x1a8_bodyAlpha));
  }
}

void CScanDisplay::Draw() {
  if (!x0_dataDot.IsLoaded()) {
    return;
  }

  // No Z-test or write
  g_Renderer->SetViewportOrtho(true, -4096.f, 4096.f);
  // Additive alpha

  const float vpRatio = g_Viewport.xc_height / 480.f;
  for (CDataDot& dot : xbc_dataDots) {
    dot.Draw(g_tweakGuiColors->GetScanDataDotColor(), g_tweakGui->GetScanDataDotRadius() * vpRatio);
  }
}
} // namespace urde
