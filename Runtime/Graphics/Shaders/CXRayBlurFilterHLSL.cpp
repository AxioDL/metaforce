#include "CXRayBlurFilter.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "GameGlobalObjects.hpp"

namespace urde
{

static const char* VS =
"struct VertData\n"
"{\n"
"    float4 posIn : POSITION;\n"
"    float4 uvIn : UV;\n"
"};\n"
"\n"
"cbuffer XRayBlurUniform : register(b0)\n"
"{\n"
"    float4x4 uv0;\n"
"    float4x4 uv1;\n"
"    float4x4 uv2;\n"
"    float4x4 uv3;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float2 uv0 : UV0;\n"
"    float2 uv1 : UV1;\n"
"    float2 uv2 : UV2;\n"
"    float2 uv3 : UV3;\n"
"};\n"
"\n"
"VertToFrag main(in VertData v)\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.uv0 = mul(uv0, float4(v.uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv0.y = -vtf.uv0.y;\n"
"    vtf.uv1 = mul(uv1, float4(v.uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv1.y = -vtf.uv1.y;\n"
"    vtf.uv2 = mul(uv2, float4(v.uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv2.y = -vtf.uv2.y;\n"
"    vtf.uv3 = mul(uv3, float4(v.uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv3.y = -vtf.uv3.y;\n"
"    vtf.position = float4(v.posIn.xyz, 1.0);\n"
"    return vtf;\n"
"}\n";

static const char* FS =
"Texture2D sceneTex : register(t0);\n"
"Texture2D paletteTex : register(t1);\n"
"SamplerState samp : register(s0);\n"
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float2 uv0 : UV0;\n"
"    float2 uv1 : UV1;\n"
"    float2 uv2 : UV2;\n"
"    float2 uv3 : UV3;\n"
"};\n"
"\n"
"static const float4 kRGBToYPrime = float4(0.299, 0.587, 0.114, 0.0);\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    float4 colorSample = paletteTex.Sample(samp, float2(dot(sceneTex.Sample(samp, vtf.uv0), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.25;\n"
"    colorSample += paletteTex.Sample(samp, float2(dot(sceneTex.Sample(samp, vtf.uv1), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.25;\n"
"    colorSample += paletteTex.Sample(samp, float2(dot(sceneTex.Sample(samp, vtf.uv2), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.25;\n"
"    colorSample += paletteTex.Sample(samp, float2(dot(sceneTex.Sample(samp, vtf.uv3), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.25;\n"
"    return colorSample;\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CXRayBlurFilter)

struct CXRayBlurFilterD3DDataBindingFactory : TShader<CXRayBlurFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    boo::IShaderPipeline* pipeline,
                                                    boo::IVertexFormat* vtxFmt,
                                                    CXRayBlurFilter& filter)
    {
        boo::ID3DDataFactory::Context& cctx = static_cast<boo::ID3DDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::ITexture* texs[] = {CGraphics::g_SpareTexture, filter.m_booTex};
        return cctx.newShaderDataBinding(pipeline, vtxFmt,
                                         filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 2, texs);
    }
};

TShader<CXRayBlurFilter>::IDataBindingFactory* CXRayBlurFilter::Initialize(boo::ID3DDataFactory::Context& ctx,
                                                                           boo::IShaderPipeline*& pipeOut,
                                                                           boo::IVertexFormat*& vtxFmtOut)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    vtxFmtOut = ctx.newVertexFormat(2, VtxVmt);
    pipeOut = ctx.newShaderPipeline(VS, FS,  ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                    vtxFmtOut, boo::BlendFactor::One,
                                    boo::BlendFactor::Zero, boo::Primitive::TriStrips, false, false, false);
    return new CXRayBlurFilterD3DDataBindingFactory;
}

}
