#include "SplashScreen.hpp"
#include "version.h"
#include "badging/Badging.hpp"

namespace metaforce {

#define SPLASH_WIDTH 555
#define SPLASH_HEIGHT 100

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
              specter::RectangleConstraint(SPLASH_WIDTH * res.pixelFactor(), SPLASH_HEIGHT * res.pixelFactor()))
, m_vm(vm)
, m_textColor(res.themeData().uiText()) {
  if (METAFORCE_WC_DATE[0] != '\0' && METAFORCE_WC_REVISION[0] != '\0' && METAFORCE_WC_BRANCH[0] != '\0') {
    m_buildInfoStr = fmt::format(FMT_STRING("{}: {}\n{}: {}\n{}: {}"),
                                 vm.translate<locale::version>(), METAFORCE_WC_DESCRIBE,
                                 vm.translate<locale::branch>(), METAFORCE_WC_BRANCH,
                                 vm.translate<locale::commit>(), METAFORCE_WC_REVISION/*,
                                 vm.translate<locale::date>(), METAFORCE_WC_DATE*/);
  }
}

void SplashScreen::think() {
  if (phase() == Phase::Done) {
    return;
  }
  OPTICK_EVENT();

  ModalWindow::think();
}

void SplashScreen::updateContentOpacity(float opacity) {
  specter::ViewResources& res = rootView().viewRes();

  if (!m_title && res.fontCacheReady()) {
    m_title.reset(new specter::TextView(res, *this, res.m_titleFont));
    zeus::CColor clearColor = res.themeData().uiText();
    clearColor[3] = 0.0;
    m_title->typesetGlyphs("Metaforce", clearColor);

    m_buildInfo.reset(new specter::MultiLineTextView(res, *this, res.m_mainFont, specter::TextView::Alignment::Right));
    m_buildInfo->typesetGlyphs(m_buildInfoStr, clearColor);

    m_badgeText.reset(new specter::TextView(res, *this, res.m_heading18));
    m_badgeText->typesetGlyphs(BADGE_PHRASE, clearColor);

    m_infoStr = std::make_unique<specter::TextView>(res, *this, res.m_mainFont);
    m_infoStr->typesetGlyphs("No game detected. Use metaforce-gui or check README for command-line options."sv, clearColor);

    updateSize();
  }

  zeus::CColor clearColor = res.themeData().uiText();
  clearColor[3] = 0.0;
  zeus::CColor color = zeus::CColor::lerp(clearColor, res.themeData().uiText(), opacity);
  m_title->colorGlyphs(color);
  m_buildInfo->colorGlyphs(color);
  m_badgeText->colorGlyphs(color);
  m_infoStr->colorGlyphs(color);
}

void SplashScreen::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub) {
  ModalWindow::resized(root, sub);
  float pf = rootView().viewRes().pixelFactor();

  boo::SWindowRect centerRect = subRect();
  centerRect.location[0] = root.size[0] / 2 - (SPLASH_WIDTH * pf / 2.0);
  centerRect.location[1] = root.size[1] / 2 - (SPLASH_HEIGHT * pf / 2.0);

  boo::SWindowRect textRect = centerRect;
  textRect.location[0] += TEXT_MARGIN * pf;
  textRect.location[1] += (SPLASH_HEIGHT - 28) * pf;
  if (m_title) {
    m_title->resized(root, textRect);

    textRect.location[1] -= m_title->nominalHeight() * 0.75;
    m_badgeText->resized(root, textRect);

    textRect.location[0] = centerRect.location[0] + (SPLASH_WIDTH - TEXT_MARGIN) * pf;
    textRect.location[1] = centerRect.location[1] + (SPLASH_HEIGHT - 36) * pf - 5 * pf;
    m_buildInfo->resized(root, textRect);

    textRect.size[0] = m_infoStr->nominalWidth();
    textRect.size[1] = m_infoStr->nominalHeight();
    textRect.location[1] = centerRect.location[1] + 20 * pf;
    textRect.location[0] = centerRect.location[0] + SPLASH_WIDTH * 2 / 4 * pf - m_infoStr->nominalWidth() / 2;
    m_infoStr->resized(root, textRect);
  }
}

void SplashScreen::draw(boo::IGraphicsCommandQueue* gfxQ) {
  if (phase() == Phase::Done)
    return;
  ModalWindow::draw(gfxQ);

  if (m_title) {
    m_title->draw(gfxQ);
    m_buildInfo->draw(gfxQ);
    m_badgeText->draw(gfxQ);
    m_infoStr->draw(gfxQ);
  }
}

} // namespace metaforce
