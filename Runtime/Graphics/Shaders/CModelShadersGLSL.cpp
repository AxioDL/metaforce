#include "CModelShaders.hpp"
#include "hecl/Backend/GLSL.hpp"

namespace urde {
using namespace std::literals;

extern const hecl::Backend::Function ExtensionLightingFuncsGLSL[];
extern const hecl::Backend::Function ExtensionPostFuncsGLSL[];

#define FOG_STRUCT_GLSL                                                                                                \
  "struct Fog\n"                                                                                                       \
  "{\n"                                                                                                                \
  "    int mode;\n"                                                                                                    \
  "    vec4 color;\n"                                                                                                  \
  "    float rangeScale;\n"                                                                                            \
  "    float start;\n"                                                                                                 \
  "};\n"

#define FOG_ALGORITHM_GLSL                                                                                             \
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
  "    return vec4(mix(colorIn, vec4(0.0), clamp(fogZ, 0.0, 1.0)).rgb, colorIn.a);\n"                                  \
  "#else\n"                                                                                                            \
  "    return vec4(mix(colorIn, fog.color, clamp(fogZ, 0.0, 1.0)).rgb, colorIn.a);\n"                                  \
  "#endif\n"

static std::string_view LightingGLSL =
"struct Light\n"
"{\n"
"    vec4 pos;\n"
"    vec4 dir;\n"
"    vec4 color;\n"
"    vec4 linAtt;\n"
"    vec4 angAtt;\n"
"};\n"
FOG_STRUCT_GLSL
"\n"
"UBINDING2 uniform LightingUniform\n"
"{\n"
"    Light lights[" _XSTR(URDE_MAX_LIGHTS) "];\n"
"    vec4 ambient;\n"
"    vec4 colorReg0;\n"
"    vec4 colorReg1;\n"
"    vec4 colorReg2;\n"
"    vec4 mulColor;\n"
"    vec4 addColor;\n"
"    Fog fog;\n"
"};\n"
"\n"
"vec4 LightingFunc(vec3 mvPosIn, vec3 mvNormIn)\n"
"{\n"
"    vec4 ret = ambient;\n"
"    \n"
"    for (int i=0 ; i<" _XSTR(URDE_MAX_LIGHTS) " ; ++i)\n"
"    {\n"
"        vec3 delta = mvPosIn - lights[i].pos.xyz;\n"
"        float dist = length(delta);\n"
"        vec3 deltaNorm = delta / dist;\n"
"        float angDot = max(dot(deltaNorm, lights[i].dir.xyz), 0.0);\n"
"        float att = 1.0 / (lights[i].linAtt[2] * dist * dist +\n"
"                           lights[i].linAtt[1] * dist +\n"
"                           lights[i].linAtt[0]);\n"
"        float angAtt = lights[i].angAtt[2] * angDot * angDot +\n"
"                       lights[i].angAtt[1] * angDot +\n"
"                       lights[i].angAtt[0];\n"
"        ret += lights[i].color * angAtt * att * max(dot(-deltaNorm, mvNormIn), 0.0);\n"
"    }\n"
"    \n"
"    return clamp(ret, 0.0, 1.0);\n"
"}\n"sv;

static std::string_view LightingShadowGLSL =
"struct Light\n"
"{\n"
"    vec4 pos;\n"
"    vec4 dir;\n"
"    vec4 color;\n"
"    vec4 linAtt;\n"
"    vec4 angAtt;\n"
"};\n"
"struct Fog\n"
"{\n"
"    int mode;\n"
"    vec4 color;\n"
"    float rangeScale;\n"
"    float start;\n"
"};\n"
"\n"
"UBINDING2 uniform LightingUniform\n"
"{\n"
"    Light lights[" _XSTR(URDE_MAX_LIGHTS) "];\n"
"    vec4 ambient;\n"
"    vec4 colorReg0;\n"
"    vec4 colorReg1;\n"
"    vec4 colorReg2;\n"
"    vec4 mulColor;\n"
"    vec4 addColor;\n"
"    Fog fog;\n"
"};\n"
"\n"
"vec4 LightingShadowFunc(vec3 mvPosIn, vec3 mvNormIn)\n"
"{\n"
"    vec2 shadowUV = vtf.extTcgs[0];\n"
"    shadowUV.y = 1.0 - shadowUV.y;\n"
"    \n"
"    vec4 ret = ambient;\n"
"    \n"
"    vec3 delta = mvPosIn - lights[0].pos.xyz;\n"
"    float dist = length(delta);\n"
"    vec3 deltaNorm = delta / dist;\n"
"    float angDot = max(dot(deltaNorm, lights[0].dir.xyz), 0.0);\n"
"    float att = 1.0 / (lights[0].linAtt[2] * dist * dist +\n"
"                       lights[0].linAtt[1] * dist +\n"
"                       lights[0].linAtt[0]);\n"
"    float angAtt = lights[0].angAtt[2] * angDot * angDot +\n"
"                   lights[0].angAtt[1] * angDot +\n"
"                   lights[0].angAtt[0];\n"
"    ret += lights[0].color * angAtt * att * max(dot(-deltaNorm, mvNormIn), 0.0) *\n"
"           texture(extTex7, shadowUV).r;\n"
"    \n"
"    for (int i=1 ; i<" _XSTR(URDE_MAX_LIGHTS) " ; ++i)\n"
"    {\n"
"        vec3 delta = mvPosIn - lights[i].pos.xyz;\n"
"        float dist = length(delta);\n"
"        vec3 deltaNorm = delta / dist;\n"
"        float angDot = max(dot(deltaNorm, lights[i].dir.xyz), 0.0);\n"
"        float att = 1.0 / (lights[i].linAtt[2] * dist * dist +\n"
"                           lights[i].linAtt[1] * dist +\n"
"                           lights[i].linAtt[0]);\n"
"        float angAtt = lights[i].angAtt[2] * angDot * angDot +\n"
"                       lights[i].angAtt[1] * angDot +\n"
"                       lights[i].angAtt[0];\n"
"        ret += lights[i].color * angAtt * att * max(dot(-deltaNorm, mvNormIn), 0.0);\n"
"    }\n"
"    \n"
"    return clamp(ret, 0.0, 1.0);\n"
"}\n"sv;

static std::string_view MainPostGLSL =
    "vec4 MainPostFunc(vec4 colorIn)\n"
    "{\n" FOG_ALGORITHM_GLSL
    "}\n"
    "\n"sv;

static std::string_view ThermalPostGLSL =
    "UBINDING2 uniform ThermalUniform\n"
    "{\n"
    "    vec4 tmulColor;\n"
    "    vec4 taddColor;\n"
    "};\n"
    "vec4 ThermalPostFunc(vec4 colorIn)\n"
    "{\n"
    "    return vec4(texture(extTex7, vtf.extTcgs[0]).rrr * tmulColor.rgb + taddColor.rgb, tmulColor.a + taddColor.a);\n"
    "}\n"
    "\n"sv;

static std::string_view SolidPostGLSL =
    "UBINDING2 uniform SolidUniform\n"
    "{\n"
    "    vec4 solidColor;\n"
    "};\n"
    "vec4 SolidPostFunc(vec4 colorIn)\n"
    "{\n"
    "    return solidColor;\n"
    "}\n"
    "\n"sv;

static std::string_view MBShadowPostGLSL =
    "UBINDING2 uniform MBShadowUniform\n"
    "{\n"
    "    vec4 shadowUp;\n"
    "    float shadowId;\n"
    "};\n"
    "vec4 MBShadowPostFunc(vec4 colorIn)\n"
    "{\n"
    "    float idTexel = texture(extTex0, vtf.extTcgs[0]).a;\n"
    "    float sphereTexel = texture(extTex1, vtf.extTcgs[1]).a;\n"
    "    float fadeTexel = texture(extTex2, vtf.extTcgs[2]).a;\n"
    "    float val = ((abs(idTexel - shadowId) < 0.001) ?\n"
    "        (dot(vtf.mvNorm.xyz, shadowUp.xyz) * shadowUp.w) : 0.0) *\n"
    "        sphereTexel * fadeTexel;\n"
    "    return vec4(0.0, 0.0, 0.0, val);\n"
    "}\n"
    "\n"sv;

static std::string_view DisintegratePostGLSL = FOG_STRUCT_GLSL
    "UBINDING2 uniform DisintegrateUniform\n"
    "{\n"
    "    vec4 daddColor;\n"
    "    Fog fog;\n"
    "};\n"
    "vec4 DisintegratePostFunc(vec4 colorIn)\n"
    "{\n"
    "    vec4 texel0 = texture(extTex7, vtf.extTcgs[0]);\n"
    "    vec4 texel1 = texture(extTex7, vtf.extTcgs[1]);\n"
    "    colorIn = mix(vec4(0.0), texel1, texel0);\n"
    "    colorIn.rgb += daddColor.rgb;\n" FOG_ALGORITHM_GLSL
    "}\n"
    "\n"sv;

const hecl::Backend::Function ExtensionLightingFuncsGLSL[] = {
    {},
    {LightingGLSL, "LightingFunc"},
    {},
    {LightingGLSL, "LightingFunc"},
    {LightingGLSL, "LightingFunc"},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {LightingShadowGLSL, "LightingShadowFunc"},
    {LightingGLSL, "LightingFunc"},
    {LightingGLSL, "LightingFunc"},
    {LightingGLSL, "LightingFunc"},
    {LightingGLSL, "LightingFunc"},
    {LightingGLSL, "LightingFunc"},
    {LightingGLSL, "LightingFunc"},
    {LightingGLSL, "LightingFunc"},
    {},
    {LightingGLSL, "LightingFunc"},
};

const hecl::Backend::Function ExtensionPostFuncsGLSL[] = {
    {},
    {MainPostGLSL, "MainPostFunc"},
    {ThermalPostGLSL, "ThermalPostFunc"},
    {MainPostGLSL, "MainPostFunc"},
    {MainPostGLSL, "MainPostFunc"},
    {SolidPostGLSL, "SolidPostFunc"},
    {SolidPostGLSL, "SolidPostFunc"},
    {SolidPostGLSL, "SolidPostFunc"},
    {SolidPostGLSL, "SolidPostFunc"},
    {SolidPostGLSL, "SolidPostFunc"},
    {SolidPostGLSL, "SolidPostFunc"},
    {MBShadowPostGLSL, "MBShadowPostFunc"},
    {MainPostGLSL, "MainPostFunc"},
    {MainPostGLSL, "MainPostFunc"},
    {MainPostGLSL, "MainPostFunc"},
    {MainPostGLSL, "MainPostFunc"},
    {MainPostGLSL, "MainPostFunc"},
    {MainPostGLSL, "MainPostFunc"},
    {MainPostGLSL, "MainPostFunc"},
    {MainPostGLSL, "MainPostFunc"},
    {DisintegratePostGLSL, "DisintegratePostFunc"},
    {MainPostGLSL, "MainPostFunc"},
};

} // namespace urde
