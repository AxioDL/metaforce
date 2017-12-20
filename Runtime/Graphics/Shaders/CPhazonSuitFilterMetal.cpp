#include "CPhazonSuitFilter.hpp"
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
"    float4 screenUvIn [[ attribute(1) ]];\n"
"    float4 indUvIn [[ attribute(2) ]];\n"
"    float4 maskUvIn [[ attribute(3) ]];\n"
"};\n"
"\n"
"struct PhazonSuitUniform\n"
"{\n"
"    float4 color;\n"
"    float4 indScaleOff;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"    float4 indScaleOff;\n"
"    float2 screenUv;\n"
"    float2 indUv;\n"
"    float2 maskUv;\n"
"};\n"
"\n"
"vertex VertToFrag vmain(VertData v [[ stage_in ]], constant PhazonSuitUniform& psu [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = psu.color;\n"
"    vtf.indScaleOff = psu.indScaleOff;\n"
"    vtf.screenUv = v.screenUvIn.xy;\n"
"    vtf.screenUv.y = 1.0 - vtf.screenUv.y;\n"
"    vtf.indUv = v.indUvIn.xy;\n"
"    vtf.maskUv = v.maskUvIn.xy;\n"
"    vtf.maskUv.y = 1.0 - vtf.maskUv.y;\n"
"    vtf.position = float4(v.posIn.xyz, 1.0);\n"
"    return vtf;\n"
"}\n";

static const char* IndFS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"constexpr sampler samp(address::repeat, filter::linear);\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"    float4 indScaleOff;\n"
"    float2 screenUv;\n"
"    float2 indUv;\n"
"    float2 maskUv;\n"
"};\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]],\n"
"                      texture2d<float> screenTex [[ texture(0) ]],\n"
"                      texture2d<float> indTex [[ texture(1) ]],\n"
"                      texture2d<float> maskTex [[ texture(2) ]],\n"
"                      texture2d<float> maskTexBlur [[ texture(3) ]])\n"
"{\n"
"    float2 indUv = (indTex.sample(samp, vtf.indUv).ra - float2(0.5, 0.5)) * \n"
"        vtf.indScaleOff.xy + vtf.indScaleOff.zw;\n"
"    float maskBlurAlpha = saturate((maskTexBlur.sample(samp, vtf.maskUv).a - maskTex.sample(samp, vtf.maskUv).a) * 2.0);\n"
"    return float4((vtf.color * screenTex.sample(samp, indUv + vtf.screenUv) * maskBlurAlpha).rgb, vtf.color.a);\n"
"}\n";

static const char* FS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"constexpr sampler samp(address::clamp_to_edge, filter::linear);\n"
"struct VertToFrag\n"
"{\n"
"    float4 color;\n"
"    float4 indScaleOff;\n"
"    float2 screenUv;\n"
"    float2 indUv;\n"
"    float2 maskUv;\n"
"};\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]],\n"
"                      texture2d<float> screenTex [[ texture(0) ]],\n"
"                      texture2d<float> maskTex [[ texture(1) ]],\n"
"                      texture2d<float> maskTexBlur [[ texture(2) ]])\n"
"{\n"
"    float maskBlurAlpha = saturate((maskTexBlur.sample(samp, vtf.maskUv).a - maskTex.sample(samp, vtf.maskUv).a) * 2.0);\n"
"    return float4((vtf.color * screenTex.sample(samp, vtf.screenUv) * maskBlurAlpha).rgb, vtf.color.a);\n"
"}\n";

static const char* BlurVS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertData\n"
"{\n"
"    float4 posIn [[ attribute(0) ]];\n"
"    float4 uvIn [[ attribute(1) ]];\n"
"};\n"
"\n"
"struct PhazonSuitBlurUniform\n"
"{\n"
"    float4 blurDir;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float2 uv;\n"
"    float2 blurDir;\n"
"};\n"
"\n"
"vertex VertToFrag vmain(VertData v [[ stage_in ]], constant PhazonSuitBlurUniform& psu [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.uv = v.uvIn.xy;\n"
"    vtf.uv.y = 1.0 - vtf.uv.y;\n"
"    vtf.blurDir = psu.blurDir.xy;\n"
"    vtf.position = float4(v.posIn.xyz, 1.0);\n"
"    return vtf;\n"
"}\n";

static const char* BlurFS =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"constexpr sampler samp(address::clamp_to_edge, filter::linear);\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float2 uv;\n"
"    float2 blurDir;\n"
"};\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]],\n"
"                      texture2d<float> maskTex [[ texture(0) ]])\n"
"{\n"
"    //this will be our alpha sum\n"
"    float sum = 0.0;\n"
"\n"
"    //apply blurring, using a 9-tap filter with predefined gaussian weights\n"
"\n"
"    sum += maskTex.sample(samp, vtf.uv - 4.0 * vtf.blurDir).a * 0.0162162162;\n"
"    sum += maskTex.sample(samp, vtf.uv - 3.0 * vtf.blurDir).a * 0.0540540541;\n"
"    sum += maskTex.sample(samp, vtf.uv - 2.0 * vtf.blurDir).a * 0.1216216216;\n"
"    sum += maskTex.sample(samp, vtf.uv - 1.0 * vtf.blurDir).a * 0.1945945946;\n"
"\n"
"    sum += maskTex.sample(samp, vtf.uv).a * 0.2270270270;\n"
"\n"
"    sum += maskTex.sample(samp, vtf.uv + 1.0 * vtf.blurDir).a * 0.1945945946;\n"
"    sum += maskTex.sample(samp, vtf.uv + 2.0 * vtf.blurDir).a * 0.1216216216;\n"
"    sum += maskTex.sample(samp, vtf.uv + 3.0 * vtf.blurDir).a * 0.0540540541;\n"
"    sum += maskTex.sample(samp, vtf.uv + 4.0 * vtf.blurDir).a * 0.0162162162;\n"
"\n"
"    return float4(1.0, 1.0, 1.0, sum);\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CPhazonSuitFilter)

