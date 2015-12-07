#include "Specter/ViewResources.hpp"

namespace Specter
{
static LogVisor::LogModule Log("Specter::ViewResources");

void ViewResources::init(boo::IGraphicsDataFactory* factory, FontCache* fcache,
                         const ThemeData& theme, unsigned dpi)
{
    m_pixelFactor = dpi / 72.f;
    m_theme = theme;
    m_factory = factory;
    m_fcache = fcache;
    m_mainFont = fcache->prepMainFont(factory, AllCharFilter, false, 10.f, dpi);
    m_monoFont = fcache->prepMonoFont(factory, AllCharFilter, false, 10.f, dpi);
    m_heading14 = fcache->prepMainFont(factory, LatinAndJapaneseCharFilter, false, 14.f, dpi);
    m_heading18 = fcache->prepMainFont(factory, LatinAndJapaneseCharFilter, false, 18.f, dpi);
    m_curveFont = fcache->prepCurvesFont(factory, AllCharFilter, false, 11.f, dpi);
    m_fontData = factory->commit();
    switch (factory->platform())
    {
    case boo::IGraphicsDataFactory::Platform::OGL:
        init<boo::GLDataFactory>(static_cast<boo::GLDataFactory*>(factory), fcache, theme);
        break;
#if _WIN32
    case boo::IGraphicsDataFactory::Platform::D3D11:
    case boo::IGraphicsDataFactory::Platform::D3D12:
        init<boo::ID3DDataFactory>(static_cast<boo::ID3DDataFactory*>(factory), fcache, theme);
        break;
#elif BOO_HAS_METAL
    case boo::IGraphicsDataFactory::Platform::Metal:
        init<boo::MetalDataFactory>(static_cast<boo::MetalDataFactory*>(factory), fcache, theme);
        break;
#endif
    default:
        Log.report(LogVisor::FatalError, _S("unable to init view system for %s"), factory->platformName());
    }
    fcache->closeBuiltinFonts();
    m_resData = factory->commit();
}

void ViewResources::resetDPI(unsigned dpi)
{
    m_pixelFactor = dpi / 72.f;
    m_mainFont = m_fcache->prepMainFont(m_factory, AllCharFilter, false, 10.f, dpi);
    m_monoFont = m_fcache->prepMonoFont(m_factory, AllCharFilter, false, 10.f, dpi);
    m_heading14 = m_fcache->prepMainFont(m_factory, LatinAndJapaneseCharFilter, false, 14.f, dpi);
    m_heading18 = m_fcache->prepMainFont(m_factory, LatinAndJapaneseCharFilter, false, 18.f, dpi);
    m_curveFont = m_fcache->prepCurvesFont(m_factory, AllCharFilter, false, 11.f, dpi);
    m_fontData = m_factory->commit();
    m_fcache->closeBuiltinFonts();
}

void ViewResources::resetTheme(const ThemeData& theme)
{
    m_theme = theme;
}

}
