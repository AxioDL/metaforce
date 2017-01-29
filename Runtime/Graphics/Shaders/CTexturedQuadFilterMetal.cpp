#include "CTexturedQuadFilter.hpp"
#include "TMultiBlendShader.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{

static const char* VSFlip =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertData\n"
"{\n"
"    float4 posIn [[ attribute(0) ]];\n"
"    float4 uvIn [[ attribute(1) ]];\n"
"};\n"
"\n"
"struct TexuredQuadUniform\n"
"{\n"
"    float4x4 mat;\n"
"    float4 color;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"};\n"
"\n"
"vertex VertToFrag vmain(VertData v [[ stage_in ]], constant TexuredQuadUniform& tqu [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = tqu.color;\n"
"    vtf.uv = v.uvIn.xy;\n"
"    vtf.position = tqu.mat * float4(v.posIn.xyz, 1.0);\n"
"    return vtf;\n"
"}\n";

static const char* VSNoFlip =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertData\n"
"{\n"
"    float4 posIn [[ attribute(0) ]];\n"
"    float4 uvIn [[ attribute(1) ]];\n"
"};\n"
"\n"
"struct TexuredQuadUniform\n"
"{\n"
"    float4x4 mat;\n"
"    float4 color;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"};\n"
"\n"
"vertex VertToFrag vmain(VertData v [[ stage_in ]], constant TexuredQuadUniform& tqu [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = tqu.color;\n"
"    vtf.uv = v.uvIn.xy;\n"
"    vtf.uv.y = -vtf.uv.y;\n"
"    vtf.position = tqu.mat * float4(v.posIn.xyz, 1.0);\n"
"    return vtf;\n"
"}\n";


static const char* FS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"constexpr sampler samp(address::repeat, filter::linear);\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"};\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]], texture2d<float> tex [[ texture(0) ]])\n"
"{\n"
"    return vtf.color * float4(tex.sample(samp, vtf.uv).rgb, 1.0);\n"
"}\n";

static const char* FSAlpha =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"constexpr sampler samp(address::repeat, filter::linear);\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"};\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]], texture2d<float> tex [[ texture(0) ]])\n"
"{\n"
"    return vtf.color * tex.sample(samp, vtf.uv);\n"
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

struct CTexturedQuadFilterMetalDataBindingFactory : TMultiBlendShader<CTexturedQuadFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    CCameraFilterPass::EFilterType type,
                                                    CTexturedQuadFilter& filter)
    {
        boo::MetalDataFactory::Context& cctx = static_cast<boo::MetalDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::ITexture* texs[] = {filter.m_booTex};
        return cctx.newShaderDataBinding(SelectPipeline(type), s_VtxFmt,
                                         filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 1, texs);
    }
};

TMultiBlendShader<CTexturedQuadFilter>::IDataBindingFactory*
CTexturedQuadFilter::Initialize(boo::MetalDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    s_VtxFmt = ctx.newVertexFormat(2, VtxVmt);
    s_AlphaPipeline = ctx.newShaderPipeline(VSNoFlip, FS, s_VtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcAlpha,
                                            boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips, false, false, false);
    s_AddPipeline = ctx.newShaderPipeline(VSNoFlip, FS, s_VtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcAlpha,
                                          boo::BlendFactor::One, boo::Primitive::TriStrips, false, false, false);
    s_MultPipeline = ctx.newShaderPipeline(VSNoFlip, FS, s_VtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcColor,
                                           boo::BlendFactor::DstColor, boo::Primitive::TriStrips, false, false, false);
    return new CTexturedQuadFilterMetalDataBindingFactory;
}

struct CTexturedQuadFilterAlphaMetalDataBindingFactory : TMultiBlendShader<CTexturedQuadFilterAlpha>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    CCameraFilterPass::EFilterType type,
                                                    CTexturedQuadFilterAlpha& filter)
    {
        boo::MetalDataFactory::Context& cctx = static_cast<boo::MetalDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::ITexture* texs[] = {filter.m_booTex};
        return cctx.newShaderDataBinding(SelectPipeline(type), s_VtxFmt,
                                         filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 1, texs);
    }
};

TMultiBlendShader<CTexturedQuadFilterAlpha>::IDataBindingFactory*
CTexturedQuadFilterAlpha::Initialize(boo::MetalDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    s_VtxFmt = ctx.newVertexFormat(2, VtxVmt);
    s_AlphaPipeline = ctx.newShaderPipeline(VSFlip, FSAlpha, s_VtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcAlpha,
                                            boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips, false, false, false);
    s_AddPipeline = ctx.newShaderPipeline(VSFlip, FSAlpha, s_VtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcAlpha,
                                          boo::BlendFactor::One, boo::Primitive::TriStrips, false, false, false);
    s_MultPipeline = ctx.newShaderPipeline(VSFlip, FSAlpha, s_VtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcColor,
                                           boo::BlendFactor::DstColor, boo::Primitive::TriStrips, false, false, false);
    return new CTexturedQuadFilterAlphaMetalDataBindingFactory;
}

}
