#include "CThermalHotFilter.hpp"
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
"cbuffer ThermalHotUniform : register(b0)\n"
"{\n"
"    float4 colorReg0;\n"
"    float4 colorReg1;\n"
"    float4 colorReg2;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float2 sceneUv : UV;\n"
"};\n"
"\n"
"VertToFrag main(in VertData v)\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.sceneUv = v.uvIn.xy;\n"
"    vtf.sceneUv.y = -vtf.sceneUv.y;\n"
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
"    float2 sceneUv : UV;\n"
"};\n"
"\n"
"static const float4 kRGBToYPrime = float4(0.299, 0.587, 0.114, 0.0);\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    float sceneSample = dot(sceneTex.Sample(samp, vtf.sceneUv), kRGBToYPrime);\n"
"    float4 colorSample = paletteTex.Sample(samp, float2(sceneSample / 17.0, 0.5));\n"
"    return colorSample * sceneSample;\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CThermalHotFilter)

static boo::IVertexFormat* s_VtxFmt = nullptr;
static boo::IShaderPipeline* s_Pipeline = nullptr;

struct CThermalHotFilterD3DDataBindingFactory : TShader<CThermalHotFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    CThermalHotFilter& filter)
    {
        boo::ID3DDataFactory::Context& cctx = static_cast<boo::ID3DDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::ITexture* texs[] = {CGraphics::g_SpareTexture, g_Renderer->GetThermoPalette()};
        return cctx.newShaderDataBinding(s_Pipeline, s_VtxFmt,
                                         filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 2, texs);
    }
};

TShader<CThermalHotFilter>::IDataBindingFactory* CThermalHotFilter::Initialize(boo::ID3DDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    s_VtxFmt = ctx.newVertexFormat(2, VtxVmt);
    s_Pipeline = ctx.newShaderPipeline(VS, FS, ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(), ComPtr<ID3DBlob>(),
                                       s_VtxFmt, boo::BlendFactor::DstAlpha, boo::BlendFactor::InvDstAlpha,
                                       boo::Primitive::TriStrips, false, false, false);
    return new CThermalHotFilterD3DDataBindingFactory;
}

}
