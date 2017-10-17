#include "CModelShaders.hpp"

namespace urde
{

static const char* LightingHLSL =
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
"cbuffer LightingUniform : register(b2)\n"
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
"static float4 LightingFunc(float4 mvPosIn, float4 mvNormIn, in VertToFrag vtf)\n"
"{\n"
"    float4 ret = ambient;\n"
"    \n"
"    for (int i=0 ; i<" _XSTR(URDE_MAX_LIGHTS) " ; ++i)\n"
"    {\n"
"        float3 delta = mvPosIn.xyz - lights[i].pos.xyz;\n"
"        float dist = length(delta);\n"
"        float angDot = saturate(dot(normalize(delta), lights[i].dir.xyz));\n"
"        float att = 1.0 / (lights[i].linAtt[2] * dist * dist +\n"
"                           lights[i].linAtt[1] * dist +\n"
"                           lights[i].linAtt[0]);\n"
"        float angAtt = lights[i].angAtt[2] * angDot * angDot +\n"
"                       lights[i].angAtt[1] * angDot +\n"
"                       lights[i].angAtt[0];\n"
"        ret += lights[i].color * saturate(angAtt) * att * saturate(dot(normalize(-delta), mvNormIn.xyz));\n"
"    }\n"
"    \n"
"    return saturate(ret);\n"
"}\n";

static const char* LightingShadowHLSL =
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
"cbuffer LightingUniform : register(b2)\n"
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
"static float4 LightingShadowFunc(float4 mvPosIn, float4 mvNormIn, in VertToFrag vtf)\n"
"{\n"
"    float4 ret = ambient;\n"
"    \n"
"    float3 delta = mvPosIn.xyz - lights[0].pos.xyz;\n"
"    float dist = length(delta);\n"
"    float angDot = saturate(dot(normalize(delta), lights[0].dir.xyz));\n"
"    float att = 1.0 / (lights[0].linAtt[2] * dist * dist +\n"
"                       lights[0].linAtt[1] * dist +\n"
"                       lights[0].linAtt[0]);\n"
"    float angAtt = lights[0].angAtt[2] * angDot * angDot +\n"
"                   lights[0].angAtt[1] * angDot +\n"
"                   lights[0].angAtt[0];\n"
"    ret += lights[0].color * saturate(angAtt) * att * saturate(dot(normalize(-delta), mvNormIn.xyz)) *\n"
"           extTex7.Sample(clampSamp, vtf.extTcgs[0]).r;\n"
"    \n"
"    for (int i=1 ; i<" _XSTR(URDE_MAX_LIGHTS) " ; ++i)\n"
"    {\n"
"        float3 delta = mvPosIn.xyz - lights[i].pos.xyz;\n"
"        float dist = length(delta);\n"
"        float angDot = saturate(dot(normalize(delta), lights[i].dir.xyz));\n"
"        float att = 1.0 / (lights[i].linAtt[2] * dist * dist +\n"
"                           lights[i].linAtt[1] * dist +\n"
"                           lights[i].linAtt[0]);\n"
"        float angAtt = lights[i].angAtt[2] * angDot * angDot +\n"
"                       lights[i].angAtt[1] * angDot +\n"
"                       lights[i].angAtt[0];\n"
"        ret += lights[i].color * saturate(angAtt) * att * saturate(dot(normalize(-delta), mvNormIn.xyz));\n"
"    }\n"
"    \n"
"    return saturate(ret);\n"
"}\n";

static const char* MainPostHLSL =
"static float4 MainPostFunc(in VertToFrag vtf, float4 colorIn)\n"
"{\n"
"    float fogZ = (-vtf.mvPos.z - fog.start) * fog.rangeScale;\n"
"    return lerp(fog.color, colorIn, saturate(exp2(-8.0 * fogZ)));\n"
"}\n"
"\n";

static const char* ThermalPostHLSL =
"cbuffer ThermalUniform : register(b2)\n"
"{\n"
"    float4 tmulColor;\n"
"    float4 addColor;\n"
"};\n"
"static float4 ThermalPostFunc(in VertToFrag vtf, float4 colorIn)\n"
"{\n"
"    return float4(extTex7.Sample(samp, vtf.extTcgs[0]).rrr * tmulColor.rgb + addColor.rgb, 1.0);\n"
"}\n"
"\n";

static const char* SolidPostHLSL =
"cbuffer SolidUniform : register(b2)\n"
"{\n"
"    float4 solidColor;\n"
"};\n"
"static float4 SolidPostFunc(in VertToFrag vtf, float4 colorIn)\n"
"{\n"
"    return solidColor;\n"
"}\n"
"\n";

static const char* MBShadowPostHLSL =
"cbuffer MBShadowUniform : register(b2)\n"
"{\n"
"    float4 shadowUp;\n"
"    float shadowId;\n"
"};\n"
"static float4 MBShadowPostFunc(in VertToFrag vtf, float4 colorIn)\n"
"{\n"
"    float idTexel = extTex0.Sample(samp, vtf.extTcgs[0]).a;\n"
"    float sphereTexel = extTex1.Sample(samp, vtf.extTcgs[1]).a;\n"
"    float fadeTexel = extTex2.Sample(samp, vtf.extTcgs[2]).a;\n"
"    float val = ((abs(idTexel - shadowId) < 0.001) ?\n"
"        (dot(vtf.mvNorm.xyz, shadowUp.xyz) * shadowUp.w) : 0.0) *\n"
"        sphereTexel * fadeTexel;\n"
"    return float4(0.0, 0.0, 0.0, val);\n"
"}\n"
"\n";

hecl::Runtime::ShaderCacheExtensions
CModelShaders::GetShaderExtensionsHLSL(boo::IGraphicsDataFactory::Platform plat)
{
    hecl::Runtime::ShaderCacheExtensions ext(plat);

    /* Normal lit shading */
    ext.registerExtensionSlot({LightingHLSL, "LightingFunc"}, {MainPostHLSL, "MainPostFunc"},
                              0, nullptr, 0, nullptr, hecl::Backend::BlendFactor::Original,
                              hecl::Backend::BlendFactor::Original, hecl::Backend::ZTest::Original,
                              false, false, false, true);

    /* Thermal Visor shading */
    ext.registerExtensionSlot({}, {ThermalPostHLSL, "ThermalPostFunc"}, 0, nullptr,
                              1, ThermalTextures, hecl::Backend::BlendFactor::One,
                              hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Original,
                              false, false, false, true);

    /* Forced alpha shading */
    ext.registerExtensionSlot({LightingHLSL, "LightingFunc"}, {MainPostHLSL, "MainPostFunc"},
                              0, nullptr, 0, nullptr, hecl::Backend::BlendFactor::SrcAlpha,
                              hecl::Backend::BlendFactor::InvSrcAlpha, hecl::Backend::ZTest::Original,
                              false, false, false, true);

    /* Forced additive shading */
    ext.registerExtensionSlot({LightingHLSL, "LightingFunc"}, {MainPostHLSL, "MainPostFunc"},
                              0, nullptr, 0, nullptr, hecl::Backend::BlendFactor::One,
                              hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Original,
                              false, false, false, true);

    /* Solid color */
    ext.registerExtensionSlot({}, {SolidPostHLSL, "SolidPostFunc"},
                              0, nullptr, 0, nullptr, hecl::Backend::BlendFactor::One,
                              hecl::Backend::BlendFactor::Zero, hecl::Backend::ZTest::LEqual,
                              false, false, false, false);

    /* Solid color additive */
    ext.registerExtensionSlot({}, {SolidPostHLSL, "SolidPostFunc"},
                              0, nullptr, 0, nullptr, hecl::Backend::BlendFactor::SrcAlpha,
                              hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::LEqual,
                              false, true, false, true);

    /* Alpha-only Solid color frontface cull, LEqual */
    ext.registerExtensionSlot({}, {SolidPostHLSL, "SolidPostFunc"},
                              0, nullptr, 0, nullptr, hecl::Backend::BlendFactor::One,
                              hecl::Backend::BlendFactor::Zero, hecl::Backend::ZTest::LEqual,
                              true, false, true, false);

    /* Alpha-only Solid color frontface cull, Always, No Z-write */
    ext.registerExtensionSlot({}, {SolidPostHLSL, "SolidPostFunc"},
                              0, nullptr, 0, nullptr, hecl::Backend::BlendFactor::One,
                              hecl::Backend::BlendFactor::Zero, hecl::Backend::ZTest::None,
                              true, true, true, false);

    /* Alpha-only Solid color backface cull, LEqual */
    ext.registerExtensionSlot({}, {SolidPostHLSL, "SolidPostFunc"},
                              0, nullptr, 0, nullptr, hecl::Backend::BlendFactor::One,
                              hecl::Backend::BlendFactor::Zero, hecl::Backend::ZTest::LEqual,
                              false, false, true, false);

    /* Alpha-only Solid color backface cull, Greater, No Z-write */
    ext.registerExtensionSlot({}, {SolidPostHLSL, "SolidPostFunc"},
                              0, nullptr, 0, nullptr, hecl::Backend::BlendFactor::One,
                              hecl::Backend::BlendFactor::Zero, hecl::Backend::ZTest::Greater,
                              false, true, true, false);

    /* MorphBall shadow shading */
    ext.registerExtensionSlot({}, {MBShadowPostHLSL, "MBShadowPostFunc"},
                              0, nullptr, 3, BallFadeTextures,
                              hecl::Backend::BlendFactor::SrcAlpha,
                              hecl::Backend::BlendFactor::InvSrcAlpha,
                              hecl::Backend::ZTest::Equal,
                              false, false, false, true);

    /* World shadow shading (modified lighting) */
    ext.registerExtensionSlot({LightingShadowHLSL, "LightingShadowFunc"}, {MainPostHLSL, "MainPostFunc"},
                              0, nullptr, 1, WorldShadowTextures, hecl::Backend::BlendFactor::Original,
                              hecl::Backend::BlendFactor::Original, hecl::Backend::ZTest::Original,
                              false, false, false, true);

    return ext;
}

}
