#include "CPhazonSuitFilter.hpp"
#include "TShader.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{

static const char* VS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec4 posIn;\n"
"layout(location=1) in vec4 screenUvIn;\n"
"layout(location=2) in vec4 indUvIn;\n"
"layout(location=3) in vec4 maskUvIn;\n"
"\n"
"UBINDING0 uniform PhazonSuitUniform\n"
"{\n"
"    vec4 color;\n"
"    vec4 indScaleOff;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec4 indScaleOff;\n"
"    vec2 screenUv;\n"
"    vec2 indUv;\n"
"    vec2 maskUv;\n"
"};\n"
"\n"
"SBINDING(0) out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.color = color;\n"
"    vtf.indScaleOff = indScaleOff;\n"
"    vtf.screenUv = screenUvIn.xy;\n"
"    vtf.indUv = indUvIn.xy;\n"
"    vtf.maskUv = maskUvIn.xy;\n"
"    gl_Position = vec4(posIn.xyz, 1.0);\n"
"}\n";

static const char* IndFS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec4 indScaleOff;\n"
"    vec2 screenUv;\n"
"    vec2 indUv;\n"
"    vec2 maskUv;\n"
"};\n"
"\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D screenTex;\n"
"TBINDING1 uniform sampler2D indTex;\n"
"TBINDING2 uniform sampler2D maskTex;\n"
"TBINDING3 uniform sampler2D maskTexBlur;\n"
"void main()\n"
"{\n"
"    vec2 indUv = (texture(indTex, vtf.indUv).rg - vec2(0.5, 0.5)) * \n"
"        vtf.indScaleOff.xy + vtf.indScaleOff.zw;\n"
"    colorOut = vtf.color * texture(screenTex, indUv + vtf.screenUv) * \n"
"        (texture(maskTexBlur, vtf.maskUv).a - texture(maskTex, vtf.maskUv).a);\n"
"}\n";

static const char* FS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec4 indScaleOff;\n"
"    vec2 screenUv;\n"
"    vec2 indUv;\n"
"    vec2 maskUv;\n"
"};\n"
"\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D screenTex;\n"
"TBINDING1 uniform sampler2D maskTex;\n"
"TBINDING2 uniform sampler2D maskTexBlur;\n"
"void main()\n"
"{\n"
"    colorOut = vtf.color * texture(screenTex, vtf.screenUv) * \n"
"        (texture(maskTexBlur, vtf.maskUv).a - texture(maskTex, vtf.maskUv).a);\n"
"}\n";

static const char* BlurVS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec4 posIn;\n"
"layout(location=1) in vec4 uvIn;\n"
"\n"
"UBINDING0 uniform PhazonSuitBlurUniform\n"
"{\n"
"    vec4 blurDir;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    vec2 uv;\n"
"    vec2 blurDir;\n"
"};\n"
"\n"
"SBINDING(0) out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.uv = uvIn.xy;\n"
"    vtf.blurDir = blurDir.xy;\n"
"    gl_Position = vec4(posIn.xyz, 1.0);\n"
"}\n";

static const char* BlurFS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"struct VertToFrag\n"
"{\n"
"    vec2 uv;\n"
"    vec2 blurDir;\n"
"};\n"
"\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D maskTex;\n"
"void main()\n"
"{\n"
"    //this will be our alpha sum\n"
"    float sum = 0.0;\n"
"\n"
"    //apply blurring, using a 9-tap filter with predefined gaussian weights\n"
"\n"
"    sum += texture(maskTex, vtf.uv - 4.0 * vtf.blurDir).a * 0.0162162162;\n"
"    sum += texture(maskTex, vtf.uv - 3.0 * vtf.blurDir).a * 0.0540540541;\n"
"    sum += texture(maskTex, vtf.uv - 2.0 * vtf.blurDir).a * 0.1216216216;\n"
"    sum += texture(maskTex, vtf.uv - 1.0 * vtf.blurDir).a * 0.1945945946;\n"
"\n"
"    sum += texture(maskTex, vtf.uv).a * 0.2270270270;\n"
"\n"
"    sum += texture(maskTex, vtf.uv - 1.0 * vtf.blurDir).a * 0.1945945946;\n"
"    sum += texture(maskTex, vtf.uv - 2.0 * vtf.blurDir).a * 0.1216216216;\n"
"    sum += texture(maskTex, vtf.uv - 3.0 * vtf.blurDir).a * 0.0540540541;\n"
"    sum += texture(maskTex, vtf.uv - 4.0 * vtf.blurDir).a * 0.0162162162;\n"
"\n"
"    colorOut = vec4(1.0, 1.0, 1.0, sum);\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CPhazonSuitFilter)

static boo::IVertexFormat* s_VtxFmt = nullptr;
static boo::IVertexFormat* s_BlurVtxFmt = nullptr;
static boo::IShaderPipeline* s_IndPipeline = nullptr;
static boo::IShaderPipeline* s_Pipeline = nullptr;
static boo::IShaderPipeline* s_BlurPipeline = nullptr;

