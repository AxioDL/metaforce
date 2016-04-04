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
"\n"
"UBINDING2 uniform LightingUniform\n"
"{\n"
"    Light lights[" _XSTR(URDE_MAX_LIGHTS) "];\n"
"    vec4 ambient;\n"
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

hecl::Runtime::ShaderCacheExtensions
CModelShaders::GetShaderExtensionsGLSL(boo::IGraphicsDataFactory::Platform plat)
{
    static const char* BlockNames[] = {HECL_GLSL_VERT_UNIFORM_BLOCK_NAME,
                                       HECL_GLSL_TEXMTX_UNIFORM_BLOCK_NAME,
                                       "LightingUniform"};
    hecl::Runtime::ShaderCacheExtensions ext(plat);
    ext.registerExtensionSlot({LightingGLSL, "LightingFunc"}, {}, 3, BlockNames);
    return ext;
}

}
