#include "CModelShaders.hpp"

namespace urde {
using namespace std::literals;

extern const hecl::Backend::Function ExtensionLightingFuncsHLSL[];
extern const hecl::Backend::Function ExtensionPostFuncsHLSL[];

#define FOG_STRUCT_HLSL                                                                                                \
  "struct Fog\n"                                                                                                       \
  "{\n"                                                                                                                \
  "    int mode;\n"                                                                                                    \
  "    float4 color;\n"                                                                                                \
  "    float rangeScale;\n"                                                                                            \
  "    float start;\n"                                                                                                 \
  "};\n"

#define FOG_ALGORITHM_HLSL                                                                                             \
  "    float fogZ, temp;\n"                                                                                            \
  "    switch (fog.mode)\n"                                                                                            \
  "    {\n"                                                                                                            \
  "    case 2:\n"                                                                                                      \
  "        fogZ = (-vtf.mvPos.z - fog.start) * fog.rangeScale;\n"                                                      \
  "        break;\n"                                                                                                   \
  "    case 4:\n"                                                                                                      \
  "        fogZ = 1.0 - exp2(-8.0 * (-vtf.mvPos.z - fog.start) * fog.rangeScale);\n"                                   \
  "        break;\n"                                                                                                   \
  "    case 5:\n"                                                                                                      \
  "        temp = (-vtf.mvPos.z - fog.start) * fog.rangeScale;\n"                                                      \
  "        fogZ = 1.0 - exp2(-8.0 * temp * temp);\n"                                                                   \
  "        break;\n"                                                                                                   \
  "    case 6:\n"                                                                                                      \
  "        fogZ = exp2(-8.0 * (fog.start + vtf.mvPos.z) * fog.rangeScale);\n"                                          \
  "        break;\n"                                                                                                   \
  "    case 7:\n"                                                                                                      \
  "        temp = (fog.start + vtf.mvPos.z) * fog.rangeScale;\n"                                                       \
  "        fogZ = exp2(-8.0 * temp * temp);\n"                                                                         \
  "        break;\n"                                                                                                   \
  "    default:\n"                                                                                                     \
  "        fogZ = 0.0;\n"                                                                                              \
  "        break;\n"                                                                                                   \
  "    }\n"                                                                                                            \
  "#ifdef BLEND_DST_ONE\n"                                                                                             \
  "    return float4(lerp(colorIn, float4(0.0, 0.0, 0.0, 0.0), saturate(fogZ)).rgb, colorIn.a);\n"                     \
  "#else\n"                                                                                                            \
  "    return float4(lerp(colorIn, fog.color, saturate(fogZ)).rgb, colorIn.a);\n"                                      \
  "#endif\n"

static std::string_view LightingHLSL =
"struct Light\n"
"{\n"
"    float4 pos;\n"
"    float4 dir;\n"
"    float4 color;\n"
"    float4 linAtt;\n"
"    float4 angAtt;\n"
"};\n"
FOG_STRUCT_HLSL
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
"static float4 LightingFunc(float3 mvPosIn, float3 mvNormIn, in VertToFrag vtf)\n"
"{\n"
"    float4 ret = ambient;\n"
"    \n"
"    for (int i=0 ; i<" _XSTR(URDE_MAX_LIGHTS) " ; ++i)\n"
"    {\n"
"        float3 delta = mvPosIn - lights[i].pos.xyz;\n"
"        float dist = length(delta);\n"
"        float angDot = saturate(dot(normalize(delta), lights[i].dir.xyz));\n"
"        float att = 1.0 / (lights[i].linAtt[2] * dist * dist +\n"
"                           lights[i].linAtt[1] * dist +\n"
"                           lights[i].linAtt[0]);\n"
"        float angAtt = lights[i].angAtt[2] * angDot * angDot +\n"
"                       lights[i].angAtt[1] * angDot +\n"
"                       lights[i].angAtt[0];\n"
"        ret += lights[i].color * angAtt * att * saturate(dot(normalize(-delta), mvNormIn));\n"
"    }\n"
"    \n"
"    return saturate(ret);\n"
"}\n"sv;

static std::string_view LightingShadowHLSL =
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
"static float4 LightingShadowFunc(float3 mvPosIn, float3 mvNormIn, in VertToFrag vtf)\n"
"{\n"
"    float4 ret = ambient;\n"
"    \n"
"    float3 delta = mvPosIn - lights[0].pos.xyz;\n"
"    float dist = length(delta);\n"
"    float angDot = saturate(dot(normalize(delta), lights[0].dir.xyz));\n"
"    float att = 1.0 / (lights[0].linAtt[2] * dist * dist +\n"
"                       lights[0].linAtt[1] * dist +\n"
"                       lights[0].linAtt[0]);\n"
"    float angAtt = lights[0].angAtt[2] * angDot * angDot +\n"
"                   lights[0].angAtt[1] * angDot +\n"
"                   lights[0].angAtt[0];\n"
"    ret += lights[0].color * saturate(angAtt) * att * saturate(dot(normalize(-delta), mvNormIn)) *\n"
"           extTex7.Sample(clampSamp, vtf.extTcgs[0]).r;\n"
"    \n"
"    for (int i=1 ; i<" _XSTR(URDE_MAX_LIGHTS) " ; ++i)\n"
"    {\n"
"        float3 delta = mvPosIn - lights[i].pos.xyz;\n"
"        float dist = length(delta);\n"
"        float angDot = saturate(dot(normalize(delta), lights[i].dir.xyz));\n"
"        float att = 1.0 / (lights[i].linAtt[2] * dist * dist +\n"
"                           lights[i].linAtt[1] * dist +\n"
"                           lights[i].linAtt[0]);\n"
"        float angAtt = lights[i].angAtt[2] * angDot * angDot +\n"
"                       lights[i].angAtt[1] * angDot +\n"
"                       lights[i].angAtt[0];\n"
"        ret += lights[i].color * saturate(angAtt) * att * saturate(dot(normalize(-delta), mvNormIn));\n"
"    }\n"
"    \n"
"    return ret;\n"
"}\n"sv;

static std::string_view MainPostHLSL =
    "static float4 MainPostFunc(in VertToFrag vtf, float4 colorIn)\n"
    "{\n" FOG_ALGORITHM_HLSL
    "}\n"
    "\n"sv;

static std::string_view ThermalPostHLSL =
    "cbuffer ThermalUniform : register(b2)\n"
    "{\n"
    "    float4 tmulColor;\n"
    "    float4 addColor;\n"
    "};\n"
    "static float4 ThermalPostFunc(in VertToFrag vtf, float4 colorIn)\n"
    "{\n"
    "    return float4(extTex7.Sample(samp, vtf.extTcgs[0]).rrr * tmulColor.rgb + addColor.rgb, tmulColor.a + "
    "addColor.a);\n"
    "}\n"
    "\n"sv;

static std::string_view SolidPostHLSL =
    "cbuffer SolidUniform : register(b2)\n"
    "{\n"
    "    float4 solidColor;\n"
    "};\n"
    "static float4 SolidPostFunc(in VertToFrag vtf, float4 colorIn)\n"
    "{\n"
    "    return solidColor;\n"
    "}\n"
    "\n"sv;

static std::string_view MBShadowPostHLSL =
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
    "\n"sv;

static std::string_view DisintegratePostHLSL = FOG_STRUCT_HLSL
    "cbuffer DisintegrateUniform : register(b2)\n"
    "{\n"
    "    float4 addColor;\n"
    "    Fog fog;\n"
    "};\n"
    "static float4 DisintegratePostFunc(in VertToFrag vtf, float4 colorIn)\n"
    "{\n"
    "    float4 texel0 = extTex7.Sample(samp, vtf.extTcgs[0]);\n"
    "    float4 texel1 = extTex7.Sample(samp, vtf.extTcgs[1]);\n"
    "    colorIn = lerp(float4(0.0,0.0,0.0,0.0), texel1, texel0);\n"
    "    colorIn.rgb += addColor.rgb;\n" FOG_ALGORITHM_HLSL
    "}\n"
    "\n"sv;

const hecl::Backend::Function ExtensionLightingFuncsHLSL[] = {{},
                                                              {LightingHLSL, "LightingFunc"},
                                                              {},
                                                              {LightingHLSL, "LightingFunc"},
                                                              {LightingHLSL, "LightingFunc"},
                                                              {},
                                                              {},
                                                              {},
                                                              {},
                                                              {},
                                                              {},
                                                              {},
                                                              {LightingShadowHLSL, "LightingShadowFunc"},
                                                              {LightingHLSL, "LightingFunc"},
                                                              {LightingHLSL, "LightingFunc"},
                                                              {LightingHLSL, "LightingFunc"},
                                                              {LightingHLSL, "LightingFunc"},
                                                              {LightingHLSL, "LightingFunc"},
                                                              {LightingHLSL, "LightingFunc"},
                                                              {LightingHLSL, "LightingFunc"},
                                                              {},
                                                              {LightingHLSL, "LightingFunc"}};

const hecl::Backend::Function ExtensionPostFuncsHLSL[] = {
    {},
    {MainPostHLSL, "MainPostFunc"},
    {ThermalPostHLSL, "ThermalPostFunc"},
    {MainPostHLSL, "MainPostFunc"},
    {MainPostHLSL, "MainPostFunc"},
    {SolidPostHLSL, "SolidPostFunc"},
    {SolidPostHLSL, "SolidPostFunc"},
    {SolidPostHLSL, "SolidPostFunc"},
    {SolidPostHLSL, "SolidPostFunc"},
    {SolidPostHLSL, "SolidPostFunc"},
    {SolidPostHLSL, "SolidPostFunc"},
    {MBShadowPostHLSL, "MBShadowPostFunc"},
    {MainPostHLSL, "MainPostFunc"},
    {MainPostHLSL, "MainPostFunc"},
    {MainPostHLSL, "MainPostFunc"},
    {MainPostHLSL, "MainPostFunc"},
    {MainPostHLSL, "MainPostFunc"},
    {MainPostHLSL, "MainPostFunc"},
    {MainPostHLSL, "MainPostFunc"},
    {MainPostHLSL, "MainPostFunc"},
    {DisintegratePostHLSL, "DisintegratePostFunc"},
    {MainPostHLSL, "MainPostFunc"},
};

} // namespace urde
