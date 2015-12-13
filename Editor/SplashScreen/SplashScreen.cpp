#include "SplashScreen.hpp"
#include "version.h"

namespace RUDE
{

#define SPLASH_WIDTH 555
#define SPLASH_HEIGHT 300

#define WIRE_START 0
#define WIRE_FRAMES 60
#define SOLID_START 40
#define SOLID_FRAMES 40
#define TEXT_START 80
#define TEXT_FRAMES 40

#define LINE_WIDTH 2
#define TEXT_MARGIN 10

void SplashScreen::setLineVerts(int width, int height, float pf, float t)
{
    std::pair<int,int> margin = m_cornersOutline[0]->queryGlyphDimensions(0);
    float t1 = Zeus::Math::clamp(0.f, t * 2.f, 1.f);
    float t2 = Zeus::Math::clamp(0.f, t * 2.f - 1.f, 1.f);

    float lineLeft = 0;
    float lineRight = pf*LINE_WIDTH;
    float lineTop = height-margin.second;
    float lineBottom = margin.second;
    m_verts.lineVerts[0].m_pos.assign(lineLeft, lineTop, 0);
    m_verts.lineVerts[1].m_pos = Zeus::CVector3f::lerp({lineLeft, lineTop, 0}, {lineLeft, lineBottom, 0}, t1);
    m_verts.lineVerts[2].m_pos.assign(lineRight, lineTop, 0);
    m_verts.lineVerts[3].m_pos = Zeus::CVector3f::lerp({lineRight, lineTop, 0}, {lineRight, lineBottom, 0}, t1);
    m_verts.lineVerts[4].m_pos = m_verts.lineVerts[3].m_pos;

    lineLeft = margin.first;
    lineRight = width-margin.first;
    lineTop = height;
    lineBottom = height-pf*LINE_WIDTH;
    m_verts.lineVerts[5].m_pos.assign(lineLeft, lineTop, 0);
    m_verts.lineVerts[6].m_pos = m_verts.lineVerts[5].m_pos;
    m_verts.lineVerts[7].m_pos.assign(lineLeft, lineBottom, 0);
    m_verts.lineVerts[8].m_pos = Zeus::CVector3f::lerp({lineLeft, lineTop, 0}, {lineRight, lineTop, 0}, t1);
    m_verts.lineVerts[9].m_pos = Zeus::CVector3f::lerp({lineLeft, lineBottom, 0}, {lineRight, lineBottom, 0}, t1);
    m_verts.lineVerts[10].m_pos = m_verts.lineVerts[9].m_pos;

    lineLeft = width-pf*LINE_WIDTH;
    lineRight = width;
    lineTop = height-margin.second;
    lineBottom = margin.second;
    m_verts.lineVerts[11].m_pos.assign(lineLeft, lineTop, 0);
    m_verts.lineVerts[12].m_pos = m_verts.lineVerts[11].m_pos;
    m_verts.lineVerts[13].m_pos = Zeus::CVector3f::lerp({lineLeft, lineTop, 0}, {lineLeft, lineBottom, 0}, t2);
    m_verts.lineVerts[14].m_pos.assign(lineRight, lineTop, 0);
    m_verts.lineVerts[15].m_pos = Zeus::CVector3f::lerp({lineRight, lineTop, 0}, {lineRight, lineBottom, 0}, t2);
    m_verts.lineVerts[16].m_pos = m_verts.lineVerts[15].m_pos;

    lineLeft = margin.first;
    lineRight = width-margin.first;
    lineTop = pf*LINE_WIDTH;
    lineBottom = 0;
    m_verts.lineVerts[17].m_pos.assign(lineLeft, lineTop, 0);
    m_verts.lineVerts[18].m_pos = m_verts.lineVerts[17].m_pos;
    m_verts.lineVerts[19].m_pos.assign(lineLeft, lineBottom, 0);
    m_verts.lineVerts[20].m_pos = Zeus::CVector3f::lerp({lineLeft, lineTop, 0}, {lineRight, lineTop, 0}, t2);
    m_verts.lineVerts[21].m_pos = Zeus::CVector3f::lerp({lineLeft, lineBottom, 0}, {lineRight, lineBottom, 0}, t2);
}

void SplashScreen::setLineColors(float t)
{
    float t1 = Zeus::Math::clamp(0.f, t * 2.f, 1.f);
    float t2 = Zeus::Math::clamp(0.f, t * 2.f - 1.f, 1.f);
    float t3 = Zeus::Math::clamp(0.f, t * 2.f - 2.f, 1.f);

    Zeus::CColor c1 = Zeus::CColor::lerp(m_splash1, m_splash2, t1);
    Zeus::CColor c2 = Zeus::CColor::lerp(m_splash1, m_splash2, t2);
    Zeus::CColor c3 = Zeus::CColor::lerp(m_splash1, m_splash2, t3);

    m_cornersOutline[0]->colorGlyphs(c1);
    if (t < 0.5)
    {
        m_cornersOutline[1]->colorGlyphs(Zeus::CColor::skClear);
        m_cornersOutline[2]->colorGlyphs(Zeus::CColor::skClear);
        m_cornersOutline[3]->colorGlyphs(Zeus::CColor::skClear);
    }
    else if (t < 1.0)
    {
        m_cornersOutline[1]->colorGlyphs(c2);
        m_cornersOutline[3]->colorGlyphs(c2);
    }
    else
    {
        m_cornersOutline[1]->colorGlyphs(c2);
        m_cornersOutline[2]->colorGlyphs(c3);
        m_cornersOutline[3]->colorGlyphs(c2);
    }

    m_verts.lineVerts[0].m_color = c1;
    m_verts.lineVerts[1].m_color = c2;
    m_verts.lineVerts[2].m_color = m_verts.lineVerts[0].m_color;
    m_verts.lineVerts[3].m_color = m_verts.lineVerts[1].m_color;
    m_verts.lineVerts[4].m_color = m_verts.lineVerts[3].m_color;

    m_verts.lineVerts[5].m_color = c1;
    m_verts.lineVerts[6].m_color = m_verts.lineVerts[5].m_color;
    m_verts.lineVerts[7].m_color = m_verts.lineVerts[6].m_color;
    m_verts.lineVerts[8].m_color = c2;
    m_verts.lineVerts[9].m_color = m_verts.lineVerts[8].m_color;
    m_verts.lineVerts[10].m_color = m_verts.lineVerts[9].m_color;

    m_verts.lineVerts[11].m_color = c2;
    m_verts.lineVerts[12].m_color = m_verts.lineVerts[11].m_color;
    m_verts.lineVerts[13].m_color = c3;
    m_verts.lineVerts[14].m_color = m_verts.lineVerts[12].m_color;
    m_verts.lineVerts[15].m_color = m_verts.lineVerts[13].m_color;
    m_verts.lineVerts[16].m_color = m_verts.lineVerts[15].m_color;

    m_verts.lineVerts[17].m_color = c2;
    m_verts.lineVerts[18].m_color = m_verts.lineVerts[17].m_color;
    m_verts.lineVerts[19].m_color = m_verts.lineVerts[18].m_color;
    m_verts.lineVerts[20].m_color = c3;
    m_verts.lineVerts[21].m_color = m_verts.lineVerts[20].m_color;
}

void SplashScreen::setFillVerts(int width, int height, float pf)
{
    std::pair<int,int> margin = m_cornersFilled[0]->queryGlyphDimensions(0);

    float fillLeft = pf*LINE_WIDTH;
    float fillRight = width-pf*LINE_WIDTH;
    float fillTop = height-margin.second;
    float fillBottom = margin.second;
    m_verts.fillVerts[0].m_pos.assign(fillLeft, fillTop, 0);
    m_verts.fillVerts[1].m_pos.assign(fillLeft, fillBottom, 0);
    m_verts.fillVerts[2].m_pos.assign(fillRight, fillTop, 0);
    m_verts.fillVerts[3].m_pos.assign(fillRight, fillBottom, 0);
    m_verts.fillVerts[4].m_pos = m_verts.fillVerts[3].m_pos;

    fillLeft = margin.first;
    fillRight = width-margin.first;
    fillTop = height-pf*LINE_WIDTH;
    fillBottom = height-margin.second;
    m_verts.fillVerts[5].m_pos.assign(fillLeft, fillTop, 0);
    m_verts.fillVerts[6].m_pos = m_verts.fillVerts[5].m_pos;
    m_verts.fillVerts[7].m_pos.assign(fillLeft, fillBottom, 0);
    m_verts.fillVerts[8].m_pos.assign(fillRight, fillTop, 0);
    m_verts.fillVerts[9].m_pos.assign(fillRight, fillBottom, 0);
    m_verts.fillVerts[10].m_pos = m_verts.fillVerts[9].m_pos;

    fillLeft = margin.first;
    fillRight = width-margin.first;
    fillTop = margin.second;
    fillBottom = pf*LINE_WIDTH;
    m_verts.fillVerts[11].m_pos.assign(fillLeft, fillTop, 0);
    m_verts.fillVerts[12].m_pos = m_verts.fillVerts[11].m_pos;
    m_verts.fillVerts[13].m_pos.assign(fillLeft, fillBottom, 0);
    m_verts.fillVerts[14].m_pos.assign(fillRight, fillTop, 0);
    m_verts.fillVerts[15].m_pos.assign(fillRight, fillBottom, 0);
}

void SplashScreen::setFillColors(float t)
{
    t = Zeus::Math::clamp(0.f, t, 1.f);
    Zeus::CColor color = Zeus::CColor::lerp(m_splashBgClear, m_splashBg, t);

    for (int i=0 ; i<16 ; ++i)
        m_verts.fillVerts[i].m_color = color;
    for (int i=0 ; i<4 ; ++i)
        m_cornersFilled[i]->colorGlyphs(color);
}

SplashScreen::SplashScreen(ViewManager& vm, Specter::ViewResources& res)
: View(res, vm.rootView()), m_vm(vm),
  m_splashBg(res.themeData().splashBackground()),
  m_splashBgClear(m_splashBg),
  m_splash1(res.themeData().splash1()),
  m_splash2(res.themeData().splash2()),
  m_textColor(res.themeData().uiText()),
  m_textColorClear(m_textColor)
{
    m_splashBgClear[3] = 0.0;
    m_textColorClear[3] = 0.0;
    m_viewBlockBuf = res.m_factory->newDynamicBuffer(boo::BufferUse::Uniform, sizeof(ViewBlock), 1);
    m_vertsBuf = res.m_factory->newDynamicBuffer(boo::BufferUse::Vertex, sizeof(SolidShaderVert), 38);

    if (!res.m_viewRes.m_solidVtxFmt)
    {
        boo::VertexElementDescriptor vdescs[] =
        {
            {m_vertsBuf, nullptr, boo::VertexSemantic::Position4},
            {m_vertsBuf, nullptr, boo::VertexSemantic::Color}
        };
        m_vertsVtxFmt = res.m_factory->newVertexFormat(2, vdescs);
        boo::IGraphicsBuffer* bufs[] = {m_viewBlockBuf};
        m_vertsShaderBinding = res.m_factory->newShaderDataBinding(res.m_viewRes.m_solidShader,
                                                                   m_vertsVtxFmt, m_vertsBuf, nullptr,
                                                                   nullptr, 1, bufs, 0, nullptr);
    }
    else
    {
        boo::IGraphicsBuffer* bufs[] = {m_viewBlockBuf};
        m_vertsShaderBinding = res.m_factory->newShaderDataBinding(res.m_viewRes.m_solidShader,
                                                                   res.m_viewRes.m_solidVtxFmt,
                                                                   m_vertsBuf, nullptr,
                                                                   nullptr, 1, bufs, 0, nullptr);
    }
    commitResources(res);

    for (int i=0 ; i<4 ; ++i)
    {
        m_cornersOutline[i].reset(new Specter::TextView(res, *this, res.m_curveFont, Specter::TextView::Alignment::Left, 1));
        m_cornersFilled[i].reset(new Specter::TextView(res, *this, res.m_curveFont, Specter::TextView::Alignment::Left, 1));
    }
    m_cornersOutline[0]->typesetGlyphs(L"\xF4F0");
    m_cornersFilled[0]->typesetGlyphs(L"\xF4F1", res.themeData().splashBackground());
    m_cornersOutline[1]->typesetGlyphs(L"\xF4F2");
    m_cornersFilled[1]->typesetGlyphs(L"\xF4F3", res.themeData().splashBackground());
    m_cornersOutline[2]->typesetGlyphs(L"\xF4F4");
    m_cornersFilled[2]->typesetGlyphs(L"\xF4F5", res.themeData().splashBackground());
    m_cornersOutline[3]->typesetGlyphs(L"\xF4F6");
    m_cornersFilled[3]->typesetGlyphs(L"\xF4F7", res.themeData().splashBackground());

    float pf = res.pixelFactor();
    setLineVerts(SPLASH_WIDTH * pf, SPLASH_HEIGHT * pf, pf, 0.0);
    setFillVerts(SPLASH_WIDTH * pf, SPLASH_HEIGHT * pf, pf);
    setLineColors(0.0);
    setFillColors(0.0);

    m_vertsBuf->load(&m_verts, sizeof(m_verts));
}

static float CubicEase(float t)
{
    t *= 2.f;
    if (t < 1) return 1.f/2.f*t*t*t;
    t -= 2.f;
    return 1.f/2.f*(t*t*t + 2.f);
}

void SplashScreen::think()
{
    Specter::ViewResources& res = rootView().viewRes();
    float pf = res.pixelFactor();

    if (!m_title && res.m_fcacheReady)
    {
        m_title.reset(new Specter::TextView(res, *this, res.m_titleFont));
        Zeus::CColor clearColor = res.themeData().uiText();
        clearColor[3] = 0.0;
        m_title->typesetGlyphs("RUDE", clearColor);

        m_buildInfo.reset(new Specter::MultiLineTextView(res, *this, res.m_mainFont, Specter::TextView::Alignment::Right));
        m_buildInfo->typesetGlyphs(HECL::Format("Branch: %s\nCommit: %s\nDate: %s",
                                                GIT_BRANCH, GIT_COMMIT_HASH, GIT_COMMIT_DATE),
                                   clearColor);

        m_newButt.m_button.reset(new Specter::Button(res, *this, &m_newProjBind, "New Project", Specter::Button::Style::Text));
        m_openButt.m_button.reset(new Specter::Button(res, *this, &m_openProjBind, "Open Project", Specter::Button::Style::Text));
        m_extractButt.m_button.reset(new Specter::Button(res, *this, &m_extractProjBind, "Extract Game", Specter::Button::Style::Text));

        updateSize();
    }

    bool loadVerts = false;
    if (m_frame > WIRE_START)
    {
        float wt = (m_frame-WIRE_START) / float(WIRE_FRAMES);
        wt = Zeus::Math::clamp(0.f, wt, 2.f);
        setLineVerts(SPLASH_WIDTH * pf, SPLASH_HEIGHT * pf, pf, CubicEase(wt));
        setLineColors(wt);
        loadVerts = true;
    }
    if (m_frame > SOLID_START)
    {
        float ft = (m_frame-SOLID_START) / float(SOLID_FRAMES);
        ft = Zeus::Math::clamp(0.f, ft, 2.f);
        setFillColors(ft);
        loadVerts = true;
    }
    if (m_title && m_frame > TEXT_START)
    {
        if (!m_textStartFrame)
            m_textStartFrame = m_frame;
        float tt = (m_frame-m_textStartFrame) / float(TEXT_FRAMES);
        tt = Zeus::Math::clamp(0.f, tt, 1.f);
        Zeus::CColor clearColor = res.themeData().uiText();
        clearColor[3] = 0.0;
        Zeus::CColor color = Zeus::CColor::lerp(clearColor, res.themeData().uiText(), tt);
        m_title->colorGlyphs(color);
        m_buildInfo->colorGlyphs(color);
        m_newButt.m_button->colorGlyphs(color);
        m_openButt.m_button->colorGlyphs(color);
        m_extractButt.m_button->colorGlyphs(color);
    }

    if (loadVerts)
        m_vertsBuf->load(&m_verts, sizeof(m_verts));

    ++m_frame;
}

void SplashScreen::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    m_newButt.mouseDown(coord, button, mod);
    m_openButt.mouseDown(coord, button, mod);
    m_extractButt.mouseDown(coord, button, mod);
}

