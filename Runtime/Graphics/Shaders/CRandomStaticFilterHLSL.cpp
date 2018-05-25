#include "CRandomStaticFilter.hpp"
#include "TMultiBlendShader.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "GameGlobalObjects.hpp"

namespace urde
{

static const char* VS =
"struct VertData\n"
"{\n"
"    float4 posIn : POSITION;\n"
"    float2 uvIn : UV;\n"
"};\n"
"\n"
"cbuffer RandomStaticUniform : register(b0)\n"
"{\n"
"    float4 color;\n"
"    float randOff;\n"
"    float discardThres;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 pos : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float2 uv : UV;\n"
"    float randOff : RANDOFF;\n"
"    float discardThres : DISCARDTHRES;\n"
"};\n"
"\n"
"VertToFrag main(in VertData v)\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = color;\n"
"    vtf.uv = v.uvIn.xy;\n"
"    vtf.randOff = randOff;\n"
"    vtf.discardThres = discardThres;\n"
"    vtf.pos = float4(v.posIn.xyz, 1.0);\n"
"    return vtf;\n"
"}\n";

static const char* FS =
"struct VertToFrag\n"
"{\n"
"    float4 pos : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float2 uv : UV;\n"
"    float randOff : RANDOFF;\n"
"    float discardThres : DISCARDTHRES;\n"
"};\n"
"\n"
"static int3 Lookup8BPP(float2 uv, float randOff)\n"
"{\n"
"    float bx;\n"
"    float rx = modf(uv.x / 8.0, bx) * 8.0;\n"
"    float by;\n"
"    float ry = modf(uv.y / 4.0, by) * 4.0;\n"
"    float bidx = by * 80.0 + bx;\n"
"    float addr = bidx * 32.0 + ry * 8.0 + rx + randOff;\n"
"    float y;\n"
"    float x = modf(addr / 1024.0, y) * 1024.0;\n"
"    return int3(x, y, 0);\n"
"}\n"
"\n"
"Texture2D tex : register(t0);\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    float4 colorOut = tex.Load(Lookup8BPP(vtf.uv, vtf.randOff)) * vtf.color;\n"
"    colorOut.a = vtf.color.a;\n"
"    return colorOut;\n"
"}\n";

static const char* FSCookieCutter =
"struct VertToFrag\n"
"{\n"
"    float4 pos : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float2 uv : UV;\n"
"    float randOff : RANDOFF;\n"
"    float discardThres : DISCARDTHRES;\n"
"};\n"
"\n"
"static int3 Lookup8BPP(float2 uv, float randOff)\n"
"{\n"
"    float bx;\n"
"    float rx = modf(uv.x / 8.0, bx) * 8.0;\n"
"    float by;\n"
"    float ry = modf(uv.y / 4.0, by) * 4.0;\n"
"    float bidx = by * 80.0 + bx;\n"
"    float addr = bidx * 32.0 + ry * 8.0 + rx + randOff;\n"
"    float y;\n"
"    float x = modf(addr / 1024.0, y) * 1024.0;\n"
"    return int3(x, y, 0);\n"
"}\n"
"\n"
"Texture2D tex : register(t0);\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    float4 colorOut = tex.Load(Lookup8BPP(vtf.uv, vtf.randOff)) * vtf.color;\n"
"    if (colorOut.a < vtf.discardThres)\n"
"        discard;\n"
"    return colorOut;\n"
"}\n";

URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CRandomStaticFilter)

static boo::ObjToken<boo::IVertexFormat> s_VtxFmt;
static boo::ObjToken<boo::IShaderPipeline> s_AlphaPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AddPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_MultPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_CookieCutterPipeline;

static boo::ObjToken<boo::IShaderPipeline> SelectPipeline(EFilterType type)
{
    switch (type)
    {
    case EFilterType::Blend:
        return s_AlphaPipeline;
    case EFilterType::Add:
        return s_AddPipeline;
    case EFilterType::Multiply:
        return s_MultPipeline;
    default:
        return {};
    }
}

struct CRandomStaticFilterD3DDataBindingFactory : TMultiBlendShader<CRandomStaticFilter>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding>
    BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                           EFilterType type, CRandomStaticFilter& filter)
    {
        boo::D3DDataFactory::Context& cctx = static_cast<boo::D3DDataFactory::Context&>(ctx);

        boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {filter.m_uniBuf.get()};
        boo::ObjToken<boo::ITexture> texs[] = {g_Renderer->GetRandomStaticEntropyTex().get()};
        return cctx.newShaderDataBinding(filter.m_cookieCutter ? s_CookieCutterPipeline : SelectPipeline(type),
                                         s_VtxFmt, filter.m_vbo.get(), nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 1, texs, nullptr, nullptr);
    }
};

TMultiBlendShader<CRandomStaticFilter>::IDataBindingFactory*
CRandomStaticFilter::Initialize(boo::D3DDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    s_VtxFmt = ctx.newVertexFormat(2, VtxVmt);
    s_AlphaPipeline = ctx.newShaderPipeline(VS, FS, nullptr, nullptr, nullptr, s_VtxFmt,
                                            boo::BlendFactor::SrcAlpha,
                                            boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                            boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_AddPipeline = ctx.newShaderPipeline(VS, FS, nullptr, nullptr, nullptr, s_VtxFmt,
                                          boo::BlendFactor::SrcAlpha,
                                          boo::BlendFactor::One, boo::Primitive::TriStrips,
                                          boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_MultPipeline = ctx.newShaderPipeline(VS, FS, nullptr, nullptr, nullptr, s_VtxFmt,
                                           boo::BlendFactor::Zero,
                                           boo::BlendFactor::SrcColor, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_CookieCutterPipeline = ctx.newShaderPipeline(VS, FSCookieCutter, nullptr, nullptr, nullptr, s_VtxFmt,
                                                   boo::BlendFactor::Zero,
                                                   boo::BlendFactor::SrcColor, boo::Primitive::TriStrips,
                                                   boo::ZTest::LEqual, true, false, false, boo::CullMode::None);
    return new CRandomStaticFilterD3DDataBindingFactory;
}

template <>
void CRandomStaticFilter::Shutdown<boo::D3DDataFactory>()
{
    s_VtxFmt.reset();
    s_AlphaPipeline.reset();
    s_AddPipeline.reset();
    s_MultPipeline.reset();
    s_CookieCutterPipeline.reset();
}
}
