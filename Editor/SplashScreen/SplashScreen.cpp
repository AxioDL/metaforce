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

SplashScreen::SplashScreen(ViewManager& vm, Specter::ViewResources& res)
: ModalWindow(res, vm.rootView(),
              Specter::RectangleConstraint(SPLASH_WIDTH * res.pixelFactor(),
                                           SPLASH_HEIGHT * res.pixelFactor())),
  m_vm(vm),
  m_textColor(res.themeData().uiText()),
  m_textColorClear(m_textColor),
  m_newProjBind(*this),
  m_openProjBind(*this),
  m_extractProjBind(*this)
{
    m_textColorClear[3] = 0.0;
    commitResources(res);
}

void SplashScreen::think()
{
    ModalWindow::think();
    if (m_fileBrowser.m_view)
        m_fileBrowser.m_view->think();
}

void SplashScreen::updateContentOpacity(float opacity)
{
    Specter::ViewResources& res = rootView().viewRes();

    if (!m_title && res.fontCacheReady())
    {
        m_title.reset(new Specter::TextView(res, *this, res.m_titleFont));
        Zeus::CColor clearColor = res.themeData().uiText();
        clearColor[3] = 0.0;
        m_title->typesetGlyphs("RUDE", clearColor);

        m_buildInfo.reset(new Specter::MultiLineTextView(res, *this, res.m_mainFont, Specter::TextView::Alignment::Right));
        m_buildInfo->typesetGlyphs(HECL::Format("Branch: %s\nCommit: %s\nDate: %s",
                                                GIT_BRANCH, GIT_COMMIT_HASH, GIT_COMMIT_DATE),
                                   clearColor);

        m_newButt.m_view.reset(new Specter::Button(res, *this, &m_newProjBind, "New Project", Specter::Button::Style::Text));
        m_openButt.m_view.reset(new Specter::Button(res, *this, &m_openProjBind, "Open Project", Specter::Button::Style::Text));
        m_extractButt.m_view.reset(new Specter::Button(res, *this, &m_extractProjBind, "Extract Game", Specter::Button::Style::Text));

        updateSize();
    }

    Zeus::CColor clearColor = res.themeData().uiText();
    clearColor[3] = 0.0;
    Zeus::CColor color = Zeus::CColor::lerp(clearColor, res.themeData().uiText(), opacity);
    m_title->colorGlyphs(color);
    m_buildInfo->colorGlyphs(color);
    m_newButt.m_view->colorGlyphs(color);
    m_openButt.m_view->colorGlyphs(color);
    m_extractButt.m_view->colorGlyphs(color);
}

void SplashScreen::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    if (skipBuildInAnimation())
        return;
    if (m_fileBrowser.m_view && !m_fileBrowser.m_view->closed())
        m_fileBrowser.m_view->mouseDown(coord, button, mod);
    else
    {
        m_newButt.mouseDown(coord, button, mod);
        m_openButt.mouseDown(coord, button, mod);
        m_extractButt.mouseDown(coord, button, mod);
    }
}

void SplashScreen::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    if (m_fileBrowser.m_view && !m_fileBrowser.m_view->closed())
        m_fileBrowser.m_view->mouseUp(coord, button, mod);
    else
    {
        m_newButt.mouseUp(coord, button, mod);
        m_openButt.mouseUp(coord, button, mod);
        m_extractButt.mouseUp(coord, button, mod);
    }
}

void SplashScreen::mouseMove(const boo::SWindowCoord& coord)
{
    if (m_fileBrowser.m_view && !m_fileBrowser.m_view->closed())
        m_fileBrowser.m_view->mouseMove(coord);
    else
    {
        m_newButt.mouseMove(coord);
        m_openButt.mouseMove(coord);
        m_extractButt.mouseMove(coord);
    }
}

void SplashScreen::mouseEnter(const boo::SWindowCoord& coord)
{
    if (m_fileBrowser.m_view && !m_fileBrowser.m_view->closed())
        m_fileBrowser.m_view->mouseEnter(coord);
    else
    {
        m_newButt.mouseEnter(coord);
        m_openButt.mouseEnter(coord);
        m_extractButt.mouseEnter(coord);
    }
}

