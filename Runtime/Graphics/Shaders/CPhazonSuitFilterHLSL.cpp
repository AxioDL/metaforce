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
"    vtf.indUv = v.indUvIn.xy;\n"
"    vtf.maskUv = v.maskUvIn.xy;\n"
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
"    float2 indUv = (indTex.Sample(samp, vtf.indUv).rg - float2(0.5, 0.5)) * \n"
"        vtf.indScaleOff.xy + vtf.indScaleOff.zw;\n"
"    return vtf.color * screenTex.Sample(samp, indUv + vtf.screenUv) * \n"
"        (maskTexBlur.Sample(samp, vtf.maskUv).a - maskTex.Sample(samp, vtf.maskUv).a);\n"
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
"SamplerState samp : register(s0);\n"
"Texture2D screenTex : register(t0);\n"
"Texture2D maskTex : register(t1);\n"
"Texture2D maskTexBlur : register(t2);\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    return vtf.color * screenTex.Sample(samp, vtf.screenUv) * \n"
"        (maskTexBlur.Sample(samp, vtf.maskUv).a - maskTex.Sample(samp, vtf.maskUv).a);\n"
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
"SBINDING(0) out VertToFrag vtf;\n"
"VertToFrag main(in VertData v)\n"
"{\n"
"    VertToFrag vtf;\n"
"    vtf.uv = uvIn.xy;\n"
"    vtf.blurDir = blurDir.xy;\n"
"    vtf.position = float4(posIn.xyz, 1.0);\n"
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
"SamplerState samp : register(s0);\n"
"Texture2D maskTex : register(t0);\n"
"float4 main(in VertToFrag vtf) : SV_Target0\n"
"{\n"
"    //this will be our alpha sum\n"
"    float sum = 0.0;\n"
"\n"
"    //apply blurring, using a 9-tap filter with predefined gaussian weights\n"
"\n"
"    sum += maskTex.Sample(samp, vtf.uv - 4.0 * vtf.blurDir).a * 0.0162162162;\n"
"    sum += maskTex.Sample(samp, vtf.uv - 3.0 * vtf.blurDir).a * 0.0540540541;\n"
"    sum += maskTex.Sample(samp, vtf.uv - 2.0 * vtf.blurDir).a * 0.1216216216;\n"
"    sum += maskTex.Sample(samp, vtf.uv - 1.0 * vtf.blurDir).a * 0.1945945946;\n"
"\n"
"    sum += maskTex.Sample(samp, vtf.uv).a * 0.2270270270;\n"
"\n"
"    sum += maskTex.Sample(samp, vtf.uv - 1.0 * vtf.blurDir).a * 0.1945945946;\n"
"    sum += maskTex.Sample(samp, vtf.uv - 2.0 * vtf.blurDir).a * 0.1216216216;\n"
"    sum += maskTex.Sample(samp, vtf.uv - 3.0 * vtf.blurDir).a * 0.0540540541;\n"
"    sum += maskTex.Sample(samp, vtf.uv - 4.0 * vtf.blurDir).a * 0.0162162162;\n"
"\n"
"    return float4(1.0, 1.0, 1.0, sum);\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CPhazonSuitFilter)

static boo::IVertexFormat* s_VtxFmt = nullptr;
static boo::IVertexFormat* s_BlurVtxFmt = nullptr;
static boo::IShaderPipeline* s_IndPipeline = nullptr;
static boo::IShaderPipeline* s_Pipeline = nullptr;
static boo::IShaderPipeline* s_BlurPipeline = nullptr;

struct CPhazonSuitFilterD3DDataBindingFactory : TShader<CPhazonSuitFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    CPhazonSuitFilter& filter)
    {
        boo::ID3DDataFactory::Context& cctx = static_cast<boo::ID3DDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBufBlurX};
        boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
        boo::ITexture* texs[4];
        int texBindIdxs[4];

        texs[0] = CGraphics::g_SpareTexture;
        texBindIdxs[0] = 1;
        filter.m_dataBindBlurX = cctx.newShaderDataBinding(s_BlurPipeline,
            s_BlurVtxFmt, filter.m_blurVbo, nullptr, nullptr,
            1, bufs, stages, nullptr, nullptr, 1, texs, texBindIdxs, nullptr);

        bufs[0] = filter.m_uniBufBlurY;
        texs[0] = CGraphics::g_SpareTexture;
        texBindIdxs[0] = 2;
        filter.m_dataBindBlurY = cctx.newShaderDataBinding(s_BlurPipeline,
            s_BlurVtxFmt, filter.m_blurVbo, nullptr, nullptr,
            1, bufs, stages, nullptr, nullptr, 1, texs, texBindIdxs, nullptr);

        bufs[0] = filter.m_uniBuf;
        size_t texCount;
        if (filter.m_indTex)
        {
            texs[0] = CGraphics::g_SpareTexture;
            texBindIdxs[0] = 0;
            texs[1] = filter.m_indTex->GetBooTexture();
            texBindIdxs[1] = 0;
            texs[2] = CGraphics::g_SpareTexture;
            texBindIdxs[2] = 1;
            texs[3] = CGraphics::g_SpareTexture;
            texBindIdxs[3] = 2;
            texCount = 4;
        }
        else
        {
            texs[0] = CGraphics::g_SpareTexture;
            texBindIdxs[0] = 0;
            texs[1] = CGraphics::g_SpareTexture;
            texBindIdxs[1] = 1;
            texs[2] = CGraphics::g_SpareTexture;
            texBindIdxs[2] = 2;
            texCount = 3;
        }

        return cctx.newShaderDataBinding(filter.m_indTex ? s_IndPipeline : s_Pipeline,
            s_VtxFmt, filter.m_vbo, nullptr, nullptr,
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
    s_IndPipeline = ctx.newShaderPipeline(VS, IndFS, nullptr, nullptr, nullptr, s_VtxFmt, boo::BlendFactor::One,
                                          boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                          boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_Pipeline = ctx.newShaderPipeline(VS, FS, nullptr, nullptr, nullptr, s_VtxFmt, boo::BlendFactor::One,
                                       boo::BlendFactor::One, boo::Primitive::TriStrips,
                                       boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_BlurPipeline = ctx.newShaderPipeline(BlurVS, BlurFS, nullptr, nullptr, nullptr, s_BlurVtxFmt, boo::BlendFactor::One,
                                           boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, false, true, boo::CullMode::None);
    return new CPhazonSuitFilterD3DDataBindingFactory;
}

}
