#include "CFogVolumePlaneShader.hpp"
#include "TShader.hpp"

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
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"};\n"
"\n"
"vertex VertToFrag vmain(VertData v [[ stage_in ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.position = v.posIn;\n"
"    return vtf;\n"
"}\n";

static const char* FS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"};\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]])\n"
"{\n"
"    return float4(1.0, 1.0, 1.0, 1.0);\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CFogVolumePlaneShader)

static boo::ObjToken<boo::IVertexFormat> s_VtxFmt;
static boo::ObjToken<boo::IShaderPipeline> s_Pipelines[4];

struct CFogVolumePlaneShaderMetalDataBindingFactory : TShader<CFogVolumePlaneShader>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                  CFogVolumePlaneShader& filter)
    {
        boo::MetalDataFactory::Context& cctx = static_cast<boo::MetalDataFactory::Context&>(ctx);

        for (int i=0 ; i<4 ; ++i)
            filter.m_dataBinds[i] = cctx.newShaderDataBinding(s_Pipelines[i], s_VtxFmt,
                filter.m_vbo.get(), nullptr, nullptr, 0, nullptr,
                nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr);
        return filter.m_dataBinds[0];
    }
};

TShader<CFogVolumePlaneShader>::IDataBindingFactory*
CFogVolumePlaneShader::Initialize(boo::MetalDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4}
    };
    s_VtxFmt = ctx.newVertexFormat(1, VtxVmt);
    s_Pipelines[0] = ctx.newShaderPipeline(VS, FS, nullptr, nullptr,
                                           s_VtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::Zero,
                                           boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                           boo::ZTest::LEqual, true, false, false, boo::CullMode::Frontface);
    s_Pipelines[1] = ctx.newShaderPipeline(VS, FS, nullptr, nullptr,
                                           s_VtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::Zero,
                                           boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, false, false, boo::CullMode::Frontface);
    s_Pipelines[2] = ctx.newShaderPipeline(VS, FS, nullptr, nullptr,
                                           s_VtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::Zero,
                                           boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                           boo::ZTest::LEqual, true, false, false, boo::CullMode::Backface);
    s_Pipelines[3] = ctx.newShaderPipeline(VS, FS, nullptr, nullptr,
                                           s_VtxFmt, CGraphics::g_ViewportSamples, boo::BlendFactor::Zero,
                                           boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                           boo::ZTest::Greater, false, false, false, boo::CullMode::Backface);
    return new CFogVolumePlaneShaderMetalDataBindingFactory;
}

template <>
void CFogVolumePlaneShader::Shutdown<boo::MetalDataFactory>()
{
    s_VtxFmt.reset();
    s_Pipelines[0].reset();
    s_Pipelines[1].reset();
    s_Pipelines[2].reset();
    s_Pipelines[3].reset();
}

}
