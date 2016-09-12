#include "CTexturedQuadFilter.hpp"
#include "TMultiBlendShader.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{

static const char* VS =
"struct VertData\n"
"{\n"
"    float4 posIn : POSITION;\n"
"    float4 uvIn : UV;\n"
"};\n"
"\n"
"cbuffer TexuredQuadUniform : register(b0)\n"
"{\n"
"    float4 color;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float2 uv : UV;\n"
"};\n"
"\n"
"VertToFrag main(in VertData v)\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = color;\n"
"    vtf.uv = v.uvIn.xy;\n"
"    vtf.uv.y = -vtf.uv.y;\n"
"    vtf.position = float4(v.posIn.xyz, 1.0);\n"
"    return vtf;\n"
"}\n";

static const char* FS =
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float2 uv : UV;\n"
"};\n"
"\n"
"Texture2D tex : register(t0);\n"
"SamplerState samp : register(s0);\n"
"\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    return vtf.color * float4(tex.Sample(samp, vtf.uv).rgb, 1.0);\n"
"}\n";

URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CTexturedQuadFilter)

struct CTexturedQuadFilterD3DDataBindingFactory : TMultiBlendShader<CTexturedQuadFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    boo::IShaderPipeline* pipeline,
                                                    boo::IVertexFormat* vtxFmt,
                                                    CTexturedQuadFilter& filter)
    {
        boo::ID3DDataFactory::Context& cctx = static_cast<boo::ID3DDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::ITexture* texs[] = {filter.m_booTex};
        return cctx.newShaderDataBinding(pipeline, vtxFmt,
                                         filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 1, texs);
    }
};

TMultiBlendShader<CTexturedQuadFilter>::IDataBindingFactory*
CTexturedQuadFilter::Initialize(boo::ID3DDataFactory::Context& ctx,
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
    alphaPipeOut = ctx.newShaderPipeline(VS, FS, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                         vtxFmtOut, boo::BlendFactor::SrcAlpha,
                                         boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips, false, false, false);
    additivePipeOut = ctx.newShaderPipeline(VS, FS, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                            vtxFmtOut, boo::BlendFactor::SrcAlpha,
                                            boo::BlendFactor::One, boo::Primitive::TriStrips, false, false, false);
    colorMultiplyPipeOut = ctx.newShaderPipeline(VS, FS, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                                 vtxFmtOut, boo::BlendFactor::SrcColor,
                                                 boo::BlendFactor::DstColor, boo::Primitive::TriStrips, false, false, false);
    return new CTexturedQuadFilterD3DDataBindingFactory;
}

}
