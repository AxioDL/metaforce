#include "CModelShaders.hpp"

namespace urde {
using namespace std::literals;

extern const hecl::Backend::Function ExtensionLightingFuncsMetal[];
extern const hecl::Backend::Function ExtensionPostFuncsMetal[];

static std::string_view LightingMetal =
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
"    int mode;\n"
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
"static float4 LightingFunc(constant LightingUniform& lu, float3 mvPosIn, float3 mvNormIn, thread VertToFrag& vtf)\n"
"{\n"
"    float4 ret = lu.ambient;\n"
"    \n"
"    for (int i=0 ; i<" _XSTR(URDE_MAX_LIGHTS) " ; ++i)\n"
"    {\n"
"        float3 delta = mvPosIn - lu.lights[i].pos.xyz;\n"
"        float dist = length(delta);\n"
"        float angDot = saturate(dot(normalize(delta), lu.lights[i].dir.xyz));\n"
"        float att = 1.0 / (lu.lights[i].linAtt[2] * dist * dist +\n"
"                           lu.lights[i].linAtt[1] * dist +\n"
"                           lu.lights[i].linAtt[0]);\n"
"        float angAtt = lu.lights[i].angAtt[2] * angDot * angDot +\n"
"                       lu.lights[i].angAtt[1] * angDot +\n"
"                       lu.lights[i].angAtt[0];\n"
"        ret += lu.lights[i].color * saturate(angAtt) * att * saturate(dot(normalize(-delta), mvNormIn));\n"
"    }\n"
"    \n"
"    return ret;\n"
"}\n"sv;

static std::string_view LightingShadowMetal =
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
"    int mode;\n"
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
"static float4 EXTLightingShadowFunc(constant LightingUniform& lu, float3 mvPosIn, float3 mvNormIn,\n"
"                                    thread VertToFrag& vtf, sampler samp, sampler clampSamp, texture2d<float> extTex7)\n"
"{\n"
"    float4 ret = lu.ambient;\n"
"    \n"
"    float3 delta = mvPosIn - lu.lights[0].pos.xyz;\n"
"    float dist = length(delta);\n"
"    float angDot = saturate(dot(normalize(delta), lu.lights[0].dir.xyz));\n"
"    float att = 1.0 / (lu.lights[0].linAtt[2] * dist * dist +\n"
"                       lu.lights[0].linAtt[1] * dist +\n"
"                       lu.lights[0].linAtt[0]);\n"
"    float angAtt = lu.lights[0].angAtt[2] * angDot * angDot +\n"
"                   lu.lights[0].angAtt[1] * angDot +\n"
"                   lu.lights[0].angAtt[0];\n"
"    ret += lu.lights[0].color * saturate(angAtt) * att * saturate(dot(normalize(-delta), mvNormIn)) *\n"
"           extTex7.sample(clampSamp, vtf.extTcgs0).r;\n"
"    \n"
"    for (int i=1 ; i<" _XSTR(URDE_MAX_LIGHTS) " ; ++i)\n"
"    {\n"
"        float3 delta = mvPosIn - lu.lights[i].pos.xyz;\n"
"        float dist = length(delta);\n"
"        float angDot = saturate(dot(normalize(delta), lu.lights[i].dir.xyz));\n"
"        float att = 1.0 / (lu.lights[i].linAtt[2] * dist * dist +\n"
"                           lu.lights[i].linAtt[1] * dist +\n"
"                           lu.lights[i].linAtt[0]);\n"
"        float angAtt = lu.lights[i].angAtt[2] * angDot * angDot +\n"
"                       lu.lights[i].angAtt[1] * angDot +\n"
"                       lu.lights[i].angAtt[0];\n"
"        ret += lu.lights[i].color * saturate(angAtt) * att * saturate(dot(normalize(-delta), mvNormIn));\n"
"    }\n"
"    \n"
"    return ret;\n"
"}\n"sv;

static std::string_view MainPostMetal =
    "float4 MainPostFunc(thread VertToFrag& vtf, constant LightingUniform& lu, float4 colorIn)\n"
    "{\n"
    "    float fogZ, temp;\n"
    "    switch (lu.fog.mode)\n"
    "    {\n"
    "    case 2:\n"
    "        fogZ = (-vtf.mvPos.z - lu.fog.start) * lu.fog.rangeScale;\n"
    "        break;\n"
    "    case 4:\n"
    "        fogZ = 1.0 - exp2(-8.0 * (-vtf.mvPos.z - lu.fog.start) * lu.fog.rangeScale);\n"
    "        break;\n"
    "    case 5:\n"
    "        temp = (-vtf.mvPos.z - lu.fog.start) * lu.fog.rangeScale;\n"
    "        fogZ = 1.0 - exp2(-8.0 * temp * temp);\n"
    "        break;\n"
    "    case 6:\n"
    "        fogZ = exp2(-8.0 * (lu.fog.start + vtf.mvPos.z) * lu.fog.rangeScale);\n"
    "        break;\n"
    "    case 7:\n"
    "        temp = (lu.fog.start + vtf.mvPos.z) * lu.fog.rangeScale;\n"
    "        fogZ = exp2(-8.0 * temp * temp);\n"
    "        break;\n"
    "    default:\n"
    "        fogZ = 0.0;\n"
    "        break;\n"
    "    }\n"
    "#ifdef BLEND_DST_ONE\n"
    "    return float4(mix(colorIn, float4(0.0), saturate(fogZ)).rgb, colorIn.a);\n"
    "#else\n"
    "    return float4(mix(colorIn, lu.fog.color, saturate(fogZ)).rgb, colorIn.a);\n"
    "#endif\n"
    "}\n"
    "\n"sv;

static std::string_view ThermalPostMetal =
    "struct ThermalUniform\n"
    "{\n"
    "    float4 tmulColor;\n"
    "    float4 addColor;\n"
    "};\n"
    "static float4 EXTThermalPostFunc(thread VertToFrag& vtf, constant ThermalUniform& lu,\n"
    "    sampler samp, sampler clampSamp, texture2d<float> extTex7, float4 colorIn)\n"
    "{\n"
    "    //return float4(vtf.extTcgs0.xy, 0.0, 1.0);\n"
    "    return float4(extTex7.sample(samp, vtf.extTcgs0).rrr * lu.tmulColor.rgb + lu.addColor.rgb,\n"
    "                  lu.tmulColor.a + lu.addColor.a);\n"
    "}\n"
    "\n"sv;

static std::string_view SolidPostMetal =
    "struct SolidUniform\n"
    "{\n"
    "    float4 solidColor;\n"
    "};\n"
    "static float4 SolidPostFunc(thread VertToFrag& vtf, constant SolidUniform& lu, float4 colorIn)\n"
    "{\n"
    "    return lu.solidColor;\n"
    "}\n"
    "\n"sv;

static std::string_view MBShadowPostMetal =
    "struct MBShadowUniform\n"
    "{\n"
    "    float4 shadowUp;\n"
    "    float shadowId;\n"
    "};\n"
    "static float4 EXTMBShadowPostFunc(thread VertToFrag& vtf, constant MBShadowUniform& su, sampler samp, sampler "
    "clampSamp,\n"
    "                                  texture2d<float> extTex0, texture2d<float> extTex1, texture2d<float> extTex2, "
    "float4 colorIn)\n"
    "{\n"
    "    float idTexel = extTex0.sample(samp, vtf.extTcgs0).a;\n"
    "    float sphereTexel = extTex1.sample(samp, vtf.extTcgs1).a;\n"
    "    float fadeTexel = extTex2.sample(samp, vtf.extTcgs2).a;\n"
    "    float val = ((fabs(idTexel - su.shadowId) < 0.001) ?\n"
    "        (dot(vtf.mvNorm.xyz, su.shadowUp.xyz) * su.shadowUp.w) : 0.0) *\n"
    "        sphereTexel * fadeTexel;\n"
    "    return float4(0.0, 0.0, 0.0, val);\n"
    "}\n"
    "\n"sv;

const hecl::Backend::Function ExtensionLightingFuncsMetal[] = {{},
                                                               {LightingMetal, "LightingFunc"},
                                                               {},
                                                               {LightingMetal, "LightingFunc"},
                                                               {LightingMetal, "LightingFunc"},
                                                               {},
                                                               {},
                                                               {},
                                                               {},
                                                               {},
                                                               {},
                                                               {},
                                                               {LightingShadowMetal, "EXTLightingShadowFunc"},
                                                               {LightingMetal, "LightingFunc"},
                                                               {LightingMetal, "LightingFunc"},
                                                               {LightingMetal, "LightingFunc"},
                                                               {LightingMetal, "LightingFunc"},
                                                               {LightingMetal, "LightingFunc"},
                                                               {LightingMetal, "LightingFunc"},
                                                               {LightingMetal, "LightingFunc"}};

const hecl::Backend::Function ExtensionPostFuncsMetal[] = {
    {},
    {MainPostMetal, "MainPostFunc"},
    {ThermalPostMetal, "EXTThermalPostFunc"},
    {MainPostMetal, "MainPostFunc"},
    {MainPostMetal, "MainPostFunc"},
    {SolidPostMetal, "SolidPostFunc"},
    {SolidPostMetal, "SolidPostFunc"},
    {SolidPostMetal, "SolidPostFunc"},
    {SolidPostMetal, "SolidPostFunc"},
    {SolidPostMetal, "SolidPostFunc"},
    {SolidPostMetal, "SolidPostFunc"},
    {MBShadowPostMetal, "EXTMBShadowPostFunc"},
    {MainPostMetal, "MainPostFunc"},
    {MainPostMetal, "MainPostFunc"},
    {MainPostMetal, "MainPostFunc"},
    {MainPostMetal, "MainPostFunc"},
    {MainPostMetal, "MainPostFunc"},
    {MainPostMetal, "MainPostFunc"},
    {MainPostMetal, "MainPostFunc"},
    {MainPostMetal, "MainPostFunc"},
};

static const char* BlockNames[] = {"LightingUniform"};
static const char* ThermalBlockNames[] = {"ThermalUniform"};
static const char* SolidBlockNames[] = {"SolidUniform"};
static const char* MBShadowBlockNames[] = {"MBShadowUniform"};

} // namespace urde
