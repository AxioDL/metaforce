#include "Runtime/GuiSys/CSplashScreen.hpp"

#include <array>

#include "Runtime/CArchitectureMessage.hpp"
#include "Runtime/CArchitectureQueue.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Camera/CCameraFilter.hpp"

namespace metaforce {

constexpr std::array SplashTextures{"TXTR_NintendoLogo"sv, "TXTR_RetroLogo"sv, "TXTR_DolbyLogo"sv};

CSplashScreen::CSplashScreen(ESplashScreen which)
: CIOWin("SplashScreen")
, x14_which(which)
, m_texture(g_SimplePool->GetObj(SplashTextures[size_t(which)])) {}

CIOWin::EMessageReturn CSplashScreen::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue) {
  switch (msg.GetType()) {
  case EArchMsgType::TimerTick: {
    if (!x25_textureLoaded) {
      if (!m_texture.IsLoaded())
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

  zeus::CColor color;
  if (x14_which == ESplashScreen::Nintendo) {
    color = zeus::CColor{0.86f, 0.f, 0.f, 1.f};
  }

  if (x18_splashTimeout > 1.5f) {
    color.a() = 1.f - (x18_splashTimeout - 1.5f) * 2.f;
  } else if (x18_splashTimeout < 0.5f) {
    color.a() = x18_splashTimeout * 2.f;
  }

  zeus::CRectangle rect;
  rect.size.x() = m_texture->GetWidth() / (480.f * g_Viewport.aspect);
  rect.size.y() = m_texture->GetHeight() / 480.f;
  rect.position.x() = 0.5f - rect.size.x() / 2.f;
  rect.position.y() = 0.5f - rect.size.y() / 2.f;

  aurora::shaders::queue_textured_quad(
      aurora::shaders::CameraFilterType::Blend,
      m_texture->GetTexture()->ref,
      aurora::shaders::ZTest::None,
      color,
      1.f,
      rect,
      0.f
  );
}

} // namespace metaforce
