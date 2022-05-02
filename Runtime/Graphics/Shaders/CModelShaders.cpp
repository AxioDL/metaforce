#include "Runtime/Graphics/Shaders/CModelShaders.hpp"

#include "Runtime/CStopwatch.hpp"
#include "Runtime/Graphics/CLight.hpp"

//#include <hecl/Backend.hpp>
//#include <hecl/Pipeline.hpp>

namespace metaforce {

//std::unordered_map<uint64_t, CModelShaders::ShaderPipelines> CModelShaders::g_ShaderPipelines;

//void CModelShaders::LightingUniform::ActivateLights(const std::vector<CLight>& lts) {
//  ambient = zeus::skClear;
//  size_t curLight = 0;
//
//  for (const CLight& light : lts) {
//    switch (light.GetType()) {
//    case ELightType::LocalAmbient:
//      ambient += light.GetColor();
//      break;
//    case ELightType::Point:
//    case ELightType::Spot:
//    case ELightType::Custom:
//    case ELightType::Directional: {
//      if (curLight >= lights.size()) {
//        continue;
//      }
//      CModelShaders::Light& lightOut = lights[curLight++];
//      lightOut.pos = CGraphics::g_CameraMatrix * light.GetPosition();
//      lightOut.dir = CGraphics::g_CameraMatrix.basis * light.GetDirection();
//      lightOut.dir.normalize();
//      lightOut.color = light.GetColor();
//      lightOut.linAtt[0] = light.GetAttenuationConstant();
//      lightOut.linAtt[1] = light.GetAttenuationLinear();
//      lightOut.linAtt[2] = light.GetAttenuationQuadratic();
//      lightOut.angAtt[0] = light.GetAngleAttenuationConstant();
//      lightOut.angAtt[1] = light.GetAngleAttenuationLinear();
//      lightOut.angAtt[2] = light.GetAngleAttenuationQuadratic();
//
//      if (light.GetType() == ELightType::Directional)
//        lightOut.pos = (-lightOut.dir) * 1048576.f;
//      break;
//    }
//    }
//  }
//
//  for (; curLight < lights.size(); ++curLight) {
//    CModelShaders::Light& lightOut = lights[curLight];
//    lightOut.pos = zeus::skZero3f;
//    lightOut.dir = zeus::skDown;
//    lightOut.color = zeus::skClear;
//    lightOut.linAtt[0] = 1.f;
//    lightOut.linAtt[1] = 0.f;
//    lightOut.linAtt[2] = 0.f;
//    lightOut.angAtt[0] = 1.f;
//    lightOut.angAtt[1] = 0.f;
//    lightOut.angAtt[2] = 0.f;
//  }
//}

//using TexCoordSource = hecl::Backend::TexCoordSource;
//
//constexpr std::array<hecl::Backend::TextureInfo, 1> ThermalTextures{{
//    {TexCoordSource::Normal, 7, true},
//}};
//
//constexpr std::array<hecl::Backend::TextureInfo, 3> BallFadeTextures{{
//    {TexCoordSource::Position, 0, false}, // ID tex
//    {TexCoordSource::Position, 0, false}, // Sphere ramp
//    {TexCoordSource::Position, 1, false}, // TXTR_BallFade
//}};
//
//constexpr std::array<hecl::Backend::TextureInfo, 1> WorldShadowTextures{{
//    {TexCoordSource::Position, 7, false}, // Shadow tex
//}};
//
//constexpr std::array<hecl::Backend::TextureInfo, 2> DisintegrateTextures{{
//    {TexCoordSource::Position, 0, false}, // Ashy tex
//    {TexCoordSource::Position, 1, false}, // Ashy tex
//}};

//static std::array<hecl::Backend::ExtensionSlot, size_t(EExtendedShader::MAX)> g_ExtensionSlots{{
//    /* Default solid shading */
//    {},
//    /* Normal lit shading */
//    {0, nullptr, hecl::Backend::BlendFactor::Original, hecl::Backend::BlendFactor::Original,
//     hecl::Backend::ZTest::Original, hecl::Backend::CullMode::Backface, false, false, true},
//    /* Thermal model shading */
//    {1, ThermalTextures.data(), hecl::Backend::BlendFactor::One, hecl::Backend::BlendFactor::One,
//     hecl::Backend::ZTest::Original, hecl::Backend::CullMode::Backface, false, false, false, true},
//    /* Thermal model shading without Z-test or Z-write */
//    {1, ThermalTextures.data(), hecl::Backend::BlendFactor::One, hecl::Backend::BlendFactor::One,
//     hecl::Backend::ZTest::None, hecl::Backend::CullMode::Backface, true, false, false, true},
//    /* Thermal static shading */
//    {0, nullptr, hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Original,
//     hecl::Backend::CullMode::Backface, false, false, false, true, false, false, true},
//    /* Thermal static shading without Z-write */
//    {0, nullptr, hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Original,
//     hecl::Backend::CullMode::Backface, true, false, false, true, false, false, false},
//    /* Forced alpha shading */
//    {0, nullptr, hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::InvSrcAlpha,
//     hecl::Backend::ZTest::Original, hecl::Backend::CullMode::Backface, false, false, true},
//    /* Forced additive shading */
//    {0, nullptr, hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Original,
//     hecl::Backend::CullMode::Backface, false, false, true},
//    /* Solid color */
//    {0, nullptr, hecl::Backend::BlendFactor::One, hecl::Backend::BlendFactor::Zero, hecl::Backend::ZTest::LEqual,
//     hecl::Backend::CullMode::Backface, false, false, false},
//    /* Solid color additive */
//    {0, nullptr, hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::LEqual,
//     hecl::Backend::CullMode::Backface, true, false, true},
//    /* Alpha-only Solid color frontface cull, LEqual */
//    {0, nullptr, hecl::Backend::BlendFactor::Zero, hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::LEqual,
//     hecl::Backend::CullMode::Frontface, false, true, false},
//    /* Alpha-only Solid color frontface cull, Always, No Z-write */
//    {0, nullptr, hecl::Backend::BlendFactor::Zero, hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::None,
//     hecl::Backend::CullMode::Frontface, true, true, false},
//    /* Alpha-only Solid color backface cull, LEqual */
//    {0, nullptr, hecl::Backend::BlendFactor::Zero, hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::LEqual,
//     hecl::Backend::CullMode::Backface, false, true, false},
//    /* Alpha-only Solid color backface cull, Greater, No Z-write */
//    {0, nullptr, hecl::Backend::BlendFactor::Zero, hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Greater,
//     hecl::Backend::CullMode::Backface, true, true, false},
//    /* MorphBall shadow shading */
//    {3, BallFadeTextures.data(), hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::InvSrcAlpha,
//     hecl::Backend::ZTest::Equal, hecl::Backend::CullMode::Backface, false, false, true, false, true},
//    /* World shadow shading (modified lighting) */
//    {1, WorldShadowTextures.data(), hecl::Backend::BlendFactor::Original, hecl::Backend::BlendFactor::Original,
//     hecl::Backend::ZTest::Original, hecl::Backend::CullMode::Backface, false, false, true},
//    /* Forced alpha shading without culling */
//    {0, nullptr, hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::InvSrcAlpha,
//     hecl::Backend::ZTest::Original, hecl::Backend::CullMode::None, false, false, true},
//    /* Forced additive shading without culling */
//    {0, nullptr, hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Original,
//     hecl::Backend::CullMode::None, false, false, true},
//    /* Forced alpha shading without Z-write */
//    {0, nullptr, hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::InvSrcAlpha,
//     hecl::Backend::ZTest::Original, hecl::Backend::CullMode::Original, true, false, true},
//    /* Forced additive shading without Z-write */
//    {0, nullptr, hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Original,
//     hecl::Backend::CullMode::Original, true, false, true},
//    /* Forced alpha shading without culling or Z-write */
//    {0, nullptr, hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::InvSrcAlpha,
//     hecl::Backend::ZTest::Original, hecl::Backend::CullMode::None, true, false, true},
//    /* Forced additive shading without culling or Z-write */
//    {0, nullptr, hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Original,
//     hecl::Backend::CullMode::None, true, false, true},
//    /* Depth GEqual no Z-write */
//    {0, nullptr, hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::InvSrcAlpha,
//     hecl::Backend::ZTest::GEqual, hecl::Backend::CullMode::Backface, true, false, true},
//    /* Disintegration */
//    {2, DisintegrateTextures.data(), hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::InvSrcAlpha,
//     hecl::Backend::ZTest::LEqual, hecl::Backend::CullMode::Original, false, false, true, false, false, true},
//    /* Forced additive shading without culling or Z-write and greater depth test */
//    {0, nullptr, hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Greater,
//     hecl::Backend::CullMode::None, true, false, true},
//    /* Thermal cold shading */
//    {0, nullptr, hecl::Backend::BlendFactor::Original, hecl::Backend::BlendFactor::Original,
//     hecl::Backend::ZTest::Original, hecl::Backend::CullMode::Original, false, false, true, false, false, false, true},
//    /* Normal lit shading with alpha */
//    {0, nullptr, hecl::Backend::BlendFactor::Original, hecl::Backend::BlendFactor::Original,
//     hecl::Backend::ZTest::Original, hecl::Backend::CullMode::Backface},
//    /* Normal lit shading with alpha without Z-write or depth test */
//    {0, nullptr, hecl::Backend::BlendFactor::Original, hecl::Backend::BlendFactor::Original, hecl::Backend::ZTest::None,
//     hecl::Backend::CullMode::Backface, true},
//    /* Normal lit shading with cube reflection */
//    {0, nullptr, hecl::Backend::BlendFactor::Original, hecl::Backend::BlendFactor::Original,
//     hecl::Backend::ZTest::Original, hecl::Backend::CullMode::Backface, false, false, true},
//    /* Normal lit shading with cube reflection and world shadow */
//    {1, WorldShadowTextures.data(), hecl::Backend::BlendFactor::Original, hecl::Backend::BlendFactor::Original,
//     hecl::Backend::ZTest::Original, hecl::Backend::CullMode::Backface, false, false, true},
//}};

//constexpr std::array<const char*, size_t(EExtendedShader::MAX)> ShaderMacros{
//    "URDE_LIGHTING",
//    "URDE_LIGHTING",
//    "URDE_THERMAL_MODEL",
//    "URDE_THERMAL_MODEL",
//    "URDE_THERMAL_STATIC",
//    "URDE_THERMAL_STATIC",
//    "URDE_LIGHTING",
//    "URDE_LIGHTING",
//    "URDE_SOLID",
//    "URDE_SOLID",
//    "URDE_SOLID",
//    "URDE_SOLID",
//    "URDE_SOLID",
//    "URDE_SOLID",
//    "URDE_MB_SHADOW",
//    "URDE_LIGHTING_SHADOW",
//    "URDE_LIGHTING",
//    "URDE_LIGHTING",
//    "URDE_LIGHTING",
//    "URDE_LIGHTING",
//    "URDE_LIGHTING",
//    "URDE_LIGHTING",
//    "URDE_LIGHTING",
//    "URDE_DISINTEGRATE",
//    "URDE_LIGHTING",
//    "URDE_THERMAL_COLD",
//    "URDE_LIGHTING",
//    "URDE_LIGHTING",
//    "URDE_LIGHTING_CUBE_REFLECTION",
//    "URDE_LIGHTING_CUBE_REFLECTION_SHADOW",
//};

void CModelShaders::Initialize() {
//  for (size_t i = 0; i < g_ExtensionSlots.size(); i++) {
//    g_ExtensionSlots[i].shaderMacro = ShaderMacros[i];
//  }
}

void CModelShaders::Shutdown() {
//  g_ShaderPipelines.clear();
}

//CModelShaders::ShaderPipelines CModelShaders::BuildExtendedShader(const hecl::Backend::ShaderTag& tag,
//                                                                  const Material& material) {
//  auto search = g_ShaderPipelines.find(tag.val64());
//  if (search != g_ShaderPipelines.cend())
//    return search->second;
//
//  ShaderPipelines& newPipelines = g_ShaderPipelines[tag.val64()];
//  newPipelines = std::make_shared<ShaderPipelinesData>();
//  CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) {
//    size_t idx = 0;
//    for (const auto& ext : g_ExtensionSlots)
//      (*newPipelines)[idx++] = hecl::conv->convert(ctx, Shader_CModelShaders(SModelShadersInfo(material, tag, ext)));
//    return true;
//  } BooTrace);
//  return newPipelines;
//}

} // namespace metaforce
