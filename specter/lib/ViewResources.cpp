#include "specter/ViewResources.hpp"

namespace specter {
static logvisor::Module Log("specter::ViewResources");

void ViewResources::init(boo::IGraphicsDataFactory* factory, FontCache* fcache, const IThemeData* theme, float pf) {
  if (!factory || !fcache || !theme)
    Log.report(logvisor::Fatal, "all arguments of ViewResources::init() must be non-null");
  m_pixelFactor = pf;
  m_factory = factory;
  m_theme = theme;
  m_fcache = fcache;
  unsigned dpi = 72.f * m_pixelFactor;

  m_curveFont = fcache->prepCurvesFont(AllCharFilter, false, 8.f, dpi);

  factory->commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx) {
    init(ctx, *theme, fcache);
    return true;
  } BooTrace);
  factory->waitUntilShadersReady();
}

void ViewResources::destroyResData() {
  m_viewRes.destroy();
  m_textRes.destroy();
  m_splitRes.destroy();
  m_toolbarRes.destroy();
  m_buttonRes.destroy();
}

void ViewResources::prepFontCacheSync() {
  unsigned dpi = 72.f * m_pixelFactor;
  if (m_fcacheInterrupt.load())
    return;
  m_mainFont = m_fcache->prepMainFont(AllCharFilter, false, 10.f, dpi);
  if (m_fcacheInterrupt.load())
    return;
  m_monoFont10 = m_fcache->prepMonoFont(AllCharFilter, false, 10.f, dpi);
  if (m_fcacheInterrupt.load())
    return;
  m_monoFont18 = m_fcache->prepMonoFont(AllCharFilter, false, 18.f, dpi);
  if (m_fcacheInterrupt.load())
    return;
  m_heading14 = m_fcache->prepMainFont(LatinAndJapaneseCharFilter, false, 14.f, dpi);
  if (m_fcacheInterrupt.load())
    return;
  m_heading18 = m_fcache->prepMainFont(LatinAndJapaneseCharFilter, false, 18.f, dpi);
  if (m_fcacheInterrupt.load())
    return;
  m_titleFont = m_fcache->prepMainFont(LatinCharFilter, false, 36.f, dpi);
  if (m_fcacheInterrupt.load())
    return;
  m_fcache->closeBuiltinFonts();
  m_fcacheReady.store(true);
}

void ViewResources::prepFontCacheAsync(boo::IWindow* window) {
  m_fcacheReady.store(false);
  m_fcacheThread = std::thread([this]() { prepFontCacheSync(); });
}

void ViewResources::resetPixelFactor(float pf) {
  m_pixelFactor = pf;
  unsigned dpi = 72.f * m_pixelFactor;
  m_curveFont = m_fcache->prepCurvesFont(AllCharFilter, false, 8.f, dpi);
  prepFontCacheSync();
}

void ViewResources::resetTheme(const IThemeData* theme) { m_theme = theme; }

} // namespace specter