void SplashScreen::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    m_newButt.mouseUp(coord, button, mod);
    m_openButt.mouseUp(coord, button, mod);
    m_extractButt.mouseUp(coord, button, mod);
}

void SplashScreen::mouseMove(const boo::SWindowCoord& coord)
{
    m_newButt.mouseMove(coord);
    m_openButt.mouseMove(coord);
    m_extractButt.mouseMove(coord);
}

void SplashScreen::mouseEnter(const boo::SWindowCoord& coord)
{
    m_newButt.mouseEnter(coord);
    m_openButt.mouseEnter(coord);
    m_extractButt.mouseEnter(coord);
}

void SplashScreen::mouseLeave(const boo::SWindowCoord& coord)
{
    m_newButt.mouseLeave(coord);
    m_openButt.mouseLeave(coord);
    m_extractButt.mouseLeave(coord);
}

void SplashScreen::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    float pf = rootView().viewRes().pixelFactor();

    boo::SWindowRect centerRect = sub;
    centerRect.location[0] = root.size[0] / 2 - (SPLASH_WIDTH * pf / 2.0);
    centerRect.location[1] = root.size[1] / 2 - (SPLASH_HEIGHT * pf / 2.0);
    View::resized(root, centerRect);
    m_viewBlock.setViewRect(root, centerRect);
    m_viewBlockBuf->load(&m_viewBlock, sizeof(m_viewBlock));

    boo::SWindowRect textRect = centerRect;
    textRect.location[0] += TEXT_MARGIN * pf;
    textRect.location[1] += (SPLASH_HEIGHT - 36) * pf;
    if (m_title)
    {
        m_title->resized(root, textRect);
        textRect.location[0] = centerRect.location[0] + (SPLASH_WIDTH - TEXT_MARGIN) * pf;
        textRect.location[1] -= 5 * pf;
        m_buildInfo->resized(root, textRect);

        textRect.size[0] = m_newButt.m_button->nominalWidth();
        textRect.size[1] = m_newButt.m_button->nominalHeight();
        textRect.location[1] = centerRect.location[1] + 20 * pf;
        textRect.location[0] = centerRect.location[0] + SPLASH_WIDTH / 4 * pf - m_newButt.m_button->nominalWidth() / 2;
        m_newButt.m_button->resized(root, textRect);

        textRect.size[0] = m_openButt.m_button->nominalWidth();
        textRect.size[1] = m_openButt.m_button->nominalHeight();
        textRect.location[1] = centerRect.location[1] + 20 * pf;
        textRect.location[0] = centerRect.location[0] + SPLASH_WIDTH * 2 / 4 * pf - m_openButt.m_button->nominalWidth() / 2;
        m_openButt.m_button->resized(root, textRect);

        textRect.size[0] = m_extractButt.m_button->nominalWidth();
        textRect.size[1] = m_extractButt.m_button->nominalHeight();
        textRect.location[1] = centerRect.location[1] + 20 * pf;
        textRect.location[0] = centerRect.location[0] + SPLASH_WIDTH * 3 / 4 * pf - m_extractButt.m_button->nominalWidth() / 2;
        m_extractButt.m_button->resized(root, textRect);
    }


    boo::SWindowRect cornerRect = centerRect;
    cornerRect.size[0] = cornerRect.size[1] = 8 * pf;
    cornerRect.location[1] = centerRect.location[1] + (SPLASH_HEIGHT - 8) * pf;
    m_cornersOutline[0]->resized(root, cornerRect);
    m_cornersFilled[0]->resized(root, cornerRect);
    cornerRect.location[0] = centerRect.location[0] + (SPLASH_WIDTH - 8) * pf;
    m_cornersOutline[1]->resized(root, cornerRect);
    m_cornersFilled[1]->resized(root, cornerRect);
    cornerRect.location[1] = centerRect.location[1];
    m_cornersOutline[2]->resized(root, cornerRect);
    m_cornersFilled[2]->resized(root, cornerRect);
    cornerRect.location[0] = centerRect.location[0];
    m_cornersOutline[3]->resized(root, cornerRect);
    m_cornersFilled[3]->resized(root, cornerRect);
}

void SplashScreen::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    gfxQ->setShaderDataBinding(m_vertsShaderBinding);
    gfxQ->setDrawPrimitive(boo::Primitive::TriStrips);
    if (m_frame > WIRE_START)
        gfxQ->draw(0, 22);
    if (m_frame > SOLID_START)
        gfxQ->draw(22, 16);
    if (m_title && m_textStartFrame && m_frame > m_textStartFrame)
    {
        m_title->draw(gfxQ);
        m_buildInfo->draw(gfxQ);
        m_newButt.m_button->draw(gfxQ);
        m_openButt.m_button->draw(gfxQ);
        m_extractButt.m_button->draw(gfxQ);
    }

    m_cornersFilled[0]->draw(gfxQ);
    m_cornersFilled[1]->draw(gfxQ);
    m_cornersFilled[2]->draw(gfxQ);
    m_cornersFilled[3]->draw(gfxQ);

    m_cornersOutline[0]->draw(gfxQ);
    m_cornersOutline[1]->draw(gfxQ);
    m_cornersOutline[2]->draw(gfxQ);
    m_cornersOutline[3]->draw(gfxQ);
}

}
