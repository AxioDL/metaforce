#include "Runtime/GuiSys/CSplashScreen.hpp"

#include "CArchitectureMessage.hpp"
#include "CArchitectureQueue.hpp"
#include "CSimplePool.hpp"
#include "GameGlobalObjects.hpp"
#include "Graphics/CCubeRenderer.hpp"

namespace metaforce {

constexpr std::array SplashTextures{"TXTR_NintendoLogo"sv, "TXTR_RetroLogo"sv, "TXTR_DolbyLogo"sv};

CSplashScreen::CSplashScreen(ESplashScreen which)
: CIOWin("SplashScreen"), x14_which(which), x28_texture(g_SimplePool->GetObj(SplashTextures[size_t(which)])) {}

CIOWin::EMessageReturn CSplashScreen::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue) {
  switch (msg.GetType()) {
  case EArchMsgType::TimerTick: {
    if (!x25_textureLoaded) {
      if (!x28_texture.IsLoaded())
        return EMessageReturn::Exit;
      x25_textureLoaded = true;
    }

    float dt = MakeMsg::GetParmTimerTick(msg).x4_parm;
    x18_splashTimeout -= dt;

    if (x18_splashTimeout <= 0.f) {
      /* HACK: If we're not compiling with Intel's IPP library we want to skip the Dolby Pro Logic II logo
       * This is purely a URDE addition and does not reflect retro's intentions. - Phil
       */
#if INTEL_IPP
      if (x14_which != ESplashScreen::Dolby)
#else
      if (x14_which != ESplashScreen::Retro)
#endif
        queue.Push(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 9999, 9999,
                                              std::make_shared<CSplashScreen>(ESplashScreen(int(x14_which) + 1))));
      return EMessageReturn::RemoveIOWinAndExit;
    }
    break;
  }
  default:
    break;
  }
  return EMessageReturn::Exit;
}

void CSplashScreen::Draw() {
  if (!x25_textureLoaded) {
    return;
  }
  SCOPED_GRAPHICS_DEBUG_GROUP("CSplashScreen::Draw", zeus::skGreen);

  zeus::CColor color = zeus::skWhite;
  if (x14_which == ESplashScreen::Nintendo) {
    color = zeus::CColor{0.86f, 0.f, 0.f, 1.f};
  }

  if (x18_splashTimeout > 1.5f) {
    color.a() = 1.f - (x18_splashTimeout - 1.5f) * 2.f;
  } else if (x18_splashTimeout < 0.5f) {
    color.a() = x18_splashTimeout * 2.f;
  }

  CGraphics::SetAlphaCompare(ERglAlphaFunc::Always, 0, ERglAlphaOp::And, ERglAlphaFunc::Always, 0);
  g_Renderer->SetModelMatrix({});
  CGraphics::SetViewPointMatrix({});
  CGraphics::SetTevOp(ERglTevStage::Stage0, CTevCombiners::kEnvModulate);
  CGraphics::SetTevOp(ERglTevStage::Stage1, CTevCombiners::kEnvPassthru);
  g_Renderer->SetBlendMode_AlphaBlended();
  auto& tex = *x28_texture.GetObj();
  const auto width = tex.GetWidth();
  const auto height = tex.GetHeight();
  tex.Load(GX_TEXMAP0, EClampMode::Clamp);
  if (x14_which == ESplashScreen::Nintendo || x14_which == ESplashScreen::Retro) {
    const auto x = static_cast<float>(133 - (width - 376) / 2);
    const auto y = static_cast<float>(170 - (height - 104) / 2);
    CGraphics::SetOrtho(-10.f, 650.f, -5.5f, 484.5f, -1.f, 1.f);
    CGraphics::SetCullMode(ERglCullMode::None);
    CGraphics::StreamBegin(GX_TRIANGLESTRIP);
    CGraphics::StreamColor(color);
    CGraphics::StreamTexcoord(0.f, 0.f);
    CGraphics::StreamVertex({x, 0.f, y + static_cast<float>(height)});
    CGraphics::StreamTexcoord(0.f, 1.f);
    CGraphics::StreamVertex({x, 0.f, y});
    CGraphics::StreamTexcoord(1.f, 0.f);
    CGraphics::StreamVertex({x + static_cast<float>(width), 0.f, y + static_cast<float>(height)});
    CGraphics::StreamTexcoord(1.f, 1.f);
    CGraphics::StreamVertex({x + static_cast<float>(width), 0.f, y});
    CGraphics::StreamEnd();
    CGraphics::SetCullMode(ERglCullMode::Front);
  } else {
    // TODO originally uses CGraphics viewport, but Render2D needs scaling fix
    CGraphics::Render2D(tex, 0, 0, 640, 448, color);
  }

  // Progressive scan options omitted
}

} // namespace metaforce
