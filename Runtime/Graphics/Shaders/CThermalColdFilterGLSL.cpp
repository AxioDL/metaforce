#include "CThermalColdFilter.hpp"

namespace urde
{

static const char* VS_GLSL_TEX =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec4 posIn;\n"
"layout(location=1) in vec4 uvIn;\n"
"\n"
"UBINDING0 uniform ThermalColdUniform\n"
"{\n"
"    mat4 shiftMtx;\n"
"    mat4 indMtx;\n"
"    vec4 colorReg0;\n"
"    vec4 colorReg1;\n"
"    vec4 colorReg2;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    mat3 indMtx;\n"
"    vec4 colorReg0;\n"
"    vec4 colorReg1;\n"
"    vec4 colorReg2;\n"
"    vec2 sceneUv;\n"
"    vec2 shiftUv;\n"
"};\n"
"\n"
"SBINDING(0) out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.indMtx = mat3(indMtx);\n"
"    vtf.colorReg0 = colorReg0;\n"
"    vtf.colorReg1 = colorReg1;\n"
"    vtf.colorReg2 = colorReg2;\n"
"    vtf.sceneUv = uvIn.xy;\n"
"    vtf.shiftUv = shiftMtx * uvIn;\n"
"    gl_Position = FLIPFROMGL(posIn);\n"
"}\n";

static const char* FS_GLSL_TEX =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"struct VertToFrag\n"
"{\n"
"    mat3 indMtx;\n"
"    vec4 colorReg0;\n"
"    vec4 colorReg1;\n"
"    vec4 colorReg2;\n"
"    vec2 sceneUv;\n"
"    vec2 shiftUv;\n"
"    vec2 shiftDelta;\n"
"};\n"
"\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D sceneTex;\n"
"TBINDING1 uniform sampler2D shiftTex;\n"
"const vec4 kRGBToYPrime = vec4(0.299, 0.587, 0.114, 0.0);\n"
"void main()\n"
"{\n"
"    vec2 shiftCoord = vtf.sceneUv + texture(shiftTex, vtf.shiftUv).xy;\n"
"    float shiftScene0 = dot(texture(sceneTex, shiftCoord - vec2(0.5)), kRGBToYPrime);\n"
"    float shiftScene1 = dot(texture(sceneTex, shiftCoord - vec2(0.5, 0.4984375)), kRGBToYPrime);\n"
"    vec2 indCoord = (indMtx * vec3(shiftScene0, shiftScene1, 1.0)).xy;\n"
"    float indScene = dot(texture(sceneTex, vtf.sceneUv + indCoord), kRGBToYPrime);\n"
"    colorOut = vtf.colorReg0 * indScene + vtf.colorReg1 * shiftScene0 + vtf.colorReg2;\n"
"}\n";

URDE_DECL_SPECIALIZE_FILTER(CThermalColdFilter)

struct CThermalColdFilterGLDataBindingFactory : TFilterShader<CThermalColdFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CThermalColdFilter& filter)
    {
        boo::GLDataFactory::Context& cctx = static_cast<boo::GLDataFactory::Context&>(ctx);

        const boo::VertexElementDescriptor VtxVmt[] =
        {
            {filter.m_vbo, nullptr, boo::VertexSemantic::Position4},
            {filter.m_vbo, nullptr, boo::VertexSemantic::UV4}
        };
        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
        boo::ITexture* texs[] = {CGraphics::g_SpareTexture, filter.m_shiftTex};
        return cctx.newShaderDataBinding(TFilterShader<CThermalColdFilter>::m_pipeline,
                                         ctx.newVertexFormat(3, VtxVmt), filter.m_vbo, nullptr, nullptr,
                                         1, bufs, stages, nullptr, nullptr, 2, texs);
    }
};

#if BOO_HAS_VULKAN
struct CThermalColdFilterVulkanDataBindingFactory : TFilterShader<CThermalColdFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CThermalColdFilter& filter)
    {
        boo::VulkanDataFactory::Context& cctx = static_cast<boo::VulkanDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::ITexture* texs[] = {CGraphics::g_SpareTexture, filter.m_shiftTex};
        return cctx.newShaderDataBinding(TFilterShader<CThermalColdFilter>::m_pipeline,
                                         TFilterShader<CThermalColdFilter>::m_vtxFmt,
                                         filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 2, texs);
    }
};
#endif

TFilterShader<CThermalColdFilter>::IDataBindingFactory* CThermalColdFilter::Initialize(boo::GLDataFactory::Context& ctx,
                                                                                       boo::IShaderPipeline*& pipeOut)
{
    const char* texNames[] = {"sceneTex", "shiftTex"};
    const char* uniNames[] = {"ThermalColdUniform"};
    pipeOut = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, 2, texNames, 1, uniNames, boo::BlendFactor::One,
                                    boo::BlendFactor::Zero, boo::Primitive::TriStrips, false, false, false);
    return new CThermalColdFilterGLDataBindingFactory;
}

#if BOO_HAS_VULKAN
TFilterShader<CThermalColdFilter>::IDataBindingFactory* CThermalColdFilter::Initialize(boo::VulkanDataFactory::Context& ctx,
                                                                                       boo::IShaderPipeline*& pipeOut,
                                                                                       boo::IVertexFormat*& vtxFmtOut)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    vtxFmtOut = ctx.newVertexFormat(2, VtxVmt);
    pipeOut = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, vtxFmtOut, boo::BlendFactor::One,
                                    boo::BlendFactor::Zero, boo::Primitive::TriStrips, false, false, false);
    return new CThermalColdFilterVulkanDataBindingFactory;
}
#endif

}
