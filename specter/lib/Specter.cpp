#include "Specter/ViewResources.hpp"

namespace Specter
{
static LogVisor::LogModule Log("Specter");

void ViewResources::init(boo::IGraphicsDataFactory* factory, FontCache* fcache, unsigned dpi)
{
    m_factory = factory;
    m_mainFont = fcache->prepMainFont(factory, AllCharFilter, false, 10.0, dpi);
    m_monoFont = fcache->prepMonoFont(factory, AllCharFilter, false, 10.0, dpi);
    m_heading14 = fcache->prepMainFont(factory, LatinAndJapaneseCharFilter, false, 14.0, dpi);
    m_heading18 = fcache->prepMainFont(factory, LatinAndJapaneseCharFilter, false, 18.0, dpi);
    switch (factory->platform())
    {
    case boo::IGraphicsDataFactory::Platform::OGL:
        init<boo::GLDataFactory>(static_cast<boo::GLDataFactory*>(factory), fcache);
        break;
#if _WIN32
    case boo::IGraphicsDataFactory::Platform::D3D11:
    case boo::IGraphicsDataFactory::Platform::D3D12:
        init<boo::ID3DDataFactory>(static_cast<boo::ID3DDataFactory*>(factory), fcache);
        break;
#elif BOO_HAS_METAL
    case boo::IGraphicsDataFactory::Platform::Metal:
        init<boo::MetalDataFactory>(static_cast<boo::MetalDataFactory*>(factory), fcache);
        break;
#endif
    default:
        Log.report(LogVisor::FatalError, _S("unable to init view system for %s"), factory->platformName());
    }
    fcache->closeBuiltinFonts();
    m_resData.reset(factory->commit());
}

}
