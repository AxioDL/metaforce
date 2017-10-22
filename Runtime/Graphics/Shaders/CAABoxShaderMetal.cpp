#include "CAABoxShader.hpp"
#include "Graphics/CBooRenderer.hpp"

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
"struct CAABoxUniform\n"
"{\n"
"    float4x4 xf;\n"
"    float4 color;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 pos [[ position ]];\n"
"    float4 color;\n"
"};\n"
"\n"
"vertex VertToFrag vmain(VertData v [[ stage_in ]], constant CAABoxUniform& bu [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = bu.color;\n"
"    vtf.pos = bu.xf * float4(v.posIn.xyz, 1.0);\n"
"    return vtf;\n"
"}\n";

static const char* FS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertToFrag\n"
"{\n"
"    float4 pos [[ position ]];\n"
"    float4 color;\n"
"};\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]])\n"
"{\n"
"    return vtf.color;\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CAABoxShader)

static boo::IVertexFormat* s_VtxFmt = nullptr;
static boo::IShaderPipeline* s_Pipeline = nullptr;
static boo::IShaderPipeline* s_zOnlyPipeline = nullptr;

struct CAABoxShaderMetalDataBindingFactory : TShader<CAABoxShader>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    CAABoxShader& filter)
    {
        boo::MetalDataFactory::Context& cctx = static_cast<boo::MetalDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        return cctx.newShaderDataBinding(filter.m_zOnly ? s_zOnlyPipeline : s_Pipeline, s_VtxFmt,
                                         filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr);
    }
};

TShader<CAABoxShader>::IDataBindingFactory* CAABoxShader::Initialize(boo::MetalDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
    };
    s_VtxFmt = ctx.newVertexFormat(1, VtxVmt);
    s_Pipeline = ctx.newShaderPipeline(VS, FS, s_VtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcAlpha,
                                       boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                       boo::ZTest::LEqual, true, true, false, boo::CullMode::None);
    s_zOnlyPipeline = ctx.newShaderPipeline(VS, FS, s_VtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::SrcAlpha,
                                            boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                            boo::ZTest::LEqual, true, false, false, boo::CullMode::None);
    return new CAABoxShaderMetalDataBindingFactory;
}

}
