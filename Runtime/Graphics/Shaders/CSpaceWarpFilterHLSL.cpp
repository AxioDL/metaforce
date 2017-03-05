#include "CSpaceWarpFilter.hpp"

namespace urde
{

static const char* VS =
"struct VertData\n"
"{\n"
"    float4 posIn : POSITION;\n"
"    float4 uvIn : UV;\n"
"};\n"
"cbuffer SpaceWarpUniform : register(b0)\n"
"{\n"
"    float4x4 mainMtx;\n"
"    float4x4 indMtx;\n"
"    float4 strength;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float2 sceneUv : SCENEUV;\n"
"    float2 indUv : INDV;\n"
"    float2 strength : STRENGTH;\n"
"};\n"
"\n"
"VertToFrag main(in VertData v)\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.position = mul(mainMtx, float4(v.posIn.xy, 0.0, 1.0));\n"
"    vtf.sceneUv = vtf.position.xy * float2(0.5, 0.5) + float2(0.5, 0.5);\n"
"    vtf.sceneUv.y = -vtf.sceneUv.y;\n"
"    vtf.indUv = mul(float3x3(indMtx[0].xyz, indMtx[1].xyz, indMtx[2].xyz), float3(v.uvIn.xy, 1.0)).xy;\n"
"    vtf.indUv.y = -vtf.indUv.y;\n"
"    vtf.strength = strength.xy;\n"
"    return vtf;\n"
"}\n";

static const char* FS =
"Texture2D sceneTex : register(t0);\n"
"Texture2D indTex : register(t1);\n"
"SamplerState samp : register(s0);\n"
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float2 sceneUv : SCENEUV;\n"
"    float2 indUv : INDV;\n"
"    float2 strength : STRENGTH;\n"
"};\n"
"\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    return sceneTex.Sample(samp, vtf.sceneUv + (indTex.Sample(samp, vtf.indUv).xy * float2(2.0, 2.0) - float2(1.0 - 1.0 / 256.0, 1.0 - 1.0 / 256.0)) * vtf.strength.xy);\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CSpaceWarpFilter)

static boo::IVertexFormat* s_VtxFmt = nullptr;
static boo::IShaderPipeline* s_Pipeline = nullptr;

struct CSpaceWarpFilterD3DDataBindingFactory : TShader<CSpaceWarpFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    CSpaceWarpFilter& filter)
    {
        boo::ID3DDataFactory::Context& cctx = static_cast<boo::ID3DDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::ITexture* texs[] = {CGraphics::g_SpareTexture, filter.m_warpTex};
        return cctx.newShaderDataBinding(s_Pipeline, s_VtxFmt,
                                         filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 2, texs);
    }
};

TShader<CSpaceWarpFilter>::IDataBindingFactory* CSpaceWarpFilter::Initialize(boo::ID3DDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    s_VtxFmt = ctx.newVertexFormat(2, VtxVmt);
    s_Pipeline = ctx.newShaderPipeline(VS, FS, nullptr, nullptr, nullptr,
                                       s_VtxFmt, boo::BlendFactor::One,
                                       boo::BlendFactor::Zero, boo::Primitive::TriStrips, false, false, false);
    return new CSpaceWarpFilterD3DDataBindingFactory;
}

}
