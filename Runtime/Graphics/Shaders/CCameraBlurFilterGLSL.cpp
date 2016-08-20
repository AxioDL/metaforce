#include "CCameraBlurFilter.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "GameGlobalObjects.hpp"

namespace urde
{

static const char* VS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec4 posIn;\n"
"layout(location=1) in vec4 uvIn;\n"
"\n"
"UBINDING0 uniform ThermalHotUniform\n"
"{\n"
"    vec4 colorReg0;\n"
"    vec4 colorReg1;\n"
"    vec4 colorReg2;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    vec2 sceneUv;\n"
"};\n"
"\n"
"SBINDING(0) out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.sceneUv = uvIn.xy;\n"
"    gl_Position = vec4(posIn.xyz, 1.0);\n"
"}\n";

static const char* FS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"struct VertToFrag\n"
"{\n"
"    vec2 sceneUv;\n"
"};\n"
"\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D sceneTex;\n"
"TBINDING1 uniform sampler2D paletteTex;\n"
"const vec4 kRGBToYPrime = vec4(0.299, 0.587, 0.114, 0.0);\n"
"void main()\n"
"{\n"
"    float sceneSample = dot(texture(sceneTex, vtf.sceneUv), kRGBToYPrime);\n"
"    vec4 colorSample = texture(paletteTex, vec2(sceneSample / 17.0, 0.5));\n"
"    colorOut = colorSample * sceneSample;\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CCameraBlurFilter)

struct CCameraBlurFilterGLDataBindingFactory : TShader<CCameraBlurFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    boo::IShaderPipeline* pipeline,
                                                    boo::IVertexFormat*,
                                                    CCameraBlurFilter& filter)
    {
        boo::GLDataFactory::Context& cctx = static_cast<boo::GLDataFactory::Context&>(ctx);

        const boo::VertexElementDescriptor VtxVmt[] =
        {
            {filter.m_vbo, nullptr, boo::VertexSemantic::Position4},
            {filter.m_vbo, nullptr, boo::VertexSemantic::UV4}
        };
        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
        boo::ITexture* texs[] = {CGraphics::g_SpareTexture, g_Renderer->GetThermoPalette()};
        return cctx.newShaderDataBinding(pipeline,
                                         ctx.newVertexFormat(2, VtxVmt), filter.m_vbo, nullptr, nullptr,
                                         1, bufs, stages, nullptr, nullptr, 2, texs);
    }
};

#if BOO_HAS_VULKAN
struct CCameraBlurFilterVulkanDataBindingFactory : TShader<CCameraBlurFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    boo::IShaderPipeline* pipeline,
                                                    boo::IVertexFormat* vtxFmt,
                                                    CCameraBlurFilter& filter)
    {
        boo::VulkanDataFactory::Context& cctx = static_cast<boo::VulkanDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::ITexture* texs[] = {CGraphics::g_SpareTexture, g_Renderer->GetThermoPalette()};
        return cctx.newShaderDataBinding(pipeline, vtxFmt,
                                         filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 2, texs);
    }
};
#endif

TShader<CCameraBlurFilter>::IDataBindingFactory* CCameraBlurFilter::Initialize(boo::GLDataFactory::Context& ctx,
                                                                               boo::IShaderPipeline*& pipeOut)
{
    const char* texNames[] = {"sceneTex", "paletteTex"};
    const char* uniNames[] = {"ThermalHotUniform"};
    pipeOut = ctx.newShaderPipeline(VS, FS, 2, texNames, 1, uniNames, boo::BlendFactor::DstAlpha,
                                    boo::BlendFactor::InvDstAlpha, boo::Primitive::TriStrips, false, false, false);
    return new CCameraBlurFilterGLDataBindingFactory;
}

#if BOO_HAS_VULKAN
TShader<CCameraBlurFilter>::IDataBindingFactory* CCameraBlurFilter::Initialize(boo::VulkanDataFactory::Context& ctx,
                                                                               boo::IShaderPipeline*& pipeOut,
                                                                               boo::IVertexFormat*& vtxFmtOut)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    vtxFmtOut = ctx.newVertexFormat(2, VtxVmt);
    pipeOut = ctx.newShaderPipeline(VS, FS, vtxFmtOut, boo::BlendFactor::DstAlpha,
                                    boo::BlendFactor::InvDstAlpha, boo::Primitive::TriStrips, false, false, false);
    return new CCameraBlurFilterVulkanDataBindingFactory;
}
#endif

}
