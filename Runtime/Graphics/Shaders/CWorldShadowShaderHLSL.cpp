#include "CWorldShadowShader.hpp"

namespace urde
{

static const char* VS =
"struct VertData\n"
"{\n"
"    float4 posIn : POSITION;\n"
"};\n"
"\n"
"cbuffer ColoredQuadUniform : register(b0)\n"
"{\n"
"    float4x4 xf;\n"
"    float4 color;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"};\n"
"\n"
"VertToFrag main(in VertData v)\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = color;\n"
"    vtf.position = mul(xf, float4(v.posIn.xyz, 1.0));\n"
"    return vtf;\n"
"}\n";

static const char* FS =
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"};\n"
"\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    return vtf.color;\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CWorldShadowShader)

static boo::IVertexFormat* s_VtxFmt = nullptr;
static boo::IShaderPipeline* s_Pipeline = nullptr;
static boo::IShaderPipeline* s_ZPipeline = nullptr;

struct CWorldShadowShaderD3DDataBindingFactory : TShader<CWorldShadowShader>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    CWorldShadowShader& filter)
    {
        boo::ID3DDataFactory::Context& cctx = static_cast<boo::ID3DDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        filter.m_dataBind = cctx.newShaderDataBinding(s_Pipeline,
            nullptr, nullptr, nullptr, s_VtxFmt,
            filter.m_vbo, nullptr, nullptr, 1, bufs,
            nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr);
        filter.m_zDataBind = cctx.newShaderDataBinding(s_ZPipeline,
            nullptr, nullptr, nullptr, s_VtxFmt,
            filter.m_vbo, nullptr, nullptr, 1, bufs,
            nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr);
        filter._buildTex(ctx);
        return nullptr;
    }
};

TShader<CWorldShadowShader>::IDataBindingFactory*
CWorldShadowShader::Initialize(boo::ID3DDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4}
    };
    s_VtxFmt = ctx.newVertexFormat(1, VtxVmt);
    s_Pipeline = ctx.newShaderPipeline(VS, FS, s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                       boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                       boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_ZPipeline = ctx.newShaderPipeline(VS, FS, s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                        boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                        boo::ZTest::LEqual, true, true, false, boo::CullMode::None);
    return new CWorldShadowShaderD3DDataBindingFactory;
}

}

