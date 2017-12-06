#include "specter/ViewResources.hpp"

namespace specter
{
static logvisor::Module Log("specter::ViewResources");

void ViewResources::init(boo::IGraphicsDataFactory* factory, FontCache* fcache,
                         const IThemeData* theme, float pf)
{
    if (!factory || !fcache || !theme)
        Log.report(logvisor::Fatal, "all arguments of ViewResources::init() must be non-null");
    m_pixelFactor = pf;
    m_factory = factory;
    m_theme = theme;
    m_fcache = fcache;
    unsigned dpi = 72.f * m_pixelFactor;

    m_curveFont = fcache->prepCurvesFont(factory, AllCharFilter, false, 8.f, dpi);

    factory->commitTransaction(
    [&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        switch (ctx.platform())
        {
#if BOO_HAS_GL
        case boo::IGraphicsDataFactory::Platform::OpenGL:
            init<boo::GLDataFactory::Context>(static_cast<boo::GLDataFactory::Context&>(ctx), *theme, fcache);
            break;
#endif
#if _WIN32
        case boo::IGraphicsDataFactory::Platform::D3D11:
        case boo::IGraphicsDataFactory::Platform::D3D12:
            init<boo::ID3DDataFactory::Context>(static_cast<boo::ID3DDataFactory::Context&>(ctx), *theme, fcache);
            break;
#endif
#if BOO_HAS_METAL
        case boo::IGraphicsDataFactory::Platform::Metal:
            init<boo::MetalDataFactory::Context>(static_cast<boo::MetalDataFactory::Context&>(ctx), *theme, fcache);
            break;
#endif
#if BOO_HAS_VULKAN
        case boo::IGraphicsDataFactory::Platform::Vulkan:
            init<boo::VulkanDataFactory::Context>(static_cast<boo::VulkanDataFactory::Context&>(ctx), *theme, fcache);
            break;
#endif
        default:
            Log.report(logvisor::Fatal, _S("unable to init view system for %s"), ctx.platformName());
        }
        return true;
    });
}

void ViewResources::destroyResData()
{
    m_viewRes.destroy();
    m_textRes.destroy();
    m_splitRes.destroy();
    m_toolbarRes.destroy();
    m_buttonRes.destroy();
}

void ViewResources::prepFontCacheSync()
{
    unsigned dpi = 72.f * m_pixelFactor;
    if (m_fcacheInterrupt) return;
    m_mainFont = m_fcache->prepMainFont(m_factory, AllCharFilter, false, 10.f, dpi);
    if (m_fcacheInterrupt) return;
    m_monoFont = m_fcache->prepMonoFont(m_factory, AllCharFilter, false, 10.f, dpi);
    if (m_fcacheInterrupt) return;
    m_heading14 = m_fcache->prepMainFont(m_factory, LatinAndJapaneseCharFilter, false, 14.f, dpi);
    if (m_fcacheInterrupt) return;
    m_heading18 = m_fcache->prepMainFont(m_factory, LatinAndJapaneseCharFilter, false, 18.f, dpi);
    if (m_fcacheInterrupt) return;
    m_titleFont = m_fcache->prepMainFont(m_factory, LatinCharFilter, false, 36.f, dpi);
    if (m_fcacheInterrupt) return;
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

void ViewResources::resetTheme(const IThemeData* theme)
{
    m_theme = theme;
}

}
