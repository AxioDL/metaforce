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
"\n"
"struct LightingUniform\n"
"{\n"
"    Light lights[" _XSTR(URDE_MAX_LIGHTS) "];\n"
"    float4 ambient;\n"
"    float4 colorReg0;\n"
"    float4 colorReg1;\n"
"    float4 colorReg2;\n"
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

static const char* ThermalPostMetal =
"struct ThermalUniform\n"
"{\n"
"    float4 mulColor;\n"
"    float4 addColor;\n"
"};\n"
"static float4 ThermalPostFunc(thread VertToFrag& vtf, constant ThermalUniform& lu, texture2d<float> tex7, float4 colorIn)\n"
"{\n"
"    return tex7.sample(samp, vtf.extTcgs0).rrrr * lu.mulColor + lu.addColor;\n"
"}\n"
"\n";

static const char* BlockNames[] = {"LightingUniform"};
static const char* ThermalBlockNames[] = {"ThermalUniform"};

hecl::Runtime::ShaderCacheExtensions
CModelShaders::GetShaderExtensionsMetal(boo::IGraphicsDataFactory::Platform plat)
{
    hecl::Runtime::ShaderCacheExtensions ext(plat);

    /* Normal lit shading */
    ext.registerExtensionSlot({LightingMetal, "LightingFunc"}, {}, 1, BlockNames, 0, nullptr,
                              hecl::Backend::BlendFactor::Original, hecl::Backend::BlendFactor::Original);

    /* Thermal Visor shading */
    ext.registerExtensionSlot({}, {ThermalPostMetal, "ThermalPostFunc"}, 1, ThermalBlockNames, 1, ThermalTextures,
                              hecl::Backend::BlendFactor::One, hecl::Backend::BlendFactor::One);

    return ext;
}

}
