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
"    float4x4 uv4;\n"
"    float4x4 uv5;\n"
"    float4x4 uv6;\n"
"    float4x4 uv7;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float2 uv0 : UV0;\n"
"    float2 uv1 : UV1;\n"
"    float2 uv2 : UV2;\n"
"    float2 uv3 : UV3;\n"
"    float2 uv4 : UV4;\n"
"    float2 uv5 : UV5;\n"
"    float2 uv6 : UV6;\n"
"    float2 uv7 : UV7;\n"
"};\n"
"\n"
"VertToFrag main(in VertData v)\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.uv0 = mul(uv0, float4(v.uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv0.y = 1.0 - vtf.uv0.y;\n"
"    vtf.uv1 = mul(uv1, float4(v.uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv1.y = 1.0 - vtf.uv1.y;\n"
"    vtf.uv2 = mul(uv2, float4(v.uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv2.y = 1.0 - vtf.uv2.y;\n"
"    vtf.uv3 = mul(uv3, float4(v.uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv3.y = 1.0 - vtf.uv3.y;\n"
"    vtf.uv4 = mul(uv4, float4(v.uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv4.y = 1.0 - vtf.uv4.y;\n"
"    vtf.uv5 = mul(uv5, float4(v.uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv5.y = 1.0 - vtf.uv5.y;\n"
"    vtf.uv6 = mul(uv6, float4(v.uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv6.y = 1.0 - vtf.uv6.y;\n"
"    vtf.uv7 = mul(uv7, float4(v.uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv7.y = 1.0 - vtf.uv7.y;\n"
"    vtf.position = float4(v.posIn.xyz, 1.0);\n"
"    return vtf;\n"
"}\n";

static const char* FS =
"Texture2D sceneTex : register(t0);\n"
"Texture2D paletteTex : register(t1);\n"
"SamplerState samp : register(s2);\n"
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float2 uv0 : UV0;\n"
"    float2 uv1 : UV1;\n"
"    float2 uv2 : UV2;\n"
"    float2 uv3 : UV3;\n"
"    float2 uv4 : UV4;\n"
"    float2 uv5 : UV5;\n"
"    float2 uv6 : UV6;\n"
"    float2 uv7 : UV7;\n"
"};\n"
"\n"
"static const float4 kRGBToYPrime = float4(0.299, 0.587, 0.114, 0.0);\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    float4 colorSample = paletteTex.Sample(samp, float2(dot(sceneTex.Sample(samp, vtf.uv0), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;\n"
"    colorSample += paletteTex.Sample(samp, float2(dot(sceneTex.Sample(samp, vtf.uv1), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;\n"
"    colorSample += paletteTex.Sample(samp, float2(dot(sceneTex.Sample(samp, vtf.uv2), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;\n"
"    colorSample += paletteTex.Sample(samp, float2(dot(sceneTex.Sample(samp, vtf.uv3), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;\n"
"    colorSample += paletteTex.Sample(samp, float2(dot(sceneTex.Sample(samp, vtf.uv4), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;\n"
"    colorSample += paletteTex.Sample(samp, float2(dot(sceneTex.Sample(samp, vtf.uv5), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;\n"
"    colorSample += paletteTex.Sample(samp, float2(dot(sceneTex.Sample(samp, vtf.uv6), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;\n"
"    colorSample += paletteTex.Sample(samp, float2(dot(sceneTex.Sample(samp, vtf.uv7), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;\n"
"    return colorSample;\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CXRayBlurFilter)

static boo::ObjToken<boo::IVertexFormat> s_VtxFmt;
static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;

struct CXRayBlurFilterD3DDataBindingFactory : TShader<CXRayBlurFilter>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding>
    BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                           CXRayBlurFilter& filter)
    {
        boo::ID3DDataFactory::Context& cctx = static_cast<boo::ID3DDataFactory::Context&>(ctx);

        boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {filter.m_uniBuf.get()};
        boo::ObjToken<boo::ITexture> texs[] = {CGraphics::g_SpareTexture.get(), filter.m_booTex.get()};
        return cctx.newShaderDataBinding(s_Pipeline, s_VtxFmt,
                                         filter.m_vbo.get(), nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 2, texs, nullptr, nullptr);
    }
};

TShader<CXRayBlurFilter>::IDataBindingFactory* CXRayBlurFilter::Initialize(boo::ID3DDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    s_VtxFmt = ctx.newVertexFormat(2, VtxVmt);
    s_Pipeline = ctx.newShaderPipeline(VS, FS,  nullptr, nullptr, nullptr,
                                       s_VtxFmt, boo::BlendFactor::One,
                                       boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                       boo::ZTest::None, false, true, false, boo::CullMode::None);
    return new CXRayBlurFilterD3DDataBindingFactory;
}

template <>
void CXRayBlurFilter::Shutdown<boo::ID3DDataFactory>()
{
    s_VtxFmt.reset();
    s_Pipeline.reset();
}
}
