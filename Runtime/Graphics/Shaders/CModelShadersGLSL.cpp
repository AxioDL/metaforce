#include "CModelShaders.hpp"
#include "hecl/Backend/GLSL.hpp"

namespace urde
{

static const char* LightingGLSL =
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
"    Fog fog;\n"
"};\n"
"\n"
"vec4 LightingFunc(vec4 mvPosIn, vec4 mvNormIn)\n"
"{\n"
"    vec4 ret = ambient;\n"
"    \n"
"    for (int i=0 ; i<" _XSTR(URDE_MAX_LIGHTS) " ; ++i)\n"
"    {\n"
"        vec3 delta = mvPosIn.xyz - lights[i].pos.xyz;\n"
"        float dist = length(delta);\n"
"        float angDot = clamp(dot(normalize(delta), lights[i].dir.xyz), 0.0, 1.0);\n"
"        float att = 1.0 / (lights[i].linAtt[2] * dist * dist +\n"
"                           lights[i].linAtt[1] * dist +\n"
"                           lights[i].linAtt[0]);\n"
"        float angAtt = lights[i].angAtt[2] * angDot * angDot +\n"
"                       lights[i].angAtt[1] * angDot +\n"
"                       lights[i].angAtt[0];\n"
"        ret += lights[i].color * clamp(angAtt, 0.0, 1.0) * att * clamp(dot(normalize(-delta), mvNormIn.xyz), 0.0, 1.0);\n"
"    }\n"
"    \n"
"    return clamp(ret, vec4(0.0,0.0,0.0,0.0), vec4(1.0,1.0,1.0,1.0));\n"
"}\n";

static const char* LightingShadowGLSL =
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
"    Fog fog;\n"
"};\n"
"\n"
"vec4 LightingShadowFunc(vec4 mvPosIn, vec4 mvNormIn)\n"
"{\n"
"    vec4 ret = ambient;\n"
"    \n"
"    vec3 delta = mvPosIn.xyz - lights[0].pos.xyz;\n"
"    float dist = length(delta);\n"
"    float angDot = clamp(dot(normalize(delta), lights[0].dir.xyz), 0.0, 1.0);\n"
"    float att = 1.0 / (lights[0].linAtt[2] * dist * dist +\n"
"                       lights[0].linAtt[1] * dist +\n"
"                       lights[0].linAtt[0]);\n"
"    float angAtt = lights[0].angAtt[2] * angDot * angDot +\n"
"                   lights[0].angAtt[1] * angDot +\n"
"                   lights[0].angAtt[0];\n"
"    ret += lights[0].color * clamp(angAtt, 0.0, 1.0) * att * clamp(dot(normalize(-delta), mvNormIn.xyz), 0.0, 1.0) *\n"
"           texture(extTex7, vtf.extTcgs[0]).r;\n"
"    \n"
"    for (int i=1 ; i<" _XSTR(URDE_MAX_LIGHTS) " ; ++i)\n"
"    {\n"
"        vec3 delta = mvPosIn.xyz - lights[i].pos.xyz;\n"
"        float dist = length(delta);\n"
"        float angDot = clamp(dot(normalize(delta), lights[i].dir.xyz), 0.0, 1.0);\n"
"        float att = 1.0 / (lights[i].linAtt[2] * dist * dist +\n"
"                           lights[i].linAtt[1] * dist +\n"
"                           lights[i].linAtt[0]);\n"
"        float angAtt = lights[i].angAtt[2] * angDot * angDot +\n"
"                       lights[i].angAtt[1] * angDot +\n"
"                       lights[i].angAtt[0];\n"
"        ret += lights[i].color * clamp(angAtt, 0.0, 1.0) * att * clamp(dot(normalize(-delta), mvNormIn.xyz), 0.0, 1.0);\n"
"    }\n"
"    \n"
"    return clamp(ret, vec4(0.0,0.0,0.0,0.0), vec4(1.0,1.0,1.0,1.0));\n"
"}\n";

static const char* MainPostGLSL =
"vec4 MainPostFunc(vec4 colorIn)\n"
"{\n"
"    float fogZ = (-vtf.mvPos.z - fog.start) * fog.rangeScale;\n"
"    return mix(fog.color, colorIn, clamp(exp2(-8.0 * fogZ), 0.0, 1.0));\n"
"}\n"
"\n";

static const char* ThermalPostGLSL =
"UBINDING2 uniform ThermalUniform\n"
"{\n"
"    vec4 tmulColor;\n"
"    vec4 addColor;\n"
"};\n"
"vec4 ThermalPostFunc(vec4 colorIn)\n"
"{\n"
"    return vec4(texture(extTex7, vtf.extTcgs[0]).rrr * tmulColor.rgb + addColor.rgb, 1.0);\n"
"}\n"
"\n";

static const char* SolidPostGLSL =
"UBINDING2 uniform SolidUniform\n"
"{\n"
"    vec4 solidColor;\n"
"};\n"
"vec4 SolidPostFunc(vec4 colorIn)\n"
"{\n"
"    return solidColor;\n"
"}\n"
"\n";

static const char* MBShadowPostGLSL =
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
"\n";

static const char* BlockNames[] = {HECL_GLSL_VERT_UNIFORM_BLOCK_NAME,
                                   HECL_GLSL_TEXMTX_UNIFORM_BLOCK_NAME,
                                   "LightingUniform"};

static const char* ThermalBlockNames[] = {HECL_GLSL_VERT_UNIFORM_BLOCK_NAME,
                                          HECL_GLSL_TEXMTX_UNIFORM_BLOCK_NAME,
                                         "ThermalUniform"};

static const char* SolidBlockNames[] = {HECL_GLSL_VERT_UNIFORM_BLOCK_NAME,
                                        HECL_GLSL_TEXMTX_UNIFORM_BLOCK_NAME,
                                        "SolidUniform"};

static const char* MBShadowBlockNames[] = {HECL_GLSL_VERT_UNIFORM_BLOCK_NAME,
                                           HECL_GLSL_TEXMTX_UNIFORM_BLOCK_NAME,
                                           "MBShadowUniform"};

hecl::Runtime::ShaderCacheExtensions
CModelShaders::GetShaderExtensionsGLSL(boo::IGraphicsDataFactory::Platform plat)
{
    hecl::Runtime::ShaderCacheExtensions ext(plat);

    /* Normal lit shading */
    ext.registerExtensionSlot({LightingGLSL, "LightingFunc"}, {MainPostGLSL, "MainPostFunc"},
                              3, BlockNames, 0, nullptr, hecl::Backend::BlendFactor::Original,
                              hecl::Backend::BlendFactor::Original, hecl::Backend::ZTest::Original,
                              hecl::Backend::CullMode::Backface, false, false, true);

    /* Thermal Visor shading */
    ext.registerExtensionSlot({}, {ThermalPostGLSL, "ThermalPostFunc"}, 3, ThermalBlockNames,
                              1, ThermalTextures, hecl::Backend::BlendFactor::One,
                              hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Original,
                              hecl::Backend::CullMode::Backface, false, false, true);

    /* Forced alpha shading */
    ext.registerExtensionSlot({LightingGLSL, "LightingFunc"}, {MainPostGLSL, "MainPostFunc"},
                              3, BlockNames, 0, nullptr, hecl::Backend::BlendFactor::SrcAlpha,
                              hecl::Backend::BlendFactor::InvSrcAlpha, hecl::Backend::ZTest::Original,
                              hecl::Backend::CullMode::Backface, false, false, true);

    /* Forced additive shading */
    ext.registerExtensionSlot({LightingGLSL, "LightingFunc"}, {MainPostGLSL, "MainPostFunc"},
                              3, BlockNames, 0, nullptr, hecl::Backend::BlendFactor::SrcAlpha,
                              hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Original,
                              hecl::Backend::CullMode::Backface, true, false, true);

    /* Solid color */
    ext.registerExtensionSlot({}, {SolidPostGLSL, "SolidPostFunc"},
                              3, SolidBlockNames, 0, nullptr, hecl::Backend::BlendFactor::One,
                              hecl::Backend::BlendFactor::Zero, hecl::Backend::ZTest::LEqual,
                              hecl::Backend::CullMode::Backface, false, false, false);

    /* Solid color additive */
    ext.registerExtensionSlot({}, {SolidPostGLSL, "SolidPostFunc"},
                              3, SolidBlockNames, 0, nullptr, hecl::Backend::BlendFactor::SrcAlpha,
                              hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::LEqual,
                              hecl::Backend::CullMode::Backface, true, false, true);

    /* Alpha-only Solid color frontface cull, LEqual */
    ext.registerExtensionSlot({}, {SolidPostGLSL, "SolidPostFunc"},
                              3, SolidBlockNames, 0, nullptr, hecl::Backend::BlendFactor::Zero,
                              hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::LEqual,
                              hecl::Backend::CullMode::Frontface, false, true, false);

    /* Alpha-only Solid color frontface cull, Always, No Z-write */
    ext.registerExtensionSlot({}, {SolidPostGLSL, "SolidPostFunc"},
                              3, SolidBlockNames, 0, nullptr, hecl::Backend::BlendFactor::Zero,
                              hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::None,
                              hecl::Backend::CullMode::Frontface, true, true, false);

    /* Alpha-only Solid color backface cull, LEqual */
    ext.registerExtensionSlot({}, {SolidPostGLSL, "SolidPostFunc"},
                              3, SolidBlockNames, 0, nullptr, hecl::Backend::BlendFactor::Zero,
                              hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::LEqual,
                              hecl::Backend::CullMode::Backface, false, true, false);

    /* Alpha-only Solid color backface cull, Greater, No Z-write */
    ext.registerExtensionSlot({}, {SolidPostGLSL, "SolidPostFunc"},
                              3, SolidBlockNames, 0, nullptr, hecl::Backend::BlendFactor::Zero,
                              hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Greater,
                              hecl::Backend::CullMode::Backface, true, true, false);

    /* MorphBall shadow shading */
    ext.registerExtensionSlot({}, {MBShadowPostGLSL, "MBShadowPostFunc"},
                              3, MBShadowBlockNames, 3, BallFadeTextures,
                              hecl::Backend::BlendFactor::SrcAlpha,
                              hecl::Backend::BlendFactor::InvSrcAlpha,
                              hecl::Backend::ZTest::Equal,
                              hecl::Backend::CullMode::Backface, false, false, true, true);

    /* World shadow shading (modified lighting) */
    ext.registerExtensionSlot({LightingShadowGLSL, "LightingShadowFunc"}, {MainPostGLSL, "MainPostFunc"},
                              3, BlockNames, 1, WorldShadowTextures, hecl::Backend::BlendFactor::Original,
                              hecl::Backend::BlendFactor::Original, hecl::Backend::ZTest::Original,
                              hecl::Backend::CullMode::Backface, false, false, true);

    /* Forced alpha shading without culling */
    ext.registerExtensionSlot({LightingGLSL, "LightingFunc"}, {MainPostGLSL, "MainPostFunc"},
                              3, BlockNames, 0, nullptr, hecl::Backend::BlendFactor::SrcAlpha,
                              hecl::Backend::BlendFactor::InvSrcAlpha, hecl::Backend::ZTest::Original,
                              hecl::Backend::CullMode::None, true, false, true);

    /* Forced additive shading without culling */
    ext.registerExtensionSlot({LightingGLSL, "LightingFunc"}, {MainPostGLSL, "MainPostFunc"},
                              3, BlockNames, 0, nullptr, hecl::Backend::BlendFactor::SrcAlpha,
                              hecl::Backend::BlendFactor::One, hecl::Backend::ZTest::Original,
                              hecl::Backend::CullMode::None, true, false, true);

    return ext;
}

}
