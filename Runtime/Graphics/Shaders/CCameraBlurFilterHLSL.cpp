#include "CCameraBlurFilter.hpp"
#include "Graphics/CBooRenderer.hpp"

namespace urde
{

static const char* VS =
"struct VertData\n"
"{\n"
"    float4 posIn : POSITION;\n"
"    float4 uvIn : UV;\n"
"};\n"
"\n"
"cbuffer CameraBlurUniform : register(b0)\n"
"{\n"
"    float4 uv0;\n"
"    float4 uv1;\n"
"    float4 uv2;\n"
"    float4 uv3;\n"
"    float4 uv4;\n"
"    float4 uv5;\n"
"    float opacity;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float2 uvReg : UV6;\n"
"    float2 uv0 : UV0;\n"
"    float2 uv1 : UV1;\n"
"    float2 uv2 : UV2;\n"
"    float2 uv3 : UV3;\n"
"    float2 uv4 : UV4;\n"
"    float2 uv5 : UV5;\n"
"    float opacity : OPACITY;\n"
"};\n"
"\n"
"VertToFrag main(in VertData v)\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.uvReg = v.uvIn.xy;\n"
"    vtf.uvReg.y = -vtf.uvReg.y;\n"
"    vtf.uv0 = uv0.xy + v.uvIn.xy;\n"
"    vtf.uv0.y = -vtf.uv0.y;\n"
"    vtf.uv1 = uv1.xy + v.uvIn.xy;\n"
"    vtf.uv1.y = -vtf.uv1.y;\n"
"    vtf.uv2 = uv2.xy + v.uvIn.xy;\n"
"    vtf.uv2.y = -vtf.uv2.y;\n"
"    vtf.uv3 = uv3.xy + v.uvIn.xy;\n"
"    vtf.uv3.y = -vtf.uv3.y;\n"
"    vtf.uv4 = uv4.xy + v.uvIn.xy;\n"
"    vtf.uv4.y = -vtf.uv4.y;\n"
"    vtf.uv5 = uv5.xy + v.uvIn.xy;\n"
"    vtf.uv5.y = -vtf.uv5.y;\n"
"    vtf.opacity = opacity;\n"
"    vtf.position = float4(v.posIn.xyz, 1.0);\n"
"    return vtf;\n"
"}\n";

static const char* FS =
"Texture2D sceneTex : register(t0);\n"
"SamplerState samp : register(s0);\n"
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float2 uvReg : UV6;\n"
"    float2 uv0 : UV0;\n"
"    float2 uv1 : UV1;\n"
"    float2 uv2 : UV2;\n"
"    float2 uv3 : UV3;\n"
"    float2 uv4 : UV4;\n"
"    float2 uv5 : UV5;\n"
"    float opacity : OPACITY;\n"
"};\n"
"\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    float4 colorSample = sceneTex.Sample(samp, vtf.uvReg) * 0.14285715;\n"
"    colorSample += sceneTex.Sample(samp, vtf.uv0) * 0.14285715;\n"
"    colorSample += sceneTex.Sample(samp, vtf.uv1) * 0.14285715;\n"
"    colorSample += sceneTex.Sample(samp, vtf.uv2) * 0.14285715;\n"
"    colorSample += sceneTex.Sample(samp, vtf.uv3) * 0.14285715;\n"
"    colorSample += sceneTex.Sample(samp, vtf.uv4) * 0.14285715;\n"
"    colorSample += sceneTex.Sample(samp, vtf.uv5) * 0.14285715;\n"
"    return float4(colorSample.rgb, vtf.opacity);\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CCameraBlurFilter)

static boo::ObjToken<boo::IVertexFormat> s_VtxFmt;
static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;

struct CCameraBlurFilterD3DDataBindingFactory : TShader<CCameraBlurFilter>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding>
    BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                           CCameraBlurFilter& filter)
    {
        boo::ID3DDataFactory::Context& cctx = static_cast<boo::ID3DDataFactory::Context&>(ctx);

        boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {filter.m_uniBuf.get()};
        boo::ObjToken<boo::ITexture> texs[] = {CGraphics::g_SpareTexture.get()};
        return cctx.newShaderDataBinding(s_Pipeline, s_VtxFmt,
                                         filter.m_vbo.get(), nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 1, texs, nullptr, nullptr);
    }
};

TShader<CCameraBlurFilter>::IDataBindingFactory* CCameraBlurFilter::Initialize(boo::ID3DDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    s_VtxFmt = ctx.newVertexFormat(2, VtxVmt);
    s_Pipeline = ctx.newShaderPipeline(VS, FS, nullptr, nullptr, nullptr,
                                       s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                       boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                       boo::ZTest::None, false, true, false, boo::CullMode::None);
    return new CCameraBlurFilterD3DDataBindingFactory;
}

template <>
void CCameraBlurFilter::Shutdown<boo::ID3DDataFactory>()
{
    s_VtxFmt.reset();
    s_Pipeline.reset();
}

}
