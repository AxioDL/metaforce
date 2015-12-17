#include <LogVisor/LogVisor.hpp>
#include "Specter/Button.hpp"
#include "Specter/ViewResources.hpp"
#include "Specter/RootView.hpp"

namespace Specter
{
static LogVisor::LogModule Log("Specter::Button");

void Button::Resources::init(boo::IGraphicsDataFactory* factory, const ThemeData& theme)
{
}

Button::Button(ViewResources& res, View& parentView,
               IButtonBinding* controlBinding, const std::string& text,
               Style style)
: Button(res, parentView, controlBinding, text, res.themeData().uiText(), style) {}

Button::Button(ViewResources& res, View& parentView,
               IButtonBinding* controlBinding, const std::string& text,
               const Zeus::CColor& textColor, Style style)
: Control(res, parentView, controlBinding), m_style(style), m_textColor(textColor), m_textStr(text)
{
    m_bVertsBuf = res.m_factory->newDynamicBuffer(boo::BufferUse::Vertex, sizeof(SolidShaderVert), 28);

    if (!res.m_viewRes.m_texVtxFmt)
    {
        boo::VertexElementDescriptor vdescs[] =
        {
            {m_bVertsBuf, nullptr, boo::VertexSemantic::Position4},
            {m_bVertsBuf, nullptr, boo::VertexSemantic::Color}
        };
        m_bVtxFmt = res.m_factory->newVertexFormat(2, vdescs);
        boo::IGraphicsBuffer* bufs[] = {m_viewVertBlockBuf};
        m_bShaderBinding = res.m_factory->newShaderDataBinding(res.m_viewRes.m_solidShader,
                                                               m_bVtxFmt, m_bVertsBuf, nullptr,
                                                               nullptr, 1, bufs, 0, nullptr);
    }
    else
    {
        boo::IGraphicsBuffer* bufs[] = {m_viewVertBlockBuf};
        m_bShaderBinding = res.m_factory->newShaderDataBinding(res.m_viewRes.m_solidShader,
                                                               res.m_viewRes.m_texVtxFmt,
                                                               m_bVertsBuf, nullptr,
                                                               nullptr, 1, bufs, 0, nullptr);
    }
    commitResources(res);

    if (style == Style::Block)
    {
        m_verts[0].m_color = res.themeData().button1Inactive();
        m_verts[1].m_color = res.themeData().button2Inactive();
        m_verts[2].m_color = res.themeData().button1Inactive();
        m_verts[3].m_color = res.themeData().button2Inactive();
        m_verts[4].m_color = res.themeData().button2Inactive();
        for (int i=5 ; i<28 ; ++i)
            m_verts[i].m_color = res.themeData().button2Inactive();
        m_bVertsBuf->load(m_verts, sizeof(SolidShaderVert) * 28);
    }
    else
    {
        for (int i=0 ; i<4 ; ++i)
            m_verts[i].m_color = Zeus::CColor::skClear;
        m_bVertsBuf->load(m_verts, sizeof(SolidShaderVert) * 4);
    }

    m_text.reset(new TextView(res, *this, res.m_mainFont));
    setText(m_textStr);
}

void Button::setText(const std::string &text)
{
    setText(text, m_textColor);
}

void Button::setText(const std::string& text, const Zeus::CColor& textColor)
{
    m_textStr = text;
    m_textColor = textColor;

    m_text->typesetGlyphs(text, textColor);
    float pf = rootView().viewRes().pixelFactor();
    float width, height;

    if (m_style == Style::Block)
    {
        width = m_text->nominalWidth() + 12 * pf;
        height = 20 * pf;
        m_verts[0].m_pos.assign(1, height+1, 0);
        m_verts[1].m_pos.assign(1, 1, 0);
        m_verts[2].m_pos.assign(width+1, height+1, 0);
        m_verts[3].m_pos.assign(width+1, 1, 0);
        m_verts[4].m_pos.assign(width+1, 1, 0);

        m_verts[5].m_pos.assign(1, height+1, 0);
        m_verts[6].m_pos.assign(1, height+1, 0);
        m_verts[7].m_pos.assign(0, height+1, 0);
        m_verts[8].m_pos.assign(1, 1, 0);
        m_verts[9].m_pos.assign(0, 1, 0);
        m_verts[10].m_pos.assign(0, 1, 0);

        m_verts[11].m_pos.assign(width+2, height+1, 0);
        m_verts[12].m_pos.assign(width+2, height+1, 0);
        m_verts[13].m_pos.assign(width+1, height+1, 0);
        m_verts[14].m_pos.assign(width+2, 1, 0);
        m_verts[15].m_pos.assign(width+1, 1, 0);
        m_verts[16].m_pos.assign(width+1, 1, 0);

        m_verts[17].m_pos.assign(1, height+2, 0);
        m_verts[18].m_pos.assign(1, height+2, 0);
        m_verts[19].m_pos.assign(1, height+1, 0);
        m_verts[20].m_pos.assign(width+1, height+2, 0);
        m_verts[21].m_pos.assign(width+1, height+1, 0);
        m_verts[22].m_pos.assign(width+1, height+1, 0);

        m_verts[23].m_pos.assign(1, 1, 0);
        m_verts[24].m_pos.assign(1, 1, 0);
        m_verts[25].m_pos.assign(1, 0, 0);
        m_verts[26].m_pos.assign(width+1, 1, 0);
        m_verts[27].m_pos.assign(width+1, 0, 0);

        m_bVertsBuf->load(m_verts, sizeof(SolidShaderVert) * 28);
    }
    else
    {
        width = m_text->nominalWidth();
        height = 10 * pf;
        m_verts[0].m_pos.assign(1*pf, -1*pf, 0);
        m_verts[1].m_pos.assign(1*pf, -2*pf, 0);
        m_verts[2].m_pos.assign(width, -1*pf, 0);
        m_verts[3].m_pos.assign(width, -2*pf, 0);

        m_bVertsBuf->load(m_verts, sizeof(SolidShaderVert) * 4);
    }

    m_nomWidth = width;
    m_nomHeight = height;
}

void Button::colorGlyphs(const Zeus::CColor& newColor)
{
    m_textColor = newColor;
    m_text->colorGlyphs(newColor);
}

void Button::setInactive()
{
    if (m_style == Style::Block)
    {
        m_verts[0].m_color = rootView().themeData().button1Inactive();
        m_verts[1].m_color = rootView().themeData().button2Inactive();
        m_verts[2].m_color = rootView().themeData().button1Inactive();
        m_verts[3].m_color = rootView().themeData().button2Inactive();
        m_verts[4].m_color = rootView().themeData().button2Inactive();
        m_bVertsBuf->load(m_verts, sizeof(SolidShaderVert) * 28);
    }
    else
    {
        for (int i=0 ; i<4 ; ++i)
            m_verts[i].m_color = Zeus::CColor::skClear;
        m_bVertsBuf->load(m_verts, sizeof(SolidShaderVert) * 4);
        m_text->colorGlyphs(m_textColor);
    }
}

void Button::setHover()
{
    if (m_style == Style::Block)
    {
        m_verts[0].m_color = rootView().themeData().button1Hover();
        m_verts[1].m_color = rootView().themeData().button2Hover();
        m_verts[2].m_color = rootView().themeData().button1Hover();
        m_verts[3].m_color = rootView().themeData().button2Hover();
        m_verts[4].m_color = rootView().themeData().button2Hover();
        m_bVertsBuf->load(m_verts, sizeof(SolidShaderVert) * 28);
    }
    else
    {
        for (int i=0 ; i<4 ; ++i)
            m_verts[i].m_color = m_textColor;
        m_bVertsBuf->load(m_verts, sizeof(SolidShaderVert) * 4);
        m_text->colorGlyphs(m_textColor);
    }
}

void Button::setPressed()
{
    if (m_style == Style::Block)
    {
        m_verts[0].m_color = rootView().themeData().button1Press();
        m_verts[1].m_color = rootView().themeData().button2Press();
        m_verts[2].m_color = rootView().themeData().button1Press();
        m_verts[3].m_color = rootView().themeData().button2Press();
        m_verts[4].m_color = rootView().themeData().button2Press();
        m_bVertsBuf->load(m_verts, sizeof(SolidShaderVert) * 28);
    }
    else
    {
        for (int i=0 ; i<4 ; ++i)
            m_verts[i].m_color = m_textColor;
        m_bVertsBuf->load(m_verts, sizeof(SolidShaderVert) * 4);
        m_text->colorGlyphs(m_textColor);
    }
}

void Button::setDisabled()
{
    if (m_style == Style::Block)
    {
        m_verts[0].m_color = rootView().themeData().button1Disabled();
        m_verts[1].m_color = rootView().themeData().button2Disabled();
        m_verts[2].m_color = rootView().themeData().button1Disabled();
        m_verts[3].m_color = rootView().themeData().button2Disabled();
        m_verts[4].m_color = rootView().themeData().button2Disabled();
        m_bVertsBuf->load(m_verts, sizeof(SolidShaderVert) * 28);
    }
    else
    {
        for (int i=0 ; i<4 ; ++i)
            m_verts[i].m_color = Zeus::CColor::skClear;
        m_bVertsBuf->load(m_verts, sizeof(SolidShaderVert) * 4);
        Zeus::CColor dimText = m_textColor;
        dimText[3] *= 0.5;
        m_text->colorGlyphs(dimText);
    }
}

void Button::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    Control::mouseDown(coord, button, mod);
    m_pressed = true;
    setPressed();
}

