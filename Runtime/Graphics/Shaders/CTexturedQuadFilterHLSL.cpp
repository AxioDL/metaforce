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
"    vtf.color = color;\n"
"    vtf.uv = v.uvIn.xy;\n"
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
"    vtf.color = color;\n"
"    vtf.uv = v.uvIn.xy;\n"
"    vtf.uv.y = -vtf.uv.y;\n"
"    vtf.position = mul(mat, float4(v.posIn.xyz, 1.0));\n"
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
"Texture2D tex : register(t0);\n"
"SamplerState samp : register(s0);\n"
"\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    return vtf.color * float4(tex.Sample(samp, vtf.uv).rgb, 1.0);\n"
"}\n";

static const char* FSAlpha =
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float2 uv : UV;\n"
"};\n"
"\n"
"Texture2D tex : register(t0);\n"
"SamplerState samp : register(s0);\n"
"\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    return vtf.color * tex.Sample(samp, vtf.uv);\n"
"}\n";

URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CTexturedQuadFilter)

static boo::IVertexFormat* s_VtxFmt = nullptr;
static boo::IShaderPipeline* s_AlphaPipeline = nullptr;
static boo::IShaderPipeline* s_AddPipeline = nullptr;
static boo::IShaderPipeline* s_MultPipeline = nullptr;

static boo::IShaderPipeline* SelectPipeline(CCameraFilterPass::EFilterType type)
{
    switch (type)
    {
    case CCameraFilterPass::EFilterType::Blend:
        return s_AlphaPipeline;
    case CCameraFilterPass::EFilterType::Add:
        return s_AddPipeline;
    case CCameraFilterPass::EFilterType::Multiply:
        return s_MultPipeline;
    default:
        return nullptr;
    }
}

struct CTexturedQuadFilterD3DDataBindingFactory : TMultiBlendShader<CTexturedQuadFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    CCameraFilterPass::EFilterType type,
                                                    CTexturedQuadFilter& filter)
    {
        boo::ID3DDataFactory::Context& cctx = static_cast<boo::ID3DDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::ITexture* texs[] = {filter.m_booTex};
        return cctx.newShaderDataBinding(SelectPipeline(type), s_VtxFmt,
                                         filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 1, texs);
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
                                            boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips, false, false,
                                            boo::CullMode::None);
    s_AddPipeline = ctx.newShaderPipeline(VSNoFlip, FS, nullptr, nullptr, nullptr,
                                          s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                          boo::BlendFactor::One, boo::Primitive::TriStrips, false, false,
                                          boo::CullMode::None);
    s_MultPipeline = ctx.newShaderPipeline(VSNoFlip, FS, nullptr, nullptr, nullptr,
                                           s_VtxFmt, boo::BlendFactor::SrcColor,
                                           boo::BlendFactor::DstColor, boo::Primitive::TriStrips, false, false,
                                           boo::CullMode::None);
    return new CTexturedQuadFilterD3DDataBindingFactory;
}

static boo::IVertexFormat* s_AVtxFmt = nullptr;
static boo::IShaderPipeline* s_AAlphaPipeline = nullptr;
static boo::IShaderPipeline* s_AAddPipeline = nullptr;
static boo::IShaderPipeline* s_AMultPipeline = nullptr;

static boo::IShaderPipeline* SelectAlphaPipeline(CCameraFilterPass::EFilterType type)
{
    switch (type)
    {
    case CCameraFilterPass::EFilterType::Blend:
        return s_AAlphaPipeline;
    case CCameraFilterPass::EFilterType::Add:
        return s_AAddPipeline;
    case CCameraFilterPass::EFilterType::Multiply:
        return s_AMultPipeline;
    default:
        return nullptr;
    }
}

struct CTexturedQuadFilterAlphaD3DDataBindingFactory : TMultiBlendShader<CTexturedQuadFilterAlpha>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    CCameraFilterPass::EFilterType type,
                                                    CTexturedQuadFilterAlpha& filter)
    {
        boo::ID3DDataFactory::Context& cctx = static_cast<boo::ID3DDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::ITexture* texs[] = {filter.m_booTex};
        return cctx.newShaderDataBinding(SelectAlphaPipeline(type), s_AVtxFmt,
                                         filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 1, texs);
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
                                             boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips, false, false,
                                             boo::CullMode::None);
    s_AAddPipeline = ctx.newShaderPipeline(VSFlip, FSAlpha, nullptr, nullptr, nullptr,
                                           s_AVtxFmt, boo::BlendFactor::SrcAlpha,
                                           boo::BlendFactor::One, boo::Primitive::TriStrips, false, false,
                                           boo::CullMode::None);
    s_AMultPipeline = ctx.newShaderPipeline(VSFlip, FSAlpha, nullptr, nullptr, nullptr,
                                            s_AVtxFmt, boo::BlendFactor::SrcColor,
                                            boo::BlendFactor::DstColor, boo::Primitive::TriStrips, false, false,
                                            boo::CullMode::None);
    return new CTexturedQuadFilterAlphaD3DDataBindingFactory;
}

}
