#include "CColoredQuadFilter.hpp"
#include "TMultiBlendShader.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{

static const char* VS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertData\n"
"{\n"
"    float4 posIn [[ attribute(0) ]];\n"
"};\n"
"\n"
"struct ColoredQuadUniform\n"
"{\n"
"    float4x4 xf;\n"
"    float4 color;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"};\n"
"\n"
"vertex VertToFrag vmain(VertData v [[ stage_in ]], constant ColoredQuadUniform& cqu [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = cqu.color;\n"
"    vtf.position = cqu.xf * float4(v.posIn.xyz, 1.0);\n"
"    return vtf;\n"
"}\n";

static const char* FS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"};\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]])\n"
"{\n"
"    return vtf.color;\n"
"}\n";

URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CColoredQuadFilter)

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

struct CColoredQuadFilterMetalDataBindingFactory : TMultiBlendShader<CColoredQuadFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    CCameraFilterPass::EFilterType type,
                                                    CColoredQuadFilter& filter)
    {
        boo::MetalDataFactory::Context& cctx = static_cast<boo::MetalDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        return cctx.newShaderDataBinding(SelectPipeline(type), s_VtxFmt,
                                         filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr);
    }
};

TMultiBlendShader<CColoredQuadFilter>::IDataBindingFactory*
CColoredQuadFilter::Initialize(boo::MetalDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4}
    };
    s_VtxFmt = ctx.newVertexFormat(1, VtxVmt);
    s_AlphaPipeline = ctx.newShaderPipeline(VS, FS, s_VtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcAlpha,
                                            boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                            boo::ZTest::None, false, true, true, boo::CullMode::None);
    s_AddPipeline = ctx.newShaderPipeline(VS, FS, s_VtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcAlpha,
                                          boo::BlendFactor::One, boo::Primitive::TriStrips,
                                          boo::ZTest::None, false, true, true, boo::CullMode::None);
    s_MultPipeline = ctx.newShaderPipeline(VS, FS, s_VtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcColor,
                                           boo::BlendFactor::DstColor, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, true, boo::CullMode::None);
    return new CColoredQuadFilterMetalDataBindingFactory;
}

}
