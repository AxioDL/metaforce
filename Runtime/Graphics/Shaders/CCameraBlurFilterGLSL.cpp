#include "CCameraBlurFilter.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "GameGlobalObjects.hpp"

namespace urde
{

static const char* VS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec4 posIn;\n"
"layout(location=1) in vec4 uvIn;\n"
"\n"
"UBINDING0 uniform CameraBlurUniform\n"
"{\n"
"    vec4 uv0;\n"
"    vec4 uv1;\n"
"    vec4 uv2;\n"
"    vec4 uv3;\n"
"    vec4 uv4;\n"
"    vec4 uv5;\n"
"    float opacity;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    vec2 uvReg;\n"
"    vec2 uv0;\n"
"    vec2 uv1;\n"
"    vec2 uv2;\n"
"    vec2 uv3;\n"
"    vec2 uv4;\n"
"    vec2 uv5;\n"
"    float opacity;\n"
"};\n"
"\n"
"SBINDING(0) out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.uvReg = uvIn.xy;\n"
"    vtf.uv0 = uv0.xy + uvIn.xy;\n"
"    vtf.uv1 = uv1.xy + uvIn.xy;\n"
"    vtf.uv2 = uv2.xy + uvIn.xy;\n"
"    vtf.uv3 = uv3.xy + uvIn.xy;\n"
"    vtf.uv4 = uv4.xy + uvIn.xy;\n"
"    vtf.uv5 = uv5.xy + uvIn.xy;\n"
"    vtf.opacity = opacity;\n"
"    gl_Position = vec4(posIn.xyz, 1.0);\n"
"}\n";

static const char* FS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"struct VertToFrag\n"
"{\n"
"    vec2 uvReg;\n"
"    vec2 uv0;\n"
"    vec2 uv1;\n"
"    vec2 uv2;\n"
"    vec2 uv3;\n"
"    vec2 uv4;\n"
"    vec2 uv5;\n"
"    float opacity;\n"
"};\n"
"\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D sceneTex;\n"
"void main()\n"
"{\n"
"    vec4 colorSample = texture(sceneTex, vtf.uvReg) * 0.14285715;\n"
"    colorSample += texture(sceneTex, vtf.uv0) * 0.14285715;\n"
"    colorSample += texture(sceneTex, vtf.uv1) * 0.14285715;\n"
"    colorSample += texture(sceneTex, vtf.uv2) * 0.14285715;\n"
"    colorSample += texture(sceneTex, vtf.uv3) * 0.14285715;\n"
"    colorSample += texture(sceneTex, vtf.uv4) * 0.14285715;\n"
"    colorSample += texture(sceneTex, vtf.uv5) * 0.14285715;\n"
"    colorOut = vec4(colorSample.rgb, vtf.opacity);\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CCameraBlurFilter)

struct CCameraBlurFilterGLDataBindingFactory : TShader<CCameraBlurFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    boo::IShaderPipeline* pipeline,
                                                    boo::IVertexFormat*,
                                                    CCameraBlurFilter& filter)
    {
        boo::GLDataFactory::Context& cctx = static_cast<boo::GLDataFactory::Context&>(ctx);

        const boo::VertexElementDescriptor VtxVmt[] =
        {
            {filter.m_vbo, nullptr, boo::VertexSemantic::Position4},
            {filter.m_vbo, nullptr, boo::VertexSemantic::UV4}
        };
        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
        boo::ITexture* texs[] = {CGraphics::g_SpareTexture};
        return cctx.newShaderDataBinding(pipeline,
                                         ctx.newVertexFormat(2, VtxVmt), filter.m_vbo, nullptr, nullptr,
                                         1, bufs, stages, nullptr, nullptr, 1, texs);
    }
};

#if BOO_HAS_VULKAN
struct CCameraBlurFilterVulkanDataBindingFactory : TShader<CCameraBlurFilter>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    boo::IShaderPipeline* pipeline,
                                                    boo::IVertexFormat* vtxFmt,
                                                    CCameraBlurFilter& filter)
    {
        boo::VulkanDataFactory::Context& cctx = static_cast<boo::VulkanDataFactory::Context&>(ctx);

        boo::IGraphicsBuffer* bufs[] = {filter.m_uniBuf};
        boo::ITexture* texs[] = {CGraphics::g_SpareTexture};
        return cctx.newShaderDataBinding(pipeline, vtxFmt,
                                         filter.m_vbo, nullptr, nullptr, 1, bufs,
                                         nullptr, nullptr, nullptr, 1, texs);
    }
};
#endif

TShader<CCameraBlurFilter>::IDataBindingFactory* CCameraBlurFilter::Initialize(boo::GLDataFactory::Context& ctx,
                                                                               boo::IShaderPipeline*& pipeOut)
{
    const char* texNames[] = {"sceneTex"};
    const char* uniNames[] = {"CameraBlurUniform"};
    pipeOut = ctx.newShaderPipeline(VS, FS, 1, texNames, 1, uniNames, boo::BlendFactor::SrcAlpha,
                                    boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips, false, false, false);
    return new CCameraBlurFilterGLDataBindingFactory;
}

#if BOO_HAS_VULKAN
TShader<CCameraBlurFilter>::IDataBindingFactory* CCameraBlurFilter::Initialize(boo::VulkanDataFactory::Context& ctx,
                                                                               boo::IShaderPipeline*& pipeOut,
                                                                               boo::IVertexFormat*& vtxFmtOut)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    vtxFmtOut = ctx.newVertexFormat(2, VtxVmt);
    pipeOut = ctx.newShaderPipeline(VS, FS, vtxFmtOut, boo::BlendFactor::SrcAlpha,
                                    boo::BlendFactor::InvSrcAlpha, boo::Primitive::TriStrips, false, false, false);
    return new CCameraBlurFilterVulkanDataBindingFactory;
}
#endif

}
