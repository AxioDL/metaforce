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
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float2 uv0;\n"
"    float2 uv1;\n"
"    float2 uv2;\n"
"    float2 uv3;\n"
"};\n"
"\n"
"vertex VertToFrag vmain(VertData v [[ stage_in ]], constant XRayBlurUniform& xbu [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.uv0 = (xbu.uv0 * float4(v.uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv0.y = -vtf.uv0.y;\n"
"    vtf.uv1 = (xbu.uv1 * float4(v.uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv1.y = -vtf.uv1.y;\n"
"    vtf.uv2 = (xbu.uv2 * float4(v.uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv2.y = -vtf.uv2.y;\n"
"    vtf.uv3 = (xbu.uv3 * float4(v.uvIn.xy, 0.0, 1.0)).xy;\n"
"    vtf.uv3.y = -vtf.uv3.y;\n"
"    vtf.position = float4(v.posIn.xyz, 1.0);\n"
"    return vtf;\n"
"}\n";

static const char* FS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"constexpr sampler samp(address::repeat, filter::linear);\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float2 uv0;\n"
"    float2 uv1;\n"
"    float2 uv2;\n"
"    float2 uv3;\n"
"};\n"
"\n"
"constant float4 kRGBToYPrime = float4(0.299, 0.587, 0.114, 0.0);\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]],\n"
"                      texture2d<float> sceneTex [[ texture(0) ]],\n"
"                      texture2d<float> paletteTex [[ texture(1) ]])\n"
"{\n"
"    float4 colorSample = paletteTex.sample(samp, float2(dot(sceneTex.sample(samp, vtf.uv0), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.25;\n"
"    colorSample += paletteTex.sample(samp, float2(dot(sceneTex.sample(samp, vtf.uv1), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.25;\n"
"    colorSample += paletteTex.sample(samp, float2(dot(sceneTex.sample(samp, vtf.uv2), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.25;\n"
"    colorSample += paletteTex.sample(samp, float2(dot(sceneTex.sample(samp, vtf.uv3), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.25;\n"
"    return colorSample;\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CXRayBlurFilter)

static boo::IVertexFormat* s_VtxFmt = nullptr;
static boo::IShaderPipeline* s_Pipeline = nullptr;

struct CXRayBlurFilterMetalDataBindingFactory : TShader<CXRayBlurFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    CXRayBlurFilter& filter)
    {
        boo::MetalDataFactory::Context& cctx = static_cast<boo::MetalDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::ITexture* texs[] = {CGraphics::g_SpareTexture, filter.m_booTex};
        return cctx.newShaderDataBinding(s_Pipeline, s_VtxFmt,
                                         filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 2, texs);
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
    s_Pipeline = ctx.newShaderPipeline(VS, FS, s_VtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::One,
                                       boo::BlendFactor::Zero, boo::Primitive::TriStrips, false, false,
                                       boo::CullMode::None);
    return new CXRayBlurFilterMetalDataBindingFactory;
}

}
