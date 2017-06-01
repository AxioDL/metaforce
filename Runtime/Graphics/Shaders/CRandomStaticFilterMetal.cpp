#include "CRandomStaticFilter.hpp"
#include "TMultiBlendShader.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "GameGlobalObjects.hpp"

namespace urde
{

static const char* VS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertData\n"
"{\n"
"    float4 posIn [[ attribute(0) ]];\n"
"    float2 uvIn [[ attribute(1) ]];\n"
"};\n"
"\n"
"struct RandomStaticUniform\n"
"{\n"
"    float4 color;\n"
"    float randOff;\n"
"    float discardThres;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 pos [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"    float randOff;\n"
"    float discardThres;\n"
"};\n"
"\n"
"vertex VertToFrag vmain(VertData v [[ stage_in ]],\n"
"                        constant RandomStaticUniform& su [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = su.color;\n"
"    vtf.uv = v.uvIn.xy;\n"
"    vtf.randOff = su.randOff;\n"
"    vtf.discardThres = su.discardThres;\n"
"    vtf.pos = float4(v.posIn.xyz, 1.0);\n"
"    return vtf;\n"
"}\n";

static const char* FS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertToFrag\n"
"{\n"
"    float4 pos [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"    float randOff;\n"
"    float discardThres;\n"
"};\n"
"\n"
"static uint2 Lookup8BPP(float2 uv, float randOff)\n"
"{\n"
"    float bx;\n"
"    float rx = modf(uv.x / 8.0, bx) * 8.0;\n"
"    float by;\n"
"    float ry = modf(uv.y / 4.0, by) * 4.0;\n"
"    float bidx = by * 80.0 + bx;\n"
"    float addr = bidx * 32.0 + ry * 8.0 + rx + randOff;\n"
"    float y;\n"
"    float x = modf(addr / 1024.0, y) * 1024.0;\n"
"    return uint2(x, y);\n"
"}\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]],\n"
"                      texture2d<float> tex [[ texture(0) ]])\n"
"{\n"
"    float4 colorOut = tex.read(Lookup8BPP(vtf.uv, vtf.randOff)) * vtf.color;\n"
"    colorOut.a = vtf.color.a;\n"
"    return colorOut;\n"
"}\n";

static const char* FSCookieCutter =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertToFrag\n"
"{\n"
"    float4 pos [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"    float randOff;\n"
"    float discardThres;\n"
"};\n"
"\n"
"static uint2 Lookup8BPP(float2 uv, float randOff)\n"
"{\n"
"    float bx;\n"
"    float rx = modf(uv.x / 8.0, bx) * 8.0;\n"
"    float by;\n"
"    float ry = modf(uv.y / 4.0, by) * 4.0;\n"
"    float bidx = by * 80.0 + bx;\n"
"    float addr = bidx * 32.0 + ry * 8.0 + rx + randOff;\n"
"    float y;\n"
"    float x = modf(addr / 1024.0, y) * 1024.0;\n"
"    return uint2(x, y);\n"
"}\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]],\n"
"                      texture2d<float> tex [[ texture(0) ]])\n"
"{\n"
"    float4 colorOut = tex.read(Lookup8BPP(vtf.uv, vtf.randOff)) * vtf.color;\n"
"    if (colorOut.a < vtf.discardThres)\n"
"        discard_fragment();\n"
"    return colorOut;\n"
"}\n";

URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CRandomStaticFilter)

static boo::IVertexFormat* s_VtxFmt = nullptr;
static boo::IShaderPipeline* s_AlphaPipeline = nullptr;
static boo::IShaderPipeline* s_AddPipeline = nullptr;
static boo::IShaderPipeline* s_MultPipeline = nullptr;
static boo::IShaderPipeline* s_CookieCutterPipeline = nullptr;

static boo::IShaderPipeline* SelectPipeline(EFilterType type)
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
        return nullptr;
    }
}

struct CRandomStaticFilterMetalDataBindingFactory : TMultiBlendShader<CRandomStaticFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    EFilterType type,
                                                    CRandomStaticFilter& filter)
    {
        boo::MetalDataFactory::Context& cctx = static_cast<boo::MetalDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::ITexture* texs[] = {g_Renderer->GetRandomStaticEntropyTex()};
        return cctx.newShaderDataBinding(filter.m_cookieCutter ? s_CookieCutterPipeline : SelectPipeline(type),
                                         s_VtxFmt, filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 1, texs, nullptr, nullptr);
    }
};

TMultiBlendShader<CRandomStaticFilter>::IDataBindingFactory*
CRandomStaticFilter::Initialize(boo::MetalDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    s_VtxFmt = ctx.newVertexFormat(2, VtxVmt);
    s_AlphaPipeline = ctx.newShaderPipeline(VS, FS, s_VtxFmt, CGraphics::g_ViewportSamples,
                                            boo::BlendFactor::SrcAlpha,
                                            boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                            boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_AddPipeline = ctx.newShaderPipeline(VS, FS, s_VtxFmt, CGraphics::g_ViewportSamples,
                                          boo::BlendFactor::SrcAlpha,
                                          boo::BlendFactor::One, boo::Primitive::TriStrips,
                                          boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_MultPipeline = ctx.newShaderPipeline(VS, FS, s_VtxFmt, CGraphics::g_ViewportSamples,
                                           boo::BlendFactor::SrcColor,
                                           boo::BlendFactor::DstColor, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_CookieCutterPipeline = ctx.newShaderPipeline(VS, FSCookieCutter, s_VtxFmt, CGraphics::g_ViewportSamples,
                                                   boo::BlendFactor::SrcColor,
                                                   boo::BlendFactor::DstColor, boo::Primitive::TriStrips,
                                                   boo::ZTest::LEqual, true, false, false, boo::CullMode::None);
    return new CRandomStaticFilterMetalDataBindingFactory;
}

}
