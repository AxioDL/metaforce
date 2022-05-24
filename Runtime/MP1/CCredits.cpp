#include "Runtime/MP1/CCredits.hpp"

#include "Runtime/CArchitectureMessage.hpp"
#include "Runtime/CArchitectureQueue.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CCubeRenderer.hpp"
#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/Graphics/CMoviePlayer.hpp"
#include "Runtime/GuiSys/CGuiTextSupport.hpp"
#include "Runtime/GuiSys/CStringTable.hpp"
#include "Runtime/Input/CFinalInput.hpp"
#include "Runtime/MP1/CPlayMovie.hpp"

namespace metaforce::MP1 {
static logvisor::Module Log("CCredits");

CCredits::CCredits()
: CIOWin("Credits")
, x18_creditsTable(g_SimplePool->GetObj(g_tweakGui->GetCreditsTable()))
, x20_creditsFont(g_SimplePool->GetObj(g_tweakGui->GetJapaneseCreditsFont()))
, x54_(g_tweakGui->x30c_) {
  x18_creditsTable.Lock();
  x20_creditsFont.Lock();
}

CIOWin::EMessageReturn CCredits::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue) {
  switch (msg.GetType()) {
  case EArchMsgType::UserInput: {
    return ProcessUserInput(MakeMsg::GetParmUserInput(msg).x4_parm);
    break;
  case EArchMsgType::TimerTick: {
    return Update(MakeMsg::GetParmTimerTick(msg).x4_parm, queue);
  }
  default:
    break;
  }
  }
  return EMessageReturn::Normal;
}

void CCredits::Draw() {
  if (x14_ != 3) {
    return;
  }
  SCOPED_GRAPHICS_DEBUG_GROUP("CCredits::Draw", zeus::skGreen);
  DrawVideo();
  DrawText();
}

CIOWin::EMessageReturn CCredits::Update(float dt, CArchitectureQueue& queue) {
  switch (x14_) {
  case 0: {
    if (!x18_creditsTable || !x20_creditsFont) {
      return EMessageReturn::Exit;
    }
    if (x30_text.empty()) {
      for (size_t i = 0; i < x18_creditsTable->GetStringCount(); ++i) {
        x30_text.emplace_back(std::make_unique<CGuiTextSupport>(
                                  g_ResFactory->GetResourceIdByName(g_tweakGui->GetCreditsFont())->id,
                                  CGuiTextProperties(true, true, EJustification::Center, EVerticalJustification::Top),
                                  g_tweakGui->GetCreditsTextFontColor(), g_tweakGui->GetCreditsTextBorderColor(),
                                  zeus::skWhite, CGraphics::GetViewportWidth() - 64, 0, g_SimplePool,
                                  CGuiWidget::EGuiModelDrawFlags::Alpha),
                              zeus::CVector2i(0, 0));
        x30_text.back().first->SetText(x18_creditsTable->GetString(i));
      }

      //      auto tmp = std::make_pair(std::make_unique<CGuiTextSupport>(
      //                                    g_ResFactory->GetResourceIdByName(g_tweakGui->GetCreditsFont())->id,
      //                                    CGuiTextProperties(true, true, EJustification::Center,
      //                                    EVerticalJustification::Top), g_tweakGui->GetCreditsTextFontColor(),
      //                                    g_tweakGui->GetCreditsTextBorderColor(), zeus::skWhite, g_Viewport.x8_width
      //                                    - 64, 0, g_SimplePool, CGuiWidget::EGuiModelDrawFlags::Alpha),
      //                                zeus::CVector2i(0, 0));
      //      tmp.first->SetText(
      //          "\n&push;&font=C29C51F1;&main-color=#89D6FF;URDE DEVELOPMENT TEAM&pop;\n"
      //          "&push;&main-color=#89D6FF;LEAD REVERSE ENGINEERING TEAM&pop\n;"
      //          "Jack \"Cirrus\" Andersen\n"
      //          "Phillip \"Antidote\" Stephens\n"
      //          "Luke \"encounter\" Street\n\n"
      //          "&push;&main-color=#89D6FF;C++ COMPLIANCE & CLEANUP&pop;\n"
      //          "Lioncache\n");
      //      x30_text.insert(x30_text.end() - 1, std::move(tmp));
      //      x30_text.back().first->SetOutlineColor(g_tweakGui->GetCreditsTextBorderColor());
    }

    for (const auto& [text, offset] : x30_text) {
      if (!text->GetIsTextSupportFinishedLoading()) {
        return EMessageReturn::Exit;
      }
    }

    int scaleY = 0;
    for (auto& [text, offset] : x30_text) {
      auto bounds = text->GetBounds();
      offset.y = (bounds.second.y - bounds.first.y);
      offset.x = scaleY;
      text->SetExtentX(CGraphics::GetViewportWidth() - 1280);
      text->SetExtentY((bounds.second.y - bounds.first.y));
      scaleY += (bounds.second.y - bounds.first.y);
    }

    x4c_ = float(scaleY + CGraphics::GetViewportHeight() - 896); // * 0.5f;
    const float divVal = std::max(g_tweakGui->x310_, g_tweakGui->x30c_);
    x50_ = x4c_ / (g_tweakGui->x308_ - divVal);
    x14_ = 1;
    break;
  }
  case 1: {
    if (!x28_) {
      x28_ = std::make_unique<CMoviePlayer>("Video/creditBG.thp", 0.f, true, true);
    }
    x14_ = 2;
    break;
  }
  case 2: {
    if (!x2c_) {
      x2c_ = std::make_unique<CStaticAudioPlayer>("Audio/ending3.rsf", 0, 0x5d7c00);
    }
    if (!x2c_->IsReady()) {
      return EMessageReturn::Exit;
    }
    x2c_->SetVolume(1.f);
    x2c_->StartMixing();
    x14_ = 3;
  }
    [[fallthrough]];
  case 3: {
    // if (!x28_->PumpIndexLoad())
    //    break;
    x28_->Update(dt);
    if (x5c_24_) {
      x5c_28_ = true;
      if (x5c_27_) {
        x5c_27_ = false;
        x58_ = g_tweakGui->x310_ - x58_;
      }
    }
    if (x5c_27_ || x5c_28_) {
      x58_ = zeus::clamp(0.f, x58_ + dt, g_tweakGui->x310_);
      if (x58_ == g_tweakGui->x310_) {
        if (x5c_27_) {
          x5c_27_ = false;
          x58_ = 0.f;
        } else if (x5c_28_) {
          x5c_25_ = true;
        }
      }

      if (x58_ != 0.f && x5c_28_) {
        const float volume = zeus::clamp(0.f, 1.f - x58_ / g_tweakGui->x310_, 1.f);
        x2c_->SetVolume(volume);
      }
    }
    x48_ = std::min(x4c_, (dt * x50_) + x48_);

    if (x48_ == x4c_ || x5c_24_) {
      x5c_24_ = true;
      x54_ = std::max(0.f, x54_ - dt);
      const float alpha = x54_ / g_tweakGui->x30c_;
      for (const auto& [text, offset] : x30_text) {
        zeus::CColor col = zeus::skWhite;
        col.a() *= alpha;
        text->SetGeometryColor(col);
      }
      if (x54_ <= 0.f) {
        x5c_26_ = true;
      }
    }

    if (x5c_26_ && x5c_25_) {
      queue.Push(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 12, 11,
                                            std::make_shared<CPlayMovie>(CPlayMovie::EWhichMovie::AfterCredits)));
      return EMessageReturn::RemoveIOWinAndExit;
    }
    break;
  }
  default:
    break;
  }
  return EMessageReturn::Exit;
}