void SplashScreen::mouseLeave(const boo::SWindowCoord& coord)
{
    if (m_fileBrowser.m_view && !m_fileBrowser.m_view->closed())
        m_fileBrowser.m_view->mouseLeave(coord);
    else
    {
        m_newButt.mouseLeave(coord);
        m_openButt.mouseLeave(coord);
        m_extractButt.mouseLeave(coord);
    }
}

void SplashScreen::scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& scroll)
{
    if (m_fileBrowser.m_view)
        m_fileBrowser.m_view->scroll(coord, scroll);
}

void SplashScreen::touchDown(const boo::STouchCoord& coord, uintptr_t tid)
{
    if (m_fileBrowser.m_view)
        m_fileBrowser.m_view->touchDown(coord, tid);
}

void SplashScreen::touchUp(const boo::STouchCoord& coord, uintptr_t tid)
{
    if (m_fileBrowser.m_view)
        m_fileBrowser.m_view->touchUp(coord, tid);
}

void SplashScreen::touchMove(const boo::STouchCoord& coord, uintptr_t tid)
{
    if (m_fileBrowser.m_view)
        m_fileBrowser.m_view->touchMove(coord, tid);
}

void SplashScreen::charKeyDown(unsigned long charCode, boo::EModifierKey mods, bool isRepeat)
{
    skipBuildInAnimation();
}

void SplashScreen::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    ModalWindow::resized(root, sub);
    float pf = rootView().viewRes().pixelFactor();

    boo::SWindowRect centerRect = subRect();
    centerRect.location[0] = root.size[0] / 2 - (SPLASH_WIDTH * pf / 2.0);
    centerRect.location[1] = root.size[1] / 2 - (SPLASH_HEIGHT * pf / 2.0);

    boo::SWindowRect textRect = centerRect;
    textRect.location[0] += TEXT_MARGIN * pf;
    textRect.location[1] += (SPLASH_HEIGHT - 36) * pf;
    if (m_title)
    {
        m_title->resized(root, textRect);
        textRect.location[0] = centerRect.location[0] + (SPLASH_WIDTH - TEXT_MARGIN) * pf;
        textRect.location[1] -= 5 * pf;
        m_buildInfo->resized(root, textRect);

        textRect.size[0] = m_newButt.m_view->nominalWidth();
        textRect.size[1] = m_newButt.m_view->nominalHeight();
        textRect.location[1] = centerRect.location[1] + 20 * pf;
        textRect.location[0] = centerRect.location[0] + SPLASH_WIDTH / 4 * pf - m_newButt.m_view->nominalWidth() / 2;
        m_newButt.m_view->resized(root, textRect);

        textRect.size[0] = m_openButt.m_view->nominalWidth();
        textRect.size[1] = m_openButt.m_view->nominalHeight();
        textRect.location[1] = centerRect.location[1] + 20 * pf;
        textRect.location[0] = centerRect.location[0] + SPLASH_WIDTH * 2 / 4 * pf - m_openButt.m_view->nominalWidth() / 2;
        m_openButt.m_view->resized(root, textRect);

        textRect.size[0] = m_extractButt.m_view->nominalWidth();
        textRect.size[1] = m_extractButt.m_view->nominalHeight();
        textRect.location[1] = centerRect.location[1] + 20 * pf;
        textRect.location[0] = centerRect.location[0] + SPLASH_WIDTH * 3 / 4 * pf - m_extractButt.m_view->nominalWidth() / 2;
        m_extractButt.m_view->resized(root, textRect);
    }

    if (m_fileBrowser.m_view)
        m_fileBrowser.m_view->resized(root, root);
}

void SplashScreen::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    ModalWindow::draw(gfxQ);

    if (m_title)
    {
        m_title->draw(gfxQ);
        m_buildInfo->draw(gfxQ);
        m_newButt.m_view->draw(gfxQ);
        m_openButt.m_view->draw(gfxQ);
        m_extractButt.m_view->draw(gfxQ);
    }

    if (m_fileBrowser.m_view)
        m_fileBrowser.m_view->draw(gfxQ);
}

}
