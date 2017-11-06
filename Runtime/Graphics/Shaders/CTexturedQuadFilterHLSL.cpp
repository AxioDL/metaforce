#include "CTexturedQuadFilter.hpp"
#include "TMultiBlendShader.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{

static const char* VSFlip =
"struct VertData\n"
"{\n"
"    float4 posIn : POSITION;\n"
"    float4 uvIn : UV;\n"
"};\n"
"\n"
"cbuffer TexuredQuadUniform : register(b0)\n"
"{\n"
"    float4x4 mat;\n"
"    float4 color;\n"
"    float lod;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float2 uv : UV;\n"
"    float lod : LOD;\n"
"};\n"
"\n"
"VertToFrag main(in VertData v)\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = color;\n"
"    vtf.uv = v.uvIn.xy;\n"
"    vtf.lod = lod;\n"
"    vtf.position = mul(mat, float4(v.posIn.xyz, 1.0));\n"
"    return vtf;\n"
"}\n";

static const char* VSNoFlip =
"struct VertData\n"
"{\n"
"    float4 posIn : POSITION;\n"
"    float4 uvIn : UV;\n"
"};\n"
"\n"
"cbuffer TexuredQuadUniform : register(b0)\n"
"{\n"
"    float4x4 mat;\n"
"    float4 color;\n"
"    float lod;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float2 uv : UV;\n"
"    float lod : LOD;\n"
"};\n"
"\n"
"VertToFrag main(in VertData v)\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = color;\n"
"    vtf.uv = v.uvIn.xy;\n"
"    vtf.uv.y = -vtf.uv.y;\n"
"    vtf.lod = lod;\n"
"    vtf.position = mul(mat, float4(v.posIn.xyz, 1.0));\n"
"    return vtf;\n"
"}\n";

static const char* FS =
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float2 uv : UV;\n"
"    float lod : LOD;\n"
"};\n"
"\n"
"Texture2D tex : register(t0);\n"
"SamplerState samp : register(s0);\n"
"\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    return vtf.color * float4(tex.SampleBias(samp, vtf.uv, vtf.lod).rgb, 1.0);\n"
"}\n";

static const char* FSAlpha =
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float2 uv : UV;\n"
"    float lod : LOD;\n"
"};\n"
"\n"
"Texture2D tex : register(t0);\n"
"SamplerState samp : register(s0);\n"
"\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    return vtf.color * tex.SampleBias(samp, vtf.uv, vtf.lod);\n"
"}\n";

URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CTexturedQuadFilter)

static boo::ObjToken<boo::IVertexFormat> s_VtxFmt;
static boo::ObjToken<boo::IShaderPipeline> s_AlphaPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AlphaGEqualPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AddPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_MultPipeline;

static boo::ObjToken<boo::IShaderPipeline> SelectPipeline(EFilterType type, bool gequal)
{
    if (gequal)
        return s_AlphaGEqualPipeline;
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

struct CTexturedQuadFilterD3DDataBindingFactory : TMultiBlendShader<CTexturedQuadFilter>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding>
    BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                           EFilterType type, CTexturedQuadFilter& filter)
    {
        boo::ID3DDataFactory::Context& cctx = static_cast<boo::ID3DDataFactory::Context&>(ctx);

        boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {filter.m_uniBuf.get()};
        boo::ObjToken<boo::ITexture> texs[] = {filter.m_booTex.get()};
        return cctx.newShaderDataBinding(SelectPipeline(type, filter.m_gequal), s_VtxFmt,
                                         filter.m_vbo.get(), nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 1, texs, nullptr, nullptr);
    }
};