CIOWin::EMessageReturn CCredits::ProcessUserInput(const CFinalInput& input) {
  if (input.DA()) {
    x48_ = zeus::clamp(0.f, x48_ - ((x50_ * input.DeltaTime())), x4c_);
  } else {
    float leftY = input.ALeftY();
    float offset = 0.f;
    if (leftY < 0.f) {
      offset = -leftY;
      leftY = 0.f;
    }
    x48_ = zeus::clamp(0.f, x48_ - (leftY - offset) * 10.f * x50_ * input.DeltaTime(), x4c_);
  }
  return EMessageReturn::Exit;
}

void CCredits::DrawVideo() {
  /* Render movie */
  if (x28_ && x28_->DrawVideo() && (x5c_27_ || x5c_28_)) {
    float alpha = x58_ / g_tweakGui->x310_;
    if (x5c_27_) {
      alpha = 1.f - alpha;
    }

    alpha = zeus::clamp(0.f, alpha, 1.f);
    zeus::CColor filterCol = zeus::skBlack;
    filterCol.a() = alpha;
    CCameraFilterPass::DrawFilter(EFilterType::Blend, EFilterShape::Fullscreen, filterCol, nullptr, 1.f);
  }
}

void CCredits::DrawText() {
  float width = 896.f * CGraphics::GetViewportAspect();
  CGraphics::SetOrtho(0.f, width, 896.f, 0.f, -4096.f, 4096.f);
  auto region = std::make_pair(zeus::CVector2f{0.f, 0.f}, zeus::CVector2f{width, 896.f});
  CGraphics::SetViewPointMatrix(zeus::CTransform());
  CGraphics::SetModelMatrix(zeus::CTransform::Translate((width - 1280.f) / 2.f, 0.f, 896.f));
  float dVar5 = (x48_ - (region.second.y() - region.first.y()));
  for (const auto& [text, offset] : x30_text) {
    if (offset.y + offset.x >= dVar5 && offset.x <= x48_) {
      DrawText(*text, {0.5f * (region.second.x() - text->GetExtentX()), 0.f, x48_ - offset.x});
    }
  }
  CCameraFilterPass::DrawFilter(EFilterType::Multiply, EFilterShape::CinemaBars, zeus::skBlack, nullptr, 1.f);
}

void CCredits::DrawText(CGuiTextSupport& text, const zeus::CVector3f& translation) {
  CGraphics::SetCullMode(ERglCullMode::None);
  g_Renderer->SetViewportOrtho(true, -4096.f, 4096.f);
  g_Renderer->SetModelMatrix(zeus::CTransform::Translate(translation));
  g_Renderer->SetDepthReadWrite(false, false);
  text.Render();
}

} // namespace metaforce::MP1
