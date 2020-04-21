#include "Runtime/MP1/CSlideShow.hpp"

#include "Editor/ProjectManager.hpp"

#include "Runtime/GameGlobalObjects.hpp"

namespace urde {

CSlideShow::CSlideShow() : CIOWin("SlideShow"), x130_(g_tweakSlideShow->GetX54()) {
  const SObjectTag* font = g_ResFactory->GetResourceIdByName(g_tweakSlideShow->GetFont());
  if (font) {
    CGuiTextProperties propsA(false, true, EJustification::Center, EVerticalJustification::Bottom);
    xc4_textA = std::make_unique<CGuiTextSupport>(font->id, propsA, g_tweakSlideShow->GetFontColor(),
                                                  g_tweakSlideShow->GetOutlineColor(), zeus::skWhite, 640, 480,
                                                  g_SimplePool, CGuiWidget::EGuiModelDrawFlags::Alpha);

    CGuiTextProperties propsB(false, true, EJustification::Right, EVerticalJustification::Bottom);
    xc8_textB = std::make_unique<CGuiTextSupport>(font->id, propsB, g_tweakSlideShow->GetFontColor(),
                                                  g_tweakSlideShow->GetOutlineColor(), zeus::skWhite, 640, 480,
                                                  g_SimplePool, CGuiWidget::EGuiModelDrawFlags::Alpha);
  }

  xf8_stickTextures.reserve(18);
  x108_buttonTextures.reserve(8);
  SObjectTag txtrTag(FOURCC('TXTR'), 0);
  for (const ITweakPlayerRes::ResId lStickId : g_tweakPlayerRes->x24_lStick) {
    txtrTag.id = lStickId;
    xf8_stickTextures.emplace_back(g_SimplePool->GetObj(txtrTag));
  }
  for (const ITweakPlayerRes::ResId cStickId : g_tweakPlayerRes->x4c_cStick) {
    txtrTag.id = cStickId;
    xf8_stickTextures.emplace_back(g_SimplePool->GetObj(txtrTag));
  }
  for (const ITweakPlayerRes::ResId lTriggerId : g_tweakPlayerRes->x74_lTrigger) {
    txtrTag.id = lTriggerId;
    xf8_stickTextures.emplace_back(g_SimplePool->GetObj(txtrTag));
  }
  for (const ITweakPlayerRes::ResId rTriggerId : g_tweakPlayerRes->x80_rTrigger) {
    txtrTag.id = rTriggerId;
    xf8_stickTextures.emplace_back(g_SimplePool->GetObj(txtrTag));
  }
  for (const ITweakPlayerRes::ResId bButtonId : g_tweakPlayerRes->xa4_bButton) {
    txtrTag.id = bButtonId;
    xf8_stickTextures.emplace_back(g_SimplePool->GetObj(txtrTag));
  }
  for (const ITweakPlayerRes::ResId yButtonId : g_tweakPlayerRes->xbc_yButton) {
    txtrTag.id = yButtonId;
    xf8_stickTextures.emplace_back(g_SimplePool->GetObj(txtrTag));
  }
}

bool CSlideShow::LoadTXTRDep(std::string_view name) {
  const SObjectTag* dgrpTag = g_ResFactory->GetResourceIdByName(name);
  if (dgrpTag && dgrpTag->type == FOURCC('DGRP')) {
    x18_galleryTXTRDeps.emplace_back(g_SimplePool->GetObj(*dgrpTag));
    return true;
  }
  return false;
}

bool CSlideShow::AreAllDepsLoaded(const std::vector<TLockedToken<CDependencyGroup>>& deps) {
  for (const TLockedToken<CDependencyGroup>& token : deps) {
    if (!token.IsLoaded())
      return false;
  }
  return true;
}

CIOWin::EMessageReturn CSlideShow::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue) {
  switch (msg.GetType()) {
  case EArchMsgType::TimerTick: {
    if (x134_29_)
      return EMessageReturn::RemoveIOWinAndExit;

    // float dt = MakeMsg::GetParmTimerTick(msg).x4_parm;

    switch (x14_phase) {
    case Phase::Zero: {
      // if (!g_resLoader->AreAllPaksLoaded())
      //{
      //    g_resLoader->AsyncIdlePakLoading();
      //    return EMessageReturn::Exit;
      //}
      x14_phase = Phase::One;
      [[fallthrough]];
    }
    case Phase::One: {
      if (x18_galleryTXTRDeps.empty()) {
        x18_galleryTXTRDeps.reserve(5);
        for (int i = 1; true; ++i) {
          std::string depResName = fmt::format(FMT_STRING("Gallery{:02d}_DGRP"), i);
          if (!LoadTXTRDep(depResName))
            break;
        }
        LoadTXTRDep("GalleryAssets_DGRP");
      }
      if (!AreAllDepsLoaded(x18_galleryTXTRDeps))
        return EMessageReturn::Exit;

      x14_phase = Phase::Three;
      [[fallthrough]];
    }
    case Phase::Two:
    case Phase::Three:
    case Phase::Four:
    case Phase::Five:
      break;
    default:
      break;
    }

    break;
  }
  case EArchMsgType::UserInput:
  default:
    break;
  }

  return EMessageReturn::Exit;
}

void CSlideShow::SSlideData::Draw() {
  if (!IsLoaded()) {
    return;
  }

  const zeus::CRectangle rect;
  m_texQuad->draw(x30_mulColor, 1.f, rect);

  const zeus::CVector2f centeredOffset((x28_canvasSize.x() - m_texQuad->GetTex()->GetWidth()) * 0.5f,
                                       (x28_canvasSize.y() - m_texQuad->GetTex()->GetHeight()) * 0.5f);
}

void CSlideShow::Draw() {
  SCOPED_GRAPHICS_DEBUG_GROUP("CSlideShow::Draw", zeus::skGreen);
  if (x14_phase == Phase::Five) {
    x5c_slideA.Draw();
    x90_slideB.Draw();
  }
}

u32 CSlideShow::SlideShowGalleryFlags() {
  u32 ret = 0;
  if (!g_GameState)
    return ret;
  if (g_GameState->SystemOptions().GetLogScanPercent() >= 50)
    ret |= 1;
  if (g_GameState->SystemOptions().GetLogScanPercent() == 100)
    ret |= 2;
  if (g_GameState->SystemOptions().GetPlayerBeatHardMode())
    ret |= 4;
  if (g_GameState->SystemOptions().GetAllItemsCollected())
    ret |= 8;
  return ret;
}

} // namespace urde
