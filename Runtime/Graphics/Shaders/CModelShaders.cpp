#include "CModelShaders.hpp"
#include "Graphics/CLight.hpp"

namespace urde
{

std::experimental::optional<CModelShaders> CModelShaders::g_ModelShaders;

void CModelShaders::LightingUniform::ActivateLights(const std::vector<CLight>& lts)
{
    ambient = zeus::CColor::skBlack;
    size_t curLight = 0;

    for (const CLight& light : lts)
    {
        switch (light.GetType())
        {
        case ELightType::LocalAmbient:
            ambient += light.GetColor();
            break;
        case ELightType::Point:
        case ELightType::Spot:
        case ELightType::Custom:
        case ELightType::Directional:
        {
            if (curLight >= URDE_MAX_LIGHTS)
                continue;
            CModelShaders::Light& lightOut = lights[curLight++];
            lightOut.pos = CGraphics::g_CameraMatrix * light.GetPosition();
            lightOut.dir = CGraphics::g_CameraMatrix.basis * light.GetDirection();
            lightOut.dir.normalize();
            lightOut.color = light.GetColor();
            lightOut.linAtt[0] = light.GetAttenuationConstant();
            lightOut.linAtt[1] = light.GetAttenuationLinear();
            lightOut.linAtt[2] = light.GetAttenuationQuadratic();
            lightOut.angAtt[0] = light.GetAngleAttenuationConstant();
            lightOut.angAtt[1] = light.GetAngleAttenuationLinear();
            lightOut.angAtt[2] = light.GetAngleAttenuationQuadratic();

            if (light.GetType() == ELightType::Directional)
                lightOut.pos = (-lightOut.dir) * 1048576.f;
            break;
        }
        }
    }

    for (; curLight<URDE_MAX_LIGHTS ; ++curLight)
    {
        CModelShaders::Light& lightOut = lights[curLight];
        lightOut.color = zeus::CColor::skClear;
        lightOut.linAtt[0] = 1.f;
        lightOut.angAtt[0] = 1.f;
    }
}

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

const hecl::Backend::TextureInfo CModelShaders::WorldShadowTextures[] =
{
    {hecl::Backend::TexGenSrc::Position, 7, 0, 7, false} // Shadow tex
};

CModelShaders::CModelShaders(const hecl::Runtime::FileStoreManager& storeMgr,
                             boo::IGraphicsDataFactory* gfxFactory)
    : m_shaderCache(storeMgr, gfxFactory, GetShaderExtensions(gfxFactory->platform())) {}

void CModelShaders::Initialize(const hecl::Runtime::FileStoreManager& storeMgr,
                               boo::IGraphicsDataFactory* gfxFactory)
{
    g_ModelShaders.emplace(storeMgr, gfxFactory);
}


void CModelShaders::Shutdown()
{
    g_ModelShaders = std::experimental::nullopt;
}
}