static boo::ObjToken<boo::IVertexFormat> s_VtxFmt;
static boo::ObjToken<boo::IVertexFormat> s_BlurVtxFmt;
static boo::ObjToken<boo::IShaderPipeline> s_IndPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;
static boo::ObjToken<boo::IShaderPipeline> s_BlurPipeline;

struct CPhazonSuitFilterMetalDataBindingFactory : TShader<CPhazonSuitFilter>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                  CPhazonSuitFilter& filter)
    {
        boo::MetalDataFactory::Context& cctx = static_cast<boo::MetalDataFactory::Context&>(ctx);

        boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {filter.m_uniBufBlurX.get()};
        boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
        boo::ObjToken<boo::ITexture> texs[4];
        int texBindIdxs[4];

        texs[0] = CGraphics::g_SpareTexture.get();
        texBindIdxs[0] = 1;
        filter.m_dataBindBlurX = cctx.newShaderDataBinding(s_BlurPipeline,
            s_BlurVtxFmt, filter.m_blurVbo.get(), nullptr, nullptr,
            1, bufs, stages, nullptr, nullptr, 1, texs, texBindIdxs, nullptr);

        bufs[0] = filter.m_uniBufBlurY.get();
        texs[0] = CGraphics::g_SpareTexture.get();
        texBindIdxs[0] = 2;
        filter.m_dataBindBlurY = cctx.newShaderDataBinding(s_BlurPipeline,
            s_BlurVtxFmt, filter.m_blurVbo.get(), nullptr, nullptr,
            1, bufs, stages, nullptr, nullptr, 1, texs, texBindIdxs, nullptr);

        bufs[0] = filter.m_uniBuf.get();
        size_t texCount;
        if (filter.m_indTex)
        {
            texs[0] = CGraphics::g_SpareTexture.get();
            texBindIdxs[0] = 0;
            texs[1] = filter.m_indTex->GetBooTexture();
            texBindIdxs[1] = 0;
            texs[2] = CGraphics::g_SpareTexture.get();
            texBindIdxs[2] = 1;
            texs[3] = CGraphics::g_SpareTexture.get();
            texBindIdxs[3] = 2;
            texCount = 4;
        }
        else
        {
            texs[0] = CGraphics::g_SpareTexture.get();
            texBindIdxs[0] = 0;
            texs[1] = CGraphics::g_SpareTexture.get();
            texBindIdxs[1] = 1;
            texs[2] = CGraphics::g_SpareTexture.get();
            texBindIdxs[2] = 2;
            texCount = 3;
        }

        return cctx.newShaderDataBinding(filter.m_indTex ? s_IndPipeline : s_Pipeline,
            s_VtxFmt, filter.m_vbo.get(), nullptr, nullptr,
            1, bufs, stages, nullptr, nullptr, texCount, texs, texBindIdxs, nullptr);
    }
};

TShader<CPhazonSuitFilter>::IDataBindingFactory*
CPhazonSuitFilter::Initialize(boo::MetalDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4, 0},
        {nullptr, nullptr, boo::VertexSemantic::UV4, 1},
        {nullptr, nullptr, boo::VertexSemantic::UV4, 2}
    };
    s_VtxFmt = ctx.newVertexFormat(4, VtxVmt);
    const boo::VertexElementDescriptor BlurVtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    s_BlurVtxFmt = ctx.newVertexFormat(2, BlurVtxVmt);
    s_IndPipeline = ctx.newShaderPipeline(VS, IndFS, nullptr, nullptr, s_VtxFmt,
                                          CGraphics::g_ViewportSamples, boo::BlendFactor::SrcAlpha,
                                          boo::BlendFactor::One, boo::Primitive::TriStrips,
                                          boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_Pipeline = ctx.newShaderPipeline(VS, FS, nullptr, nullptr, s_VtxFmt,
                                       CGraphics::g_ViewportSamples, boo::BlendFactor::SrcAlpha,
                                       boo::BlendFactor::One, boo::Primitive::TriStrips,
                                       boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_BlurPipeline = ctx.newShaderPipeline(BlurVS, BlurFS, nullptr, nullptr, s_BlurVtxFmt,
                                           CGraphics::g_ViewportSamples, boo::BlendFactor::One,
                                           boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, false, true, boo::CullMode::None);
    return new CPhazonSuitFilterMetalDataBindingFactory;
}

template <>
void CPhazonSuitFilter::Shutdown<boo::MetalDataFactory>()
{
    s_VtxFmt.reset();
    s_BlurVtxFmt.reset();
    s_IndPipeline.reset();
    s_Pipeline.reset();
    s_BlurPipeline.reset();
}

}
