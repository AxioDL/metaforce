#include "SplashScreen.hpp"
#include "version.h"

namespace urde
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

SplashScreen::SplashScreen(ViewManager& vm, specter::ViewResources& res)
: ModalWindow(res, vm.rootView(),
              specter::RectangleConstraint(SPLASH_WIDTH * res.pixelFactor(),
                                           SPLASH_HEIGHT * res.pixelFactor())),
  m_vm(vm),
  m_textColor(res.themeData().uiText()),
  m_textColorClear(m_textColor),
  m_newString(m_vm.translateOr("new_project", "New Project")),
  m_newProjBind(*this),
  m_openString(m_vm.translateOr("open_project", "Open Project")),
  m_openProjBind(*this),
  m_extractString(m_vm.translateOr("extract_game", "Extract Game")),
  m_extractProjBind(*this)
{
    if (GIT_COMMIT_DATE[0] != '\0' &&
        GIT_COMMIT_HASH[0] != '\0' &&
        GIT_BRANCH[0]      != '\0')
    {
        m_buildInfoStr = hecl::Format("%s: %s\n%s: %s\n%s: %s",
                                      vm.translateOr("branch", "Branch").c_str(), GIT_BRANCH,
                                      vm.translateOr("commit", "Commit").c_str(), GIT_COMMIT_HASH,
                                      vm.translateOr("date", "Date").c_str(), GIT_COMMIT_DATE);
    }

    m_openProjBind.m_openRecentMenuRoot.m_text = vm.translateOr("recent_projects", "Recent Projects");
    m_textColorClear[3] = 0.0;
}

void SplashScreen::think()
{
    if (phase() == Phase::Done)
    {
        if (m_fileBrowser.m_view)
            m_fileBrowser.m_view.reset();
        return;
    }

    ModalWindow::think();
    if (m_fileBrowser.m_view)
        m_fileBrowser.m_view->think();

    if (m_openButt.m_view)
        m_openButt.m_view->think();

    if (m_newProjBind.m_deferPath.size())
    {
        Log.report(logvisor::Info, _S("Making project '%s'"), m_newProjBind.m_deferPath.c_str());
        m_vm.projectManager().newProject(m_newProjBind.m_deferPath);
        m_newProjBind.m_deferPath.clear();
    }
    else if (m_openProjBind.m_deferPath.size())
    {
        Log.report(logvisor::Info, _S("Opening project '%s'"), m_openProjBind.m_deferPath.c_str());
        m_vm.projectManager().openProject(m_openProjBind.m_deferPath);
        m_openProjBind.m_deferPath.clear();
    }
    else if (m_extractProjBind.m_deferPath.size())
    {
        Log.report(logvisor::Info, _S("Extracting game '%s'"), m_extractProjBind.m_deferPath.c_str());
        m_vm.projectManager().extractGame(m_extractProjBind.m_deferPath);
        m_extractProjBind.m_deferPath.clear();
    }
}

void SplashScreen::updateContentOpacity(float opacity)
{
    specter::ViewResources& res = rootView().viewRes();

    if (!m_title && res.fontCacheReady())
    {
        m_title.reset(new specter::TextView(res, *this, res.m_titleFont));
        zeus::CColor clearColor = res.themeData().uiText();
        clearColor[3] = 0.0;
        m_title->typesetGlyphs("URDE", clearColor);

        m_buildInfo.reset(new specter::MultiLineTextView(res, *this, res.m_mainFont, specter::TextView::Alignment::Right));
        m_buildInfo->typesetGlyphs(m_buildInfoStr, clearColor);

        m_newButt.m_view.reset(new specter::Button(res, *this, &m_newProjBind, m_newString,
                                                   nullptr, specter::Button::Style::Text));
        m_openButt.m_view.reset(new specter::Button(res, *this, &m_openProjBind, m_openString,
                                                    nullptr, specter::Button::Style::Text));
        m_extractButt.m_view.reset(new specter::Button(res, *this, &m_extractProjBind, m_extractString,
                                                       nullptr, specter::Button::Style::Text));

        updateSize();
    }

    zeus::CColor clearColor = res.themeData().uiText();
    clearColor[3] = 0.0;
    zeus::CColor color = zeus::CColor::lerp(clearColor, res.themeData().uiText(), opacity);
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
    if (skipBuildInAnimation())
        return;
    if (m_fileBrowser.m_view)
        m_fileBrowser.m_view->charKeyDown(charCode, mods, isRepeat);
}

void SplashScreen::specialKeyDown(boo::ESpecialKey key, boo::EModifierKey mods, bool isRepeat)
{
    if (skipBuildInAnimation())
        return;
    if (m_fileBrowser.m_view)
        m_fileBrowser.m_view->specialKeyDown(key, mods, isRepeat);
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
    if (phase() == Phase::Done)
        return;
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
