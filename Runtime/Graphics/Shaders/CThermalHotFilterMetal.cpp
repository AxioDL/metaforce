#include "CThermalHotFilter.hpp"
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
"    float4 uvIn [[ attribute(1) ]];\n"
"};\n"
"\n"
"struct ThermalHotUniform\n"
"{\n"
"    float4 colorReg0;\n"
"    float4 colorReg1;\n"
"    float4 colorReg2;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float2 sceneUv;\n"
"};\n"
"\n"
"vertex VertToFrag vmain(VertData v [[ stage_in ]], constant ThermalHotUniform& thu [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.sceneUv = v.uvIn.xy;\n"
"    vtf.sceneUv.y = -vtf.sceneUv.y;\n"
"    vtf.position = float4(v.posIn.xyz, 1.0);\n"
"    return vtf;\n"
"}\n";

static const char* FS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"constexpr sampler samp(address::repeat, filter::linear);\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float2 sceneUv;\n"
"};\n"
"\n"
"constant float4 kRGBToYPrime = float4(0.299, 0.587, 0.114, 0.0);\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]],\n"
"                      texture2d<float> sceneTex [[ texture(0) ]],\n"
"                      texture2d<float> paletteTex [[ texture(1) ]])\n"
"{\n"
"    float sceneSample = dot(sceneTex.sample(samp, vtf.sceneUv), kRGBToYPrime);\n"
"    float4 colorSample = paletteTex.sample(samp, float2(sceneSample / 17.0, 0.5));\n"
"    return colorSample * sceneSample;\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CThermalHotFilter)

struct CThermalHotFilterMetalDataBindingFactory : TShader<CThermalHotFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CThermalHotFilter& filter)
    {
        boo::MetalDataFactory::Context& cctx = static_cast<boo::MetalDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::ITexture* texs[] = {CGraphics::g_SpareTexture, g_Renderer->GetThermoPalette()};
        return cctx.newShaderDataBinding(TShader<CThermalHotFilter>::m_pipeline,
                                         TShader<CThermalHotFilter>::m_vtxFmt,
                                         filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 2, texs);
    }
};

TShader<CThermalHotFilter>::IDataBindingFactory* CThermalHotFilter::Initialize(boo::MetalDataFactory::Context& ctx,
                                                                               boo::IShaderPipeline*& pipeOut,
                                                                               boo::IVertexFormat*& vtxFmtOut)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    vtxFmtOut = ctx.newVertexFormat(2, VtxVmt);
    pipeOut = ctx.newShaderPipeline(VS, FS, vtxFmtOut, CGraphics::g_ViewportSamples,
                                    boo::BlendFactor::DstAlpha, boo::BlendFactor::InvDstAlpha,
                                    boo::Primitive::TriStrips, false, false, false);
    return new CThermalHotFilterMetalDataBindingFactory;
}

}
