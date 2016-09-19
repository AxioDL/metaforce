#include "CTexturedQuadFilter.hpp"
#include "TMultiBlendShader.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{

static const char* VS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertData\n"
"{\n"
"    float4 posIn [[ attribute(0) ]];\n"
"    float4 uvIn [[ attribute(1) ]];\n"
"};\n"
"\n"
"struct TexuredQuadUniform\n"
"{\n"
"    float4x4 mat;\n"
"    float4 color;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"};\n"
"\n"
"vertex VertToFrag vmain(VertData v [[ stage_in ]], constant TexuredQuadUniform& tqu [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = tqu.color;\n"
"    vtf.uv = v.uvIn.xy;\n"
"    vtf.uv.y = -vtf.uv.y;\n"
"    vtf.position = tqu.mat * float4(v.posIn.xyz, 1.0);\n"
"    return vtf;\n"
"}\n";

static const char* FS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"constexpr sampler samp(address::repeat, filter::linear);\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"};\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]], texture2d<float> tex [[ texture(0) ]])\n"
"{\n"
"    return vtf.color * float4(tex.sample(samp, vtf.uv).rgb, 1.0);\n"
"}\n";

static const char* FSAlpha =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"constexpr sampler samp(address::repeat, filter::linear);\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"};\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]], texture2d<float> tex [[ texture(0) ]])\n"
"{\n"
"    return vtf.color * tex.sample(samp, vtf.uv);\n"
"}\n";

URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CTexturedQuadFilter)

struct CTexturedQuadFilterMetalDataBindingFactory : TMultiBlendShader<CTexturedQuadFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    boo::IShaderPipeline* pipeline,
                                                    boo::IVertexFormat* vtxFmt,
                                                    CTexturedQuadFilter& filter)
    {
        boo::MetalDataFactory::Context& cctx = static_cast<boo::MetalDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::ITexture* texs[] = {filter.m_booTex};
        return cctx.newShaderDataBinding(pipeline, vtxFmt,
                                         filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 1, texs);
    }
};

TMultiBlendShader<CTexturedQuadFilter>::IDataBindingFactory*
CTexturedQuadFilter::Initialize(boo::MetalDataFactory::Context& ctx,
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
    alphaPipeOut = ctx.newShaderPipeline(VS, FS, vtxFmtOut, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcAlpha,
                                         boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips, false, false, false);
    additivePipeOut = ctx.newShaderPipeline(VS, FS, vtxFmtOut, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcAlpha,
                                            boo::BlendFactor::One, boo::Primitive::TriStrips, false, false, false);
    colorMultiplyPipeOut = ctx.newShaderPipeline(VS, FS, vtxFmtOut, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcColor,
                                                 boo::BlendFactor::DstColor, boo::Primitive::TriStrips, false, false, false);
    return new CTexturedQuadFilterMetalDataBindingFactory;
}

struct CTexturedQuadFilterAlphaMetalDataBindingFactory : TMultiBlendShader<CTexturedQuadFilterAlpha>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    boo::IShaderPipeline* pipeline,
                                                    boo::IVertexFormat* vtxFmt,
                                                    CTexturedQuadFilterAlpha& filter)
    {
        boo::MetalDataFactory::Context& cctx = static_cast<boo::MetalDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::ITexture* texs[] = {filter.m_booTex};
        return cctx.newShaderDataBinding(pipeline, vtxFmt,
                                         filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 1, texs);
    }
};

TMultiBlendShader<CTexturedQuadFilterAlpha>::IDataBindingFactory*
CTexturedQuadFilterAlpha::Initialize(boo::MetalDataFactory::Context& ctx,
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
    alphaPipeOut = ctx.newShaderPipeline(VS, FSAlpha, vtxFmtOut, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcAlpha,
                                         boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips, false, false, false);
    additivePipeOut = ctx.newShaderPipeline(VS, FSAlpha, vtxFmtOut, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcAlpha,
                                            boo::BlendFactor::One, boo::Primitive::TriStrips, false, false, false);
    colorMultiplyPipeOut = ctx.newShaderPipeline(VS, FSAlpha, vtxFmtOut, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcColor,
                                                 boo::BlendFactor::DstColor, boo::Primitive::TriStrips, false, false, false);
    return new CTexturedQuadFilterAlphaMetalDataBindingFactory;
}

}
