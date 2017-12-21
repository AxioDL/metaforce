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
"    vec2 indUv = (texture(indTex, vtf.indUv).ra - vec2(0.5, 0.5)) * \n"
"        vtf.indScaleOff.xy + vtf.indScaleOff.zw;\n"
"    float maskBlurAlpha = clamp(0.0, (texture(maskTexBlur, vtf.maskUv).a - texture(maskTex, vtf.maskUv).a) * 2.0, 1.0);\n"
"    colorOut = vtf.color * texture(screenTex, indUv + vtf.screenUv) * maskBlurAlpha;\n"
"    colorOut.a = vtf.color.a;\n"
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
"    float maskBlurAlpha = clamp(0.0, (texture(maskTexBlur, vtf.maskUv).a - texture(maskTex, vtf.maskUv).a) * 2.0, 1.0);\n"
"    colorOut = vtf.color * texture(screenTex, vtf.screenUv) * maskBlurAlpha;\n"
"    colorOut.a = vtf.color.a;\n"
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
"    //apply blurring, using a 23-tap filter with predefined gaussian weights\n"
"    sum += texture(maskTex, vtf.uv + -11.0 * vtf.blurDir).a * 0.007249;\n"
"    sum += texture(maskTex, vtf.uv + -10.0 * vtf.blurDir).a * 0.011032;\n"
"    sum += texture(maskTex, vtf.uv + -9.0 * vtf.blurDir).a * 0.016133;\n"
"    sum += texture(maskTex, vtf.uv + -8.0 * vtf.blurDir).a * 0.022665;\n"
"    sum += texture(maskTex, vtf.uv + -7.0 * vtf.blurDir).a * 0.030595;\n"
"    sum += texture(maskTex, vtf.uv + -6.0 * vtf.blurDir).a * 0.039680;\n"
"    sum += texture(maskTex, vtf.uv + -5.0 * vtf.blurDir).a * 0.049444;\n"
"    sum += texture(maskTex, vtf.uv + -4.0 * vtf.blurDir).a * 0.059195;\n"
"    sum += texture(maskTex, vtf.uv + -3.0 * vtf.blurDir).a * 0.068091;\n"
"    sum += texture(maskTex, vtf.uv + -2.0 * vtf.blurDir).a * 0.075252;\n"
"    sum += texture(maskTex, vtf.uv + -1.0 * vtf.blurDir).a * 0.079905;\n"
"    sum += texture(maskTex, vtf.uv + 0.0 * vtf.blurDir).a * 0.081519;\n"
"    sum += texture(maskTex, vtf.uv + 1.0 * vtf.blurDir).a * 0.079905;\n"
"    sum += texture(maskTex, vtf.uv + 2.0 * vtf.blurDir).a * 0.075252;\n"
"    sum += texture(maskTex, vtf.uv + 3.0 * vtf.blurDir).a * 0.068091;\n"
"    sum += texture(maskTex, vtf.uv + 4.0 * vtf.blurDir).a * 0.059195;\n"
"    sum += texture(maskTex, vtf.uv + 5.0 * vtf.blurDir).a * 0.049444;\n"
"    sum += texture(maskTex, vtf.uv + 6.0 * vtf.blurDir).a * 0.039680;\n"
"    sum += texture(maskTex, vtf.uv + 7.0 * vtf.blurDir).a * 0.030595;\n"
"    sum += texture(maskTex, vtf.uv + 8.0 * vtf.blurDir).a * 0.022665;\n"
"    sum += texture(maskTex, vtf.uv + 9.0 * vtf.blurDir).a * 0.016133;\n"
"    sum += texture(maskTex, vtf.uv + 10.0 * vtf.blurDir).a * 0.011032;\n"
"    sum += texture(maskTex, vtf.uv + 11.0 * vtf.blurDir).a * 0.007249;\n"
"\n"
"    colorOut = vec4(1.0, 1.0, 1.0, sum);\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CPhazonSuitFilter)

static boo::ObjToken<boo::IVertexFormat> s_VtxFmt;
static boo::ObjToken<boo::IVertexFormat> s_BlurVtxFmt;
static boo::ObjToken<boo::IShaderPipeline> s_IndPipeline;
static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;
static boo::ObjToken<boo::IShaderPipeline> s_BlurPipeline;

