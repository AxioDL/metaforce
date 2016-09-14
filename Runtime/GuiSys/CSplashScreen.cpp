#include "CSplashScreen.hpp"
#include "CArchitectureMessage.hpp"
#include "CArchitectureQueue.hpp"
#include "CSimplePool.hpp"
#include "GameGlobalObjects.hpp"

namespace urde
{

static const char* SplashTextures[]
{
    "MP1/NoARAM/TXTR_NintendoLogo.png",
    "MP1/NoARAM/TXTR_RetroLogo.png",
    "MP1/NoARAM/TXTR_DolbyLogo.png"
};

CSplashScreen::CSplashScreen(ESplashScreen which)
: CIOWin("SplashScreen"), x14_which(which),
  m_quad(CCameraFilterPass::EFilterType::Blend,
         g_SimplePool->GetObj(SplashTextures[int(which)]))
{
}

CIOWin::EMessageReturn CSplashScreen::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue)
{
    switch (msg.GetType())
    {
    case EArchMsgType::TimerTick:
    {
        if (!x25_textureLoaded)
        {
            if (!m_quad.GetTex().IsLoaded())
                return EMessageReturn::Exit;
            x25_textureLoaded = true;
        }

        float dt = MakeMsg::GetParmTimerTick(msg).x4_parm;
        x18_splashTimeout -= dt;

        if (x18_splashTimeout <= 0.f)
        {
            if (x14_which != ESplashScreen::Dolby)
                queue.Push(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 9999, 9999,
                                                      new CSplashScreen(ESplashScreen(int(x14_which) + 1))));
            return EMessageReturn::RemoveIOWinAndExit;
        }
        break;
    }
    default: break;
    }
    return EMessageReturn::Exit;
}

void CSplashScreen::Draw() const
{
    if (!x25_textureLoaded)
        return;

    zeus::CColor color;
    if (x14_which == ESplashScreen::Nintendo)
        color = zeus::CColor{0.86f, 0.f, 0.f, 1.f};

    if (x18_splashTimeout > 1.5f)
        color.a = 1.f - (x18_splashTimeout - 1.5f) * 2.f;
    else if (x18_splashTimeout < 0.5f)
        color.a = x18_splashTimeout * 2.f;

    zeus::CRectangle rect;
    float aspect = CGraphics::g_ViewportResolution.x / float(CGraphics::g_ViewportResolution.y);
    rect.size.x = m_quad.GetTex()->GetWidth() / (640.f * aspect);
    rect.size.y = m_quad.GetTex()->GetHeight() / 480.f;
    rect.position.x = 0.5f - rect.size.x / 2.f;
    rect.position.y = 0.5f - rect.size.y / 2.f;

    const_cast<CTexturedQuadFilterAlpha&>(m_quad).draw(color, 1.f, rect);
}

}
