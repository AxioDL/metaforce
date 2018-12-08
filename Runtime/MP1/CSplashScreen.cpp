#include "CSplashScreen.hpp"
#include "Graphics/CGraphics.hpp"
#include "CSimplePool.hpp"
#include "CArchitectureQueue.hpp"
#include "CArchitectureMessage.hpp"

#include "zeus/CColor.hpp"
namespace urde {
extern CSimplePool* g_simplePool;
namespace MP1 {
static const char* SplashTextures[] = {"TXTR_NintendLogo", "TXTR_RetroLogo", "TXTR_DolbyLogo"};

CSplashScreen::CSplashScreen(ESplashScreen splash) : CIOWin("SplashScreen"), x14_splashScreen(splash) {
  x28_texture = g_simplePool->GetObj(SplashTextures[(u32)splash]);
}

CIOWin::EMessageReturn CSplashScreen::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue) {
  if (msg.GetType() == EArchMsgType::UserInput && x25_) {
    CFinalInput input = MakeMsg::GetParmUserInput(msg).x4_parm;
    if (x20_phase == 1) {
      if (input.x8_anaLeftX <= -0.7f || input.x2d_b26_DPLeft) {
        x24_progressiveEnabled = true;
        x1c_ = 10.f;
      } else if (input.x8_anaLeftX >= 0.7f || input.x2d_b24_DPRight) {
        x24_progressiveEnabled = false;
        x1c_ = 10.f;
      } else if (input.x2d_b28_PA || input.x2d_b27_Start) {
        // CGraphics::SetProgressiveMode(x24_progressiveEnabled);
        x1c_ = 5.f;
        x20_phase = 2;
      }
    } else if (x20_phase == 2) {
      if (input.x2d_b28_PA || input.x2e_b31_PStart)
        x1c_ = 0.f;
    }
  } else if (msg.GetType() == EArchMsgType::TimerTick) {
    if (!x25_) {
      if (x28_texture)
        x25_ = true;
    } else {
    }
  }

  return EMessageReturn::Exit;
}

void CSplashScreen::Draw() const {
  zeus::CColor col;
  if (x14_splashScreen == ESplashScreen::Nintendo)
    col = zeus::CColor(0.86f, 0.f, 0.f);

  float tmp = x18_;
  if (tmp <= 1.5f) {
    if (x18_ <= 0.5f)
      tmp /= 0.5f;
    else
      tmp = 1.0f;
  } else {
    tmp -= 1.5f;
    tmp = 1.0f - (tmp / 1.5f);
  }

  CGraphics::SetAlphaCompare(ERglAlphaFunc::Always, 0, ERglAlphaOp::And, ERglAlphaFunc::Always, 0);
}

} // namespace MP1
} // namespace urde
