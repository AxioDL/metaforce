#include "CMapSurfaceShader.hpp"
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
"struct MapSurfaceUniform\n"
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
"vertex VertToFrag vmain(VertData v [[ stage_in ]], constant MapSurfaceUniform& msu [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = msu.color;\n"
"    vtf.position = msu.xf * float4(v.posIn.xyz, 1.0);\n"
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

URDE_DECL_SPECIALIZE_SHADER(CMapSurfaceShader)

static boo::ObjToken<boo::IVertexFormat> s_VtxFmt;
static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;

struct CMapSurfaceShaderMetalDataBindingFactory : TShader<CMapSurfaceShader>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                  CMapSurfaceShader& filter)
    {
        boo::MetalDataFactory::Context& cctx = static_cast<boo::MetalDataFactory::Context&>(ctx);

        boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {filter.m_uniBuf.get()};
        filter.m_dataBind = cctx.newShaderDataBinding(s_Pipeline, s_VtxFmt,
            filter.m_vbo.get(), nullptr, filter.m_ibo.get(), 1, bufs,
            nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr);
        return filter.m_dataBind;
    }
};

TShader<CMapSurfaceShader>::IDataBindingFactory*
CMapSurfaceShader::Initialize(boo::MetalDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4}
    };
    s_VtxFmt = ctx.newVertexFormat(1, VtxVmt);
    s_Pipeline = ctx.newShaderPipeline(VS, FS, nullptr, nullptr,
        s_VtxFmt, CGraphics::g_ViewportSamples,
        boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
        boo::ZTest::LEqual, false, true, false, boo::CullMode::Backface);
    return new CMapSurfaceShaderMetalDataBindingFactory;
}

template <>
void CMapSurfaceShader::Shutdown<boo::MetalDataFactory>()
{
    s_VtxFmt.reset();
    s_Pipeline.reset();
}

}