struct CPhazonSuitFilterGLDataBindingFactory : TShader<CPhazonSuitFilter>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                  CPhazonSuitFilter& filter)
    {
        boo::GLDataFactory::Context& cctx = static_cast<boo::GLDataFactory::Context&>(ctx);

        const boo::VertexElementDescriptor BlurVtxVmt[] =
        {
            {filter.m_blurVbo.get(), nullptr, boo::VertexSemantic::Position4},
            {filter.m_blurVbo.get(), nullptr, boo::VertexSemantic::UV4}
        };
        boo::ObjToken<boo::IVertexFormat> blurVtxFmt = ctx.newVertexFormat(2, BlurVtxVmt);

        boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {filter.m_uniBufBlurX.get()};
        boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
        boo::ObjToken<boo::ITexture> texs[4];
        int texBindIdxs[4];

        texs[0] = CGraphics::g_SpareTexture.get();
        texBindIdxs[0] = 1;
        filter.m_dataBindBlurX = cctx.newShaderDataBinding(s_BlurPipeline,
            blurVtxFmt, filter.m_blurVbo.get(), nullptr, nullptr,
            1, bufs, stages, nullptr, nullptr, 1, texs, texBindIdxs, nullptr);

        bufs[0] = filter.m_uniBufBlurY.get();
        texs[0] = CGraphics::g_SpareTexture.get();
        texBindIdxs[0] = 2;
        filter.m_dataBindBlurY = cctx.newShaderDataBinding(s_BlurPipeline,
            blurVtxFmt, filter.m_blurVbo.get(), nullptr, nullptr,
            1, bufs, stages, nullptr, nullptr, 1, texs, texBindIdxs, nullptr);

        const boo::VertexElementDescriptor VtxVmt[] =
        {
            {filter.m_vbo.get(), nullptr, boo::VertexSemantic::Position4},
            {filter.m_vbo.get(), nullptr, boo::VertexSemantic::UV4, 0},
            {filter.m_vbo.get(), nullptr, boo::VertexSemantic::UV4, 1},
            {filter.m_vbo.get(), nullptr, boo::VertexSemantic::UV4, 2}
        };
        boo::ObjToken<boo::IVertexFormat> vtxFmt = ctx.newVertexFormat(4, VtxVmt);

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
            vtxFmt, filter.m_vbo.get(), nullptr, nullptr,
            1, bufs, stages, nullptr, nullptr, texCount, texs, texBindIdxs, nullptr);
    }
};

#if BOO_HAS_VULKAN
struct CPhazonSuitFilterVulkanDataBindingFactory : TShader<CPhazonSuitFilter>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding>
    BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                           CPhazonSuitFilter& filter)
    {
        boo::VulkanDataFactory::Context& cctx = static_cast<boo::VulkanDataFactory::Context&>(ctx);

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
#endif

TShader<CPhazonSuitFilter>::IDataBindingFactory*
CPhazonSuitFilter::Initialize(boo::GLDataFactory::Context& ctx)
{
    const char* uniNames[] = {"PhazonSuitUniform"};
    const char* texNames[] = {"screenTex", "indTex", "maskTex", "maskTexBlur"};
    s_IndPipeline = ctx.newShaderPipeline(VS, IndFS, 4, texNames, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                          boo::BlendFactor::One, boo::Primitive::TriStrips,
                                          boo::ZTest::None, false, true, false, boo::CullMode::None);
    texNames[1] = "maskTex";
    texNames[2] = "maskTexBlur";
    s_Pipeline = ctx.newShaderPipeline(VS, FS, 3, texNames, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                       boo::BlendFactor::One, boo::Primitive::TriStrips,
                                       boo::ZTest::None, false, true, false, boo::CullMode::None);
    uniNames[0] = "PhazonSuitBlurUniform";
    texNames[0] = "maskTex";
    s_BlurPipeline = ctx.newShaderPipeline(BlurVS, BlurFS, 1, texNames, 1, uniNames, boo::BlendFactor::One,
                                           boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, false, true, boo::CullMode::None);
    return new CPhazonSuitFilterGLDataBindingFactory;
}

template <>
void CPhazonSuitFilter::Shutdown<boo::GLDataFactory>()
{
    s_IndPipeline.reset();
    s_Pipeline.reset();
    s_BlurPipeline.reset();
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
    s_IndPipeline = ctx.newShaderPipeline(VS, IndFS, s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                          boo::BlendFactor::One, boo::Primitive::TriStrips,
                                          boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_Pipeline = ctx.newShaderPipeline(VS, FS, s_VtxFmt, boo::BlendFactor::SrcAlpha,
                                       boo::BlendFactor::One, boo::Primitive::TriStrips,
                                       boo::ZTest::None, false, true, false, boo::CullMode::None);
    s_BlurPipeline = ctx.newShaderPipeline(BlurVS, BlurFS, s_BlurVtxFmt, boo::BlendFactor::One,
                                           boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, false, true, boo::CullMode::None);
    return new CPhazonSuitFilterVulkanDataBindingFactory;
}

template <>
void CPhazonSuitFilter::Shutdown<boo::VulkanDataFactory>()
{
    s_VtxFmt.reset();
    s_BlurVtxFmt.reset();

    s_IndPipeline.reset();
    s_Pipeline.reset();
    s_BlurPipeline.reset();
}
#endif

}
