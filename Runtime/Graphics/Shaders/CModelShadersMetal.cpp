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
"static float4 LightingFunc(constant LightingUniform& lu, float4 mvPosIn, float4 mvNormIn)\n"
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
"static float4 ThermalPostFunc(thread VertToFrag& vtf, constant ThermalUniform& lu, texture2d<float> tex7, float4 colorIn)\n"
"{\n"
"    return float4(tex7.sample(samp, vtf.extTcgs0).rrr * lu.tmulColor.rgb + lu.addColor.rgb, 1.0);\n"
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
"UBINDING2 uniform MBShadowUniform\n"
"{\n"
"    vec4 shadowUp;\n"
"    float shadowId;\n"
"};\n"
"static float4 MBShadowPostFunc(thread VertToFrag& vtf, constant MBShadowUniform& su,\n"
"                               texture2d<float> tex0, texture2d<float> tex1, texture2d<float> tex2, float4 colorIn)\n"
"{\n"
"    float idTexel = tex0.sample(samp, vtf.extTcgs0).a;\n"
"    float sphereTexel = tex1.sample(samp, vtf.extTcgs1).a;\n"
"    float fadeTexel = tex2.sample(samp, vtf.extTcgs2).a;\n"
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
                              hecl::Backend::BlendFactor::Original);

    /* Thermal Visor shading */
    ext.registerExtensionSlot({}, {ThermalPostMetal, "ThermalPostFunc"}, 1, ThermalBlockNames,
                              1, ThermalTextures, hecl::Backend::BlendFactor::One,
                              hecl::Backend::BlendFactor::One);

    /* Forced alpha shading */
    ext.registerExtensionSlot({LightingMetal, "LightingFunc"}, {MainPostMetal, "MainPostFunc"},
                              1, BlockNames, 0, nullptr, hecl::Backend::BlendFactor::SrcAlpha,
                              hecl::Backend::BlendFactor::InvSrcAlpha);

    /* Forced additive shading */
    ext.registerExtensionSlot({LightingMetal, "LightingFunc"}, {MainPostMetal, "MainPostFunc"},
                              1, BlockNames, 0, nullptr, hecl::Backend::BlendFactor::One,
                              hecl::Backend::BlendFactor::One);

    /* Solid shading */
    ext.registerExtensionSlot({}, {SolidPostMetal, "SolidPostFunc"},
                              1, SolidBlockNames, 0, nullptr, hecl::Backend::BlendFactor::One,
                              hecl::Backend::BlendFactor::Zero);

    /* MorphBall shadow shading */
    ext.registerExtensionSlot({}, {MBShadowPostMetal, "MBShadowPostFunc"},
                              1, MBShadowBlockNames, 3, BallFadeTextures,
                              hecl::Backend::BlendFactor::SrcAlpha,
                              hecl::Backend::BlendFactor::InvSrcAlpha);

    return ext;
}

}
