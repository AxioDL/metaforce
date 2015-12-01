#include "Specter/ViewSystem.hpp"

namespace Specter
{
static LogVisor::LogModule Log("Specter");

void ViewSystem::init(boo::IGraphicsDataFactory* factory, FontCache* fcache)
{
    m_factory = factory;
    m_mainFont = fcache->prepMainFont(factory, FontCache::DefaultCharFilter, true, 10.0, 72);
    m_monoFont = fcache->prepMonoFont(factory, FontCache::DefaultCharFilter, false, 10.0, 72);
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
    m_sysData.reset(factory->commit());
}

}
