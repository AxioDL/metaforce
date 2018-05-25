#include "CScanLinesFilter.hpp"
#include "TMultiBlendShader.hpp"
#include "Graphics/CTexture.hpp"
#include "GameGlobalObjects.hpp"
#include "Graphics/CBooRenderer.hpp"

namespace urde
{

static const char* VS =
"struct VertData\n"
"{\n"
"    float4 posIn : POSITION;\n"
"};\n"
"\n"
"cbuffer ScanLinesUniform : register(b0)\n"
"{\n"
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
"    vtf.position = float4(v.posIn.xyz, 1.0);\n"
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

URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CScanLinesFilter)

static boo::ObjToken<boo::IVertexFormat> s_VtxFmt;
static boo::ObjToken<boo::IShaderPipeline> s_AlphaPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_AddPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_MultPipeline;

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
        return nullptr;
    }
}

struct CScanLinesFilterD3DDataBindingFactory : TMultiBlendShader<CScanLinesFilter>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding>
    BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                           EFilterType type, CScanLinesFilter& filter)
    {
        boo::D3DDataFactory::Context& cctx = static_cast<boo::D3DDataFactory::Context&>(ctx);

        boo::ObjToken<boo::IGraphicsBuffer> vbo = filter.m_even ?
            g_Renderer->GetScanLinesEvenVBO().get() : g_Renderer->GetScanLinesOddVBO().get();
        boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {filter.m_uniBuf.get()};
        return cctx.newShaderDataBinding(SelectPipeline(type), s_VtxFmt,
                                         vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr);
    }
};

TMultiBlendShader<CScanLinesFilter>::IDataBindingFactory*
CScanLinesFilter::Initialize(boo::D3DDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4}
    };
    s_VtxFmt = ctx.newVertexFormat(1, VtxVmt);
    s_AlphaPipeline = ctx.newShaderPipeline(VS, FS, nullptr, nullptr, nullptr,
                                            s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                            boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                            boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_AddPipeline = ctx.newShaderPipeline(VS, FS, nullptr, nullptr, nullptr,
                                          s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                          boo::BlendFactor::One, boo::Primitive::TriStrips,
                                          boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_MultPipeline = ctx.newShaderPipeline(VS, FS, nullptr, nullptr, nullptr,
                                           s_VtxFmt, boo::BlendFactor::Zero,
                                           boo::BlendFactor::SrcColor, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, true, false, boo::CullMode::None);
    return new CScanLinesFilterD3DDataBindingFactory;
}

template <>
void CScanLinesFilter::Shutdown<boo::D3DDataFactory>()
{
    s_VtxFmt.reset();
    s_AlphaPipeline.reset();
    s_AddPipeline.reset();
    s_MultPipeline.reset();
}

}