struct CPhazonSuitFilterGLDataBindingFactory : TShader<CPhazonSuitFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    CPhazonSuitFilter& filter)
    {
        boo::GLDataFactory::Context& cctx = static_cast<boo::GLDataFactory::Context&>(ctx);

        const boo::VertexElementDescriptor BlurVtxVmt[] =
        {
            {filter.m_blurVbo, nullptr, boo::VertexSemantic::Position4},
            {filter.m_blurVbo, nullptr, boo::VertexSemantic::UV4}
        };
        boo::IVertexFormat* blurVtxFmt = ctx.newVertexFormat(2, BlurVtxVmt);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBufBlurX};
        boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
        boo::ITexture* texs[4];
        int texBindIdxs[4];

        texs[0] = CGraphics::g_SpareTexture;
        texBindIdxs[0] = 1;
        filter.m_dataBindBlurX = cctx.newShaderDataBinding(s_BlurPipeline,
            blurVtxFmt, filter.m_blurVbo, nullptr, nullptr,
            1, bufs, stages, nullptr, nullptr, 1, texs, texBindIdxs, nullptr);

        bufs[0] = filter.m_uniBufBlurY;
        texs[0] = CGraphics::g_SpareTexture;
        texBindIdxs[0] = 2;
        filter.m_dataBindBlurY = cctx.newShaderDataBinding(s_BlurPipeline,
            blurVtxFmt, filter.m_blurVbo, nullptr, nullptr,
            1, bufs, stages, nullptr, nullptr, 1, texs, texBindIdxs, nullptr);

        const boo::VertexElementDescriptor VtxVmt[] =
        {
            {filter.m_vbo, nullptr, boo::VertexSemantic::Position4},
            {filter.m_vbo, nullptr, boo::VertexSemantic::UV4, 0},
            {filter.m_vbo, nullptr, boo::VertexSemantic::UV4, 1},
            {filter.m_vbo, nullptr, boo::VertexSemantic::UV4, 2}
        };
        boo::IVertexFormat* vtxFmt = ctx.newVertexFormat(4, VtxVmt);

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
            vtxFmt, filter.m_vbo, nullptr, nullptr,
            1, bufs, stages, nullptr, nullptr, texCount, texs, texBindIdxs, nullptr);
    }
};

#if BOO_HAS_VULKAN
struct CPhazonSuitFilterVulkanDataBindingFactory : TShader<CPhazonSuitFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    CPhazonSuitFilter& filter)
    {
        boo::VulkanDataFactory::Context& cctx = static_cast<boo::VulkanDataFactory::Context&>(ctx);

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
#endif

TShader<CPhazonSuitFilter>::IDataBindingFactory*
CPhazonSuitFilter::Initialize(boo::GLDataFactory::Context& ctx)
{
    const char* uniNames[] = {"PhazonSuitUniform"};
    const char* texNames[] = {"screenTex", "indTex", "maskTex", "maskTexBlur"};
    s_IndPipeline = ctx.newShaderPipeline(VS, IndFS, 4, texNames, 1, uniNames, boo::BlendFactor::One,
                                          boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                          boo::ZTest::None, false, true, false, boo::CullMode::None);
    texNames[1] = "maskTex";
    texNames[2] = "maskTexBlur";
    s_Pipeline = ctx.newShaderPipeline(VS, FS, 3, texNames, 1, uniNames, boo::BlendFactor::One,
                                       boo::BlendFactor::One, boo::Primitive::TriStrips,
                                       boo::ZTest::None, false, true, false, boo::CullMode::None);
    uniNames[0] = "PhazonSuitBlurUniform";
    texNames[0] = "maskTex";
    s_BlurPipeline = ctx.newShaderPipeline(BlurVS, BlurFS, 1, texNames, 1, uniNames, boo::BlendFactor::One,
                                           boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, false, true, boo::CullMode::None);
    return new CPhazonSuitFilterGLDataBindingFactory;
}

#if BOO_HAS_VULKAN
TShader<CPhazonSuitFilter>::IDataBindingFactory*
CPhazonSuitFilter::Initialize(boo::VulkanDataFactory::Context& ctx)
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
    s_IndPipeline = ctx.newShaderPipeline(VS, IndFS, s_VtxFmt, boo::BlendFactor::One,
                                          boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips,
                                          boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_Pipeline = ctx.newShaderPipeline(VS, FS, s_VtxFmt, boo::BlendFactor::One,
                                       boo::BlendFactor::One, boo::Primitive::TriStrips,
                                       boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_BlurPipeline = ctx.newShaderPipeline(BlurVS, BlurFS, s_BlurVtxFmt, boo::BlendFactor::One,
                                           boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, false, true, boo::CullMode::None);
    return new CPhazonSuitFilterVulkanDataBindingFactory;
}
#endif

}
