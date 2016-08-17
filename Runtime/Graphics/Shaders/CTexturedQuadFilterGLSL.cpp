#include "CTexturedQuadFilter.hpp"
#include "TMultiBlendShader.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{

static const char* VS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec4 posIn;\n"
"layout(location=1) in vec4 uvIn;\n"
"\n"
"UBINDING0 uniform TexuredQuadUniform\n"
"{\n"
"    vec4 color;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec2 uv;\n"
"};\n"
"\n"
"SBINDING(0) out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.color = color;\n"
"    vtf.uv = uvIn.xy;\n"
"    gl_Position = vec4(posIn.xyz, 1.0);\n"
"}\n";

static const char* FS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec2 uv;\n"
"};\n"
"\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D tex;\n"
"void main()\n"
"{\n"
"    colorOut = vtf.color * texture(tex, vtf.uv);\n"
"}\n";

URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CTexturedQuadFilter)

struct CTexturedQuadFilterGLDataBindingFactory : TMultiBlendShader<CTexturedQuadFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    boo::IShaderPipeline* pipeline,
                                                    boo::IVertexFormat*,
                                                    CTexturedQuadFilter& filter)
    {
        boo::GLDataFactory::Context& cctx = static_cast<boo::GLDataFactory::Context&>(ctx);

        const boo::VertexElementDescriptor VtxVmt[] =
        {
            {filter.m_vbo, nullptr, boo::VertexSemantic::Position4},
            {filter.m_vbo, nullptr, boo::VertexSemantic::UV4}
        };
        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
        boo::ITexture* texs[] = {filter.m_tex->GetBooTexture()};
        return cctx.newShaderDataBinding(pipeline,
                                         ctx.newVertexFormat(2, VtxVmt), filter.m_vbo, nullptr, nullptr,
                                         1, bufs, stages, nullptr, nullptr, 1, texs);
    }
};

#if BOO_HAS_VULKAN
struct CTexturedQuadFilterVulkanDataBindingFactory : TMultiBlendShader<CTexturedQuadFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    boo::IShaderPipeline* pipeline,
                                                    boo::IVertexFormat* vtxFmt,
                                                    CTexturedQuadFilter& filter)
    {
        boo::VulkanDataFactory::Context& cctx = static_cast<boo::VulkanDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::ITexture* texs[] = {filter.m_tex->GetBooTexture()};
        return cctx.newShaderDataBinding(pipeline, vtxFmt,
                                         filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 1, texs);
    }
};
#endif

TMultiBlendShader<CTexturedQuadFilter>::IDataBindingFactory*
CTexturedQuadFilter::Initialize(boo::GLDataFactory::Context& ctx,
                                boo::IShaderPipeline*& alphaPipeOut,
                                boo::IShaderPipeline*& additivePipeOut,
                                boo::IShaderPipeline*& colorMultiplyPipeOut)
{
    const char* texNames[] = {"tex"};
    const char* uniNames[] = {"TexuredQuadUniform"};
    alphaPipeOut = ctx.newShaderPipeline(VS, FS, 1, texNames, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                         boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips, false, false, false);
    additivePipeOut = ctx.newShaderPipeline(VS, FS, 1, texNames, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                            boo::BlendFactor::One, boo::Primitive::TriStrips, false, false, false);
    colorMultiplyPipeOut = ctx.newShaderPipeline(VS, FS, 1, texNames, 1, uniNames, boo::BlendFactor::SrcColor,
                                                 boo::BlendFactor::DstColor, boo::Primitive::TriStrips, false, false, false);
    return new CTexturedQuadFilterGLDataBindingFactory;
}

#if BOO_HAS_VULKAN
TMultiBlendShader<CTexturedQuadFilter>::IDataBindingFactory*
CTexturedQuadFilter::Initialize(boo::VulkanDataFactory::Context& ctx,
                                boo::IShaderPipeline*& alphaPipeOut,
                                boo::IShaderPipeline*& additivePipeOut,
                                boo::IShaderPipeline*& colorMultiplyPipeOut,
                                boo::IVertexFormat*& vtxFmtOut)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    vtxFmtOut = ctx.newVertexFormat(2, VtxVmt);
    alphaPipeOut = ctx.newShaderPipeline(VS, FS, vtxFmtOut, boo::BlendFactor::SrcAlpha,
                                         boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips, false, false, false);
    additivePipeOut = ctx.newShaderPipeline(VS, FS, vtxFmtOut, boo::BlendFactor::SrcAlpha,
                                            boo::BlendFactor::One, boo::Primitive::TriStrips, false, false, false);
    colorMultiplyPipeOut = ctx.newShaderPipeline(VS, FS, vtxFmtOut, boo::BlendFactor::SrcColor,
                                                 boo::BlendFactor::DstColor, boo::Primitive::TriStrips, false, false, false);
    return new CTexturedQuadFilterVulkanDataBindingFactory;
}
#endif

}
