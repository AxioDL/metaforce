#include "CThermalColdFilter.hpp"

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
"struct ThermalColdUniform\n"
"{\n"
"    float4x4 shiftMtx;\n"
"    float4x4 indMtx;\n"
"    float4 shiftScale;\n"
"    float4 colorReg0;\n"
"    float4 colorReg1;\n"
"    float4 colorReg2;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float3 indMtx0;\n"
"    float3 indMtx1;\n"
"    float3 indMtx2;\n"
"    float4 colorReg0;\n"
"    float4 colorReg1;\n"
"    float4 colorReg2;\n"
"    float2 sceneUv;\n"
"    float2 shiftUv;\n"
"    float2 shiftScale;\n"
"};\n"
"\n"
"vertex VertToFrag vmain(VertData v [[ stage_in ]], constant ThermalColdUniform& tcu [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.indMtx0 = tcu.indMtx[0].xyz;\n"
"    vtf.indMtx1 = tcu.indMtx[1].xyz;\n"
"    vtf.indMtx2 = tcu.indMtx[2].xyz;\n"
"    vtf.colorReg0 = tcu.colorReg0;\n"
"    vtf.colorReg1 = tcu.colorReg1;\n"
"    vtf.colorReg2 = tcu.colorReg2;\n"
"    vtf.sceneUv = v.uvIn.xy;\n"
"    vtf.sceneUv.y = -vtf.sceneUv.y;\n"
"    vtf.shiftUv = (float3x3(tcu.shiftMtx[0].xyz, tcu.shiftMtx[1].xyz, tcu.shiftMtx[2].xyz) * v.uvIn.xyz).xy;\n"
"    vtf.shiftScale = tcu.shiftScale.xy;\n"
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
"    float3 indMtx0;\n"
"    float3 indMtx1;\n"
"    float3 indMtx2;\n"
"    float4 colorReg0;\n"
"    float4 colorReg1;\n"
"    float4 colorReg2;\n"
"    float2 sceneUv;\n"
"    float2 shiftUv;\n"
"    float2 shiftScale;\n"
"};\n"
"\n"
"constant float4 kRGBToYPrime = {0.299, 0.587, 0.114, 0.0};\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]], texture2d<float> sceneTex [[ texture(0) ]], texture2d<float> shiftTex [[ texture(1) ]])\n"
"{\n"
"    float2 shiftCoordTexel = shiftTex.sample(samp, vtf.shiftUv).xy;\n"
"    float2 shiftCoord = vtf.sceneUv + shiftCoordTexel * vtf.shiftScale;\n"
"    float shiftScene0 = dot(sceneTex.sample(samp, shiftCoord), kRGBToYPrime);\n"
"    float shiftScene1 = dot(sceneTex.sample(samp, shiftCoord + float2(vtf.shiftScale.x / 8.0, 0.0)), kRGBToYPrime);\n"
"    float2 indCoord = (float3x3(vtf.indMtx0, vtf.indMtx1, vtf.indMtx2) * float3(shiftScene0 - 0.5, shiftScene1 - 0.5, 1.0)).xy;\n"
"    float indScene = dot(sceneTex.sample(samp, vtf.sceneUv + indCoord), kRGBToYPrime);\n"
"    return vtf.colorReg0 * indScene + vtf.colorReg1 * shiftScene0 + vtf.colorReg2;\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CThermalColdFilter)

struct CThermalColdFilterMetalDataBindingFactory : TShader<CThermalColdFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CThermalColdFilter& filter)
    {
        boo::MetalDataFactory::Context& cctx = static_cast<boo::MetalDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::ITexture* texs[] = {CGraphics::g_SpareTexture, filter.m_shiftTex};
        return cctx.newShaderDataBinding(TShader<CThermalColdFilter>::m_pipeline,
                                         TShader<CThermalColdFilter>::m_vtxFmt,
                                         filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 2, texs);
    }
};

TShader<CThermalColdFilter>::IDataBindingFactory* CThermalColdFilter::Initialize(boo::MetalDataFactory::Context& ctx,
                                                                                       boo::IShaderPipeline*& pipeOut,
                                                                                       boo::IVertexFormat*& vtxFmtOut)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    vtxFmtOut = ctx.newVertexFormat(2, VtxVmt);
    pipeOut = ctx.newShaderPipeline(VS, FS, vtxFmtOut, CGraphics::g_ViewportSamples, boo::BlendFactor::One,
                                    boo::BlendFactor::Zero, boo::Primitive::TriStrips, false, false, false);
    return new CThermalColdFilterMetalDataBindingFactory;
}

}
