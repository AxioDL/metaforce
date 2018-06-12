#include "CXRayBlurFilter.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "GameGlobalObjects.hpp"

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
"struct XRayBlurUniform\n"
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
"    float4 position [[ position ]];\n"
"    float2 uv0;\n"
"    float2 uv1;\n"
"    float2 uv2;\n"
"    float2 uv3;\n"
"    float2 uv4;\n"
"    float2 uv5;\n"
"    float2 uv6;\n"
"    float2 uv7;\n"
"};\n"
"\n"
"vertex VertToFrag vmain(VertData v [[ stage_in ]], constant XRayBlurUniform& xbu [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.uv0 = (xbu.uv0 * float4(v.uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv0.y = 1.0 - vtf.uv0.y;\n"
"    vtf.uv1 = (xbu.uv1 * float4(v.uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv1.y = 1.0 - vtf.uv1.y;\n"
"    vtf.uv2 = (xbu.uv2 * float4(v.uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv2.y = 1.0 - vtf.uv2.y;\n"
"    vtf.uv3 = (xbu.uv3 * float4(v.uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv3.y = 1.0 - vtf.uv3.y;\n"
"    vtf.uv4 = (xbu.uv4 * float4(v.uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv4.y = 1.0 - vtf.uv4.y;\n"
"    vtf.uv5 = (xbu.uv5 * float4(v.uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv5.y = 1.0 - vtf.uv5.y;\n"
"    vtf.uv6 = (xbu.uv6 * float4(v.uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv6.y = 1.0 - vtf.uv6.y;\n"
"    vtf.uv7 = (xbu.uv7 * float4(v.uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv7.y = 1.0 - vtf.uv7.y;\n"
"    vtf.position = float4(v.posIn.xyz, 1.0);\n"
"    return vtf;\n"
"}\n";

static const char* FS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float2 uv0;\n"
"    float2 uv1;\n"
"    float2 uv2;\n"
"    float2 uv3;\n"
"    float2 uv4;\n"
"    float2 uv5;\n"
"    float2 uv6;\n"
"    float2 uv7;\n"
"};\n"
"\n"
"constant float4 kRGBToYPrime = float4(0.299, 0.587, 0.114, 0.0);\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]],\n"
"                      sampler samp [[ sampler(3) ]],\n"
"                      texture2d<float> sceneTex [[ texture(0) ]],\n"
"                      texture2d<float> paletteTex [[ texture(1) ]])\n"
"{\n"
"    float4 colorSample = paletteTex.sample(samp, float2(dot(sceneTex.sample(samp, vtf.uv0), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;\n"
"    colorSample += paletteTex.sample(samp, float2(dot(sceneTex.sample(samp, vtf.uv1), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;\n"
"    colorSample += paletteTex.sample(samp, float2(dot(sceneTex.sample(samp, vtf.uv2), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;\n"
"    colorSample += paletteTex.sample(samp, float2(dot(sceneTex.sample(samp, vtf.uv3), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;\n"
"    colorSample += paletteTex.sample(samp, float2(dot(sceneTex.sample(samp, vtf.uv4), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;\n"
"    colorSample += paletteTex.sample(samp, float2(dot(sceneTex.sample(samp, vtf.uv5), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;\n"
"    colorSample += paletteTex.sample(samp, float2(dot(sceneTex.sample(samp, vtf.uv6), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;\n"
"    colorSample += paletteTex.sample(samp, float2(dot(sceneTex.sample(samp, vtf.uv7), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;\n"
"    return colorSample;\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CXRayBlurFilter)

static boo::ObjToken<boo::IVertexFormat> s_VtxFmt;
static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;

struct CXRayBlurFilterMetalDataBindingFactory : TShader<CXRayBlurFilter>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                  CXRayBlurFilter& filter)
    {
        boo::MetalDataFactory::Context& cctx = static_cast<boo::MetalDataFactory::Context&>(ctx);

        boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {filter.m_uniBuf.get()};
        boo::ObjToken<boo::ITexture> texs[] = {CGraphics::g_SpareTexture.get(), filter.m_booTex};
        return cctx.newShaderDataBinding(s_Pipeline, s_VtxFmt,
                                         filter.m_vbo.get(), nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 2, texs, nullptr, nullptr);
    }
};

TShader<CXRayBlurFilter>::IDataBindingFactory* CXRayBlurFilter::Initialize(boo::MetalDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    s_VtxFmt = ctx.newVertexFormat(2, VtxVmt);
    s_Pipeline = ctx.newShaderPipeline(VS, FS, nullptr, nullptr,
                                       s_VtxFmt, boo::BlendFactor::One,
                                       boo::BlendFactor::Zero, boo::Primitive::TriStrips, boo::ZTest::None, false,
                                       true, false, boo::CullMode::None);
    return new CXRayBlurFilterMetalDataBindingFactory;
}

template <>
void CXRayBlurFilter::Shutdown<boo::MetalDataFactory>()
{
    s_VtxFmt.reset();
    s_Pipeline.reset();
}

}
