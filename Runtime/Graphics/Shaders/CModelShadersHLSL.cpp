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
"    Fog fog;\n"
"};\n"
"\n"
"static float4 LightingFunc(float4 mvPosIn, float4 mvNormIn)\n"
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
"    float4 mulColor;\n"
"    float4 addColor;\n"
"};\n"
"static float4 ThermalPostFunc(in VertToFrag vtf, float4 colorIn)\n"
"{\n"
"    return float4(extTex7.Sample(samp, vtf.extTcgs[0]).rrr * mulColor.rgb + addColor.rgb, 1.0);\n"
"}\n"
"\n";

hecl::Runtime::ShaderCacheExtensions
CModelShaders::GetShaderExtensionsHLSL(boo::IGraphicsDataFactory::Platform plat)
{
    hecl::Runtime::ShaderCacheExtensions ext(plat);

    /* Normal lit shading */
    ext.registerExtensionSlot({LightingHLSL, "LightingFunc"}, {MainPostHLSL, "MainPostFunc"},
                              0, nullptr, 0, nullptr, hecl::Backend::BlendFactor::Original,
                              hecl::Backend::BlendFactor::Original);

    /* Thermal Visor shading */
    ext.registerExtensionSlot({}, {ThermalPostHLSL, "ThermalPostFunc"}, 0, nullptr,
                              1, ThermalTextures, hecl::Backend::BlendFactor::One,
                              hecl::Backend::BlendFactor::One);

    /* Forced alpha shading */
    ext.registerExtensionSlot({LightingHLSL, "LightingFunc"}, {MainPostHLSL, "MainPostFunc"},
                              0, nullptr, 0, nullptr, hecl::Backend::BlendFactor::SrcAlpha,
                              hecl::Backend::BlendFactor::InvSrcAlpha);

    /* Forced additive shading */
    ext.registerExtensionSlot({LightingHLSL, "LightingFunc"}, {MainPostHLSL, "MainPostFunc"},
                              0, nullptr, 0, nullptr, hecl::Backend::BlendFactor::One,
                              hecl::Backend::BlendFactor::One);

    return ext;
}

}
