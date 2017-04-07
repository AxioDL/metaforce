#include "CRadarPaintShader.hpp"
#include "TShader.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{

static const char* VS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertData\n"
"{\n"
"    float4 posIn[4];\n"
"    float4 uvIn[4];\n"
"    float4 colorIn;\n"
"};\n"
"\n"
"struct RadarPaintUniform\n"
"{\n"
"    float4x4 xf;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"};\n"
"\n"
"vertex VertToFrag vmain(constant VertData* va [[ buffer(1) ]],\n"
"                        uint vertId [[ vertex_id ]], uint instId [[ instance_id ]],\n"
"                        constant RadarPaintUniform& rpu [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    constant VertData& v = va[instId];\n"
"    vtf.color = v.colorIn;\n"
"    vtf.uv = v.uvIn[vertId].xy;\n"
"    vtf.position = rpu.xf * float4(v.posIn[vertId].xyz, 1.0);\n"
"    return vtf;\n"
"}\n";

static const char* FS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"constexpr sampler samp(address::repeat, filter::linear, mip_filter::linear);\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"};\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]],\n"
"                      texture2d<float> tex [[ texture(0) ]])\n"
"{\n"
"    return vtf.color * tex.sample(samp, vtf.uv);\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CRadarPaintShader)

static boo::IVertexFormat* s_VtxFmt = nullptr;
static boo::IShaderPipeline* s_Pipeline = nullptr;

struct CRadarPaintShaderMetalDataBindingFactory : TShader<CRadarPaintShader>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    CRadarPaintShader& filter)
    {
        boo::MetalDataFactory::Context& cctx = static_cast<boo::MetalDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::ITexture* texs[] = {filter.m_tex->GetBooTexture()};
        return cctx.newShaderDataBinding(s_Pipeline, s_VtxFmt,
                                         nullptr, filter.m_vbo, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 1, texs, nullptr, nullptr);
    }
};

TShader<CRadarPaintShader>::IDataBindingFactory*
CRadarPaintShader::Initialize(boo::MetalDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4, 0},
        {nullptr, nullptr, boo::VertexSemantic::Position4, 1},
        {nullptr, nullptr, boo::VertexSemantic::Position4, 2},
        {nullptr, nullptr, boo::VertexSemantic::Position4, 3},
        {nullptr, nullptr, boo::VertexSemantic::UV4, 0},
        {nullptr, nullptr, boo::VertexSemantic::UV4, 1},
        {nullptr, nullptr, boo::VertexSemantic::UV4, 2},
        {nullptr, nullptr, boo::VertexSemantic::UV4, 3},
        {nullptr, nullptr, boo::VertexSemantic::Color}
    };
    s_VtxFmt = ctx.newVertexFormat(2, VtxVmt);
    s_Pipeline = ctx.newShaderPipeline(VS, FS, s_VtxFmt, CGraphics::g_ViewportSamples,
                                       boo::BlendFactor::SrcAlpha,
                                       boo::BlendFactor::One, boo::Primitive::TriStrips,
                                       boo::ZTest::None, false, true, false, boo::CullMode::None);
    return new CRadarPaintShaderMetalDataBindingFactory;
}

}
