#include "CSlideShow.hpp"
#include "GameGlobalObjects.hpp"
#include "Editor/ProjectManager.hpp"

namespace urde
{

CSlideShow::CSlideShow()
: CIOWin("SlideShow"), x5c_slideA(*this), x90_slideB(*this)
{
    x130_ = g_tweakSlideShow->GetX54();
    x134_24_ = true;
    x134_30_ = true;
    x135_24_ = true;

    const SObjectTag* font = g_ResFactory->GetResourceIdByName(g_tweakSlideShow->GetFont().c_str());
    if (font)
    {
        CGuiTextProperties propsA(false, true, EJustification::Center, EVerticalJustification::Bottom);
        xc4_textA = std::make_unique<CGuiTextSupport>(font->id, propsA,
                                                      g_tweakSlideShow->GetFontColor(),
                                                      g_tweakSlideShow->GetOutlineColor(),
                                                      zeus::CColor::skWhite, 640, 480, g_SimplePool);

        CGuiTextProperties propsB(false, true, EJustification::Right, EVerticalJustification::Bottom);
        xc8_textB = std::make_unique<CGuiTextSupport>(font->id, propsB,
                                                      g_tweakSlideShow->GetFontColor(),
                                                      g_tweakSlideShow->GetOutlineColor(),
                                                      zeus::CColor::skWhite, 640, 480, g_SimplePool);
    }

    xf8_.reserve(18);
}

CIOWin::EMessageReturn CSlideShow::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue)
{
    return EMessageReturn::Exit;
}

void CSlideShow::SSlideData::Draw() const
{
    if (!IsLoaded())
        return;

    zeus::CRectangle rect;
    const_cast<CTexturedQuadFilterAlpha&>(*m_texQuad).draw(x30_mulColor, 1.f, rect);

    zeus::CVector2f centeredOffset((x28_canvasSize.x - m_texQuad->GetTex()->GetWidth()) * 0.5f,
                                   (x28_canvasSize.y - m_texQuad->GetTex()->GetHeight()) * 0.5f);
}

void CSlideShow::Draw() const
{
    if (x14_phase == Phase::Five)
    {
        x5c_slideA.Draw();
        x90_slideB.Draw();
    }
}

}