void Button::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    Control::mouseUp(coord, button, mod);
    if (m_pressed && m_hovered)
    {
        Log.report(LogVisor::Info, "button '%s' activated", m_textStr.c_str());
        if (m_controlBinding && dynamic_cast<IButtonBinding*>(m_controlBinding))
            static_cast<IButtonBinding&>(*m_controlBinding).activated(coord);
    }
    m_pressed = false;
    if (m_hovered)
        setHover();
    else
        setInactive();
}

void Button::mouseEnter(const boo::SWindowCoord& coord)
{
    Control::mouseEnter(coord);
    m_hovered = true;
    if (m_pressed)
        setPressed();
    else
        setHover();
}

void Button::mouseLeave(const boo::SWindowCoord& coord)
{
    Control::mouseLeave(coord);
    m_hovered = false;
    setInactive();
}

void Button::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    View::resized(root, sub);
    boo::SWindowRect textRect = sub;
    float pf = rootView().viewRes().pixelFactor();
    if (m_style == Style::Block)
    {
        textRect.location[0] += 5 * pf;
        textRect.location[1] += 8 * pf;
    }
    m_text->resized(root, textRect);
}

void Button::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    View::draw(gfxQ);
    gfxQ->setShaderDataBinding(m_bShaderBinding);
    gfxQ->setDrawPrimitive(boo::Primitive::TriStrips);
    if (m_style == Style::Block)
        gfxQ->draw(0, 28);
    else
        gfxQ->draw(0, 4);
    m_text->draw(gfxQ);
}

}

