#include "CPhazonSuitFilter.hpp"
#include "TShader.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{

static const char* VS =
"struct VertData {\n"
"    float4 posIn : POSITION;\n"
"    float4 screenUvIn : UV0;\n"
"    float4 indUvIn : UV1;\n"
"    float4 maskUvIn : UV2;\n"
"};\n"
"\n"
"cbuffer PhazonSuitUniform : register(b0)\n"
"{\n"
"    float4 color;\n"
"    float4 indScaleOff;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float4 indScaleOff : SCALEOFF;\n"
"    float2 screenUv : UV0;\n"
"    float2 indUv : UV1;\n"
"    float2 maskUv : UV2;\n"
"};\n"
"\n"
"VertToFrag main(in VertData v)\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.color = color;\n"
"    vtf.indScaleOff = indScaleOff;\n"
"    vtf.screenUv = v.screenUvIn.xy;\n"
"    vtf.screenUv.y = 1.0 - vtf.screenUv.y;\n"
"    vtf.indUv = v.indUvIn.xy;\n"
"    vtf.maskUv = v.maskUvIn.xy;\n"
"    vtf.maskUv.y = 1.0 - vtf.maskUv.y;\n"
"    vtf.position = float4(v.posIn.xyz, 1.0);\n"
"    return vtf;\n"
"}\n";

static const char* IndFS =
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float4 indScaleOff : SCALEOFF;\n"
"    float2 screenUv : UV0;\n"
"    float2 indUv : UV1;\n"
"    float2 maskUv : UV2;\n"
"};\n"
"\n"
"SamplerState samp : register(s0);\n"
"Texture2D screenTex : register(t0);\n"
"Texture2D indTex : register(t1);\n"
"Texture2D maskTex : register(t2);\n"
"Texture2D maskTexBlur : register(t3);\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    float2 indUv = (indTex.Sample(samp, vtf.indUv).ra - float2(0.5, 0.5)) * \n"
"        vtf.indScaleOff.xy + vtf.indScaleOff.zw;\n"
"    float maskBlurAlpha = saturate((maskTexBlur.Sample(samp, vtf.maskUv).a - maskTex.Sample(samp, vtf.maskUv).a) * 2.0);\n"
"    return float4((vtf.color * screenTex.Sample(samp, indUv + vtf.screenUv) * maskBlurAlpha).rgb, vtf.color.a);\n"
"}\n";

static const char* FS =
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float4 color : COLOR;\n"
"    float4 indScaleOff : SCALEOFF;\n"
"    float2 screenUv : UV0;\n"
"    float2 indUv : UV1;\n"
"    float2 maskUv : UV2;\n"
"};\n"
"\n"
"SamplerState samp : register(s2);\n"
"Texture2D screenTex : register(t0);\n"
"Texture2D maskTex : register(t1);\n"
"Texture2D maskTexBlur : register(t2);\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    float maskBlurAlpha = saturate((maskTexBlur.Sample(samp, vtf.maskUv).a - maskTex.Sample(samp, vtf.maskUv).a) * 2.0);\n"
"    return float4((vtf.color * screenTex.Sample(samp, vtf.screenUv) * maskBlurAlpha).rgb, vtf.color.a);\n"
"}\n";

static const char* BlurVS =
"struct VertData {\n"
"    float4 posIn : POSITION;\n"
"    float4 uvIn : UV;\n"
"};\n"
"\n"
"cbuffer PhazonSuitBlurUniform : register(b0)\n"
"{\n"
"    float4 blurDir;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float2 uv : UV;\n"
"    float2 blurDir : BLURDIR;\n"
"};\n"
"\n"
"VertToFrag main(in VertData v)\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.uv = v.uvIn.xy;\n"
"    vtf.uv.y = 1.0 - vtf.uv.y;\n"
"    vtf.blurDir = blurDir.xy;\n"
"    vtf.position = float4(v.posIn.xyz, 1.0);\n"
"    return vtf;\n"
"}\n";

