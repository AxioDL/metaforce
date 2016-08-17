#include "CColoredQuadFilter.hpp"
#include "TMultiBlendShader.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{

static const char* VS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec4 posIn;\n"
"\n"
"UBINDING0 uniform ColoredQuadUniform\n"
"{\n"
"    vec4 color;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"};\n"
"\n"
"SBINDING(0) out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.color = color;\n"
"    gl_Position = vec4(posIn.xyz, 1.0);\n"
"}\n";

static const char* FS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"};\n"
"\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"void main()\n"
"{\n"
"    colorOut = vtf.color;\n"
"}\n";

URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CColoredQuadFilter)

struct CColoredQuadFilterGLDataBindingFactory : TMultiBlendShader<CColoredQuadFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    boo::IShaderPipeline* pipeline,
                                                    boo::IVertexFormat*,
                                                    CColoredQuadFilter& filter)
    {
        boo::GLDataFactory::Context& cctx = static_cast<boo::GLDataFactory::Context&>(ctx);

        const boo::VertexElementDescriptor VtxVmt[] =
        {
            {filter.m_vbo, nullptr, boo::VertexSemantic::Position4}
        };
        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
        return cctx.newShaderDataBinding(pipeline,
                                         ctx.newVertexFormat(1, VtxVmt), filter.m_vbo, nullptr, nullptr,
                                         1, bufs, stages, nullptr, nullptr, 0, nullptr);
    }
};

#if BOO_HAS_VULKAN
struct CColoredQuadFilterVulkanDataBindingFactory : TMultiBlendShader<CColoredQuadFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    boo::IShaderPipeline* pipeline,
                                                    boo::IVertexFormat* vtxFmt,
                                                    CColoredQuadFilter& filter)
    {
        boo::VulkanDataFactory::Context& cctx = static_cast<boo::VulkanDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        return cctx.newShaderDataBinding(pipeline, vtxFmt,
                                         filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 0, nullptr);
    }
};
#endif

TMultiBlendShader<CColoredQuadFilter>::IDataBindingFactory*
CColoredQuadFilter::Initialize(boo::GLDataFactory::Context& ctx,
                               boo::IShaderPipeline*& alphaPipeOut,
                               boo::IShaderPipeline*& additivePipeOut,
                               boo::IShaderPipeline*& colorMultiplyPipeOut)
{
    const char* uniNames[] = {"ColoredQuadUniform"};
    alphaPipeOut = ctx.newShaderPipeline(VS, FS, 0, nullptr, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                         boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips, false, false, false);
    additivePipeOut = ctx.newShaderPipeline(VS, FS, 0, nullptr, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                            boo::BlendFactor::One, boo::Primitive::TriStrips, false, false, false);
    colorMultiplyPipeOut = ctx.newShaderPipeline(VS, FS, 0, nullptr, 1, uniNames, boo::BlendFactor::SrcColor,
                                                 boo::BlendFactor::DstColor, boo::Primitive::TriStrips, false, false, false);
    return new CColoredQuadFilterGLDataBindingFactory;
}

#if BOO_HAS_VULKAN
TMultiBlendShader<CColoredQuadFilter>::IDataBindingFactory*
CColoredQuadFilter::Initialize(boo::VulkanDataFactory::Context& ctx,
                               boo::IShaderPipeline*& alphaPipeOut,
                               boo::IShaderPipeline*& additivePipeOut,
                               boo::IShaderPipeline*& colorMultiplyPipeOut,
                               boo::IVertexFormat*& vtxFmtOut)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4}
    };
    vtxFmtOut = ctx.newVertexFormat(1, VtxVmt);
    alphaPipeOut = ctx.newShaderPipeline(VS, FS, vtxFmtOut, boo::BlendFactor::SrcAlpha,
                                         boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips, false, false, false);
    additivePipeOut = ctx.newShaderPipeline(VS, FS, vtxFmtOut, boo::BlendFactor::SrcAlpha,
                                            boo::BlendFactor::One, boo::Primitive::TriStrips, false, false, false);
    colorMultiplyPipeOut = ctx.newShaderPipeline(VS, FS, vtxFmtOut, boo::BlendFactor::SrcColor,
                                                 boo::BlendFactor::DstColor, boo::Primitive::TriStrips, false, false, false);
    return new CColoredQuadFilterVulkanDataBindingFactory;
}
#endif

}
