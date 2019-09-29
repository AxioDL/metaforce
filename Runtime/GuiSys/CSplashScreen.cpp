#include "Runtime/GuiSys/CSplashScreen.hpp"

#include "Runtime/CArchitectureMessage.hpp"
#include "Runtime/CArchitectureQueue.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Camera/CCameraFilter.hpp"

namespace urde {

static const char* SplashTextures[]{"TXTR_NintendoLogo", "TXTR_RetroLogo", "TXTR_DolbyLogo"};

CSplashScreen::CSplashScreen(ESplashScreen which)
: CIOWin("SplashScreen")
, x14_which(which)
, m_quad(EFilterType::Blend, g_SimplePool->GetObj(SplashTextures[int(which)])) {}

CIOWin::EMessageReturn CSplashScreen::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue) {
  switch (msg.GetType()) {
  case EArchMsgType::TimerTick: {
    if (!x25_textureLoaded) {
      if (!m_quad.GetTex().IsLoaded())
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

void CSplashScreen::Draw() const {
  if (!x25_textureLoaded)
    return;
  SCOPED_GRAPHICS_DEBUG_GROUP("CSplashScreen::Draw", zeus::skGreen);

  zeus::CColor color;
  if (x14_which == ESplashScreen::Nintendo)
    color = zeus::CColor{0.86f, 0.f, 0.f, 1.f};

  if (x18_splashTimeout > 1.5f)
    color.a() = 1.f - (x18_splashTimeout - 1.5f) * 2.f;
  else if (x18_splashTimeout < 0.5f)
    color.a() = x18_splashTimeout * 2.f;

  zeus::CRectangle rect;
  rect.size.x() = m_quad.GetTex()->GetWidth() / (480.f * g_Viewport.aspect);
  rect.size.y() = m_quad.GetTex()->GetHeight() / 480.f;
  rect.position.x() = 0.5f - rect.size.x() / 2.f;
  rect.position.y() = 0.5f - rect.size.y() / 2.f;

  const_cast<CTexturedQuadFilterAlpha&>(m_quad).draw(color, 1.f, rect);
}

} // namespace urde
