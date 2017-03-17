#include "CFogVolumeFilter.hpp"
#include "TShader.hpp"
#include "GameGlobalObjects.hpp"
#include "Graphics/CBooRenderer.hpp"

namespace urde
{

static const char* VS =
"struct VertData\n"
"{\n"
"    float4 posIn : POSITION;\n"
"    float2 uvIn : UV;\n"
"};\n"
"\n"
"cbuffer FogVolumeFilterUniform : register(b0)\n"
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
"    vtf.position = float4(v.posIn.x, -v.posIn.y, 0.0, 1.0);\n"
"    vtf.color = color;\n"
"    vtf.uv = v.uvIn;\n"
"    return vtf;\n"
"}\n";

static const char* FS1Way =
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float2 uv : UV;\n"
"};\n"
"\n"
"Texture2D zFrontfaceTex : register(t0);\n"
"Texture2D zBackfaceTex : register(t1);\n"
"Texture2D zLinearizer : register(t2);\n"
"SamplerState samp : register(s0);\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    float y;\n"
"    const float linScale = 65535.0 / 65536.0 * 256.0;\n"
"    float x = modf((1.0 - zFrontfaceTex.Sample(samp, vtf.uv).r) * linScale, y);\n"
"    const float uvBias = 0.5 / 256.0;\n"
"    float alpha = zLinearizer.Sample(samp, float2(x * 255.0 / 256.0 + uvBias, y / 256.0 + uvBias)).r * 10.0;\n"
"    return vtf.color * alpha;\n"
"}\n";

static const char* FS2Way =
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float2 uv : UV;\n"
"};\n"
"\n"
"Texture2D zFrontfaceTex : register(t0);\n"
"Texture2D zBackfaceTex : register(t1);\n"
"Texture2D zLinearizer : register(t2);\n"
"SamplerState samp : register(s0);\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    float frontY;\n"
"    float backY;\n"
"    const float linScale = 65535.0 / 65536.0 * 256.0;\n"
"    float frontX = modf((1.0 - zFrontfaceTex.Sample(samp, vtf.uv).r) * linScale, frontY);\n"
"    float backX = modf((1.0 - zBackfaceTex.Sample(samp, vtf.uv).r) * linScale, backY);\n"
"    const float uvBias = 0.5 / 256.0;\n"
"    float frontLin = zLinearizer.Sample(samp, float2(frontX * 255.0 / 256.0 + uvBias, frontY / 256.0 + uvBias)).r;\n"
"    float backLin = zLinearizer.Sample(samp, float2(backX * 255.0 / 256.0 + uvBias, backY / 256.0 + uvBias)).r;\n"
"    return float4(vtf.color.rgb, (frontLin - backLin) * 10.0);\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CFogVolumeFilter)

static boo::IVertexFormat* s_VtxFmt = nullptr;
static boo::IShaderPipeline* s_1WayPipeline = nullptr;
static boo::IShaderPipeline* s_2WayPipeline = nullptr;

struct CFogVolumeFilterD3DDataBindingFactory : TShader<CFogVolumeFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    CFogVolumeFilter& filter)
    {
        boo::ID3DDataFactory::Context& cctx = static_cast<boo::ID3DDataFactory::Context&>(ctx);
        boo::ITexture* texs[] = { CGraphics::g_SpareTexture, CGraphics::g_SpareTexture,
                                  g_Renderer->GetFogRampTex() };
        int bindIdxs[] = {0, 1, 0};
        bool bindDepth[] = {true, true, false};
        boo::IGraphicsBuffer* ubufs[] = {filter.m_uniBuf};

        filter.m_dataBind1Way = cctx.newShaderDataBinding(s_1WayPipeline, s_VtxFmt,
            filter.m_vbo, nullptr, nullptr, 1, ubufs,
            nullptr, nullptr, nullptr, 3, texs, bindIdxs, bindDepth);
        filter.m_dataBind2Way = cctx.newShaderDataBinding(s_2WayPipeline, s_VtxFmt,
            filter.m_vbo, nullptr, nullptr, 1, ubufs,
            nullptr, nullptr, nullptr, 3, texs, bindIdxs, bindDepth);
        return filter.m_dataBind1Way;
    }
};

TShader<CFogVolumeFilter>::IDataBindingFactory*
CFogVolumeFilter::Initialize(boo::ID3DDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    s_VtxFmt = ctx.newVertexFormat(2, VtxVmt);
    s_1WayPipeline = ctx.newShaderPipeline(VS, FS1Way, nullptr, nullptr, nullptr, s_VtxFmt, boo::BlendFactor::DstAlpha,
                                           boo::BlendFactor::One, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_2WayPipeline = ctx.newShaderPipeline(VS, FS2Way, nullptr, nullptr, nullptr, s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                           boo::BlendFactor::One, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, false, boo::CullMode::None);
    return new CFogVolumeFilterD3DDataBindingFactory;
}

}