static const char* BlurFS =
"struct VertToFrag\n"
"{\n"
"    float4 position : SV_Position;\n"
"    float2 uv : UV;\n"
"    float2 blurDir : BLURDIR;\n"
"};\n"
"\n"
"SamplerState samp : register(s2);\n"
"Texture2D maskTex : register(t0);\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    //this will be our alpha sum\n"
"    float sum = 0.0;\n"
"\n"
"    //apply blurring, using a 23-tap filter with predefined gaussian weights\n"
"    sum += maskTex.Sample(samp, vtf.uv + -11.0 * vtf.blurDir).a * 0.007249;\n"
"    sum += maskTex.Sample(samp, vtf.uv + -10.0 * vtf.blurDir).a * 0.011032;\n"
"    sum += maskTex.Sample(samp, vtf.uv + -9.0 * vtf.blurDir).a * 0.016133;\n"
"    sum += maskTex.Sample(samp, vtf.uv + -8.0 * vtf.blurDir).a * 0.022665;\n"
"    sum += maskTex.Sample(samp, vtf.uv + -7.0 * vtf.blurDir).a * 0.030595;\n"
"    sum += maskTex.Sample(samp, vtf.uv + -6.0 * vtf.blurDir).a * 0.039680;\n"
"    sum += maskTex.Sample(samp, vtf.uv + -5.0 * vtf.blurDir).a * 0.049444;\n"
"    sum += maskTex.Sample(samp, vtf.uv + -4.0 * vtf.blurDir).a * 0.059195;\n"
"    sum += maskTex.Sample(samp, vtf.uv + -3.0 * vtf.blurDir).a * 0.068091;\n"
"    sum += maskTex.Sample(samp, vtf.uv + -2.0 * vtf.blurDir).a * 0.075252;\n"
"    sum += maskTex.Sample(samp, vtf.uv + -1.0 * vtf.blurDir).a * 0.079905;\n"
"    sum += maskTex.Sample(samp, vtf.uv + 0.0 * vtf.blurDir).a * 0.081519;\n"
"    sum += maskTex.Sample(samp, vtf.uv + 1.0 * vtf.blurDir).a * 0.079905;\n"
"    sum += maskTex.Sample(samp, vtf.uv + 2.0 * vtf.blurDir).a * 0.075252;\n"
"    sum += maskTex.Sample(samp, vtf.uv + 3.0 * vtf.blurDir).a * 0.068091;\n"
"    sum += maskTex.Sample(samp, vtf.uv + 4.0 * vtf.blurDir).a * 0.059195;\n"
"    sum += maskTex.Sample(samp, vtf.uv + 5.0 * vtf.blurDir).a * 0.049444;\n"
"    sum += maskTex.Sample(samp, vtf.uv + 6.0 * vtf.blurDir).a * 0.039680;\n"
"    sum += maskTex.Sample(samp, vtf.uv + 7.0 * vtf.blurDir).a * 0.030595;\n"
"    sum += maskTex.Sample(samp, vtf.uv + 8.0 * vtf.blurDir).a * 0.022665;\n"
"    sum += maskTex.Sample(samp, vtf.uv + 9.0 * vtf.blurDir).a * 0.016133;\n"
"    sum += maskTex.Sample(samp, vtf.uv + 10.0 * vtf.blurDir).a * 0.011032;\n"
"    sum += maskTex.Sample(samp, vtf.uv + 11.0 * vtf.blurDir).a * 0.007249;\n"
"\n"
"    return float4(1.0, 1.0, 1.0, sum);\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CPhazonSuitFilter)

static boo::ObjToken<boo::IVertexFormat> s_VtxFmt;
static boo::ObjToken<boo::IVertexFormat> s_BlurVtxFmt;
static boo::ObjToken<boo::IShaderPipeline> s_IndPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;
static boo::ObjToken<boo::IShaderPipeline> s_BlurPipeline;

struct CPhazonSuitFilterD3DDataBindingFactory : TShader<CPhazonSuitFilter>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding>
    BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                           CPhazonSuitFilter& filter)
    {
        boo::ID3DDataFactory::Context& cctx = static_cast<boo::ID3DDataFactory::Context&>(ctx);

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
CPhazonSuitFilter::Initialize(boo::ID3DDataFactory::Context& ctx)
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
    s_IndPipeline = ctx.newShaderPipeline(VS, IndFS, nullptr, nullptr, nullptr, s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                          boo::BlendFactor::One, boo::Primitive::TriStrips,
                                          boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_Pipeline = ctx.newShaderPipeline(VS, FS, nullptr, nullptr, nullptr, s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                       boo::BlendFactor::One, boo::Primitive::TriStrips,
                                       boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_BlurPipeline = ctx.newShaderPipeline(BlurVS, BlurFS, nullptr, nullptr, nullptr, s_BlurVtxFmt, boo::BlendFactor::One,
                                           boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, false, true, boo::CullMode::None);
    return new CPhazonSuitFilterD3DDataBindingFactory;
}

template <>
void CPhazonSuitFilter::Shutdown<boo::ID3DDataFactory>()
{
    s_VtxFmt.reset();
    s_BlurVtxFmt.reset();

    s_IndPipeline.reset();
    s_Pipeline.reset();
    s_BlurPipeline.reset();
}
}
