#include "Specter/ViewResources.hpp"

namespace Specter
{
static LogVisor::LogModule Log("Specter::ViewResources");

void ViewResources::init(boo::IGraphicsDataFactory* factory, FontCache* fcache,
                         const ThemeData& theme, float pf)
{
    m_pixelFactor = pf;
    m_theme = theme;
    m_factory = factory;
    m_fcache = fcache;
    unsigned dpi = 72.f * m_pixelFactor;
    m_curveFont = fcache->prepCurvesFont(m_factory, AllCharFilter, false, 8.f, dpi);
    switch (factory->platform())
    {
    case boo::IGraphicsDataFactory::Platform::OGL:
        init<boo::GLDataFactory>(static_cast<boo::GLDataFactory*>(factory), theme, fcache);
        break;
#if _WIN32
    case boo::IGraphicsDataFactory::Platform::D3D11:
    case boo::IGraphicsDataFactory::Platform::D3D12:
        init<boo::ID3DDataFactory>(static_cast<boo::ID3DDataFactory*>(factory), theme, fcache);
        break;
#elif BOO_HAS_METAL
    case boo::IGraphicsDataFactory::Platform::Metal:
        init<boo::MetalDataFactory>(static_cast<boo::MetalDataFactory*>(factory), theme, fcache);
        break;
#endif
    default:
        Log.report(LogVisor::FatalError, _S("unable to init view system for %s"), factory->platformName());
    }
    m_resData = factory->commit();
}

void ViewResources::prepFontCacheSync()
{
    unsigned dpi = 72.f * m_pixelFactor;
    m_mainFont = m_fcache->prepMainFont(m_factory, AllCharFilter, false, 10.f, dpi);
    m_monoFont = m_fcache->prepMonoFont(m_factory, AllCharFilter, false, 10.f, dpi);
    m_heading14 = m_fcache->prepMainFont(m_factory, LatinAndJapaneseCharFilter, false, 14.f, dpi);
    m_heading18 = m_fcache->prepMainFont(m_factory, LatinAndJapaneseCharFilter, false, 18.f, dpi);
    m_titleFont = m_fcache->prepMainFont(m_factory, LatinCharFilter, false, 36.f, dpi);
    m_fcache->closeBuiltinFonts();
    m_fcacheReady = true;
}

void ViewResources::prepFontCacheAsync(boo::IWindow* window)
{
    m_fcacheReady = false;
    m_fcacheThread = std::thread([this, window]()
    {
        window->getLoadContextDataFactory();
        prepFontCacheSync();
    });
}

void ViewResources::resetPixelFactor(float pf)
{
    m_pixelFactor = pf;
    unsigned dpi = 72.f * m_pixelFactor;
    m_curveFont = m_fcache->prepCurvesFont(m_factory, AllCharFilter, false, 8.f, dpi);
    prepFontCacheSync();
}

void ViewResources::resetTheme(const ThemeData& theme)
{
    m_theme = theme;
}

}
