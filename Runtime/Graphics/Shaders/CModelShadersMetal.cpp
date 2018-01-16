#include "CModelShaders.hpp"

namespace urde
{

static const char* LightingMetal =
"struct Light\n"
"{\n"
"    float4 pos;\n"
"    float4 dir;\n"
"    float4 color;\n"
"    float4 linAtt;\n"
"    float4 angAtt;\n"
"};\n"
"struct Fog\n"
"{\n"
"    float4 color;\n"
"    float rangeScale;\n"
"    float start;\n"
"};\n"
"\n"
"struct LightingUniform\n"
"{\n"
"    Light lights[" _XSTR(URDE_MAX_LIGHTS) "];\n"
"    float4 ambient;\n"
"    float4 colorReg0;\n"
"    float4 colorReg1;\n"
"    float4 colorReg2;\n"
"    float4 mulColor;\n"
"    Fog fog;\n"
"};\n"
"\n"
"static float4 LightingFunc(constant LightingUniform& lu, float4 mvPosIn, float4 mvNormIn, thread VertToFrag& vtf)\n"
"{\n"
"    float4 ret = lu.ambient;\n"
"    \n"
"    for (int i=0 ; i<" _XSTR(URDE_MAX_LIGHTS) " ; ++i)\n"
"    {\n"
"        float3 delta = mvPosIn.xyz - lu.lights[i].pos.xyz;\n"
"        float dist = length(delta);\n"
"        float angDot = saturate(dot(normalize(delta), lu.lights[i].dir.xyz));\n"
"        float att = 1.0 / (lu.lights[i].linAtt[2] * dist * dist +\n"
"                           lu.lights[i].linAtt[1] * dist +\n"
"                           lu.lights[i].linAtt[0]);\n"
"        float angAtt = lu.lights[i].angAtt[2] * angDot * angDot +\n"
"                       lu.lights[i].angAtt[1] * angDot +\n"
"                       lu.lights[i].angAtt[0];\n"
"        ret += lu.lights[i].color * saturate(angAtt) * att * saturate(dot(normalize(-delta), mvNormIn.xyz));\n"
"    }\n"
"    \n"
"    return saturate(ret);\n"
"}\n";

static const char* LightingShadowMetal =
"struct Light\n"
"{\n"
"    float4 pos;\n"
"    float4 dir;\n"
"    float4 color;\n"
"    float4 linAtt;\n"
"    float4 angAtt;\n"
"};\n"
"struct Fog\n"
"{\n"
"    float4 color;\n"
"    float rangeScale;\n"
"    float start;\n"
"};\n"
"\n"
"struct LightingUniform\n"
"{\n"
"    Light lights[" _XSTR(URDE_MAX_LIGHTS) "];\n"
"    float4 ambient;\n"
"    float4 colorReg0;\n"
"    float4 colorReg1;\n"
"    float4 colorReg2;\n"
"    float4 mulColor;\n"
"    Fog fog;\n"
"};\n"
"\n"
"static float4 EXTLightingShadowFunc(constant LightingUniform& lu, float4 mvPosIn, float4 mvNormIn,\n"
"                                    thread VertToFrag& vtf, sampler samp, sampler clampSamp, texture2d<float> extTex7)\n"
"{\n"
"    float4 ret = lu.ambient;\n"
"    \n"
"    float3 delta = mvPosIn.xyz - lu.lights[0].pos.xyz;\n"
"    float dist = length(delta);\n"
"    float angDot = saturate(dot(normalize(delta), lu.lights[0].dir.xyz));\n"
"    float att = 1.0 / (lu.lights[0].linAtt[2] * dist * dist +\n"
"                       lu.lights[0].linAtt[1] * dist +\n"
"                       lu.lights[0].linAtt[0]);\n"
"    float angAtt = lu.lights[0].angAtt[2] * angDot * angDot +\n"
"                   lu.lights[0].angAtt[1] * angDot +\n"
"                   lu.lights[0].angAtt[0];\n"
"    ret += lu.lights[0].color * saturate(angAtt) * att * saturate(dot(normalize(-delta), mvNormIn.xyz)) *\n"
"           extTex7.sample(clampSamp, vtf.extTcgs0).r;\n"
"    \n"
"    for (int i=1 ; i<" _XSTR(URDE_MAX_LIGHTS) " ; ++i)\n"
"    {\n"
"        float3 delta = mvPosIn.xyz - lu.lights[i].pos.xyz;\n"
"        float dist = length(delta);\n"
"        float angDot = saturate(dot(normalize(delta), lu.lights[i].dir.xyz));\n"
"        float att = 1.0 / (lu.lights[i].linAtt[2] * dist * dist +\n"
"                           lu.lights[i].linAtt[1] * dist +\n"
"                           lu.lights[i].linAtt[0]);\n"
"        float angAtt = lu.lights[i].angAtt[2] * angDot * angDot +\n"
"                       lu.lights[i].angAtt[1] * angDot +\n"
"                       lu.lights[i].angAtt[0];\n"
"        ret += lu.lights[i].color * saturate(angAtt) * att * saturate(dot(normalize(-delta), mvNormIn.xyz));\n"
"    }\n"
"    \n"
"    return saturate(ret);\n"
"}\n";

static const char* MainPostMetal =
"float4 MainPostFunc(thread VertToFrag& vtf, constant LightingUniform& lu, float4 colorIn)\n"
"{\n"
"    float fogZ = (-vtf.mvPos.z - lu.fog.start) * lu.fog.rangeScale;\n"
"    return mix(lu.fog.color, colorIn, saturate(exp2(-8.0 * fogZ)));\n"
"}\n"
"\n";

static const char* ThermalPostMetal =
"struct ThermalUniform\n"
"{\n"
"    float4 tmulColor;\n"
"    float4 addColor;\n"
"};\n"
"static float4 EXTThermalPostFunc(thread VertToFrag& vtf, constant ThermalUniform& lu,\n"
"    sampler samp, sampler clampSamp, texture2d<float> extTex7, float4 colorIn)\n"
"{\n"
"    return float4(extTex7.sample(samp, vtf.extTcgs0).rrr * lu.tmulColor.rgb + lu.addColor.rgb, 1.0);\n"
"}\n"
"\n";

static const char* SolidPostMetal =
"struct SolidUniform\n"
"{\n"
"    float4 solidColor;\n"
"};\n"
"static float4 SolidPostFunc(thread VertToFrag& vtf, constant SolidUniform& lu, float4 colorIn)\n"
"{\n"
"    return lu.solidColor;\n"
"}\n"
"\n";

static const char* MBShadowPostMetal =
"struct MBShadowUniform\n"
"{\n"
"    float4 shadowUp;\n"
"    float shadowId;\n"
"};\n"
"static float4 EXTMBShadowPostFunc(thread VertToFrag& vtf, constant MBShadowUniform& su, sampler samp, sampler clampSamp,\n"
"                                  texture2d<float> extTex0, texture2d<float> extTex1, texture2d<float> extTex2, float4 colorIn)\n"
"{\n"
"    float idTexel = extTex0.sample(samp, vtf.extTcgs0).a;\n"
"    float sphereTexel = extTex1.sample(samp, vtf.extTcgs1).a;\n"
"    float fadeTexel = extTex2.sample(samp, vtf.extTcgs2).a;\n"
"    float val = ((fabs(idTexel - su.shadowId) < 0.001) ?\n"
"        (dot(vtf.mvNorm.xyz, su.shadowUp.xyz) * su.shadowUp.w) : 0.0) *\n"
"        sphereTexel * fadeTexel;\n"
"    return float4(0.0, 0.0, 0.0, val);\n"
"}\n"
"\n";

static const char* BlockNames[] = {"LightingUniform"};
static const char* ThermalBlockNames[] = {"ThermalUniform"};
static const char* SolidBlockNames[] = {"SolidUniform"};
static const char* MBShadowBlockNames[] = {"MBShadowUniform"};

hecl::Runtime::ShaderCacheExtensions
CModelShaders::GetShaderExtensionsMetal(boo::IGraphicsDataFactory::Platform plat)
{
    hecl::Runtime::ShaderCacheExtensions ext(plat);

    /* Normal lit shading */
    ext.registerExtensionSlot({LightingMetal, "LightingFunc"}, {MainPostMetal, "MainPostFunc"},
                              1, BlockNames, 0, nullptr, hecl::Backend::BlendFactor::Original,
                              hecl::Backend::BlendFactor::Original, hecl::Backend::ZTest::Original,
                              hecl::Backend::CullMode::Backface, false, false, true);

    /* Thermal Visor shading */
    ext.registerExtensionSlot({}, {ThermalPostMetal, "EXTThermalPostFunc"}, 1, ThermalBlockNames,
                              1, ThermalTextures, hecl::Backend::BlendFactor::One,
                              hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Original,
                              hecl::Backend::CullMode::Backface, false, false, true);

    /* Forced alpha shading */
    ext.registerExtensionSlot({LightingMetal, "LightingFunc"}, {MainPostMetal, "MainPostFunc"},
                              1, BlockNames, 0, nullptr, hecl::Backend::BlendFactor::SrcAlpha,
                              hecl::Backend::BlendFactor::InvSrcAlpha, hecl::Backend::ZTest::Original,
                              hecl::Backend::CullMode::Backface, false, false, true);

    /* Forced additive shading */
    ext.registerExtensionSlot({LightingMetal, "LightingFunc"}, {MainPostMetal, "MainPostFunc"},
                              1, BlockNames, 0, nullptr, hecl::Backend::BlendFactor::SrcAlpha,
                              hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Original,
                              hecl::Backend::CullMode::Backface, true, false, true);

    /* Solid color */
    ext.registerExtensionSlot({}, {SolidPostMetal, "SolidPostFunc"},
                              1, SolidBlockNames, 0, nullptr, hecl::Backend::BlendFactor::One,
                              hecl::Backend::BlendFactor::Zero, hecl::Backend::ZTest::LEqual,
                              hecl::Backend::CullMode::Backface, false, false, false);

    /* Solid color additive */
    ext.registerExtensionSlot({}, {SolidPostMetal, "SolidPostFunc"},
                              1, SolidBlockNames, 0, nullptr, hecl::Backend::BlendFactor::SrcAlpha,
                              hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::LEqual,
                              hecl::Backend::CullMode::Backface, true, false, true);

    /* Alpha-only Solid color frontface cull, LEqual */
    ext.registerExtensionSlot({}, {SolidPostMetal, "SolidPostFunc"},
                              1, SolidBlockNames, 0, nullptr, hecl::Backend::BlendFactor::Zero,
                              hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::LEqual,
                              hecl::Backend::CullMode::Frontface, false, true, false);

    /* Alpha-only Solid color frontface cull, Always, No Z-write */
    ext.registerExtensionSlot({}, {SolidPostMetal, "SolidPostFunc"},
                              1, SolidBlockNames, 0, nullptr, hecl::Backend::BlendFactor::Zero,
                              hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::None,
                              hecl::Backend::CullMode::Frontface, true, true, false);

    /* Alpha-only Solid color backface cull, LEqual */
    ext.registerExtensionSlot({}, {SolidPostMetal, "SolidPostFunc"},
                              1, SolidBlockNames, 0, nullptr, hecl::Backend::BlendFactor::Zero,
                              hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::LEqual,
                              hecl::Backend::CullMode::Backface, false, true, false);

    /* Alpha-only Solid color backface cull, Greater, No Z-write */
    ext.registerExtensionSlot({}, {SolidPostMetal, "SolidPostFunc"},
                              1, SolidBlockNames, 0, nullptr, hecl::Backend::BlendFactor::Zero,
                              hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Greater,
                              hecl::Backend::CullMode::Backface, true, true, false);

    /* MorphBall shadow shading */
    ext.registerExtensionSlot({}, {MBShadowPostMetal, "EXTMBShadowPostFunc"},
                              1, MBShadowBlockNames, 3, BallFadeTextures,
                              hecl::Backend::BlendFactor::SrcAlpha,
                              hecl::Backend::BlendFactor::InvSrcAlpha,
                              hecl::Backend::ZTest::Equal,
                              hecl::Backend::CullMode::Backface, false, false, true, true);

    /* World shadow shading (modified lighting) */
    ext.registerExtensionSlot({LightingShadowMetal, "EXTLightingShadowFunc"}, {MainPostMetal, "MainPostFunc"},
                              1, BlockNames, 1, WorldShadowTextures, hecl::Backend::BlendFactor::Original,
                              hecl::Backend::BlendFactor::Original, hecl::Backend::ZTest::Original,
                              hecl::Backend::CullMode::Backface, false, false, true);

    /* Forced alpha shading without culling */
    ext.registerExtensionSlot({LightingMetal, "LightingFunc"}, {MainPostMetal, "MainPostFunc"},
                              1, BlockNames, 0, nullptr, hecl::Backend::BlendFactor::SrcAlpha,
                              hecl::Backend::BlendFactor::InvSrcAlpha, hecl::Backend::ZTest::Original,
                              hecl::Backend::CullMode::None, false, false, true);

    /* Forced additive shading without culling */
    ext.registerExtensionSlot({LightingMetal, "LightingFunc"}, {MainPostMetal, "MainPostFunc"},
                              1, BlockNames, 0, nullptr, hecl::Backend::BlendFactor::SrcAlpha,
                              hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Original,
                              hecl::Backend::CullMode::None, false, false, true);

    /* Forced alpha shading without culling or Z-write */
    ext.registerExtensionSlot({LightingMetal, "LightingFunc"}, {MainPostMetal, "MainPostFunc"},
                              1, BlockNames, 0, nullptr, hecl::Backend::BlendFactor::SrcAlpha,
                              hecl::Backend::BlendFactor::InvSrcAlpha, hecl::Backend::ZTest::Original,
                              hecl::Backend::CullMode::None, true, false, true);

    /* Forced additive shading without culling or Z-write */
    ext.registerExtensionSlot({LightingMetal, "LightingFunc"}, {MainPostMetal, "MainPostFunc"},
                              1, BlockNames, 0, nullptr, hecl::Backend::BlendFactor::SrcAlpha,
                              hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Original,
                              hecl::Backend::CullMode::None, true, false, true);

    return ext;
}

}
