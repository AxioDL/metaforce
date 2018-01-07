#include "CEnergyBarShader.hpp"
#include "TShader.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{

static const char* VS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertData\n"
"{\n"
"    float4 posIn [[ attribute(0) ]];\n"
"    float4 uvIn [[ attribute(1) ]];\n"
"};\n"
"\n"
"struct EnergyBarUniform\n"
"{\n"
"    float4x4 xf;\n"
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
"vertex VertToFrag vmain(VertData v [[ stage_in ]], constant EnergyBarUniform& ebu [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = ebu.color;\n"
"    vtf.uv = v.uvIn.xy;\n"
"    vtf.position = ebu.xf * float4(v.posIn.xyz, 1.0);\n"
"    return vtf;\n"
"}\n";

static const char* FS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"};\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]],\n"
"                      sampler samp [[ sampler(0) ]],\n"
"                      texture2d<float> tex [[ texture(0) ]])\n"
"{\n"
"    return vtf.color * tex.sample(samp, vtf.uv);\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CEnergyBarShader)

static boo::ObjToken<boo::IVertexFormat> s_VtxFmt;
static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;

struct CEnergyBarShaderMetalDataBindingFactory : TShader<CEnergyBarShader>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                  CEnergyBarShader& filter)
    {
        boo::MetalDataFactory::Context& cctx = static_cast<boo::MetalDataFactory::Context&>(ctx);

        boo::ObjToken<boo::IGraphicsBuffer> bufs[1];
        boo::ObjToken<boo::ITexture> texs[] = {filter.m_tex->GetBooTexture()};
        for (int i=0 ; i<3 ; ++i)
        {
            bufs[0] = filter.m_uniBuf[i].get();
            filter.m_dataBind[i] = cctx.newShaderDataBinding(s_Pipeline, s_VtxFmt,
                filter.m_vbo.get(), nullptr, nullptr, 1, bufs,
                nullptr, nullptr, nullptr, 1, texs, nullptr, nullptr);
        }
        return filter.m_dataBind[0];
    }
};

TShader<CEnergyBarShader>::IDataBindingFactory*
CEnergyBarShader::Initialize(boo::MetalDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    s_VtxFmt = ctx.newVertexFormat(2, VtxVmt);
    s_Pipeline = ctx.newShaderPipeline(VS, FS, nullptr, nullptr, s_VtxFmt,
                                       boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                       boo::Primitive::TriStrips, boo::ZTest::LEqual,
                                       false, true, false, boo::CullMode::None);
    return new CEnergyBarShaderMetalDataBindingFactory;
}

template <>
void CEnergyBarShader::Shutdown<boo::MetalDataFactory>()
{
    s_VtxFmt.reset();
    s_Pipeline.reset();
}

}