TMultiBlendShader<CTexturedQuadFilter>::IDataBindingFactory*
CTexturedQuadFilter::Initialize(boo::ID3DDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    s_VtxFmt = ctx.newVertexFormat(2, VtxVmt);
    s_AlphaPipeline = ctx.newShaderPipeline(VSNoFlip, FS, nullptr, nullptr, nullptr,
                                            s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                            boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                            boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_AlphaGEqualPipeline = ctx.newShaderPipeline(VSNoFlip, FS, nullptr, nullptr, nullptr,
                                                  s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                                  boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                                  boo::ZTest::GEqual, true, true, false, boo::CullMode::None);
    s_AddPipeline = ctx.newShaderPipeline(VSNoFlip, FS, nullptr, nullptr, nullptr,
                                          s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                          boo::BlendFactor::One, boo::Primitive::TriStrips,
                                          boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_MultPipeline = ctx.newShaderPipeline(VSNoFlip, FS, nullptr, nullptr, nullptr,
                                           s_VtxFmt, boo::BlendFactor::SrcColor,
                                           boo::BlendFactor::DstColor, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, false, boo::CullMode::None);
    return new CTexturedQuadFilterD3DDataBindingFactory;
}

template <>
void CTexturedQuadFilter::Shutdown<boo::ID3DDataFactory>()
{
    s_VtxFmt.reset();
    s_AlphaPipeline.reset();
    s_AlphaGEqualPipeline.reset();
    s_AddPipeline.reset();
    s_MultPipeline.reset();
}

static boo::ObjToken<boo::IVertexFormat> s_AVtxFmt;
static boo::ObjToken<boo::IShaderPipeline> s_AAlphaPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AAddPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AMultPipeline;

static boo::ObjToken<boo::IShaderPipeline> SelectAlphaPipeline(EFilterType type)
{
    switch (type)
    {
    case EFilterType::Blend:
        return s_AAlphaPipeline;
    case EFilterType::Add:
        return s_AAddPipeline;
    case EFilterType::Multiply:
        return s_AMultPipeline;
    default:
        return {};
    }
}

struct CTexturedQuadFilterAlphaD3DDataBindingFactory : TMultiBlendShader<CTexturedQuadFilterAlpha>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding>
    BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                           EFilterType type, CTexturedQuadFilterAlpha& filter)
    {
        boo::ID3DDataFactory::Context& cctx = static_cast<boo::ID3DDataFactory::Context&>(ctx);

        boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {filter.m_uniBuf.get()};
        boo::ObjToken<boo::ITexture> texs[] = {filter.m_booTex.get()};
        return cctx.newShaderDataBinding(SelectAlphaPipeline(type), s_AVtxFmt,
                                         filter.m_vbo.get(), nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 1, texs, nullptr, nullptr);
    }
};

TMultiBlendShader<CTexturedQuadFilterAlpha>::IDataBindingFactory*
CTexturedQuadFilterAlpha::Initialize(boo::ID3DDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    s_AVtxFmt = ctx.newVertexFormat(2, VtxVmt);
    s_AAlphaPipeline = ctx.newShaderPipeline(VSFlip, FSAlpha, nullptr, nullptr, nullptr,
                                             s_AVtxFmt, boo::BlendFactor::SrcAlpha,
                                             boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                             boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_AAddPipeline = ctx.newShaderPipeline(VSFlip, FSAlpha, nullptr, nullptr, nullptr,
                                           s_AVtxFmt, boo::BlendFactor::SrcAlpha,
                                           boo::BlendFactor::One, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_AMultPipeline = ctx.newShaderPipeline(VSFlip, FSAlpha, nullptr, nullptr, nullptr,
                                            s_AVtxFmt, boo::BlendFactor::SrcColor,
                                            boo::BlendFactor::DstColor, boo::Primitive::TriStrips,
                                            boo::ZTest::None, false, true, false, boo::CullMode::None);
    return new CTexturedQuadFilterAlphaD3DDataBindingFactory;
}

template <>
void CTexturedQuadFilterAlpha::Shutdown<boo::ID3DDataFactory>()
{
    s_AVtxFmt.reset();
    s_AAlphaPipeline.reset();
    s_AAddPipeline.reset();
    s_AMultPipeline.reset();
}
}
