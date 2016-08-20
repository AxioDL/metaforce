#include "CColoredQuadFilter.hpp"
#include "TMultiBlendShader.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{

static const char* VS =
"struct VertData\n"
"{\n"
"    float4 posIn : POSITION;\n"
"};\n"
"\n"
"cbuffer ColoredQuadUniform : register(b0)\n"
"{\n"
"    float4x4 xf;\n"
"    float4 color;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"};\n"
"\n"
"VertToFrag main(in VertData v)\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = color;\n"
"    vtf.position = mul(xf, float4(v.posIn.xyz, 1.0));\n"
"    return vtf;\n"
"}\n";

static const char* FS =
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"};\n"
"\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    return vtf.color;\n"
"}\n";

URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CColoredQuadFilter)

struct CColoredQuadFilterD3DDataBindingFactory : TMultiBlendShader<CColoredQuadFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    boo::IShaderPipeline* pipeline,
                                                    boo::IVertexFormat* vtxFmt,
                                                    CColoredQuadFilter& filter)
    {
        boo::ID3DDataFactory::Context& cctx = static_cast<boo::ID3DDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        return cctx.newShaderDataBinding(pipeline, vtxFmt,
                                         filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 0, nullptr);
    }
};

TMultiBlendShader<CColoredQuadFilter>::IDataBindingFactory*
CColoredQuadFilter::Initialize(boo::ID3DDataFactory::Context& ctx,
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
    alphaPipeOut = ctx.newShaderPipeline(VS, FS, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                         vtxFmtOut, boo::BlendFactor::SrcAlpha,
                                         boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips, false, false, false);
    additivePipeOut = ctx.newShaderPipeline(VS, FS, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                            vtxFmtOut, boo::BlendFactor::SrcAlpha,
                                            boo::BlendFactor::One, boo::Primitive::TriStrips, false, false, false);
    colorMultiplyPipeOut = ctx.newShaderPipeline(VS, FS, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                                 vtxFmtOut, boo::BlendFactor::SrcColor,
                                                 boo::BlendFactor::DstColor, boo::Primitive::TriStrips, false, false, false);
    return new CColoredQuadFilterD3DDataBindingFactory;
}

}
