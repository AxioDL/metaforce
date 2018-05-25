#include "CRadarPaintShader.hpp"
#include "TShader.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{

static const char* VS =
"struct VertData\n"
"{\n"
"    float4 posIn[4] : POSITION;\n"
"    float4 uvIn[4] : UV;\n"
"    float4 colorIn : COLOR;\n"
"};\n"
"\n"
"cbuffer RadarPaintUniform : register(b0)\n"
"{\n"
"    float4x4 xf;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float2 uv : UV;\n"
"};\n"
"\n"
"VertToFrag main(in VertData v, in uint vertId : SV_VertexID)\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = v.colorIn;\n"
"    vtf.uv = v.uvIn[vertId].xy;\n"
"    vtf.position = mul(xf, float4(v.posIn[vertId].xyz, 1.0));\n"
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
"SamplerState samp : register(s0);\n"
"Texture2D tex : register(t0);\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    return vtf.color * tex.Sample(samp, vtf.uv);\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CRadarPaintShader)

static boo::ObjToken<boo::IVertexFormat> s_VtxFmt;
static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;

struct CRadarPaintShaderD3DDataBindingFactory : TShader<CRadarPaintShader>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding>
    BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                           CRadarPaintShader& filter)
    {
        boo::D3DDataFactory::Context& cctx = static_cast<boo::D3DDataFactory::Context&>(ctx);

        boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {filter.m_uniBuf.get()};
        boo::ObjToken<boo::ITexture> texs[] = {filter.m_tex->GetBooTexture()};
        return cctx.newShaderDataBinding(s_Pipeline, s_VtxFmt,
                                         nullptr, filter.m_vbo.get(), nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 1, texs, nullptr, nullptr);
    }
};

TShader<CRadarPaintShader>::IDataBindingFactory*
CRadarPaintShader::Initialize(boo::D3DDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4, 0},
        {nullptr, nullptr, boo::VertexSemantic::Position4, 1},
        {nullptr, nullptr, boo::VertexSemantic::Position4, 2},
        {nullptr, nullptr, boo::VertexSemantic::Position4, 3},
        {nullptr, nullptr, boo::VertexSemantic::UV4, 0},
        {nullptr, nullptr, boo::VertexSemantic::UV4, 1},
        {nullptr, nullptr, boo::VertexSemantic::UV4, 2},
        {nullptr, nullptr, boo::VertexSemantic::UV4, 3},
        {nullptr, nullptr, boo::VertexSemantic::Color}
    };
    s_VtxFmt = ctx.newVertexFormat(9, VtxVmt);
    s_Pipeline = ctx.newShaderPipeline(VS, FS, nullptr, nullptr, nullptr,
                                       s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                       boo::BlendFactor::One, boo::Primitive::TriStrips,
                                       boo::ZTest::None, false, true, false, boo::CullMode::None);
    return new CRadarPaintShaderD3DDataBindingFactory;
}

template <>
void CRadarPaintShader::Shutdown<boo::D3DDataFactory>()
{
    s_VtxFmt.reset();
    s_Pipeline.reset();
}
}
