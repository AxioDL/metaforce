#include "CModelShaders.hpp"

namespace urde
{

std::experimental::optional<CModelShaders> CModelShaders::g_ModelShaders;

hecl::Runtime::ShaderCacheExtensions
CModelShaders::GetShaderExtensions(boo::IGraphicsDataFactory::Platform plat)
{
    switch (plat)
    {
    case boo::IGraphicsDataFactory::Platform::OpenGL:
    case boo::IGraphicsDataFactory::Platform::Vulkan:
        return GetShaderExtensionsGLSL(plat);
#if _WIN32
    case boo::IGraphicsDataFactory::Platform::D3D11:
    case boo::IGraphicsDataFactory::Platform::D3D12:
        return GetShaderExtensionsHLSL(plat);
#endif
#if BOO_HAS_METAL
    case boo::IGraphicsDataFactory::Platform::Metal:
        return GetShaderExtensionsMetal(plat);
#endif
    default:
        return {boo::IGraphicsDataFactory::Platform::Null};
    }
}

const hecl::Backend::TextureInfo CModelShaders::ThermalTextures[] =
{
    {hecl::Backend::TexGenSrc::Normal, 7, 0, 7, true}
};

const hecl::Backend::TextureInfo CModelShaders::BallFadeTextures[] =
{
    {hecl::Backend::TexGenSrc::Position, 0, 0, 0, false}, // ID tex
    {hecl::Backend::TexGenSrc::Position, 1, 0, 0, false}, // Sphere ramp
    {hecl::Backend::TexGenSrc::Position, 2, 0, 1, false}  // TXTR_BallFade
};

CModelShaders::CModelShaders(const hecl::Runtime::FileStoreManager& storeMgr,
                             boo::IGraphicsDataFactory* gfxFactory)
    : m_shaderCache(storeMgr, gfxFactory, GetShaderExtensions(gfxFactory->platform())) {}

void CModelShaders::Initialize(const hecl::Runtime::FileStoreManager& storeMgr,
                               boo::IGraphicsDataFactory* gfxFactory)
{
    g_ModelShaders.emplace(storeMgr, gfxFactory);
}

}
