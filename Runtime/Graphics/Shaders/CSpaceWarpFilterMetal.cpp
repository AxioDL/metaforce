#include "CSpaceWarpFilter.hpp"

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
"struct SpaceWarpUniform\n"
"{\n"
"    float4x4 mainMtx;\n"
"    float4x4 indMtx;\n"
"    float4 strength;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float2 sceneUv;\n"
"    float2 indUv;\n"
"    float2 strength;\n"
"};\n"
"\n"
"vertex VertToFrag vmain(VertData v [[ stage_in ]], constant SpaceWarpUniform& swu [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.position = swu.mainMtx * float4(v.posIn.xy, 0.0, 1.0);\n"
"    vtf.sceneUv = vtf.position.xy * float2(0.5) + float2(0.5);\n"
"    vtf.sceneUv.y = -vtf.sceneUv.y;\n"
"    vtf.indUv = (float3x3(swu.indMtx[0].xyz, swu.indMtx[1].xyz, swu.indMtx[2].xyz) * float3(v.uvIn.xy, 1.0)).xy;\n"
"    vtf.indUv.y = -vtf.indUv.y;\n"
"    vtf.strength = swu.strength.xy;\n"
"    return vtf;\n"
"}\n";

static const char* FS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"constexpr sampler samp(address::repeat, filter::linear);\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float2 sceneUv;\n"
"    float2 indUv;\n"
"    float2 strength;\n"
"};\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]], texture2d<float> sceneTex [[ texture(0) ]], texture2d<float> indTex [[ texture(1) ]])\n"
"{\n"
"    return sceneTex.sample(samp, vtf.sceneUv + (indTex.sample(samp, vtf.indUv).xy * float2(2.0) - float2(1.0 - 1.0 / 256.0)) * vtf.strength.xy);\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CSpaceWarpFilter)

static boo::ObjToken<boo::IVertexFormat> s_VtxFmt;
static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;

struct CSpaceWarpFilterMetalDataBindingFactory : TShader<CSpaceWarpFilter>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                  CSpaceWarpFilter& filter)
    {
        boo::MetalDataFactory::Context& cctx = static_cast<boo::MetalDataFactory::Context&>(ctx);

        boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {filter.m_uniBuf.get()};
        boo::ObjToken<boo::ITexture> texs[] = {CGraphics::g_SpareTexture.get(), filter.m_warpTex};
        return cctx.newShaderDataBinding(s_Pipeline, s_VtxFmt,
                                         filter.m_vbo.get(), nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 2, texs, nullptr, nullptr);
    }
};

TShader<CSpaceWarpFilter>::IDataBindingFactory* CSpaceWarpFilter::Initialize(boo::MetalDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    s_VtxFmt = ctx.newVertexFormat(2, VtxVmt);
    s_Pipeline = ctx.newShaderPipeline(VS, FS, nullptr, nullptr,
                                       s_VtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::One,
                                       boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                       boo::ZTest::None, false, true, false, boo::CullMode::None);
    return new CSpaceWarpFilterMetalDataBindingFactory;
}

template <>
void CSpaceWarpFilter::Shutdown<boo::MetalDataFactory>()
{
    s_VtxFmt.reset();
    s_Pipeline.reset();
}

}
