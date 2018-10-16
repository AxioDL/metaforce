#include "CModelShaders.hpp"
#include "Graphics/CLight.hpp"
#include "hecl/Pipeline.hpp"

namespace urde
{

std::unordered_map<uint64_t, CModelShaders::ShaderPipelines> CModelShaders::g_ShaderPipelines;

void CModelShaders::LightingUniform::ActivateLights(const std::vector<CLight>& lts)
{
    ambient = zeus::CColor::skClear;
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

static const hecl::Backend::TextureInfo ThermalTextures[] =
{
    {hecl::Backend::TexGenSrc::Normal, 7, 0, 7, true}
};

static const hecl::Backend::TextureInfo BallFadeTextures[] =
{
    {hecl::Backend::TexGenSrc::Position, 0, 0, 0, false}, // ID tex
    {hecl::Backend::TexGenSrc::Position, 1, 0, 0, false}, // Sphere ramp
    {hecl::Backend::TexGenSrc::Position, 2, 0, 1, false}  // TXTR_BallFade
};

static const hecl::Backend::TextureInfo WorldShadowTextures[] =
{
    {hecl::Backend::TexGenSrc::Position, 7, 0, 7, false} // Shadow tex
};

static const char* BlockNames[] = {"LightingUniform"};
static const char* ThermalBlockNames[] = {"ThermalUniform"};
static const char* SolidBlockNames[] = {"SolidUniform"};
static const char* MBShadowBlockNames[] = {"MBShadowUniform"};

static hecl::Backend::ExtensionSlot g_ExtensionSlots[] =
{
    /* Default solid shading */
    {},
    /* Normal lit shading */
    {1, BlockNames, 0, nullptr, hecl::Backend::BlendFactor::Original,
        hecl::Backend::BlendFactor::Original, hecl::Backend::ZTest::Original,
        hecl::Backend::CullMode::Backface, false, false, true},
    /* Thermal Visor shading */
    {1, ThermalBlockNames, 1, ThermalTextures, hecl::Backend::BlendFactor::One,
        hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Original,
        hecl::Backend::CullMode::Backface, false, false, false, true},
    /* Forced alpha shading */
    {1, BlockNames, 0, nullptr, hecl::Backend::BlendFactor::SrcAlpha,
        hecl::Backend::BlendFactor::InvSrcAlpha, hecl::Backend::ZTest::Original,
        hecl::Backend::CullMode::Backface, false, false, true},
    /* Forced additive shading */
    {1, BlockNames, 0, nullptr, hecl::Backend::BlendFactor::SrcAlpha,
        hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Original,
        hecl::Backend::CullMode::Backface, true, false, true},
    /* Solid color */
    {1, SolidBlockNames, 0, nullptr, hecl::Backend::BlendFactor::One,
        hecl::Backend::BlendFactor::Zero, hecl::Backend::ZTest::LEqual,
        hecl::Backend::CullMode::Backface, false, false, false},
    /* Solid color additive */
    {1, SolidBlockNames, 0, nullptr, hecl::Backend::BlendFactor::SrcAlpha,
        hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::LEqual,
        hecl::Backend::CullMode::Backface, true, false, true},
    /* Alpha-only Solid color frontface cull, LEqual */
    {1, SolidBlockNames, 0, nullptr, hecl::Backend::BlendFactor::Zero,
        hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::LEqual,
        hecl::Backend::CullMode::Frontface, false, true, false},
    /* Alpha-only Solid color frontface cull, Always, No Z-write */
    {1, SolidBlockNames, 0, nullptr, hecl::Backend::BlendFactor::Zero,
        hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::None,
        hecl::Backend::CullMode::Frontface, true, true, false},
    /* Alpha-only Solid color backface cull, LEqual */
    {1, SolidBlockNames, 0, nullptr, hecl::Backend::BlendFactor::Zero,
        hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::LEqual,
        hecl::Backend::CullMode::Backface, false, true, false},
    /* Alpha-only Solid color backface cull, Greater, No Z-write */
    {1, SolidBlockNames, 0, nullptr, hecl::Backend::BlendFactor::Zero,
        hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Greater,
        hecl::Backend::CullMode::Backface, true, true, false},
    /* MorphBall shadow shading */
    {1, MBShadowBlockNames, 3, BallFadeTextures,
        hecl::Backend::BlendFactor::SrcAlpha,
        hecl::Backend::BlendFactor::InvSrcAlpha,
        hecl::Backend::ZTest::Equal,
        hecl::Backend::CullMode::Backface, false, false, true, false, true},
    /* World shadow shading (modified lighting) */
    {1, BlockNames, 1, WorldShadowTextures, hecl::Backend::BlendFactor::Original,
        hecl::Backend::BlendFactor::Original, hecl::Backend::ZTest::Original,
        hecl::Backend::CullMode::Backface, false, false, true},
    /* Forced alpha shading without culling */
    {1, BlockNames, 0, nullptr, hecl::Backend::BlendFactor::SrcAlpha,
        hecl::Backend::BlendFactor::InvSrcAlpha, hecl::Backend::ZTest::Original,
        hecl::Backend::CullMode::None, false, false, true},
    /* Forced additive shading without culling */
    {1, BlockNames, 0, nullptr, hecl::Backend::BlendFactor::SrcAlpha,
        hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Original,
        hecl::Backend::CullMode::None, false, false, true},
    /* Forced alpha shading without Z-write */
    {1, BlockNames, 0, nullptr, hecl::Backend::BlendFactor::SrcAlpha,
        hecl::Backend::BlendFactor::InvSrcAlpha, hecl::Backend::ZTest::Original,
        hecl::Backend::CullMode::Original, true, false, true},
    /* Forced additive shading without Z-write */
    {1, BlockNames, 0, nullptr, hecl::Backend::BlendFactor::SrcAlpha,
        hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Original,
        hecl::Backend::CullMode::Original, true, false, true},
    /* Forced alpha shading without culling or Z-write */
    {1, BlockNames, 0, nullptr, hecl::Backend::BlendFactor::SrcAlpha,
        hecl::Backend::BlendFactor::InvSrcAlpha, hecl::Backend::ZTest::Original,
        hecl::Backend::CullMode::None, true, false, true},
    /* Forced additive shading without culling or Z-write */
    {1, BlockNames, 0, nullptr, hecl::Backend::BlendFactor::SrcAlpha,
        hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Original,
        hecl::Backend::CullMode::None, true, false, true},
    /* Depth GEqual no Z-write */
    {1, BlockNames, 0, nullptr, hecl::Backend::BlendFactor::SrcAlpha,
        hecl::Backend::BlendFactor::InvSrcAlpha, hecl::Backend::ZTest::GEqual,
        hecl::Backend::CullMode::Backface, true, false, true}
};

extern const hecl::Backend::Function ExtensionLightingFuncsGLSL[];
extern const hecl::Backend::Function ExtensionPostFuncsGLSL[];
extern const hecl::Backend::Function ExtensionLightingFuncsHLSL[];
extern const hecl::Backend::Function ExtensionPostFuncsHLSL[];
extern const hecl::Backend::Function ExtensionLightingFuncsMetal[];
extern const hecl::Backend::Function ExtensionPostFuncsMetal[];

void CModelShaders::Initialize()
{
    const hecl::Backend::Function* lightingFuncs;
    const hecl::Backend::Function* postFuncs;
    switch (CGraphics::g_BooPlatform)
    {
    case boo::IGraphicsDataFactory::Platform::OpenGL:
    case boo::IGraphicsDataFactory::Platform::Vulkan:
    case boo::IGraphicsDataFactory::Platform::NX:
    default:
        lightingFuncs = ExtensionLightingFuncsGLSL;
        postFuncs = ExtensionPostFuncsGLSL;
        break;
    case boo::IGraphicsDataFactory::Platform::D3D11:
        lightingFuncs = ExtensionLightingFuncsHLSL;
        postFuncs = ExtensionPostFuncsHLSL;
        break;
    case boo::IGraphicsDataFactory::Platform::Metal:
        lightingFuncs = ExtensionLightingFuncsMetal;
        postFuncs = ExtensionPostFuncsMetal;
        break;
    }
    for (auto& ext : g_ExtensionSlots)
    {
        ext.lighting = *lightingFuncs++;
        ext.post = *postFuncs++;
    }
}

void CModelShaders::Shutdown()
{
    g_ShaderPipelines.clear();
}

CModelShaders::ShaderPipelines CModelShaders::BuildExtendedShader(const hecl::Backend::ShaderTag& tag,
                                                                  const hecl::Frontend::IR& ir)
{
    auto search = g_ShaderPipelines.find(tag.val64());
    if (search != g_ShaderPipelines.cend())
        return search->second;
    ShaderPipelines& newPipelines = g_ShaderPipelines[tag.val64()];
    newPipelines = std::make_shared<ShaderPipelinesData>();
    size_t idx = 0;
    for (const auto& ext : g_ExtensionSlots)
        (*newPipelines)[idx++] = hecl::conv->convert(hecl::HECLIR(ir, tag, ext));
    return newPipelines;
}
}
