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
"    float lod;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"    float lod;\n"
"};\n"
"\n"
"vertex VertToFrag vmain(VertData v [[ stage_in ]], constant TexuredQuadUniform& tqu [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = tqu.color;\n"
"    vtf.uv = v.uvIn.xy;\n"
"    vtf.lod = tqu.lod;\n"
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
"    float lod;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"    float lod;\n"
"};\n"
"\n"
"vertex VertToFrag vmain(VertData v [[ stage_in ]], constant TexuredQuadUniform& tqu [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = tqu.color;\n"
"    vtf.uv = v.uvIn.xy;\n"
"    vtf.lod = tqu.lod;\n"
"    vtf.position = tqu.mat * float4(v.posIn.xyz, 1.0);\n"
"    return vtf;\n"
"}\n";


static const char* FS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"constexpr sampler samp(address::clamp_to_edge, filter::linear);\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"    float lod;\n"
"};\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]], texture2d<float> tex [[ texture(0) ]])\n"
"{\n"
"    return vtf.color * float4(tex.sample(samp, vtf.uv, bias(vtf.lod)).rgb, 1.0);\n"
"}\n";

static const char* FSAlpha =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"constexpr sampler samp(address::clamp_to_edge, filter::linear);\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"    float lod;\n"
"};\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]], texture2d<float> tex [[ texture(0) ]])\n"
"{\n"
"    return vtf.color * tex.sample(samp, vtf.uv, bias(vtf.lod));\n"
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

struct CTexturedQuadFilterMetalDataBindingFactory : TMultiBlendShader<CTexturedQuadFilter>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                  EFilterType type, CTexturedQuadFilter& filter)
    {
        boo::MetalDataFactory::Context& cctx = static_cast<boo::MetalDataFactory::Context&>(ctx);

        boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {filter.m_uniBuf.get()};
        boo::ObjToken<boo::ITexture> texs[] = {filter.m_booTex.get()};
        return cctx.newShaderDataBinding(SelectPipeline(type, filter.m_gequal), s_VtxFmt,
                                         filter.m_vbo.get(), nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 1, texs, nullptr, nullptr);
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
    s_AlphaPipeline = ctx.newShaderPipeline(VSNoFlip, FS, nullptr, nullptr,
                                            s_VtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcAlpha,
                                            boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips, boo::ZTest::None, false,
                                            true, false, boo::CullMode::None);
    s_AlphaGEqualPipeline = ctx.newShaderPipeline(VSNoFlip, FS, nullptr, nullptr,
                                                  s_VtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcAlpha,
                                                  boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips, boo::ZTest::GEqual, true,
                                                  true, false, boo::CullMode::None);
    s_AddPipeline = ctx.newShaderPipeline(VSNoFlip, FS, nullptr, nullptr,
                                          s_VtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcAlpha,
                                          boo::BlendFactor::One, boo::Primitive::TriStrips, boo::ZTest::None, false,
                                          true, false, boo::CullMode::None);
    s_MultPipeline = ctx.newShaderPipeline(VSNoFlip, FS, nullptr, nullptr,
                                           s_VtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::Zero,
                                           boo::BlendFactor::SrcColor, boo::Primitive::TriStrips, boo::ZTest::None, false,
                                           true, false, boo::CullMode::None);
    return new CTexturedQuadFilterMetalDataBindingFactory;
}

template <>
void CTexturedQuadFilter::Shutdown<boo::MetalDataFactory>()
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

struct CTexturedQuadFilterAlphaMetalDataBindingFactory : TMultiBlendShader<CTexturedQuadFilterAlpha>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                  EFilterType type, CTexturedQuadFilterAlpha& filter)
    {
        boo::MetalDataFactory::Context& cctx = static_cast<boo::MetalDataFactory::Context&>(ctx);

        boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {filter.m_uniBuf.get()};
        boo::ObjToken<boo::ITexture> texs[] = {filter.m_booTex.get()};
        return cctx.newShaderDataBinding(SelectAlphaPipeline(type), s_AVtxFmt,
                                         filter.m_vbo.get(), nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 1, texs, nullptr, nullptr);
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
    s_AVtxFmt = ctx.newVertexFormat(2, VtxVmt);
    s_AAlphaPipeline = ctx.newShaderPipeline(VSFlip, FSAlpha, nullptr, nullptr,
                                             s_AVtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcAlpha,
                                             boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                             boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_AAddPipeline = ctx.newShaderPipeline(VSFlip, FSAlpha, nullptr, nullptr,
                                           s_AVtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcAlpha,
                                           boo::BlendFactor::One, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_AMultPipeline = ctx.newShaderPipeline(VSFlip, FSAlpha, nullptr, nullptr,
                                            s_AVtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::Zero,
                                            boo::BlendFactor::SrcColor, boo::Primitive::TriStrips,
                                            boo::ZTest::None, false, true, false, boo::CullMode::None);
    return new CTexturedQuadFilterAlphaMetalDataBindingFactory;
}

template <>
void CTexturedQuadFilterAlpha::Shutdown<boo::MetalDataFactory>()
{
    s_AVtxFmt.reset();
    s_AAlphaPipeline.reset();
    s_AAddPipeline.reset();
    s_AMultPipeline.reset();
}

}
